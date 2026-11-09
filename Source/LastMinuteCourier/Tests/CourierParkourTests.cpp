#include "../CourierPrototype.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/App.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Parkour moves and set pieces, checked with real character movement frames.
namespace
{
    ACourierObstacle* FindObstacle(ACourierGameMode* Mode,ECourierObstacleKind Kind,int32 Order)
    {
        for(const auto& O:Mode->Obstacles)if(O && O->Kind==Kind && O->OrderIndex==Order)return O;
        return nullptr;
    }
    // Clock value at which a shutter is at fraction F of its cycle.
    float ShutterClock(const ACourierObstacle* O,float F){float C=(F-O->InitialPhase)*O->Period;while(C<0.f)C+=O->Period;return C;}
}

class FCourierParkourCheck final : public IAutomationLatentCommand
{
public:
    FCourierParkourCheck(FAutomationTestBase* InTest,UWorld* InWorld,ACourierGameMode* InMode,ACourierCharacter* InPlayer)
        : Test(InTest),World(InWorld),Mode(InMode),Player(InPlayer)
    {
        PreviousFixed=FApp::UseFixedTimeStep();PreviousDelta=FApp::GetFixedDeltaTime();
        FApp::SetFixedDeltaTime(1./60.);FApp::SetUseFixedTimeStep(true);
    }
    virtual ~FCourierParkourCheck() override
    {FApp::SetFixedDeltaTime(PreviousDelta);FApp::SetUseFixedTimeStep(PreviousFixed);}

    void Put(const FVector& At)
    {
        Player->CancelMoves();
        Player->GetCharacterMovement()->StopMovementImmediately();
        Player->SetActorLocation(At,false,nullptr,ETeleportType::TeleportPhysics);
        Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        Mode->RecoverySeconds=0.f;Mode->StunSeconds=0.f;Mode->CollisionProtection=0.f;
        StageStart=World->GetTimeSeconds();
    }

