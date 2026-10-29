#include "CourierPrototype.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/Canvas.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Engine.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
    ACourierGameMode* Game(const UObject* Context)
    {
        return Cast<ACourierGameMode>(UGameplayStatics::GetGameMode(Context));
    }

    UMaterialInstanceDynamic* Paint(UStaticMeshComponent* Mesh, const FLinearColor& Color)
    {
        auto* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
        auto* Material = UMaterialInstanceDynamic::Create(Base, Mesh);
        Material->SetVectorParameterValue(TEXT("Color"), Color);
        Mesh->SetMaterial(0, Material);
        return Material;
    }

    AStaticMeshActor* Box(UWorld* World, const FVector& Location, const FVector& Size,
                          const FLinearColor& Color, bool bSolid = true)
    {
        auto* Actor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
        auto* Mesh = Actor->GetStaticMeshComponent();
        Mesh->SetMobility(EComponentMobility::Movable);
        Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
        Mesh->SetWorldScale3D(Size / 100.f);
        Mesh->SetCollisionEnabled(bSolid ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Paint(Mesh, Color);
        return Actor;
    }

    void Sign(UWorld* World, const FVector& Location, const FString& Text)
    {
        auto* Actor = World->SpawnActor<AActor>(Location, FRotator(0, 180, 0));
        auto* Label = NewObject<UTextRenderComponent>(Actor);
        Actor->SetRootComponent(Label);
        Label->RegisterComponent();
        Label->SetWorldLocationAndRotation(Location, FRotator(0, 180, 0));
        Label->SetText(FText::FromString(Text));
        Label->SetWorldSize(42.f);
        Label->SetHorizontalAlignment(EHTA_Center);
        Label->SetTextRenderColor(FColor(245, 235, 207));
    }

    FString Localized(const TSharedPtr<FJsonObject>& Object, const TCHAR* Language)
    {
        FString Value;
        if (Object) Object->TryGetStringField(Language, Value);
        return Value;
    }
}

ACourierGameMode::ACourierGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    DefaultPawnClass = ACourierCharacter::StaticClass();
    PlayerControllerClass = ACourierController::StaticClass();
    HUDClass = ACourierHUD::StaticClass();
}

void ACourierGameMode::BeginPlay()
{
    Super::BeginPlay();
    LoadCopy();
    BuildPracticeRoute();
    if (auto* Player = UGameplayStatics::GetPlayerCharacter(this, 0))
    {
        Player->SetActorLocation(Checkpoint);
        if (auto* Controller = UGameplayStatics::GetPlayerController(this, 0))
            Controller->SetControlRotation(FRotator(-12, 0, 0));
    }
    RefreshInput();
}

bool ACourierGameMode::IsPlaying() const
{
    return Phase == ECourierPhase::Practice || Phase == ECourierPhase::Delivery;
}

bool ACourierGameMode::CanMove() const
{
    return IsPlaying() || Phase == ECourierPhase::AwaitPickup;
}

float ACourierGameMode::DeviceRate() const
{
    return IsPlaying() ? (bSlowing ? .25f : 1.f) : 0.f;
}

FString ACourierGameMode::Copy(const TCHAR* English, const TCHAR* Chinese) const
{
    return bChinese ? FString(Chinese) : FString(English);
}

void ACourierGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!IsPlaying()) return;
    auto* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!Player) return;
    CollisionProtection = FMath::Max(0.f, CollisionProtection - DeltaSeconds);
    if (bSlowing)
    {
        SlowRemaining = FMath::Max(0.f, SlowRemaining - DeltaSeconds);
        if (SlowRemaining <= 0.f) EndSlow();
    }
    else if (Cooldown > 0.f)
    {
        Cooldown = FMath::Max(0.f, Cooldown - DeltaSeconds);
        if (Cooldown <= 0.f) SlowRemaining = 3.f;
    }
    const FVector Position = Player->GetActorLocation();
    if (Phase == ECourierPhase::Practice)
    {
        if (Tutorial == 0 && Position.X > 550.f) Tutorial = 1;
        if (Tutorial == 1 && Position.X > 1100.f) Tutorial = 2;
        if (Tutorial == 2 && Position.X > 1900.f && Carts.Num() > 0 && Carts[0]->bSlowedOnce)
            Tutorial = 3;
        if (Tutorial == 3 && FVector::Dist2D(Position, Pickup) < 240.f)
        {
            Phase = ECourierPhase::AwaitPickup;
            EndSlow();
        }
    }
    else
    {
        Remaining = FMath::Max(0.f, Remaining - DeltaSeconds);
        OrderElapsed += DeltaSeconds;
        if (Position.X > 4500.f && Checkpoint.X < 4000.f) Checkpoint = FVector(4500, 0, 120);
        if (Position.X > 6500.f && Checkpoint.X < 6000.f) Checkpoint = FVector(6500, 0, 120);
        if (Remaining <= 0.f)
        {
            EndSlow();
            Phase = ECourierPhase::Failed;
            RefreshInput();
            return;
        }
    }
    if (Position.Z < -200.f || FMath::Abs(Position.Y) > 650.f)
    {
        RestoreCheckpoint();
        return;
    }
    if (CollisionProtection <= 0.f)
    {
        for (const auto& Cart : Carts)
        {
            if (Cart && Cart->Touching(Position))
            {
                RestoreCheckpoint();
                break;
            }
        }
    }
}

void ACourierGameMode::Continue()
{
    if (Phase == ECourierPhase::Opening)
    {
        if (++Card >= CardTitles.Num())
        {
            Phase = ECourierPhase::Practice;
            Card = 0;
        }
    }
    else if (Phase == ECourierPhase::Failed || Phase == ECourierPhase::Delivered) Retry();
    else if (Phase == ECourierPhase::Paused) TogglePause();
    RefreshInput();
}

void ACourierGameMode::Back()
{
    if (Phase == ECourierPhase::Opening) Card = FMath::Max(0, Card - 1);
    else if (Phase == ECourierPhase::Paused) TogglePause();
}

void ACourierGameMode::Interact()
{
    auto* Player = Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (!Player) return;
    if (Phase == ECourierPhase::AwaitPickup && FVector::Dist2D(Player->GetActorLocation(), Pickup) < 240.f)
    {
        Phase = ECourierPhase::Delivery;
        Remaining = 140.f;
        OrderElapsed = 0.f;
        Checkpoint = FVector(2400, 0, 120);
        SlowRemaining = 3.f;
        Cooldown = 0.f;
        Player->Bag->SetVisibility(true);
    }
    else if (Phase == ECourierPhase::Delivery && FVector::Dist2D(Player->GetActorLocation(), Destination) < 240.f)
    {
        EndSlow();
        Phase = ECourierPhase::Delivered;
        Player->Bag->SetVisibility(false);
        SetCoffeeLit(true);
    }
    RefreshInput();
}

void ACourierGameMode::StartSlow()
{
    if (IsPlaying() && Cooldown <= 0.f && SlowRemaining > 0.f) bSlowing = true;
}

void ACourierGameMode::EndSlow()
{
    if (bSlowing)
    {
        bSlowing = false;
        Cooldown = 4.f;
    }
}

void ACourierGameMode::TogglePause()
{
    auto* Controller = UGameplayStatics::GetPlayerController(this, 0);
    if (!Controller || Phase == ECourierPhase::Opening) return;
    if (Phase == ECourierPhase::Paused)
    {
        Controller->SetPause(false);
        Phase = BeforePause;
        bHelp = false;
    }
    else
    {
        BeforePause = Phase;
        Phase = ECourierPhase::Paused;
        EndSlow();
        bHelp = false;
        Controller->SetPause(true);
    }
    RefreshInput();
}

void ACourierGameMode::ToggleHelp()
{
    if (Phase != ECourierPhase::Paused) TogglePause();
    if (Phase == ECourierPhase::Paused) bHelp = !bHelp;
}

void ACourierGameMode::ToggleLanguage() { bChinese = !bChinese; }

