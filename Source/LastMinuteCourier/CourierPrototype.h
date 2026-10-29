#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "CourierPrototype.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class USpringArmComponent;
class UCameraComponent;

enum class ECourierPhase : uint8
{
    Opening, Practice, AwaitPickup, Delivery, Delivered, Failed, Paused
};

// Native greybox for the FIRST order only. Full three-order level is a later milestone.
UCLASS()
class LASTMINUTECOURIER_API ACourierGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ACourierGameMode();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    ECourierPhase Phase = ECourierPhase::Opening;
    ECourierPhase BeforePause = ECourierPhase::Practice;
    int32 Card = 0;
    int32 Tutorial = 0;
    int32 Retries = 0;
    FString Language = TEXT("en"); // Always English on a new run, independent of OS language.
    bool bHelp = false;
    bool bSlowing = false;
    bool bLockCamera = false;
    float SlowRemaining = 3.f;
    float Cooldown = 0.f;
    float Remaining = 140.f;
    float OrderElapsed = 0.f;
    float CollisionProtection = 0.f;
    float MouseSensitivity = .6f;
    float CameraFov = 90.f;
    FVector Checkpoint = FVector(100, 0, 120);
    const FVector Pickup = FVector(2400, 0, 100);
    const FVector Destination = FVector(8000, 0, 100);
    TArray<FString> CardTitles;
    TArray<FString> CardBodies;
    TArray<FString> CardTitlesZh;
    TArray<FString> CardBodiesZh;
    TArray<FString> TutorialText;
    TArray<FString> TutorialTextZh;
    bool IsPlaying() const;
    bool CanMove() const;
    float DeviceRate() const;
    FString Copy(const TCHAR* English, const TCHAR* Chinese) const;
    void SetLanguage(const FString& Code);
    bool UsesCjkFont() const;
    static const TArray<FString>& Languages();
    static FString LanguageName(const FString& Code);
    FString InteractionPrompt() const;
    void Continue();
    void Back();
    void Interact();
    void StartSlow();
    void EndSlow();
    void TogglePause();
    void ToggleHelp();
    void ToggleLanguage();
    void Retry();
private:
    UPROPERTY() TArray<TObjectPtr<class ACourierCart>> Carts;
    UPROPERTY() TObjectPtr<AActor> CoffeeLamp;
    void BuildPracticeRoute();
    void LoadCopy();
    void LoadTranslations();
    TMap<FString,TMap<FString,FString>> Translations;
    void RefreshInput();
    void RestoreCheckpoint();
    void ResetDevices();
    void SetCoffeeLit(bool bLit);
};

UCLASS()
class LASTMINUTECOURIER_API ACourierCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    ACourierCharacter();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    void SetCameraFov(float Value);
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Bag;
private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Head;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Limbs;
    float WalkCycle = 0.f;
    UPROPERTY() TObjectPtr<USpringArmComponent> Arm;
    UPROPERTY() TObjectPtr<UCameraComponent> Camera;
    void Forward(float Value);
    void Right(float Value);
    void Yaw(float Value);
    void Pitch(float Value);
    void JumpPressed();
    void JumpReleased();
};

UCLASS()
class LASTMINUTECOURIER_API ACourierController : public APlayerController
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
private:
    void Next();
    void Previous();
    void Interact();
    void SlowPressed();
    void SlowReleased();
    void PausePressed();
    void RetryPressed();
    void HelpPressed();
    void LanguagePressed();
};

UCLASS()
class LASTMINUTECOURIER_API ACourierCart : public AActor
{
    GENERATED_BODY()
public:
    ACourierCart();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> Material;
    FVector Origin = FVector::ZeroVector;
    float Clock = 0.f;
    float InitialPhase = 0.f;
    float Period = 5.f;
    float Amplitude = 420.f;
    bool bTraining = false;
    bool bSlowedOnce = false;
    void Reset();
    bool Touching(const FVector& Player) const;
};

UCLASS()
class LASTMINUTECOURIER_API ACourierHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    TSharedPtr<class SWidget> Interface;
};
