#include "../CourierPrototype.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourierJourneyRegression,"Courier.Regression.ThreeOrderJourney",
    EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)

bool FCourierJourneyRegression::RunTest(const FString&)
{
    UWorld* World=nullptr;
    for(const auto& Context:GEngine->GetWorldContexts())
        if(Context.World() && Context.World()->GetAuthGameMode<ACourierGameMode>())World=Context.World();
    if(!TestNotNull(TEXT("Courier world exists"),World))return false;
    auto* Mode=World->GetAuthGameMode<ACourierGameMode>();
    auto* Player=Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(World,0));
    auto* PC=UGameplayStatics::GetPlayerController(World,0);
    if(!TestNotNull(TEXT("Courier exists"),Player) || !TestNotNull(TEXT("Controller exists"),PC))return false;
    Mode->Phase=ECourierPhase::Complete;Mode->RestartRun();Mode->SetLanguage(TEXT("en"));
    auto Place=[Mode,Player](FVector P,bool Ground=true)
    {
        Mode->RecoverySeconds=0;Player->StopJumping();
        Player->GetCharacterMovement()->StopMovementImmediately();
        Player->SetBase(static_cast<FMovementBaseInterfaceData*>(nullptr));
        Player->SetActorLocation(P,false,nullptr,ETeleportType::TeleportPhysics);
        Player->GetCharacterMovement()->SetMovementMode(Ground?MOVE_Walking:MOVE_Falling);
    };
    const auto Visible=[World](FName Tag)
    {
        int32 Count=0;
        for(TActorIterator<AActor> It(World);It;++It)if(It->ActorHasTag(Tag) && !It->IsHidden())++Count;
        return Count;
    };
    FCollisionQueryParams WalkParams;WalkParams.AddIgnoredActor(Player);
    const auto WalkClear=[&](FVector A,FVector B,const TCHAR* Name)
    {
        FHitResult Hit;
        const bool Blocked=World->SweepSingleByChannel(Hit,A,B,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(34,88),WalkParams);
        const bool Wall=Blocked && (Hit.bStartPenetrating || Hit.ImpactNormal.Z<Player->GetCharacterMovement()->GetWalkableFloorZ());
        if(Wall)AddInfo(FString::Printf(TEXT("Blocked sweep %s: actor=%s at %s scale %s profile %s, point=%s, normal=%s, initial=%d"),Name,*GetNameSafe(Hit.GetActor()),Hit.GetActor()?*Hit.GetActor()->GetActorLocation().ToString():TEXT("-"),Hit.GetComponent()?*Hit.GetComponent()->GetComponentScale().ToString():TEXT("-"),Hit.GetComponent()?*Hit.GetComponent()->GetCollisionProfileName().ToString():TEXT("-"),*Hit.ImpactPoint.ToString(),*Hit.ImpactNormal.ToString(),Hit.bStartPenetrating));
        // A sweep may touch the walkable ramp while leaving it. Only a wall or
        // initial penetration obstructs this walking route.
        TestFalse(Name,Wall);
    };
    WalkClear(FVector(14100,1900,410),FVector(14800,1900,410),TEXT("Garden ramp corner has no rail gate"));
    WalkClear(FVector(14800,1900,410),FVector(14800,0,410),TEXT("Garden ramp joins raised walk without a rail gate"));
    WalkClear(FVector(24900,2100,490),FVector(25700,2100,490),TEXT("Projection ramp corner has no rail gate"));
    WalkClear(FVector(25700,2100,490),FVector(25700,0,490),TEXT("Projection ramp exit is open"));
    WalkClear(FVector(42200,2200,510),FVector(43000,2200,510),TEXT("Final terrace ramp corner is open"));
    WalkClear(FVector(43000,2200,510),FVector(43000,0,510),TEXT("Final terrace ramp joins upper walk"));
    WalkClear(FVector(7600,0,90),FVector(8000,0,90),TEXT("Coffee counter approach is clear"));
    WalkClear(FVector(45300,-950,510),FVector(46000,-950,510),TEXT("Cinema side console approach is clear"));
    // Cart depots stand at lane ends; none may sit on a side path.
    WalkClear(FVector(30120,250,90),FVector(30190,1300,90),TEXT("Market arcade entry is clear of cart depots"));
    WalkClear(FVector(36920,250,90),FVector(36990,1300,90),TEXT("Second market arcade entry is clear of cart depots"));
    WalkClear(FVector(14800,1700,410),FVector(14800,200,410),TEXT("Garden ramp landing is clear of cart depots"));
    for(TActorIterator<AActor> It(World);It;++It)if(It->ActorHasTag(TEXT("CourierNextMarker")))
    {
        TArray<AActor*> Parts;It->GetAttachedActors(Parts);
        for(AActor* Part:Parts)if(auto* M=Part->FindComponentByClass<UStaticMeshComponent>())
        {
            AddInfo(FString::Printf(TEXT("Marker part %s enabled=%d profile=%s pawn=%d actorCollision=%d"),*Part->GetName(),int32(M->GetCollisionEnabled()),
                *M->GetCollisionProfileName().ToString(),int32(M->GetCollisionResponseToChannel(ECC_Pawn)),Part->GetActorEnableCollision()));
            TestTrue(TEXT("Waypoint marker parts never collide"),!Part->GetActorEnableCollision() || M->GetCollisionEnabled()==ECollisionEnabled::NoCollision);
        }
    }
    {
        // Nov 8 playtest: the chase camera slid inside the barista at the coffee stand.
        FHitResult Hit;
        const bool bPerson=World->LineTraceSingleByChannel(Hit,FVector(8280,-500,140),FVector(8280,150,140),ECC_Camera,WalkParams);
        TestTrue(TEXT("Camera stops in front of a person instead of entering them"),bPerson && Hit.ImpactPoint.Y<-140.f);
        const bool bSky=World->LineTraceSingleByChannel(Hit,FVector(4000,0,500),FVector(4000,0,6000),ECC_Camera,WalkParams);
        TestFalse(TEXT("The sky dome never blocks the camera"),bSky);
    }
    // Nov 7 autoplay: a side-path rail fenced the shuttle's boarding edge.
    WalkClear(FVector(33950,-60,95),FVector(34450,-60,95),TEXT("Shuttle boarding edge has no side-path rail"));
    WalkClear(FVector(33950,60,95),FVector(34450,60,95),TEXT("Shuttle boarding edge is open on both halves"));
    WalkClear(FVector(18450,-60,95),FVector(18650,-60,95),TEXT("Stepping-stone takeoff has no side-path rail"));
    {
        // Nov 7 autoplay: the raised final lift ended 2.25 m short of the terrace.
        FHitResult Floor;
        const bool bLanding=World->LineTraceSingleByChannel(Floor,FVector(40740,0,620),FVector(40740,0,250),ECC_Visibility,WalkParams);
        TestTrue(TEXT("Final lift landing has a walkable lip"),bLanding && Floor.GetActor() &&
            Floor.GetActor()->ActorHasTag(TEXT("CourierWalkable")) && FMath::Abs(Floor.ImpactPoint.Z-420.f)<5.f);
    }
    PC->SetControlRotation(FRotator(-22,179,0));Player->RecenterView(-179.f);Player->Tick(.2f);
    TestTrue(TEXT("Recenter takes the short arc across 180 degrees"),FMath::Abs(FMath::FindDeltaAngleDegrees(179.f,PC->GetControlRotation().Yaw))<2.1f);
    Player->Tick(.5f);
    TestTrue(TEXT("Recenter reaches its requested direction"),FMath::Abs(FMath::FindDeltaAngleDegrees(-179.f,PC->GetControlRotation().Yaw))<.01f);
    PC->SetControlRotation(FRotator(-22,0,0));
    TestEqual(TEXT("Three orders loaded from runtime data"),Mode->Orders.Num(),3);
    TestEqual(TEXT("Restart clears completed stamps"),Mode->CompletedOrders,0);
    TestEqual(TEXT("Projection initially off"),Visible(TEXT("CourierProjection")),0);
    TestEqual(TEXT("Final scene initially off"),Visible(TEXT("CourierFinale")),0);
    while(Mode->Phase==ECourierPhase::Opening)Mode->Continue();
    int32 ActiveGuides=0;
    for(TActorIterator<AActor> It(World);It;++It)if(It->ActorHasTag(TEXT("CourierRouteGuide")))
    {
        if(!It->IsHidden()){++ActiveGuides;TestTrue(TEXT("Only the practice route is highlighted initially"),It->ActorHasTag(TEXT("CourierRoutePractice")));}
        TArray<UInstancedStaticMeshComponent*> Markings;It->GetComponents(Markings);
        for(auto* M:Markings)
        {
            TestTrue(TEXT("Route markings have no collision"),M->GetCollisionEnabled()==ECollisionEnabled::NoCollision);
            TestTrue(TEXT("Every route includes large painted guidance"),M->GetInstanceCount()>0);
        }
    }
    TestEqual(TEXT("Exactly one current route group is shown"),ActiveGuides,1);
    Mode->Tutorial=4;Place(Mode->Pickup-FVector(0,0,12));Mode->Tick(.01f);
    TestTrue(TEXT("Finishing practice at the desk opens the first brief"),Mode->Phase==ECourierPhase::Briefing);
    for(int32 Order=0;Order<3;++Order)
    {
        TestEqual(TEXT("Correct active order"),Mode->CurrentOrder,Order);
        TestTrue(TEXT("Budget matches runtime order data"),Mode->Remaining==Mode->OrderBudget());
        // After a delivery the next brief opens by itself; close it to test reopening.
        if(Mode->Phase==ECourierPhase::Briefing)Mode->Back();
        Place(Mode->Pickup-FVector(0,0,12));Mode->Interact();
        TestTrue(TEXT("E opens a brief, not a running clock"),Mode->Phase==ECourierPhase::Briefing);
        Mode->StartSlow();const float BeforeBrief=Mode->Remaining;Mode->Tick(20.f);
        TestFalse(TEXT("Borrower inactive while reading"),Mode->bSlowing);
        TestEqual(TEXT("Reading does not use delivery time"),Mode->Remaining,BeforeBrief);
        Mode->Back();TestTrue(TEXT("Can cancel brief"),Mode->Phase==ECourierPhase::AwaitPickup);
        Mode->Interact();Mode->TogglePause();
        TestTrue(TEXT("Esc closes an open brief instead of doing nothing"),Mode->Phase==ECourierPhase::AwaitPickup);
        Mode->Interact();Mode->Interact();
        TestTrue(TEXT("Pressing E again at the brief takes the parcel"),Mode->Phase==ECourierPhase::Delivery);
        {
            const FString Before=Mode->Language;Mode->ToggleLanguage();
            TestEqual(TEXT("L cannot change language mid-delivery"),Mode->Language,Before);
            const float Clock=Mode->Remaining;Mode->RequestRetry();
            TestTrue(TEXT("R mid-delivery asks before restarting"),Mode->Phase==ECourierPhase::Paused && Mode->bConfirmRetry && Mode->Remaining==Clock);
            Mode->TogglePause();
            TestTrue(TEXT("Declining keeps the delivery running"),Mode->Phase==ECourierPhase::Delivery && !Mode->bConfirmRetry);
            Mode->Cooldown=2.f;Mode->StartSlow();
            TestTrue(TEXT("Q without charge gives visible feedback"),!Mode->bSlowing && Mode->QDenied>0.f);
            Mode->Cooldown=0.f;Mode->SlowRemaining=3.f;Mode->QDenied=0.f;
        }
        Mode->Retry();Place(Mode->Pickup-FVector(0,0,12));
        Mode->Interact();Mode->Continue();
        TestTrue(TEXT("Accept starts the current delivery"),Mode->Phase==ECourierPhase::Delivery);
        ActiveGuides=0;
        for(TActorIterator<AActor> It(World);It;++It)if(It->ActorHasTag(TEXT("CourierRouteGuide")) && !It->IsHidden())
        {++ActiveGuides;TestTrue(TEXT("Highlighted arrows belong to this delivery"),It->ActorHasTag(FName(*FString::Printf(TEXT("CourierRoute%d"),Order))));}
        TestEqual(TEXT("Later delivery arrows do not compete with the current route"),ActiveGuides,1);

        // The guide/checkpoint targets must have real collision support at the intended height.
        FCollisionQueryParams Params;Params.AddIgnoredActor(Player);
        for(int32 N=0;N<16;++N)
        {
            Mode->RouteNode=N;const FVector Target=Mode->NavigationTarget();FHitResult Hit;
            const bool Found=World->LineTraceSingleByChannel(Hit,Target+FVector(0,0,650),Target-FVector(0,0,180),ECC_Visibility,Params);
            TestTrue(*FString::Printf(TEXT("Order %d guide %d has supported ground"),Order+1,N),Found);
            if(Found)TestTrue(TEXT("Guide target height matches floor"),FMath::Abs(Hit.ImpactPoint.Z-(Target.Z-100.f))<30.f);
            if(Target.Equals(Mode->Destination))break;
        }
        if(Order==1)
        {
            Place(FVector(14800,1900,410));Mode->Tick(.01f);
            TestTrue(TEXT("Side ramp saves its safe upper landing"),Mode->Checkpoint.Equals(FVector(14800,1900,440)));
            Mode->RouteNode=99;Place(FVector(14800,1900,-200),false);Mode->Tick(.01f);
            TestTrue(TEXT("Recovery rewinds route progress with the courier"),Mode->RouteNode<99);
            TestTrue(TEXT("Recovery guidance follows ramp exit before crossing the water"),Mode->NavigationTarget().Equals(FVector(14800,0,420)));
            Mode->RecoverySeconds=0.f;Mode->Checkpoint=Mode->Pickup+FVector(0,0,20);Mode->LensCondition=3;
            {
                // Nov 8 playtest: the paint led onto the side ramp while the marker led to the lift.
                int32 OnSideRamp=0;
                for(TActorIterator<AActor> It(World);It;++It)if(It->ActorHasTag(TEXT("CourierRoute1")))
                {
                    TArray<UInstancedStaticMeshComponent*> Paint;It->GetComponents(Paint);
                    for(auto* Mesh:Paint)for(int32 K=0;K<Mesh->GetInstanceCount();++K)
                    {FTransform T;Mesh->GetInstanceTransform(K,T,true);if(FVector::Dist2D(T.GetLocation(),FVector(13200,1900,0))<500.f)++OnSideRamp;}
                }
                TestEqual(TEXT("The painted line does not lead onto the side ramp"),OnSideRamp,0);
                Mode->RouteNode=6;Place(FVector(18050,-1350,90));
                TestTrue(TEXT("On the main ramp the marker stays on the painted line"),Mode->NavigationTarget().Y>-1500.f);
                for(const auto& Leg:Mode->GuideLegs(1))for(const FVector& Point:Leg)
                {
                    FHitResult Hit;
                    const bool Ok=World->LineTraceSingleByChannel(Hit,Point+FVector(0,0,150),Point-FVector(0,0,120),ECC_Visibility,Params);
                    TestTrue(*FString::Printf(TEXT("Guide point %s lies on a walkable surface"),*Point.ToCompactString()),Ok && FMath::Abs(Hit.ImpactPoint.Z-Point.Z)<35.f);
                }
            }
            for(int32 I=0;I<=19;++I)
            {
                const FVector Floor(12200+I*100,1900,I*320.f/19.f);FHitResult Hit;
                const bool Supported=World->LineTraceSingleByChannel(Hit,Floor+FVector(0,0,180),Floor-FVector(0,0,80),ECC_Visibility,Params);
                TestTrue(TEXT("Garden walk ramp is continuously supported"),Supported);
                if(Supported)TestTrue(TEXT("Garden ramp has no hidden landing lip"),FMath::Abs(Hit.ImpactPoint.Z-Floor.Z)<25.f);
            }
            Place(FVector(13150,1900,248));
            TestTrue(TEXT("Navigation follows selected ramp instead of crossing water"),FMath::IsNearlyEqual(Mode->NavigationTarget().Y,1900.f));
            FHitResult GapHit;
            TestFalse(TEXT("Stepping-stone gap is a real gap"),World->LineTraceSingleByChannel(GapHit,FVector(19350,0,150),FVector(19350,0,-150),ECC_Visibility,Params));
            ACourierLift* Lift=nullptr;
            for(TActorIterator<ACourierLift> It(World);It;++It)if(It->OrderIndex==1){Lift=*It;break;}
            if(TestNotNull(TEXT("Garden moving lift exists"),Lift))
            {
                Place(FVector(12150,0,88));Mode->Tick(4.f);Lift->Reset();
                TestTrue(TEXT("Low platform prompt explicitly teaches the jump"),Mode->ContextHint().Contains(TEXT("Space")));
                Mode->StartSlow();
                TestTrue(TEXT("Active Q does not hide boarding instructions"),Mode->ContextHint().Contains(TEXT("Space")));
                Lift->SetActorLocation(Lift->Origin+Lift->Travel);
                TestTrue(TEXT("Raised lift prompt explains to release Q and wait"),Mode->ContextHint().Contains(TEXT("come down")));
                Player->SetBase(Lift->Mesh);
                TestTrue(TEXT("Upper landing prompts the rider to step off"),Mode->ContextHint().Contains(TEXT("Walk forward")));
                Lift->Reset();
                TestTrue(TEXT("Boarded rider is taught to release Q"),Mode->ContextHint().Contains(TEXT("Release Q")));
                Player->SetBase(static_cast<FMovementBaseInterfaceData*>(nullptr));
                Mode->EndSlow();Mode->Cooldown=0.f;Mode->SlowRemaining=3.f;
                Lift->Reset();const float Initial=Lift->Clock;
                Mode->StartSlow();Lift->Tick(1.f);
                TestTrue(TEXT("Platform uses quarter device time"),FMath::IsNearlyEqual(Lift->Clock-Initial,.25f));
                const float PausedClock=Lift->Clock;const FVector PausedPosition=Lift->GetActorLocation();
                Mode->TogglePause();Lift->Tick(1.f);Mode->Tick(1.f);
                TestTrue(TEXT("Pause freezes platform time and position"),Lift->Clock==PausedClock && Lift->GetActorLocation()==PausedPosition);
                Mode->TogglePause();
                TestFalse(TEXT("Resume resolves released Q"),Mode->bSlowing);
                TestFalse(TEXT("No launch velocity inherited from lifts"),Player->GetCharacterMovement()->bImpartBaseVelocityZ);
            }
            const float SavedFirst=Mode->DeliveryTimes[0];
            Mode->Remaining=.01f;Mode->Tick(.02f);
            TestTrue(TEXT("Second-order timeout reaches failure"),Mode->Phase==ECourierPhase::Failed);
            Mode->ToggleHelp();TestTrue(TEXT("Failed run can open route hint"),Mode->bHelp && Mode->Phase==ECourierPhase::Failed);
            Mode->Retry();
            TestEqual(TEXT("Retry keeps earlier stamp"),Mode->CompletedOrders,1);
            TestEqual(TEXT("Retry keeps earlier delivery time"),Mode->DeliveryTimes[0],SavedFirst);
            TestEqual(TEXT("Retry remains on order two"),Mode->CurrentOrder,1);
            TestTrue(TEXT("Retry resets second budget without starting"),Mode->Remaining==120 && Mode->Phase==ECourierPhase::Briefing);
            Place(Mode->Pickup-FVector(0,0,12));Mode->Interact();Mode->Continue();
        }
        if(Order==2)
        {
            for(int32 I=0;I<=23;++I)
            {
                const FVector Floor(39900+I*100,2200,I*420.f/23.f);FHitResult Hit;
                const bool Supported=World->LineTraceSingleByChannel(Hit,Floor+FVector(0,0,180),Floor-FVector(0,0,80),ECC_Visibility,Params);
                TestTrue(TEXT("Final terrace ramp has walkable support"),Supported);
                if(Supported)TestTrue(TEXT("Final ramp has no hidden landing lip"),FMath::Abs(Hit.ImpactPoint.Z-Floor.Z)<25.f);
            }
        }
        Place(Mode->Pickup-FVector(0,0,12));Mode->Tick(.25f);
        // Just past the marker, the counter is still reachable. Do not send the
        // player backward while E is already offering the delivery action.
        Place(Mode->Destination+FVector(50,0,-12));PC->SetControlRotation(FRotator(-22,0,0));
        TestEqual(TEXT("At a reachable counter navigation says arrived, not behind"),Mode->NavigationText(),FString(TEXT("ARRIVED")));
        Mode->Interact();
        TestEqual(TEXT("Delivery adds exactly one stamp"),Mode->CompletedOrders,Order+1);
        TestFalse(TEXT("Delivery hides package"),Player->Bag->IsVisible());
        if(Order<2)
        {
            TestTrue(TEXT("Intermediate result is not final ending"),Mode->Phase==ECourierPhase::Delivered);
            if(Order==1)TestTrue(TEXT("Second delivery lights projector"),Visible(TEXT("CourierProjection"))>0);
            TestEqual(TEXT("Finale waits for all deliveries"),Visible(TEXT("CourierFinale")),0);
            const FVector Station=Mode->Destination;Mode->Continue();
            TestTrue(TEXT("Next brief opens at the delivery station without starting its clock"),Mode->Phase==ECourierPhase::Briefing && Mode->Pickup.Equals(Station));
        }
    }
    TestTrue(TEXT("Third delivery reaches complete ending"),Mode->Phase==ECourierPhase::Complete);
    TestTrue(TEXT("Final landscape and lights visible"),Visible(TEXT("CourierFinale"))>5);
    const float Finished=Mode->Remaining;Mode->Tick(10.f);
    TestEqual(TEXT("Ending does not run delivery clock"),Mode->Remaining,Finished);
    Mode->Retry();TestTrue(TEXT("Retry cannot undo completed game"),Mode->Phase==ECourierPhase::Complete);
    Mode->Continue();TestTrue(TEXT("Play again returns to opening"),Mode->Phase==ECourierPhase::Opening);
    TestEqual(TEXT("New run has zero stamps"),Mode->CompletedOrders,0);
    TestEqual(TEXT("New run has zero retries"),Mode->Retries,0);
    TestEqual(TEXT("New run turns final scene off"),Visible(TEXT("CourierFinale")),0);
    AddInfo(TEXT("Controlled state/geometry regression; not a complete real-input playthrough, human feedback or a 5–10 minute measurement."));
    return true;
}
#endif