void ACourierGameMode::RestoreCheckpoint()
{
    if (auto* Player = UGameplayStatics::GetPlayerCharacter(this, 0))
    {
        Player->GetCharacterMovement()->StopMovementImmediately();
        Player->SetActorLocation(Checkpoint, false, nullptr, ETeleportType::TeleportPhysics);
        CollisionProtection = 1.f;
    }
}

void ACourierGameMode::ResetDevices()
{
    for (const auto& Cart : Carts) if (Cart) Cart->Reset();
    bSlowing = false;
    SlowRemaining = 3.f;
    Cooldown = 0.f;
}

void ACourierGameMode::Retry()
{
    if (Phase == ECourierPhase::Opening || Phase == ECourierPhase::Practice) return;
    if (Phase == ECourierPhase::Paused)
        if (auto* PC = UGameplayStatics::GetPlayerController(this, 0)) PC->SetPause(false);
    Phase = ECourierPhase::AwaitPickup;
    bHelp = false;
    ++Retries;
    Remaining = 140.f;
    OrderElapsed = 0.f;
    Checkpoint = FVector(2400, 0, 120);
    ResetDevices();
    SetCoffeeLit(false);
    RestoreCheckpoint();
    if (auto* Player = Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
        Player->Bag->SetVisibility(false);
    RefreshInput();
}

FString ACourierGameMode::InteractionPrompt() const
{
    const auto* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!Player) return FString();
    if (Phase == ECourierPhase::AwaitPickup && FVector::Dist2D(Player->GetActorLocation(), Pickup) < 240.f)
        return Copy(TEXT("E  Collect coffee box - 140 second delivery"), TEXT("E 领取咖啡保温箱 · 限时 140 秒"));
    if (Phase == ECourierPhase::Delivery && FVector::Dist2D(Player->GetActorLocation(), Destination) < 240.f)
        return Copy(TEXT("E  Deliver the coffee box"), TEXT("E 交付咖啡保温箱"));
    return FString();
}

void ACourierGameMode::RefreshInput()
{
    if (auto* Controller = UGameplayStatics::GetPlayerController(this, 0))
    {
        Controller->SetIgnoreMoveInput(false);
        Controller->SetIgnoreLookInput(false);
        Controller->bShowMouseCursor = !CanMove();
        Controller->bEnableClickEvents = !CanMove();
        if (CanMove()) Controller->SetInputMode(FInputModeGameOnly());
        else
        {
            FInputModeGameAndUI Mode;
            Mode.SetHideCursorDuringCapture(false);
            Controller->SetInputMode(Mode);
        }
        if (auto* Player = UGameplayStatics::GetPlayerCharacter(this, 0))
            if (!CanMove()) Player->GetCharacterMovement()->StopMovementImmediately();
    }
}

