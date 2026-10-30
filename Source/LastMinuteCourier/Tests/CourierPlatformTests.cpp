#include "../CourierPrototype.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/App.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Runs actual character movement across engine frames, unlike the state tests.
class FCourierRideCheck final : public IAutomationLatentCommand
{
public:
    FCourierRideCheck(FAutomationTestBase* InTest,UWorld* InWorld,ACourierGameMode* InMode,ACourierCharacter* InPlayer,ACourierLift* InLift)
        : Test(InTest),World(InWorld),Mode(InMode),Player(InPlayer),Lift(InLift)
    {
        PreviousFixed=FApp::UseFixedTimeStep();PreviousDelta=FApp::GetFixedDeltaTime();
        FApp::SetFixedDeltaTime(1./60.);FApp::SetUseFixedTimeStep(true);
        Start=World->GetTimeSeconds();InitialZ=Player->GetActorLocation().Z;
    }
    virtual ~FCourierRideCheck() override
    {FApp::SetFixedDeltaTime(PreviousDelta);FApp::SetUseFixedTimeStep(PreviousFixed);}

    virtual bool Update() override
    {
        const float Elapsed=World->GetTimeSeconds()-Start;
        if(Elapsed>.35f && Stage<3)
        {
            const float Error=FMath::Abs(Player->GetActorLocation().Z-(Lift->GetActorLocation().Z+108.f));
            LargestFootError=FMath::Max(LargestFootError,Error);
        }
        if(Stage==0 && Elapsed>=1.8f)
        {
            Test->TestTrue(TEXT("Character rides upward with the moving platform"),Player->GetActorLocation().Z>InitialZ+40.f);
            Test->TestTrue(TEXT("Character remains grounded on moving deck"),Player->GetCharacterMovement()->IsMovingOnGround());
            SlowClock=Lift->Clock;DeliveryClock=Mode->Remaining;PhaseStart=Elapsed;
            Mode->StartSlow();Stage=1;
        }
        else if(Stage==1 && Elapsed-PhaseStart>=.8f)
        {
            const float Interval=Elapsed-PhaseStart;
            Test->TestTrue(TEXT("Actual-frame platform clock slows by 0.25"),FMath::Abs((Lift->Clock-SlowClock)-Interval*.25f)<.04f);
            Test->TestTrue(TEXT("Actual-frame delivery time remains normal"),FMath::Abs((DeliveryClock-Mode->Remaining)-Interval)<.04f);
            Mode->EndSlow();PhaseStart=Elapsed;Stage=2;
        }
        else if(Stage==2 && Elapsed-PhaseStart>=.5f)
        {
            Test->TestTrue(TEXT("No foot separation when slowdown starts or ends"),LargestFootError<14.f);
            Test->TestTrue(TEXT("Restoring device speed does not launch the courier"),Player->GetCharacterMovement()->IsMovingOnGround());
            Mode->Retry();PhaseStart=Elapsed;Stage=3;
        }
        else if(Stage==3 && Elapsed-PhaseStart>=.7f)
        {
            Test->TestTrue(TEXT("Retry detaches from former moving base"),FVector::Dist2D(Player->GetActorLocation(),Mode->Pickup)<5.f);
            Test->TestTrue(TEXT("Retry settles onto pickup floor"),FMath::Abs(Player->GetActorLocation().Z-88.f)<5.f);
            Test->TestTrue(TEXT("Recovery does not inherit lift launch speed"),FMath::Abs(Player->GetVelocity().Z)<5.f);
            Mode->Phase=ECourierPhase::Complete;Mode->RestartRun();
            Test->AddInfo(TEXT("Actual engine-frame lift riding, slowdown transitions and retry detachment. No human comfort or performance claim."));
            return true;
        }
        return false;
    }
private:
    FAutomationTestBase* Test;
    UWorld* World;
    ACourierGameMode* Mode;
    ACourierCharacter* Player;
    ACourierLift* Lift;
    bool PreviousFixed=false;
    double PreviousDelta=0;
    float Start=0,InitialZ=0,PhaseStart=0,SlowClock=0,DeliveryClock=0,LargestFootError=0;
    int32 Stage=0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourierPlatformRegression,"Courier.Regression.PlatformRide",
    EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)

bool FCourierPlatformRegression::RunTest(const FString&)
{
    UWorld* World=nullptr;
    for(const auto& Context:GEngine->GetWorldContexts())
        if(Context.World() && Context.World()->GetAuthGameMode<ACourierGameMode>())World=Context.World();
    if(!TestNotNull(TEXT("Courier world exists"),World))return false;
    auto* Mode=World->GetAuthGameMode<ACourierGameMode>();
    auto* Player=Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(World,0));
    if(!TestNotNull(TEXT("Courier exists"),Player))return false;
    Mode->Phase=ECourierPhase::Complete;Mode->RestartRun();
    Mode->Phase=ECourierPhase::Delivered;Mode->CompletedOrders=1;Mode->NextOrder();
    Mode->Phase=ECourierPhase::Delivery;Mode->RecoverySeconds=0;
    ACourierLift* Platform=nullptr;
    for(TActorIterator<ACourierLift> It(World);It;++It)if(It->OrderIndex==1){Platform=*It;break;}
    if(!TestNotNull(TEXT("Physical lift exists"),Platform))return false;
    Platform->Reset();Player->GetCharacterMovement()->StopMovementImmediately();
    Player->SetBase(static_cast<FMovementBaseInterfaceData*>(nullptr));
    Player->SetActorLocation(Platform->GetActorLocation()+FVector(0,0,110),false,nullptr,ETeleportType::TeleportPhysics);
    Player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
    ADD_LATENT_AUTOMATION_COMMAND(FCourierRideCheck(this,World,Mode,Player,Platform));
    return true;
}
#endif
