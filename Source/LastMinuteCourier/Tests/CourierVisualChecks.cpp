#include "../CourierPrototype.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Explicit developer-only inspection starts, excluded from the regression suite.
// These use controlled positions and are not evidence of a full input playthrough.
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FCourierVisualInspection,"Courier.Visual",
    EAutomationTestFlags::ClientContext|EAutomationTestFlags::EngineFilter)

void FCourierVisualInspection::GetTests(TArray<FString>& Names,TArray<FString>& Commands) const
{
    for(const TCHAR* Name:{TEXT("Coffee"),TEXT("GardenCorner"),TEXT("ProjectionCorner"),TEXT("Cinema")})
    {Names.Add(Name);Commands.Add(Name);}
}

bool FCourierVisualInspection::RunTest(const FString& Name)
{
    UWorld* World=nullptr;
    for(const auto& Context:GEngine->GetWorldContexts())
        if(Context.World() && Context.World()->GetAuthGameMode<ACourierGameMode>())World=Context.World();
    if(!TestNotNull(TEXT("Courier game world"),World))return false;
    auto* Mode=World->GetAuthGameMode<ACourierGameMode>();
    auto* Player=Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(World,0));
    auto* PC=UGameplayStatics::GetPlayerController(World,0);
    if(!Player || !PC)return false;
    PC->SetPause(false);Mode->Phase=ECourierPhase::Complete;Mode->RestartRun();Mode->SetLanguage(TEXT("en"));
    const auto Place=[&](FVector At)
    {
        Player->SetBase(static_cast<FMovementBaseInterfaceData*>(nullptr));
        Player->GetCharacterMovement()->StopMovementImmediately();Player->StopJumping();
        Player->SetActorLocation(At,false,nullptr,ETeleportType::TeleportPhysics);
        Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);Mode->RecoverySeconds=0;
    };
    while(Mode->Phase==ECourierPhase::Opening)Mode->Continue();
    Mode->Tutorial=4;Place(Mode->Pickup-FVector(0,0,12));Mode->Tick(.01f);
    const int32 Target=Name==TEXT("Coffee")?0:(Name==TEXT("Cinema")?2:1);
    for(int32 I=0;I<=Target;++I)
    {
        Place(Mode->Pickup-FVector(0,0,12));Mode->Interact();Mode->Continue();
        if(I<Target){Place(Mode->Destination-FVector(0,0,12));Mode->Interact();Mode->Continue();}
    }
    Place(Name==TEXT("Coffee")?FVector(6750,0,90):(Name==TEXT("GardenCorner")?FVector(14400,1900,410):
        (Name==TEXT("ProjectionCorner")?FVector(24900,2100,490):FVector(44500,0,510))));
    Mode->bLockCamera=true;PC->SetControlRotation(FRotator(-22,0,0));
    AddInfo(TEXT("Controlled visual inspection start only. Verify the resulting rendered view and movement separately; this does not measure level completion or player feedback."));
    return true;
}
#endif