void ACourierGameMode::LoadCopy()
{
    // Fallback keeps the prototype usable if staged copy is missing; missing data is still a release check failure.
    CardTitles = {TEXT("Tonight: Movie Night"), TEXT("Three Borrowed Seconds"), TEXT("Three Deliveries. One Evening.")};
    CardBodies = {TEXT("The campus screening starts soon. Coffee, a lens and the final reel are still on the way. You are the last courier on shift."),
                  TEXT("Hold Q to slow devices with blue time tags. You move normally. The delivery clock keeps ticking."),
                  TEXT("Follow the package and destination markers. Collect the next order where you finish the last. You can retry the current order.")};
    CardTitlesZh = {TEXT("今晚，电影夜"), TEXT("借来的三秒"), TEXT("三份急件，一个晚上")};
    CardBodiesZh = {TEXT("钟楼广场马上开场。咖啡、镜头和胶片还在路上。你是今晚最后一位快递员。"),
                    TEXT("按住 Q，让带蓝色时间标签的装置减速；你照常行动。订单倒计时不会变慢。"),
                    TEXT("沿着包裹与目的地标记前进。送达后在同一地点领取下一单。出错可以重试当前订单。")};
    TutorialText = {TEXT("WASD to move. Move the mouse to look around. Reach the marked spot."), TEXT("Space to jump. Cross the low step."),
                    TEXT("Hold Q to slow the blue-tagged cart, then cross its lane."), TEXT("Ready! Reach the parcel desk. The clock starts when you collect the order.")};
    TutorialTextZh = {TEXT("WASD 移动，鼠标转动视角。走到标记位置。"), TEXT("按空格跳过低台阶。"),
                      TEXT("按住 Q 让蓝色标签运输车减速，趁它变慢时通过。"), TEXT("准备好了！走到取件台，领取订单后才开始计时。")};
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *(FPaths::ProjectDir() / TEXT("DesignData/game_content.json"))))
    {
        UE_LOG(LogTemp, Warning, TEXT("Courier copy file unavailable; using fallback copy."));
        return;
    }
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root) return;
    const TArray<TSharedPtr<FJsonValue>>* Cards = nullptr;
    if (Root->TryGetArrayField(TEXT("opening_cards"), Cards) && Cards->Num() == 3)
    {
        for (int32 I = 0; I < 3; ++I)
        {
            const auto Item = (*Cards)[I]->AsObject();
            if (!Item) continue;
            const auto Title = Item->GetObjectField(TEXT("title"));
            const auto Body = Item->GetObjectField(TEXT("body"));
            CardTitles[I] = Localized(Title, TEXT("en"));
            CardBodies[I] = Localized(Body, TEXT("en"));
            CardTitlesZh[I] = Localized(Title, TEXT("zh"));
            CardBodiesZh[I] = Localized(Body, TEXT("zh"));
        }
    }
}

void ACourierGameMode::SetCoffeeLit(bool bLit)
{
    if (CoffeeLamp) CoffeeLamp->SetActorHiddenInGame(!bLit);
}

void ACourierGameMode::BuildPracticeRoute()
{
    UWorld* World = GetWorld();
    const FLinearColor Ground(.17f, .22f, .24f);
    const FLinearColor Cream(.72f, .57f, .36f);
    const FLinearColor Teal(.04f, .6f, .75f);
    Box(World, FVector(4100,0,-35), FVector(8800,1150,70), Ground);
    Box(World, FVector(900,0,30), FVector(130,1150,60), Cream);
    for (float X = 1000.f; X < 8000.f; X += 500.f)
        Box(World, FVector(X,0,2), FVector(100,24,4), FLinearColor(1,.65f,.15f), false);
    Box(World, Pickup - FVector(0,0,75), FVector(220,220,50), Teal, false);
    Box(World, Destination - FVector(0,0,75), FVector(250,250,50), FLinearColor(1,.65f,.15f), false);
    Box(World, FVector(8200,0,170), FVector(200,700,340), Cream);
    Box(World, FVector(8150,0,365), FVector(350,850,50), FLinearColor(.3f,.1f,.08f));
    Sign(World, FVector(550,0,200), TEXT("PRACTICE YARD"));
    Sign(World, FVector(2400,0,210), TEXT("PARCEL DESK"));
    Sign(World, FVector(7950,0,430), TEXT("COFFEE STAND"));
    Sign(World, FVector(5900,0,200), TEXT("NEXT CHECKPOINT"));
    const float Lanes[] = {1550.f, 3300.f, 3800.f, 5300.f, 5900.f, 7200.f};
    for (int32 I = 0; I < 6; ++I)
    {
        auto* Cart = World->SpawnActor<ACourierCart>(FVector(Lanes[I],0,55), FRotator::ZeroRotator);
        Cart->Origin = FVector(Lanes[I],0,55);
        Cart->bTraining = I == 0;
        Cart->Period = 4.5f + (I % 3) * .8f;
        Cart->InitialPhase = I * 1.3f;
        Cart->Reset();
        Carts.Add(Cart);
        Box(World, FVector(Lanes[I],0,3), FVector(250,1100,6), FLinearColor(.12f,.32f,.4f), false);
    }
    // Low buildings provide scale without blocking the prototype route.
    for (int32 I = 0; I < 10; ++I)
    {
        const float Side = I % 2 ? 1.f : -1.f;
        Box(World, FVector(I*800.f+400,Side*1000.f,250), FVector(480,400,500), Cream);
    }
    auto* Sun = World->SpawnActor<ADirectionalLight>(FVector(0,0,1200), FRotator(-48,-25,0));
    Sun->SetActorRotation(FRotator(-48,-25,0));
    auto* Lamp = World->SpawnActor<APointLight>(FVector(7800,0,280), FRotator::ZeroRotator);
    if (auto* Light = Lamp->FindComponentByClass<UPointLightComponent>())
    {
        Light->SetIntensity(12000.f);
        Light->SetLightColor(FLinearColor(1,.6f,.2f));
    }
    CoffeeLamp = Lamp;
    SetCoffeeLit(false);
}