    virtual bool Update() override
    {
        const float T=World->GetTimeSeconds()-StageStart;
        const FVector P=Player->GetActorLocation();
        switch(Stage)
        {
        case 0: // Practice: standing, the beam stops you.
            Mode->Phase=ECourierPhase::Practice;Mode->CurrentOrder=0;Mode->Tutorial=2;Mode->Checkpoint=FVector(1150,0,120);
            Put(FVector(1050,0,90));Stage=1;return false;
        case 1:
            Player->AddMovementInput(FVector(1,0,0),1.f);
            if(T<1.2f)return false;
            Test->TestTrue(TEXT("The practice beam blocks a standing courier"),P.X<1240.f);
            Test->TestEqual(TEXT("Standing does not pass the slide lesson"),Mode->Tutorial,2);
            Put(FVector(1000,0,90));Stage=2;return false;
        case 2: // ...a sprint and a slide carry you under it.
            Player->SetSprint(true);Player->AddMovementInput(FVector(1,0,0),1.f);
            // Press slide once grounded and running, like a player would.
            if(T>.15f && !bSlid){Player->StartSlide();bSlid=Player->IsSliding();if(bSlid)SlideAt=T;}
            if(bSlid && T>SlideAt+.05f && T<SlideAt+.3f)MinHalf=FMath::Min(MinHalf,Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
            if(T<2.2f)return false;
            Test->TestTrue(TEXT("Sliding halves the capsule"),MinHalf<50.f);
            Test->TestTrue(TEXT("A slide passes under the practice beam"),P.X>1300.f);
            Test->TestTrue(TEXT("Passing the beam completes the slide lesson"),Mode->Tutorial>=3);
            Test->TestTrue(TEXT("The courier stands up again after the slide"),Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()>80.f);
            Player->SetSprint(false);
            // Coffee order: climb the crate wall with a jump.
            Mode->Phase=ECourierPhase::Delivery;Mode->CurrentOrder=0;Mode->Remaining=60.f;Mode->Checkpoint=FVector(4200,0,120);
            Put(FVector(4300,300,90));bJumped=false;Stage=3;return false;
        case 3:
            Player->AddMovementInput(FVector(1,0,0),1.f);
            if(T>.35f && !bJumped){Player->JumpPressed();bJumped=true;}
            Highest=FMath::Max(Highest,P.Z);
            if(T<2.f)return false;
            Test->TestTrue(TEXT("A jump at the crates climbs onto them"),Highest>150.f+88.f-6.f);
            Test->TestTrue(TEXT("The courier crosses the crate wall"),P.X>4650.f);
            {
                // Shutter, shut: it is a solid door and does not hurt.
                Shutter=FindObstacle(Mode,ECourierObstacleKind::Shutter,0);
                Test->TestNotNull(TEXT("Coffee route has a shutter"),Shutter);
                if(!Shutter)return true;
                Shutter->Clock=ShutterClock(Shutter,.70f);Shutter->Pose();
                Test->TestTrue(TEXT("Shut door sits at the floor"),Shutter->DoorBottom(Shutter->Clock)<1.f);
                FHitResult CameraHit;
                const FVector DoorMid=Shutter->Origin+FVector(0,0,150);
                Test->TestTrue(TEXT("Opaque shutter protects the following camera from clipping inside"),
                    World->LineTraceSingleByChannel(CameraHit,DoorMid-FVector(300,0,0),DoorMid+FVector(300,0,0),ECC_Camera));
                if(CameraHit.GetComponent())Test->TestTrue(TEXT("Camera-facing shutter geometry does not add a pawn wall"),
                    CameraHit.GetComponent()->GetCollisionResponseToChannel(ECC_Pawn)==ECR_Ignore);
            }
            Put(FVector(6050,0,90));Stage=4;return false;
        case 4:
            Shutter->Clock=ShutterClock(Shutter,.70f);Shutter->Pose();
            Player->AddMovementInput(FVector(1,0,0),1.f);
            if(T<1.f)return false;
            Test->TestTrue(TEXT("A shut shutter blocks the way"),P.X<6300.f);
            Test->TestTrue(TEXT("Walking into a shut door is not a hit"),Mode->StunSeconds<=0.f);
            Put(FVector(6050,0,90));Stage=5;return false;
        case 5: // Open: walk straight through.
            Shutter->Clock=ShutterClock(Shutter,.10f);Shutter->Pose();
            Player->AddMovementInput(FVector(1,0,0),1.f);
            if(T<1.2f)return false;
            Test->TestTrue(TEXT("An open shutter lets the courier through"),P.X>6400.f);
            {
                // Closing onto a standing courier knocks them back; a slide fits.
                Shutter->Clock=ShutterClock(Shutter,.523f);Shutter->Pose();
                Test->TestTrue(TEXT("Mid-close door leaves slide room only"),Shutter->DoorBottom(Shutter->Clock)>100.f && Shutter->DoorBottom(Shutter->Clock)<170.f);
                Put(FVector(6300,0,90));Mode->Tick(.001f);
                Shutter->Check(*Mode,*Player,.001f);
                Test->TestTrue(TEXT("A closing shutter knocks a standing courier back"),Mode->StunSeconds>0.f);
                Mode->StunSeconds=0.f;Mode->CollisionProtection=0.f;Player->GetCharacterMovement()->PendingLaunchVelocity=FVector::ZeroVector;
            }
            Put(FVector(6300,0,90));Player->StartSlide();Stage=6;return false;
        case 6:
            if(T<.1f)return false;
            Shutter->Clock=ShutterClock(Shutter,.523f);Shutter->Pose();Mode->CollisionProtection=0.f;
            Shutter->Check(*Mode,*Player,.016f);
            Test->TestTrue(TEXT("A sliding courier fits under a half-closed shutter"),Mode->StunSeconds<=0.f);
            {
                // Turnstile: the low arm needs a jump, the high arm a slide.
                ACourierObstacle* Sweeper=FindObstacle(Mode,ECourierObstacleKind::Sweeper,1);
                Test->TestNotNull(TEXT("Garden has a turnstile"),Sweeper);
                if(!Sweeper)return true;
                Mode->CurrentOrder=1;Mode->Phase=ECourierPhase::Delivery;Mode->LensCondition=3;
                Sweeper->Clock=0.f;Sweeper->Pose();
                const float Yaw=FMath::DegreesToRadians(Sweeper->ArmYaw(0.f));
                const FVector Low=Sweeper->Origin+FVector(FMath::Cos(Yaw),FMath::Sin(Yaw),0)*250.f;
                Put(Low+FVector(0,0,90));Mode->CollisionProtection=0.f;
                Sweeper->Check(*Mode,*Player,.016f);
                Test->TestTrue(TEXT("The low arm sweeps a courier on the ground"),Mode->StunSeconds>0.f);
                Test->TestEqual(TEXT("A sweep knocks the lens case"),Mode->LensCondition,2);
                Mode->StunSeconds=0.f;Mode->CollisionProtection=0.f;Player->GetCharacterMovement()->PendingLaunchVelocity=FVector::ZeroVector;
                Put(Low+FVector(0,0,90+95));Player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
                const int32 Bank=Mode->MoteBank;Sweeper->CloseCallCooldown=0.f;
                Sweeper->Check(*Mode,*Player,.016f);
                Test->TestTrue(TEXT("Jumping the low arm is safe"),Mode->StunSeconds<=0.f);
                Test->TestTrue(TEXT("Clearing an arm is a close call worth sparks"),Mode->MoteBank>Bank || Mode->Charges>0);
                const FVector High=Sweeper->Origin-FVector(FMath::Cos(Yaw),FMath::Sin(Yaw),0)*250.f;
                Put(High+FVector(0,0,90));Mode->CollisionProtection=0.f;
                Sweeper->Check(*Mode,*Player,.016f);
                Test->TestTrue(FString::Printf(TEXT("The high arm sweeps a standing courier (half %.0f, z %.0f)"),
                    Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),Player->GetActorLocation().Z),Mode->StunSeconds>0.f);
                Mode->LensCondition=3;
                // Springboard.
                ACourierObstacle* Spring=FindObstacle(Mode,ECourierObstacleKind::Spring,2);
                Test->TestNotNull(TEXT("Reel route has a springboard"),Spring);
                if(!Spring)return true;
                Mode->CurrentOrder=2;Spring->CloseCallCooldown=0.f;
                Put(Spring->Origin+FVector(0,0,30+88));
                Player->GetCharacterMovement()->Velocity=FVector(600,0,0);
                Spring->Check(*Mode,*Player,.016f);
                Test->TestTrue(FString::Printf(TEXT("The springboard launches the courier upward (half %.0f, z %.0f, vz %.0f)"),
                    Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),Player->GetActorLocation().Z,Player->GetCharacterMovement()->Velocity.Z),
                    Player->GetCharacterMovement()->PendingLaunchVelocity.Z>900.f);
                Player->GetCharacterMovement()->PendingLaunchVelocity=FVector::ZeroVector;
            }
            Test->AddInfo(TEXT("Actual engine-frame slide, ledge climb and shutter blocking; contact rules for shutters, turnstiles and springboards."));
            Mode->Phase=ECourierPhase::Complete;Mode->RestartRun();
            return true;
        default:return true;
        }
    }
