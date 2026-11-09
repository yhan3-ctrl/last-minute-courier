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

// A gold hourglass pickup (+1 Q use). Order -1 belongs to the practice yard.
struct FCourierToken
{
    TWeakObjectPtr<AActor> Actor;
    FVector Base=FVector::ZeroVector;
    FVector Home=FVector::ZeroVector;
    int32 Order=0;
    bool bTaken=false;
};

// A gold time spark: small pickups laid in lines along the routes, like coins.
// Every MotesPerUse sparks add one use of borrowed time.
struct FCourierMote
{
    TWeakObjectPtr<AActor> Actor;
    FVector Base=FVector::ZeroVector;
    int32 Order=0;
    bool bTaken=false;
};

enum class ECourierObstacleKind : uint8 {Shutter,Sweeper,Spring,Beam};

// Purely visual props never block the courier, but large ones block the camera
// probe so the chase camera cannot slide inside them.
void CourierVisualCollision(class UPrimitiveComponent* Component,const FVector& Size);

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
    bool bConfirmRetry = false; // R during a delivery asks before discarding progress.
    float QDenied = 0.f;        // Brief HUD flash when Q is pressed without charge.
    int32 Charges = 3;          // Borrowed-time uses left in the current delivery.
    static constexpr float RechargeSeconds = 1.5f;
    static constexpr float InteractRadius = 300.f;
    int32 OrderCharges() const {return CurrentOrder==0?3:(CurrentOrder==1?4:5);}
    bool GateOpen() const;
    bool bNearPickup = false;
    // Golden clocks, star ratings and feel.
    static constexpr float TokenSeconds = 5.f;
    TArray<FCourierToken> Tokens;
    TArray<int32> Stars = {0,0,0};
    float SlowFx = 0.f, HitFlash = 0.f, TokenFlash = 0.f, TokenSpin = 0.f;
    int32 LastTickSecond = -1;
    float StunSeconds = 0.f;   // stagger after a cart bump: no steering
    int32 LensCondition = 3;   // the lens case survives two knocks
    int32 FailReason = 0;      // 0 time ran out, 1 lens cracked
    void Bump(const class ACourierCart* Cart);
    void DamageLens(const TCHAR* English,const TCHAR* Chinese);
    void SpawnTokens();
    bool TokenVisible(const FCourierToken& Token) const;
    int32 TokensTotal(int32 Order) const;
    int32 TokensTaken(int32 Order) const;
    void ResetTokens(int32 Order);
    void UpdateTokens(float DeltaSeconds);
    void CollectTokens(const FVector& Position);
    int32 RateDelivery() const;
    int32 TotalStars() const;
    FString RankTitle() const;
    void Celebrate(const FVector& At,int32 Count);
    void UpdateFeel(float DeltaSeconds);
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
    void RequestRetry();
    void RestartRun();
    void NextOrder();
    void SetAudioEnabled(bool bEnabled);
    FString OrderTitle() const;
    FString OrderBrief() const;
    FString OrderDestination() const;
    FString OrderOutcome() const;
    FString OrderLabel() const;
    FString OrderPackage() const;
    FString InteractionLabel() const;
    FString RouteHint() const;
    FString NavigationText() const;
    FString ProgressText() const;
    FVector NavigationTarget() const;
    float OrderBudget() const;
    float RecoverySeconds = 0.f;
    float GateAngle = 0.f;
    float NoticeSeconds = 0.f;
    void ResetDevices();
    // Parkour rewards and devices.
    TArray<FCourierMote> Motes;
    int32 MoteBank=0;                      // sparks toward the next Q use
    static constexpr int32 MotesPerUse=25;
    int32 MoteChain=0;
    float MoteChainSeconds=0.f;
    FString Popup;
    float PopupSeconds=0.f;
    UPROPERTY() TArray<TObjectPtr<class ACourierObstacle>> Obstacles;
    void ShowPopup(const FString& Text);
    void Sound(const TCHAR* Name,float Volume=.3f,float Pitch=1.f);
    void SpawnMotes();
    void UpdateMotes(float DeltaSeconds);
    void CollectMotes(const FVector& Position);
    int32 MotesTotal(int32 Order) const;
    int32 MotesTaken(int32 Order) const;
    void AwardMotes(int32 Count);
    void ResetMotes(int32 Order);
    void CloseCall(int32 Bonus);
    void KnockBack(const FVector& Push,const TCHAR* English,const TCHAR* Chinese);
    void BuildParkour();
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
    UPROPERTY() TObjectPtr<AActor> GateArm;
    UPROPERTY() TObjectPtr<UPrimitiveComponent> GateBlocker;
    UPROPERTY() TArray<TObjectPtr<AActor>> GateSign;
    void UpdateGate(float DeltaSeconds);
    TArray<FVector> Route;
    void BuildRouteGuide();