ACourierCharacter::ACourierCharacter()
{
    GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0, 600, 0);
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->JumpZVelocity = 500.f;
    GetCharacterMovement()->AirControl = .35f;
    Arm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
    Arm->SetupAttachment(RootComponent);
    Arm->TargetArmLength = 450.f;
    Arm->SocketOffset = FVector(0,0,90);
    Arm->bUsePawnControlRotation = true;
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(Arm);
    auto* Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GreyboxCourier"));
    Body->SetupAttachment(RootComponent);
    Body->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
    Body->SetRelativeScale3D(FVector(.5f,.5f,1.4f));
    Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Bag = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Parcel"));
    Bag->SetupAttachment(RootComponent);
    Bag->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
    Bag->SetRelativeLocation(FVector(-35,0,15));
    Bag->SetRelativeScale3D(FVector(.35f,.5f,.5f));
    Bag->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Bag->SetVisibility(false);
}

void ACourierCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    Input->BindAxis(TEXT("Forward"), this, &ACourierCharacter::Forward);
    Input->BindAxis(TEXT("Right"), this, &ACourierCharacter::Right);
    Input->BindAxis(TEXT("LookYaw"), this, &ACourierCharacter::Yaw);
    Input->BindAxis(TEXT("LookPitch"), this, &ACourierCharacter::Pitch);
    Input->BindAction(TEXT("Jump"), IE_Pressed, this, &ACourierCharacter::JumpPressed);
    Input->BindAction(TEXT("Jump"), IE_Released, this, &ACourierCharacter::JumpReleased);
}

void ACourierCharacter::Forward(float Value)
{
    if (Controller && Game(this) && Game(this)->CanMove())
        AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::X),Value);
}
void ACourierCharacter::Right(float Value)
{
    if (Controller && Game(this) && Game(this)->CanMove())
        AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),Value);
}
void ACourierCharacter::Yaw(float Value) { if (Game(this) && Game(this)->CanMove()) AddControllerYawInput(Value); }
void ACourierCharacter::Pitch(float Value) { if (Game(this) && Game(this)->CanMove()) AddControllerPitchInput(Value); }
void ACourierCharacter::JumpPressed() { if (Game(this) && Game(this)->CanMove()) Jump(); }
void ACourierCharacter::JumpReleased() { StopJumping(); }

