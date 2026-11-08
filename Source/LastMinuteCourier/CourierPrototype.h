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
    Opening, Practice, AwaitPickup, Briefing, Delivery, Delivered, Complete, Failed, Paused
};

struct FCourierOrder
{
    FString Title,TitleZh,Package,PackageZh,Destination,DestinationZh;
    FString Brief,BriefZh,Label,LabelZh,Delivered,DeliveredZh;
    float Seconds=140.f;
};

// One connected campus, three deliveries, current-session progress.
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
    int32 CurrentOrder = 0;
    int32 CompletedOrders = 0;
    int32 RouteNode = 0;
    float SessionSeconds = 0.f;
    TArray<float> DeliveryTimes = {0.f,0.f,0.f};
    TArray<FCourierOrder> Orders;
    FString Language = TEXT("en"); // Always English on a new run, independent of OS language.
    bool bHelp = false;
    bool bSlowing = false;
    bool bLockCamera = false;
    bool bTrainingJumped = false;
    bool bAudioEnabled = true;
    float SlowRemaining = 3.f;
    float Cooldown = 0.f;
    float Remaining = 60.f;
    float OrderElapsed = 0.f;
    float CollisionProtection = 0.f;
    float MouseSensitivity = .6f;
    float CameraFov = 90.f;
    FVector Checkpoint = FVector(100, 0, 120);
    FVector Pickup = FVector(2400, 0, 100);
    FVector Destination = FVector(8000, 0, 100);
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
    bool CanInteractAt(const FVector& Target) const;
    FString StatusNotice() const;
    FString ContextHint() const;
    void Notify(const TCHAR* English,const TCHAR* Chinese);
    void Continue();
    void Back();
    void Interact();
    void StartSlow();
    void EndSlow();
    void TogglePause();
    void ToggleHelp();
    void ToggleLanguage();
    void RecenterCamera();
    void Retry();
    void RestartRun();
    void NextOrder();
    void SetAudioEnabled(bool bEnabled);
    FString OrderTitle() const;
    FString OrderBrief() const;
    FString OrderDestination() const;
    FString OrderOutcome() const;
    FString OrderLabel() const;
    FString RouteHint() const;
    FString NavigationText() const;
    FString ProgressText() const;
    FVector NavigationTarget() const;
    float OrderBudget() const;
    float RecoverySeconds = 0.f;
private:
    friend class FCourierAutopilot;
    TSharedPtr<class FCourierAutopilot> Autopilot;
    UPROPERTY() TArray<TObjectPtr<class ACourierCart>> Carts;
    UPROPERTY() TObjectPtr<AActor> CoffeeLamp;
    UPROPERTY() TArray<TObjectPtr<class ACourierLift>> Platforms;
    UPROPERTY() TArray<TObjectPtr<AActor>> FinaleActors;
    UPROPERTY() TArray<TObjectPtr<AActor>> ProjectionActors;
    UPROPERTY() TObjectPtr<class UAudioComponent> Music;
    UPROPERTY() TArray<TObjectPtr<class UAudioComponent>> MusicLayers;
    UPROPERTY() TArray<TObjectPtr<AActor>> RouteGuideGroups;
    UPROPERTY() TObjectPtr<AActor> RouteBeacon;
    TArray<FVector> Route;
    void BuildRouteGuide();
    void AddRouteGuideSpan(int32 Order,const FVector& A,const FVector& B);
    void UpdateRouteGuide();
    void BuildPracticeRoute();
    void BuildDeliveryDistricts();
    void ConfigureOrder(int32 Index);
    void AcceptOrder();
    void Sound(const TCHAR* Name,float Volume=.3f);
    void UpdateCelebration();
    void LoadCopy();
    void LoadTranslations();
    TMap<FString,TMap<FString,FString>> Translations;
    FString NoticeEn,NoticeZh;
    float NoticeSeconds = 0.f;
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
    virtual void OnJumped_Implementation() override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    void SetCameraFov(float Value);
    void SetParcelStyle(int32 Order);
    void ShowPackage(bool bVisible);
    void RecenterView(float Yaw, float Pitch=-22.f);
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Bag;
private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Head;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> PackageMark;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Limbs;
    float WalkCycle = 0.f;
    float FootstepDistance = 0.f;
    float RecenterTime = -1.f;
    FRotator RecenterFrom, RecenterTo;
    UPROPERTY() TObjectPtr<class USoundBase> FootstepCue;
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
    void RecenterPressed();
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
    UPROPERTY() TObjectPtr<UStaticMeshComponent> ClockHand;
    FVector Origin = FVector::ZeroVector;
    float Clock = 0.f;
    float InitialPhase = 0.f;
    float Period = 5.f;
    float Amplitude = 420.f;
    bool bTraining = false;
    int32 OrderIndex = 0;
    void Reset();
    bool Touching(const FVector& Player) const;
};

UCLASS()
class LASTMINUTECOURIER_API ACourierLift : public AActor
{
    GENERATED_BODY()
public:
    ACourierLift();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> Material;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> ClockHand;
    FVector Origin=FVector::ZeroVector;
    FVector Travel=FVector(0,0,320);
    float Clock=0.f;
    float Period=9.f;
    int32 OrderIndex=1;
    void Reset();
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