public:
    // The recommended line for an order, on real walking surfaces (floor Z).
    // Gaps between legs are where you ride a platform or jump between stones.
    TArray<TArray<FVector>> GuideLegs(int32 Order) const;
private:
    void AddRouteGuideSpan(int32 Order,const FVector& A,const FVector& B);
    void UpdateRouteGuide();
    void BuildPracticeRoute();
    void BuildDeliveryDistricts();
    void ConfigureOrder(int32 Index);
    void AcceptOrder();
    void OpenBrief();
    void UpdateCelebration();
    void LoadCopy();
    void LoadTranslations();
    TMap<FString,TMap<FString,FString>> Translations;
    FString NoticeEn,NoticeZh;
    void RefreshInput();
    void RestoreCheckpoint();
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
    // Parkour moves: Shift sprints, Ctrl slides, a jump at a ledge climbs it.
    static constexpr float WalkSpeed=520.f, SprintSpeed=820.f, SlideTime=.75f;
    bool bSprintHeld=false;
    float SlideSeconds=0.f;
    bool IsSliding() const {return SlideSeconds>0.f;}
    bool IsMantling() const {return MantleTime>=0.f;}
    void SetSprint(bool bOn);
    void CancelMoves();
    void StartSlide();
    void JumpPressed();
    bool TryMantle();
private:
    UPROPERTY() TObjectPtr<USceneComponent> Visual;
    FVector SlideDirection=FVector::ForwardVector;
    float CoyoteSeconds=0.f;
    float JumpBufferSeconds=0.f;
    float MantleTime=-1.f;
    FVector MantleFrom,MantleTo,MantleForward;
    float BaseFov=90.f;
    float FovKick=0.f;
    float Lean=0.f;
    void SprintPressed();
    void SprintReleased();
    void SlidePressed();
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
    float Amplitude = 420.f;   // unused legacy field
    float Span = 600.f;        // lane half-width: depot door to depot door
    float Direction = 1.f;     // +1 drives toward +Y, -1 toward -Y
    float NearMissCooldown = 0.f;
    bool bTraining = false;
    int32 OrderIndex = 0;
    void Reset();
    bool Touching(const FVector& Player) const;
    // Traffic: each cycle a cart leaves one depot, crosses the lane and parks in
    // the other. Q slows its clock, so it stays in the depot longer.
    float CycleSeconds() const {return Period*.5f;}
    bool OnStreet(float ClockValue) const;
    float YAt(float ClockValue) const;
    void Place();
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

// Parkour set pieces. Shutters and sweepers are blue clock-tagged devices that Q
// slows; springboards and beams are plain campus fixtures.
UCLASS()
class LASTMINUTECOURIER_API ACourierObstacle : public AActor
{
    GENERATED_BODY()
public:
    ACourierObstacle();
    virtual void Tick(float DeltaSeconds) override;
    ECourierObstacleKind Kind=ECourierObstacleKind::Shutter;
    int32 OrderIndex=0;          // -1 practice yard
    FVector Origin=FVector::ZeroVector;
    float Width=1000.f;          // shutter / beam span across the path (Y)
    float ArmLength=450.f;       // sweeper reach
    float Period=7.f;
    float InitialPhase=0.f;
    float Clock=0.f;
    float SideBefore=0.f;
    float CloseCallCooldown=0.f;
    bool bSlowable() const {return Kind==ECourierObstacleKind::Shutter || Kind==ECourierObstacleKind::Sweeper;}
    void Build();
    void Reset();
    bool Relevant() const;
    float DoorBottom(float ClockValue) const;  // shutter clearance above its floor
    bool Closing(float ClockValue) const;
    float ArmYaw(float ClockValue) const;      // sweeper: low arm heading, high arm opposite
    void Check(ACourierGameMode& Mode,class ACourierCharacter& Player,float DeltaSeconds);
    UPROPERTY() TObjectPtr<USceneComponent> Root;
    UPROPERTY() TObjectPtr<USceneComponent> Moving;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Blocker;
    UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> Tinted;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> ClockHand;
    void Pose();
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