private:
    FAutomationTestBase* Test;
    UWorld* World;
    ACourierGameMode* Mode;
    ACourierCharacter* Player;
    ACourierObstacle* Shutter=nullptr;
    bool PreviousFixed=false;
    double PreviousDelta=0;
    float StageStart=0,MinHalf=1000.f,Highest=-1e6f,SlideAt=0;
    bool bSlid=false,bJumped=false;
    int32 Stage=0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourierParkourRegression,"Courier.Regression.Parkour",
    EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)

bool FCourierParkourRegression::RunTest(const FString&)
{
    UWorld* World=nullptr;
    for(const auto& Context:GEngine->GetWorldContexts())
        if(Context.World() && Context.World()->GetAuthGameMode<ACourierGameMode>())World=Context.World();
    if(!TestNotNull(TEXT("Courier world exists"),World))return false;
    auto* Mode=World->GetAuthGameMode<ACourierGameMode>();
    auto* Player=Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(World,0));
    if(!TestNotNull(TEXT("Courier exists"),Player))return false;
    Mode->Phase=ECourierPhase::Complete;Mode->RestartRun();
    {
        // Sparks: lines on every route; 15 make one more use; a retry restores them.
        TestTrue(TEXT("Every delivery has a line of sparks"),Mode->MotesTotal(0)>=15 && Mode->MotesTotal(1)>=30 && Mode->MotesTotal(2)>=30);
        Mode->Phase=ECourierPhase::Delivery;Mode->CurrentOrder=1;Mode->Charges=4;Mode->MoteBank=0;
        int32 Taken=0;
        for(const auto& Mote:Mode->Motes)
        {
            if(Mote.Order!=1 || Taken>=ACourierGameMode::MotesPerUse)continue;
            Mode->CollectMotes(Mote.Base-FVector(0,0,10));++Taken;
        }
        TestEqual(TEXT("A full bar of sparks adds one Q use"),Mode->Charges,5);
        TestEqual(TEXT("Sparks are counted"),Mode->MotesTaken(1),ACourierGameMode::MotesPerUse);
        Mode->ResetTokens(1);
        TestEqual(TEXT("Restarting an order restores its sparks"),Mode->MotesTaken(1),0);
        bool bAllOnFloor=true;
        FCollisionQueryParams Params;Params.AddIgnoredActor(Player);
        for(const auto& Mote:Mode->Motes)
        {
            if(!Mote.Actor.IsValid() || (Mote.Base.X>39750.f && Mote.Base.X<41000.f && FMath::Abs(Mote.Base.Y-380.f)<60.f))continue; // springboard flight arc
            FHitResult Hit;
            const bool bHit=World->LineTraceSingleByChannel(Hit,Mote.Base,Mote.Base-FVector(0,0,700),ECC_Visibility,Params);
            if(!bHit)bAllOnFloor=false;
        }
        TestTrue(TEXT("Every spark floats over something you can stand on"),bAllOnFloor);
    }
    {
        // Turnstiles are gates: a wall runs from the arms' reach to the hedge, so
        // the only way on is through them. Their arms turn slowly enough to read.
        FCollisionQueryParams Params;Params.AddIgnoredActor(Player);
        const FCollisionShape Body=FCollisionShape::MakeCapsule(34.f,88.f);
        int32 Gates=0;
        for(const auto& O:Mode->Obstacles)if(O && O->Kind==ECourierObstacleKind::Sweeper)
        {
            ++Gates;
            TestTrue(*FString::Printf(TEXT("Turnstile %d turns slowly (period >= 7 s)"),O->OrderIndex+1),O->Period>=7.f);
            for(float Side:{-1.f,1.f})for(float Y:{O->ArmLength+120.f,O->ArmLength+500.f})
            {
                FHitResult Hit;
                const FVector From(O->Origin.X-300.f,Side*Y,O->Origin.Z+100.f),To(O->Origin.X+300.f,Side*Y,O->Origin.Z+100.f);
                TestTrue(*FString::Printf(TEXT("Turnstile %d cannot be walked around (y %.0f)"),O->OrderIndex+1,Side*Y),
                    World->SweepSingleByChannel(Hit,From,To,FQuat::Identity,ECC_Pawn,Body,Params));
            }
        }
        TestEqual(TEXT("Every delivery has a turnstile gate"),Gates,3);
    }
    ADD_LATENT_AUTOMATION_COMMAND(FCourierParkourCheck(this,World,Mode,Player));
    return true;
}
#endif