void ACourierController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindAction(TEXT("Continue"),IE_Pressed,this,&ACourierController::Next).bExecuteWhenPaused=true;
    InputComponent->BindAction(TEXT("Back"),IE_Pressed,this,&ACourierController::Previous).bExecuteWhenPaused=true;
    InputComponent->BindAction(TEXT("Pause"),IE_Pressed,this,&ACourierController::PausePressed).bExecuteWhenPaused=true;
    InputComponent->BindAction(TEXT("Retry"),IE_Pressed,this,&ACourierController::RetryPressed).bExecuteWhenPaused=true;
    InputComponent->BindAction(TEXT("Help"),IE_Pressed,this,&ACourierController::HelpPressed).bExecuteWhenPaused=true;
    InputComponent->BindAction(TEXT("Language"),IE_Pressed,this,&ACourierController::LanguagePressed).bExecuteWhenPaused=true;
    InputComponent->BindAction(TEXT("Interact"),IE_Pressed,this,&ACourierController::Interact);
    InputComponent->BindAction(TEXT("Slow"),IE_Pressed,this,&ACourierController::SlowPressed);
    InputComponent->BindAction(TEXT("Slow"),IE_Released,this,&ACourierController::SlowReleased).bExecuteWhenPaused=true;
}
void ACourierController::Next() { if(Game(this))Game(this)->Continue(); }
void ACourierController::Previous() { if(Game(this))Game(this)->Back(); }
void ACourierController::Interact() { if(Game(this))Game(this)->Interact(); }
void ACourierController::SlowPressed() { if(Game(this))Game(this)->StartSlow(); }
void ACourierController::SlowReleased() { if(Game(this))Game(this)->EndSlow(); }
void ACourierController::PausePressed() { if(Game(this))Game(this)->TogglePause(); }
void ACourierController::RetryPressed() { if(Game(this))Game(this)->Retry(); }
void ACourierController::HelpPressed() { if(Game(this))Game(this)->ToggleHelp(); }
void ACourierController::LanguagePressed() { if(Game(this))Game(this)->ToggleLanguage(); }

ACourierCart::ACourierCart()
{
    PrimaryActorTick.bCanEverTick=true;
    Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cart"));
    SetRootComponent(Mesh);
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
    Mesh->SetRelativeScale3D(FVector(1.7f,2.2f,1.1f));
    // Soft hazard detection is explicit; avoiding physics impulses keeps retries predictable.
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void ACourierCart::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* Mode=Game(this);
    if(!Mode) return;
    const bool bEnabled=bTraining ? Mode->Phase==ECourierPhase::Practice : Mode->Phase==ECourierPhase::Delivery;
    if(!Material) Material=Paint(Mesh,FLinearColor(.02f,.42f,.75f));
    if(!bEnabled) return;
    Clock+=DeltaSeconds*Mode->DeviceRate();
    if(Mode->bSlowing) bSlowedOnce=true;
    SetActorLocation(Origin+FVector(0,FMath::Sin(Clock*2.f*PI/Period+InitialPhase)*Amplitude,0));
    Material->SetVectorParameterValue(TEXT("Color"),Mode->bSlowing?FLinearColor(.15f,.95f,1.f):FLinearColor(.02f,.42f,.75f));
}
void ACourierCart::Reset()
{
    Clock=0.f; bSlowedOnce=false;
    SetActorLocation(Origin+FVector(0,FMath::Sin(InitialPhase)*Amplitude,0));
}
bool ACourierCart::Touching(const FVector& Player) const
{
    const auto* Mode=Game(this);
    if(!Mode || (bTraining ? Mode->Phase!=ECourierPhase::Practice : Mode->Phase!=ECourierPhase::Delivery)) return false;
    const FVector D=Player-GetActorLocation();
    return FMath::Abs(D.X)<112.f && FMath::Abs(D.Y)<137.f && D.Z<132.f && D.Z>-120.f;
}

