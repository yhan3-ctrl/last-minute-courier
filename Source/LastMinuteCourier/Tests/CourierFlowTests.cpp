#include "../CourierPrototype.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourierFlowRegression,"Courier.Regression.FirstOrderBoundaries",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FCourierFlowRegression::RunTest(const FString&)
{
    UWorld* World=nullptr;
    for(const auto& Context:GEngine->GetWorldContexts())
        if(Context.World() && Context.World()->GetAuthGameMode<ACourierGameMode>())World=Context.World();
    if(!TestNotNull(TEXT("Run with the courier map in -game mode"),World))return false;
    auto* Mode=World->GetAuthGameMode<ACourierGameMode>();
    auto* Player=Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(World,0));
    auto* PC=UGameplayStatics::GetPlayerController(World,0);
    if(!TestNotNull(TEXT("Player spawned"),Player) || !TestNotNull(TEXT("Controller spawned"),PC))return false;
    ACourierCart* Training=nullptr;
    ACourierCart* DeliveryCart=nullptr;
    for(TActorIterator<ACourierCart> It(World);It;++It)
        if(It->bTraining)Training=*It;else if(It->OrderIndex==0)DeliveryCart=*It;
    if(!TestNotNull(TEXT("Training cart exists"),Training) || !TestNotNull(TEXT("Delivery cart exists"),DeliveryCart))return false;

    auto Place=[Player,Mode](const FVector& Position,bool Grounded=true)
    {
        Mode->RecoverySeconds=0.f; // Controlled placement represents arrival after recovery.
        Player->StopJumping();
        Player->GetCharacterMovement()->StopMovementImmediately();
        Player->SetActorLocation(Position,false,nullptr,ETeleportType::TeleportPhysics);
        Player->GetCharacterMovement()->SetMovementMode(Grounded?MOVE_Walking:MOVE_Falling);
    };
    const auto Act=[Mode](){Mode->Interact();if(Mode->Phase==ECourierPhase::Briefing)Mode->Continue();};
    TestEqual(TEXT("Fresh run defaults to English"),Mode->Language,FString(TEXT("en")));
    TestEqual(TEXT("Three focused opening cards"),Mode->CardTitles.Num(),3);
    Mode->Back();TestEqual(TEXT("Back on first card stays at zero"),Mode->Card,0);
    Mode->StartSlow();Act();Mode->Tick(2.f);
    TestFalse(TEXT("Ability cannot start during opening"),Mode->bSlowing);
    TestTrue(TEXT("Opening does not start clock"),FMath::IsNearlyEqual(Mode->Remaining,40.f));
    Mode->Continue();Mode->Continue();Mode->Back();
    TestEqual(TEXT("Card back navigation"),Mode->Card,1);
    for(const FString& Code:Mode->Languages())
    {
        Mode->SetLanguage(Code);
        const FString Title=Mode->Copy(*Mode->CardTitles[0],*Mode->CardTitlesZh[0]);
        TestFalse(*FString::Printf(TEXT("Nonempty opening translation: %s"),*Code),Title.IsEmpty());
        if(Code!=TEXT("en"))TestTrue(*FString::Printf(TEXT("Localized title: %s"),*Code),Title!=Mode->CardTitles[0]);
    }
    Mode->SetLanguage(TEXT("unsupported"));TestEqual(TEXT("Invalid locale rejected"),Mode->Language,FString(TEXT("de")));
    Mode->SetLanguage(TEXT("en"));
    while(Mode->Phase==ECourierPhase::Opening)Mode->Continue();
    TestTrue(TEXT("Opening enters practice"),Mode->Phase==ECourierPhase::Practice);
    Mode->Retry();TestTrue(TEXT("Retry cannot skip practice"),Mode->Phase==ECourierPhase::Practice);
    Mode->TogglePause();Mode->Retry();
    TestTrue(TEXT("Paused practice retry cannot skip practice"),Mode->Phase==ECourierPhase::Paused && Mode->BeforePause==ECourierPhase::Practice);
    Mode->TogglePause();
    Place(FVector(600,0,88));Mode->Tick(.01f);
    TestEqual(TEXT("Move marker advances tutorial"),Mode->Tutorial,1);
    Place(FVector(1200,0,88));Mode->Tick(.01f);
    TestEqual(TEXT("Crossing without jumping does not pass jump lesson"),Mode->Tutorial,1);
    // Complete the real Character jump callback near the lesson, then cross.
    Place(FVector(560,0,88));Player->OnJumped_Implementation();
    TestTrue(TEXT("A jump immediately after its prompt is accepted"),Mode->bTrainingJumped);
    TestTrue(TEXT("Jump navigation stays ahead until its exit"),Mode->NavigationTarget().X>1100.f);
    Place(FVector(1200,0,88));Mode->Tick(.01f);
    TestEqual(TEXT("Learned jump advances tutorial"),Mode->Tutorial,2);
    // Slide lesson: the beam only lets a crouched (sliding) courier through.
    Place(FVector(1200,0,88));Mode->Tick(.01f);
    TestEqual(TEXT("Standing before the beam does not pass the slide lesson"),Mode->Tutorial,2);
    Place(FVector(1400,0,88));Mode->Tick(.01f);
    TestEqual(TEXT("Past the beam advances to the cart lesson"),Mode->Tutorial,3);
    // A safe crossing is valid without Q. Do not disguise an instruction as a collision.
    Place(FVector(2000,0,88));Mode->Tick(.01f);
    TestEqual(TEXT("Safe crossing without Q completes the lesson"),Mode->Tutorial,4);
    TestTrue(TEXT("Safe crossing is never teleported backward"),Player->GetActorLocation().Equals(FVector(2000,0,88)));
    TestEqual(TEXT("Crossing saves forward progress"),Mode->Checkpoint.X,2000.0);
    Mode->Tutorial=3;Mode->Checkpoint=FVector(1360,0,120);
    Place(FVector(1450,0,88));Mode->StartSlow();Training->Tick(.1f);Mode->EndSlow();
    Place(FVector(2000,0,88));Mode->Tick(.01f);
    TestEqual(TEXT("Early Q release does not invalidate a safe crossing"),Mode->Tutorial,4);
    Mode->Tutorial=3;Mode->Cooldown=0;Mode->SlowRemaining=3;
    Place(FVector(1450,0,88));Mode->StartSlow();Training->Tick(.1f);
    Place(FVector(2000,0,88));Mode->Tick(.01f);
    TestEqual(TEXT("Crossing with Q is equally accepted"),Mode->Tutorial,4);
    Mode->EndSlow();Mode->Tutorial=3;Mode->Checkpoint=FVector(1360,0,120);
    Training->SetActorLocation(FVector(1550,420,55));
    Place(FVector(1550,0,88));Mode->CollisionProtection=0;Mode->Tick(.01f);
    TestTrue(TEXT("A visible cart gap does not cause recovery"),Player->GetActorLocation().Equals(FVector(1550,0,88)));
    {
        // Traffic: carts drive out of one depot and into the other, never swing back.
        float OnLane=-1.f;
        for(float C=0.f;C<Training->CycleSeconds();C+=.02f)if(Training->OnStreet(C) && FMath::Abs(Training->YAt(C))<25.f){OnLane=C;break;}
        TestTrue(TEXT("The training cart crosses the lane centre"),OnLane>=0.f);
        float Previous=Training->YAt(0.f)*Training->Direction;bool bOneWay=true;
        for(float C=.02f;C<Training->CycleSeconds();C+=.02f)
            if(Training->OnStreet(C) && Training->OnStreet(C-.02f)){const float Y=Training->YAt(C)*Training->Direction;if(Y<Previous)bOneWay=false;Previous=Y;}
            else Previous=-1e6f;
        TestTrue(TEXT("Carts drive one way, depot to depot"),bOneWay);
        Training->Clock=OnLane;Training->Place();
        Place(Training->GetActorLocation()+FVector(0,0,33));Mode->CollisionProtection=0;Mode->Tick(.01f);
        TestTrue(TEXT("A cart knocks the courier aside instead of teleporting"),Player->GetCharacterMovement()->PendingLaunchVelocity.Size()>300.f && Mode->StunSeconds>0.f);
        TestTrue(TEXT("The bump is explained"),Mode->StatusNotice().Contains(TEXT("Bumped")));
        Mode->StunSeconds=0.f;Player->GetCharacterMovement()->PendingLaunchVelocity=FVector::ZeroVector;
    }
    Place(FVector(2000,0,88));Mode->Tick(.01f);
    TestEqual(TEXT("Can complete the safe crossing after a collision"),Mode->Tutorial,4);
    {
        // The gate holds anyone who skips the desk, with a reason, and the carts
        // beyond it keep moving so the yard never looks frozen.
        Place(FVector(3000,0,88));Mode->Tick(.01f);
        TestTrue(TEXT("Closed yard gate returns a courier who skipped the desk"),Player->GetActorLocation().X<2760.f);
        TestTrue(TEXT("Gate return explains why"),Mode->StatusNotice().Contains(TEXT("gate")));
        Mode->RecoverySeconds=0.f;
        const float Before=DeliveryCart->Clock;DeliveryCart->Tick(.5f);
        TestTrue(TEXT("Delivery carts move during practice"),DeliveryCart->Clock>Before);
        Mode->NoticeSeconds=0.f;
        Place(FVector(1700,0,88));Mode->Interact();
        TestTrue(TEXT("E with nothing to do explains what to do"),!Mode->StatusNotice().IsEmpty());
        Mode->NoticeSeconds=0.f;
    }
    Place(FVector(2400,-230,88));Mode->Tick(.01f);
    TestTrue(TEXT("Walking up to the desk counter opens the first brief"),Mode->Phase==ECourierPhase::Briefing);
    Mode->Back();Mode->Tick(.01f);
    TestTrue(TEXT("A closed brief does not reopen while standing at the desk"),Mode->Phase==ECourierPhase::AwaitPickup);
    TestTrue(TEXT("Practice reaches pickup"),Mode->Phase==ECourierPhase::AwaitPickup);
    Mode->Tick(5.f);TestTrue(TEXT("Pickup waiting has no delivery countdown"),FMath::IsNearlyEqual(Mode->Remaining,40.f));
    Place(FVector(2400,0,-100),false);Act();
    TestTrue(TEXT("Cannot collect a package from under the ground"),Mode->Phase==ECourierPhase::AwaitPickup);
    // Restore a valid pickup state even when running this test against an old build.
    if(Mode->Phase!=ECourierPhase::AwaitPickup)Mode->Retry();
    Place(FVector(2400,0,330),false);Act();
    TestTrue(TEXT("Cannot collect high above the desk"),Mode->Phase==ECourierPhase::AwaitPickup);
    if(Mode->Phase!=ECourierPhase::AwaitPickup)Mode->Retry();
    Place(FVector(2400,0,88));Act();
    TestTrue(TEXT("Grounded collection starts order"),Mode->Phase==ECourierPhase::Delivery);
    for(int32 I=0;I<20;++I)Mode->Tick(.1f);
    TestTrue(TEXT("Accepting the first order opens the yard gate"),Mode->GateOpen() && Mode->GateAngle>40.f);
    TestEqual(TEXT("First delivery gives three uses of borrowed time"),Mode->Charges,3);
    {
        int32 Uses=0;float Recharge=0.f;
        for(int32 I=0;I<4;++I)
        {
            Mode->Cooldown=0.f;Mode->SlowRemaining=3.f;Mode->StartSlow();
            if(Mode->bSlowing){++Uses;Mode->EndSlow();Recharge=Mode->Cooldown;}
        }
        TestEqual(TEXT("Only three uses are granted"),Uses,3);
        TestTrue(TEXT("Out of uses gives visible feedback"),Mode->Charges==0 && Mode->QDenied>0.f && Mode->StatusNotice().Contains(TEXT("No borrowed time")));
        TestTrue(TEXT("Recharge between uses is short"),FMath::IsNearlyEqual(Recharge,ACourierGameMode::RechargeSeconds));
        Mode->ResetDevices();Mode->NoticeSeconds=0.f;Mode->QDenied=0.f;
        TestEqual(TEXT("Resetting the order restores its uses"),Mode->Charges,3);
    }
    {
        // Golden clocks: practice teaches one, each delivery route offers several.
        TestEqual(TEXT("Practice yard has one teaching clock"),Mode->TokensTotal(-1),1);
        TestTrue(TEXT("Every delivery has golden clocks"),Mode->TokensTotal(0)>=3 && Mode->TokensTotal(1)>=4 && Mode->TokensTotal(2)>=5);
        FVector Spot=FVector::ZeroVector;
        for(const auto& Token:Mode->Tokens)if(Token.Order==0){Spot=Token.Base;break;}
        const float Before=Mode->Remaining;const int32 Uses=Mode->Charges;
        Place(Spot-FVector(0,0,22));Mode->CollectTokens(Player->GetActorLocation());
        TestTrue(TEXT("A Borrower cell adds one Q use"),Mode->Charges==Uses+1 && FMath::IsNearlyEqual(Mode->Remaining,Before));
        TestEqual(TEXT("The clock is counted"),Mode->TokensTaken(0),1);
        Mode->CollectTokens(Player->GetActorLocation());
        TestTrue(TEXT("A collected cell cannot be taken twice"),Mode->Charges==Uses+1);
        Mode->ResetTokens(0);TestEqual(TEXT("Restarting an order restores its clocks"),Mode->TokensTaken(0),0);
        const float Budget=Mode->OrderBudget();
        Mode->Remaining=Budget*.5f;TestEqual(TEXT("Half the clock left earns three stars"),Mode->RateDelivery(),3);
        Mode->Remaining=Budget*.3f;TestEqual(TEXT("A third left earns two stars"),Mode->RateDelivery(),2);
        Mode->Remaining=Budget*.05f;TestEqual(TEXT("Any on-time delivery earns a star"),Mode->RateDelivery(),1);
        Mode->Remaining=Before;Mode->NoticeSeconds=0.f;Mode->Charges=Uses;
        // The lens is judged by care: two knocks leave one star, the third cracks it.
        Mode->CurrentOrder=1;Mode->LensCondition=3;
        TestEqual(TEXT("An unknocked lens earns three stars"),Mode->RateDelivery(),3);
        Mode->DamageLens(TEXT("test"),TEXT("test"));Mode->DamageLens(TEXT("test"),TEXT("test"));
        TestEqual(TEXT("Two knocks leave one star"),Mode->RateDelivery(),1);
        TestTrue(TEXT("The lens survives two knocks"),Mode->Phase==ECourierPhase::Delivery);
        Mode->DamageLens(TEXT("test"),TEXT("test"));
        TestTrue(TEXT("The third knock cracks the lens and fails the order"),Mode->Phase==ECourierPhase::Failed && Mode->FailReason==1);
        Mode->Phase=ECourierPhase::Delivery;Mode->FailReason=0;Mode->CurrentOrder=0;Mode->LensCondition=3;Mode->NoticeSeconds=0.f;
    }
    TestTrue(TEXT("Collection reveals package"),Player->Bag->IsVisible());
    Place(FVector(2600,0,88));
    const float ClockBefore=Mode->Remaining;
    Mode->StartSlow();const float CartBefore=DeliveryCart->Clock;
    DeliveryCart->Tick(1.f);Mode->Tick(1.f);
    TestTrue(TEXT("Slowdown affects device by one quarter"),FMath::IsNearlyEqual(DeliveryCart->Clock-CartBefore,.25f,.001f));
    TestTrue(TEXT("Slowdown does not slow delivery timer"),FMath::IsNearlyEqual(ClockBefore-Mode->Remaining,1.f,.001f));
    TestTrue(TEXT("Ability consumes charge"),FMath::IsNearlyEqual(Mode->SlowRemaining,2.f,.001f));
    Mode->TogglePause();
    TestTrue(TEXT("Pause really pauses Unreal world"),World->IsPaused());
    Mode->bLockCamera=true;
    PC->SetControlRotation(FRotator(20,75,0));Mode->RecenterCamera();
    TestTrue(TEXT("Explicit recenter works while paused and camera locked"),PC->GetControlRotation().Equals(FRotator(-22,0,0),.01f));
    TestTrue(TEXT("Recenter preserves pause and camera lock"),World->IsPaused() && Mode->bLockCamera);
    Mode->bLockCamera=false;
    const float PausedClock=Mode->Remaining,PausedCharge=Mode->SlowRemaining,PausedCart=DeliveryCart->Clock;
    Mode->Tick(1.f);DeliveryCart->Tick(1.f);
    TestTrue(TEXT("Pause freezes countdown"),Mode->Remaining==PausedClock);
    TestTrue(TEXT("Pause freezes ability charge"),Mode->SlowRemaining==PausedCharge);
    TestTrue(TEXT("Pause freezes cart"),DeliveryCart->Clock==PausedCart);
    Mode->TogglePause();
    TestFalse(TEXT("Resume unpauses world"),World->IsPaused());
    TestFalse(TEXT("Released Q during pause ends ability on resume"),Mode->bSlowing);
    TestTrue(TEXT("Resume enters the short recharge"),FMath::IsNearlyEqual(Mode->Cooldown,ACourierGameMode::RechargeSeconds));
    Mode->StartSlow();TestFalse(TEXT("Ability cannot restart during cooldown"),Mode->bSlowing);
    Mode->Tick(4.f);TestTrue(TEXT("Cooldown restores full charge"),Mode->Cooldown==0 && Mode->SlowRemaining==3.f);
    Mode->StartSlow();Mode->Tick(3.f);
    TestTrue(TEXT("Exhausted ability ends with the short recharge"),!Mode->bSlowing && FMath::IsNearlyEqual(Mode->Cooldown,ACourierGameMode::RechargeSeconds));

    Mode->Checkpoint=FVector(4500,0,120);Place(FVector(4500,1500,88));Mode->Tick(.1f);
    TestTrue(TEXT("Off-road recovery uses checkpoint"),FVector::Dist2D(Player->GetActorLocation(),Mode->Checkpoint)<1.f);
    TestTrue(TEXT("Recovery grants collision protection"),Mode->CollisionProtection>0);
    Place(FVector(2400,0,88));Mode->Remaining=.01f;Mode->Tick(.02f);
    TestTrue(TEXT("Timeout reaches failure state"),Mode->Phase==ECourierPhase::Failed);
    TestFalse(TEXT("Failure blocks player movement"),Mode->CanMove());
    const int32 RetriesBefore=Mode->Retries;Mode->Retry();
    TestTrue(TEXT("Retry reopens the brief at the desk without starting timer"),Mode->Phase==ECourierPhase::Briefing && Mode->Remaining==40.f);
    TestEqual(TEXT("Retry increments count once"),Mode->Retries,RetriesBefore+1);
    TestFalse(TEXT("Retry hides package"),Player->Bag->IsVisible());
    TestTrue(TEXT("Retry resets ability and all cooldown"),!Mode->bSlowing && Mode->Cooldown==0 && Mode->SlowRemaining==3.f);
    Place(FVector(2400,0,88));Act();
    Place(FVector(8000,0,330),false);Act();
    TestTrue(TEXT("Cannot deliver high above destination"),Mode->Phase==ECourierPhase::Delivery);
    Place(FVector(8000,0,88));Act();
    TestTrue(TEXT("Grounded delivery reaches success"),Mode->Phase==ECourierPhase::Delivered);
    TestFalse(TEXT("Success hides package"),Player->Bag->IsVisible());
    const float FinishedClock=Mode->Remaining;Mode->Tick(5.f);
    TestTrue(TEXT("Success stops delivery countdown"),Mode->Remaining==FinishedClock);
    Mode->Retry();TestTrue(TEXT("Cannot undo a completed delivery with retry"),Mode->Phase==ECourierPhase::Delivered);
    AddInfo(TEXT("State/clock tests use explicit engine ticks and controlled locations. They do not replace input, visual, performance or human playtesting."));
    return true;
}
#endif