void ACourierHUD::Rect(float X,float Y,float W,float H,FLinearColor Color)
{
    DrawRect(Color,OX+X*S,OY+Y*S,W*S,H*S);
}
void ACourierHUD::Label(const FString& Text,float X,float Y,float Size,FLinearColor Color)
{
    DrawText(Text,Color,OX+X*S,OY+Y*S,GEngine->GetMediumFont(),Size*S/16.f,false);
}
void ACourierHUD::Paragraph(const FString& Text,float X,float Y,float Width,float Size)
{
    // Greedy character wrapping also handles copy without spaces; font coverage requires in-engine visual QA.
    FString Line;
    float LineY=Y;
    for(TCHAR Character : Text)
    {
        FString Next=Line; Next.AppendChar(Character);
        float W=0,H=0;
        GetTextSize(Next,W,H,GEngine->GetMediumFont(),Size*S/16.f);
        if(Character=='\n' || (W>Width*S && !Line.IsEmpty()))
        {
            Label(Line,X,LineY,Size,FLinearColor(.9f,.91f,.9f));
            LineY+=Size*1.5f;
            Line.Empty();
            if(Character!='\n') Line.AppendChar(Character);
        }
        else Line=Next;
    }
    if(!Line.IsEmpty()) Label(Line,X,LineY,Size,FLinearColor(.9f,.91f,.9f));
}
void ACourierHUD::Button(FName Id,const FString& Text,float X,float Y,float Width)
{
    Rect(X,Y,Width,48,FLinearColor(.08f,.48f,.54f));
    Label(Text,X+18,Y+12,19,FLinearColor::White);
    AddHitBox(FVector2D(OX+X*S,OY+Y*S),FVector2D(Width*S,48*S),Id,true,0);
}
void ACourierHUD::DrawHUD()
{
    Super::DrawHUD();
    auto* Mode=Game(this);
    if(!Canvas || !Mode) return;
    S=FMath::Min(Canvas->SizeX/1280.f,Canvas->SizeY/720.f);
    OX=(Canvas->SizeX-1280.f*S)*.5f;
    OY=(Canvas->SizeY-720.f*S)*.5f;
    const FLinearColor Gold(1.f,.72f,.27f);
    const FLinearColor White(.96f,.94f,.88f);
    auto Copy=[Mode](const TCHAR* En,const TCHAR* Zh){return Mode->Copy(En,Zh);};
    if(Mode->Phase==ECourierPhase::Opening || Mode->Phase==ECourierPhase::Paused ||
       Mode->Phase==ECourierPhase::Delivered || Mode->Phase==ECourierPhase::Failed)
    {
        DrawRect(FLinearColor(.025f,.035f,.05f,.92f),0,0,Canvas->SizeX,Canvas->SizeY);
        Label(TEXT("LAST MINUTE COURIER"),150,110,18,Gold);
        if(Mode->Phase==ECourierPhase::Opening)
        {
            const int32 I=FMath::Clamp(Mode->Card,0,2);
            Label((Mode->bChinese?Mode->CardTitlesZh:Mode->CardTitles)[I],150,170,38,White);
            Paragraph((Mode->bChinese?Mode->CardBodiesZh:Mode->CardBodies)[I],150,245,900,25);
            Label(FString::Printf(TEXT("%d / 3"),I+1),150,480,18,Gold);
            Button(TEXT("Next"),I==2?Copy(TEXT("Enter Practice Yard"),TEXT("进入练习区")):Copy(TEXT("Continue"),TEXT("继续")),150,530,310);
            if(I>0) Button(TEXT("Back"),Copy(TEXT("Back"),TEXT("返回")),490,530,180);
            Label(Copy(TEXT("Enter: continue  |  Backspace: back  |  L: language"),TEXT("Enter 继续 · Backspace 返回 · L 切换语言")),150,620,18,White);
        }
        else if(Mode->Phase==ECourierPhase::Paused)
        {
            Label(Copy(TEXT("Paused"),TEXT("已暂停")),150,170,38,White);
            if(Mode->bHelp)
            {
                Paragraph(Copy(TEXT("Movie night needs three deliveries. Hold Q to slow blue-tagged devices for up to 3 seconds; recharge takes 4 seconds. Your delivery clock keeps ticking. WASD moves, mouse looks, Space jumps, E collects and delivers. This prototype contains only the first delivery. Progress lasts for this session."),TEXT("电影夜还缺三份急件。按住 Q 让蓝色标签装置减速，最长 3 秒，结束后充能 4 秒。订单倒计时不会变慢。WASD 移动，鼠标转向，空格跳跃，E 接单和交付。此原型仅包含第一单。进度在本次运行中保留。")),150,240,930,23);
            }
            else Paragraph(Copy(TEXT("Take your time. The clock and devices are paused."),TEXT("慢慢来，倒计时和装置都已暂停。")),150,250,900,25);
            Button(TEXT("Next"),Copy(TEXT("Resume"),TEXT("继续游戏")),150,530,220);
            Button(TEXT("Help"),Copy(TEXT("Story & Controls"),TEXT("故事与操作")),400,530,260);
            Button(TEXT("Retry"),Copy(TEXT("Restart order"),TEXT("重试本单")),690,530,240);
        }
        else
        {
            const bool bSuccess=Mode->Phase==ECourierPhase::Delivered;
            Label(bSuccess?Copy(TEXT("Coffee is ready!"),TEXT("咖啡准备好了！")):Copy(TEXT("Almost! Try again."),TEXT("差一点，再送一次。")),150,170,38,White);
            Paragraph(bSuccess?Copy(TEXT("The coffee stand is lit. First-delivery prototype complete. The projector and final reel routes are still in development."),TEXT("咖啡摊亮起来了。第一单原型流程完成；镜头和胶片路线仍待制作。")):Copy(TEXT("Retry this order. Your next clock starts when you collect the package."),TEXT("重试这一单，再次领取包裹后才开始计时。")),150,250,900,25);
            if(bSuccess) Label(FString::Printf(TEXT("%.1f seconds | %d retries"),Mode->OrderElapsed,Mode->Retries),150,440,22,Gold);
            Button(TEXT("Retry"),Copy(TEXT("Play this order again"),TEXT("重新配送这一单")),150,530,330);
        }
        return;
    }
    Rect(25,25,420,80,FLinearColor(.025f,.035f,.05f,.85f));
    Label(Copy(TEXT("FIRST DELIVERY PROTOTYPE"),TEXT("第一单原型")),45,40,17,Gold);
    Label(Mode->Phase==ECourierPhase::Delivery?Copy(TEXT("Coffee Stand"),TEXT("目的地：咖啡摊")):Copy(TEXT("Practice Yard / Parcel Desk"),TEXT("练习区 / 取件台")),45,70,22,White);
    if(Mode->Phase==ECourierPhase::Delivery)
    {
        const int32 Seconds=FMath::CeilToInt(Mode->Remaining);
        Label(FString::Printf(TEXT("%02d:%02d"),Seconds/60,Seconds%60),1100,35,36,Gold);
        const auto* Player=UGameplayStatics::GetPlayerCharacter(this,0);
        if(Player) Label(FString::Printf(TEXT("%.0f m"),FVector::Dist2D(Player->GetActorLocation(),Mode->Destination)/100.f),1100,82,20,White);
    }
    Rect(30,640,240,12,FLinearColor(.12f,.16f,.18f));
    const float Fraction=Mode->Cooldown>0.f ? 1.f-Mode->Cooldown/4.f : Mode->SlowRemaining/3.f;
    Rect(30,640,240*FMath::Clamp(Fraction,0.f,1.f),12,FLinearColor(.1f,.8f,.9f));
    Label(Mode->Cooldown>0.f?Copy(TEXT("RECHARGING"),TEXT("充能中")):Copy(TEXT("READY - Hold Q"),TEXT("就绪 · 按住 Q")),30,665,18,White);
    Label(Copy(TEXT("Esc: pause / help"),TEXT("Esc 暂停 / 帮助")),1000,665,18,White);
    if(Mode->Phase==ECourierPhase::Practice)
    {
        Rect(240,535,800,75,FLinearColor(.025f,.035f,.05f,.85f));
        Paragraph((Mode->bChinese?Mode->TutorialTextZh:Mode->TutorialText)[FMath::Clamp(Mode->Tutorial,0,3)],265,550,750,20);
    }
    const FString Prompt=Mode->InteractionPrompt();
    if(!Prompt.IsEmpty())
    {
        Rect(250,450,800,60,FLinearColor(.025f,.035f,.05f,.9f));
        Label(Prompt,275,466,22,Gold);
    }
}

void ACourierHUD::NotifyHitBoxClick(FName BoxName)
{
    Super::NotifyHitBoxClick(BoxName);
    auto* Mode=Game(this);
    if(!Mode) return;
    if(BoxName==TEXT("Next")) Mode->Continue();
    else if(BoxName==TEXT("Back")) Mode->Back();
    else if(BoxName==TEXT("Retry")) Mode->Retry();
    else if(BoxName==TEXT("Help")) Mode->ToggleHelp();
}
