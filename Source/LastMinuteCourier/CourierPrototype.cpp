#include "CourierPrototype.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/DirectionalLightComponent.h"
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
#include "InputCoreTypes.h"
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
        auto* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CourierMatte.M_CourierMatte"));
        if (!Base) Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
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

    FLinearColor Swatch(const TCHAR* Hex)
    {
        return FLinearColor::FromSRGBColor(FColor::FromHex(Hex));
    }

    AStaticMeshActor* Shape(UWorld* World,const TCHAR* Path,const FVector& Location,const FVector& Size,
                           const FLinearColor& Color)
    {
        auto* Actor=Box(World,Location,Size,Color,false);
        Actor->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,Path));
        return Actor;
    }

    void Tree(UWorld* World,float X,float Y,float Height=430.f)
    {
        Shape(World,TEXT("/Engine/BasicShapes/Cylinder.Cylinder"),FVector(X,Y,Height*.3f),FVector(45,45,Height*.6f),Swatch(TEXT("81634B")));
        Shape(World,TEXT("/Engine/BasicShapes/Sphere.Sphere"),FVector(X,Y,Height*.76f),FVector(290,270,Height*.72f),Swatch(TEXT("63856A")));
        Shape(World,TEXT("/Engine/BasicShapes/Sphere.Sphere"),FVector(X-70,Y+35,Height*.7f),FVector(190,230,Height*.55f),Swatch(TEXT("8B9E75")));
    }

    void Bench(UWorld* World,float X,float Y)
    {
        const auto Wood=Swatch(TEXT("B17B50"));
        Box(World,FVector(X,Y,55),FVector(190,65,14),Wood,false);
        Box(World,FVector(X,Y+28,94),FVector(190,12,66),Wood,false);
        for(float DX:{-65.f,65.f})Box(World,FVector(X+DX,Y,26),FVector(12,48,50),Swatch(TEXT("455C57")),false);
    }

    void Sign(UWorld* World, const FVector& Location, const FString& Text)
    {
        // Keep cream lettering readable against the sky and pale buildings.
        Box(World,Location+FVector(6,0,8),FVector(8,Text.Len()*13.f+30.f,46),Swatch(TEXT("243D43")),false);
        auto* Actor = World->SpawnActor<AActor>(Location, FRotator(0, 180, 0));
        auto* Label = NewObject<UTextRenderComponent>(Actor);
        Actor->SetRootComponent(Label);
        Label->RegisterComponent();
        Label->SetWorldLocationAndRotation(Location, FRotator(0, 180, 0));
        Label->SetText(FText::FromString(Text));
        Label->SetTextMaterial(LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/EngineMaterials/UnlitText.UnlitText")));
        Label->SetWorldSize(22.f);
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
    LoadTranslations();
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
    if(Language!=TEXT("en"))
        if(const auto* Entry=Translations.Find(English))
            if(const auto* Value=Entry->Find(Language)) return *Value;
    if(Language==TEXT("zh")) return FString(Chinese);
    return FString(English);
}

const TArray<FString>& ACourierGameMode::Languages()
{
    static const TArray<FString> Codes={TEXT("en"),TEXT("zh"),TEXT("ja"),TEXT("ko"),TEXT("es"),TEXT("fr"),TEXT("de")};
    return Codes;
}

FString ACourierGameMode::LanguageName(const FString& Code)
{
    static const TMap<FString,FString> Names={{TEXT("en"),TEXT("English")},{TEXT("zh"),TEXT("简体中文")},
        {TEXT("ja"),TEXT("日本語")},{TEXT("ko"),TEXT("한국어")},{TEXT("es"),TEXT("Español")},
        {TEXT("fr"),TEXT("Français")},{TEXT("de"),TEXT("Deutsch")}};
    const auto* Name=Names.Find(Code);
    return Name?*Name:TEXT("English");
}

void ACourierGameMode::SetLanguage(const FString& Code)
{
    if(Languages().Contains(Code))Language=Code;
}

bool ACourierGameMode::UsesCjkFont() const
{
    return Language==TEXT("zh") || Language==TEXT("ja") || Language==TEXT("ko");
}

void ACourierGameMode::LoadTranslations()
{
    FString Json;
    if(!FFileHelper::LoadFileToString(Json,*(FPaths::ProjectDir()/TEXT("DesignData/localization.json"))))
    {
        UE_LOG(LogTemp,Warning,TEXT("Courier translations unavailable; English fallback remains usable."));
        return;
    }
    TSharedPtr<FJsonObject> Root;
    if(!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json),Root) || !Root)return;
    const TSharedPtr<FJsonObject>* Strings=nullptr;
    if(!Root->TryGetObjectField(TEXT("strings"),Strings))return;
    for(const auto& Pair:(*Strings)->Values)
    {
        const auto Entry=Pair.Value->AsObject();
        if(!Entry)continue;
        auto& Locales=Translations.FindOrAdd(FString(*Pair.Key));
        for(const auto& Locale:Entry->Values)Locales.Add(FString(*Locale.Key),Locale.Value->AsString());
    }
}

void ACourierGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    // AwaitPickup still allows movement, so falling must recover in that state too.
    if (!CanMove()) return;
    auto* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!Player) return;
    CollisionProtection = FMath::Max(0.f, CollisionProtection - DeltaSeconds);
    NoticeSeconds=FMath::Max(0.f,NoticeSeconds-DeltaSeconds);
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
        if (Tutorial == 0 && Position.X > 550.f)
        {
            Tutorial = 1;
            Checkpoint = FVector(600, 0, 120);
        }
        if (Tutorial == 1 && Position.X > 1100.f)
        {
            if(bTrainingJumped)
            {
                Tutorial = 2;
                Checkpoint = FVector(1200, 0, 120);
            }
            else
            {
                Notify(TEXT("Try a jump here: press Space, then cross the step."),TEXT("在这里试着跳跃：按空格，再越过台阶。"));
                RestoreCheckpoint();
                return;
            }
        }
        if (Tutorial == 2 && Position.X > 1900.f)
        {
            if(Carts.Num()>0 && Carts[0]->bSlowedOnce)Tutorial=3;
            else
            {
                Notify(TEXT("Use Q near the blue cart before crossing its lane."),TEXT("先在蓝色运输车附近按住 Q，再通过车道。"));
                RestoreCheckpoint();
                return;
            }
        }
        if (Tutorial == 3 && FVector::Dist2D(Position, Pickup) < 240.f)
        {
            Phase = ECourierPhase::AwaitPickup;
            Checkpoint = FVector(2400, 0, 120);
            EndSlow();
        }
    }
    else if (Phase == ECourierPhase::Delivery)
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
        Notify(TEXT("Back on the path. Your checkpoint is safe."),TEXT("回到路上了。已恢复到检查点。"));
        RestoreCheckpoint();
        return;
    }
    if (CollisionProtection <= 0.f)
    {
        for (const auto& Cart : Carts)
        {
            if (Cart && Cart->Touching(Position))
            {
                Notify(TEXT("Cart ahead! Back at your checkpoint. Wait for a gap or hold Q."),TEXT("注意运输车！已回到检查点，等待空隙或按住 Q。"));
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
    if (Phase == ECourierPhase::AwaitPickup && CanInteractAt(Pickup))
    {
        Phase = ECourierPhase::Delivery;
        Remaining = 140.f;
        OrderElapsed = 0.f;
        Checkpoint = FVector(2400, 0, 120);
        SlowRemaining = 3.f;
        Cooldown = 0.f;
        Player->Bag->SetVisibility(true);
    }
    else if (Phase == ECourierPhase::Delivery && CanInteractAt(Destination))
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
        // A release during pause should take effect on resume, without consuming charge while paused.
        if (bSlowing && !Controller->IsInputKeyDown(EKeys::Q)) EndSlow();
        bHelp = false;
    }
    else
    {
        BeforePause = Phase;
        Phase = ECourierPhase::Paused;
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

void ACourierGameMode::ToggleLanguage()
{
    const auto& Codes=Languages();
    SetLanguage(Codes[(Codes.Find(Language)+1)%Codes.Num()]);
}

void ACourierGameMode::RecenterCamera()
{
    if(!CanMove() && Phase!=ECourierPhase::Paused)return;
    if(auto* PC=UGameplayStatics::GetPlayerController(this,0))
    {
        // This preview's route runs along +X. Recenter is explicit, including
        // when direction lock is enabled; never turn the camera automatically.
        PC->SetControlRotation(FRotator(-12.f,0.f,0.f));
    }
}

void ACourierGameMode::RestoreCheckpoint()
{
    if (auto* Player = UGameplayStatics::GetPlayerCharacter(this, 0))
    {
        Player->GetCharacterMovement()->StopMovementImmediately();
        Player->StopJumping();
        Player->ConsumeMovementInputVector();
        Player->SetActorLocation(Checkpoint, false, nullptr, ETeleportType::TeleportPhysics);
        if (auto* PC=UGameplayStatics::GetPlayerController(this,0); PC && PC->PlayerCameraManager)
            PC->PlayerCameraManager->StartCameraFade(1.f,0.f,.3f,FLinearColor::Black,false,false);
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
    if (Phase == ECourierPhase::Opening || Phase == ECourierPhase::Practice ||
        (Phase==ECourierPhase::Paused && BeforePause==ECourierPhase::Practice)) return;
    if (Phase == ECourierPhase::Paused)
        if (auto* PC = UGameplayStatics::GetPlayerController(this, 0)) PC->SetPause(false);
    Phase = ECourierPhase::AwaitPickup;
    bHelp = false;
    NoticeSeconds=0;
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
    if (Phase == ECourierPhase::AwaitPickup && CanInteractAt(Pickup))
        return Copy(TEXT("E  Collect coffee box - 140 second delivery"), TEXT("E 领取咖啡保温箱 · 限时 140 秒"));
    if (Phase == ECourierPhase::Delivery && CanInteractAt(Destination))
        return Copy(TEXT("E  Deliver the coffee box"), TEXT("E 交付咖啡保温箱"));
    return FString();
}

bool ACourierGameMode::CanInteractAt(const FVector& Target) const
{
    const auto* Player=UGameplayStatics::GetPlayerCharacter(this,0);
    return Player && Player->GetCharacterMovement()->IsMovingOnGround() &&
        FVector::Dist2D(Player->GetActorLocation(),Target)<240.f &&
        FMath::Abs(Player->GetActorLocation().Z-Target.Z)<110.f;
}

void ACourierGameMode::Notify(const TCHAR* English,const TCHAR* Chinese)
{
    NoticeEn=English;NoticeZh=Chinese;NoticeSeconds=3.5f;
}

FString ACourierGameMode::StatusNotice() const
{
    return NoticeSeconds>0?Copy(*NoticeEn,*NoticeZh):FString();
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
    const auto Cream=Swatch(TEXT("E5D7BB"));
    const auto Ink=Swatch(TEXT("354F53"));
    const auto Orange=Swatch(TEXT("C86C43"));
    const auto Teal=Swatch(TEXT("438C95"));
    const auto Grass=Swatch(TEXT("8FA17B"));
    // The walking surface is broad and flat; side scenery stays beyond the camera corridor.
    Box(World,FVector(4100,0,-85),FVector(16000,11000,70),Grass,false);
    Box(World,FVector(4100,0,-35),FVector(8800,1150,70),Swatch(TEXT("C5BBA4")));
    Box(World,FVector(900,0,30),FVector(130,1150,60),Cream);
    for(float Side:{-1.f,1.f})
    {
        Box(World,FVector(4100,Side*565,12),FVector(8800,24,24),Cream);
        Box(World,FVector(4100,Side*590,-8),FVector(8800,28,16),Ink,false);
    }
    // Quiet, widely spaced route dashes avoid high-frequency floor patterns.
    for(float X=350;X<8000;X+=650)
        Box(World,FVector(X,0,2),FVector(90,22,4),Orange,false);

    // Parcel depot sits to the side so the route and the next landmark stay visible.
    Box(World,FVector(2400,-400,66),FVector(270,120,132),Teal);
    Box(World,FVector(2400,-400,145),FVector(300,150,25),Cream,false);
    Box(World,FVector(2400,-400,290),FVector(320,175,28),Orange,false);
    for(float X:{2270.f,2530.f})Box(World,FVector(X,-460,210),FVector(16,16,170),Ink,false);
    for(float X:{2340.f,2420.f})Box(World,FVector(X,-400,180),FVector(60,60,48),Swatch(TEXT("D6AB73")),false);
    Sign(World,FVector(2400,-335,330),TEXT("PARCEL DESK"));
    Box(World,Pickup-FVector(0,0,95),FVector(220,230,10),Teal,false);

    // A cafe pavilion gives the first destination a clear silhouette and warm palette.
    Box(World,FVector(8320,0,150),FVector(240,680,300),Cream);
    Box(World,FVector(8170,0,95),FVector(65,700,190),Teal);
    Box(World,FVector(8190,0,320),FVector(450,820,40),Orange,false);
    for(int I=0;I<7;++I)
        Box(World,FVector(7960,-350+I*116,310),FVector(24,56,30),Cream,false);
    for(float Y:{-365.f,365.f})Box(World,FVector(8000,Y,150),FVector(20,20,300),Ink,false);
    Box(World,FVector(8140,0,228),FVector(12,500,88),Cream,false);
    Sign(World,FVector(8126,0,252),TEXT("COFFEE / MOVIE NIGHT"));
    Box(World,Destination-FVector(0,0,95),FVector(250,250,10),Orange,false);
    Bench(World,7760,-850);
    Tree(World,8650,900,520);

    Sign(World,FVector(520,-420,185),TEXT("PRACTICE YARD"));
    // Marker panels sit beside the path, never in the middle of the camera view.
    for(float X:{4500.f,6500.f})
    {
        Box(World,FVector(X,445,75),FVector(100,16,150),Teal,false);
        Sign(World,FVector(X-3,425,150),TEXT("CHECKPOINT"));
    }
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
        Box(World,FVector(Lanes[I],0,3),FVector(250,1090,6),Swatch(TEXT("829C99")),false);
        for(float Side:{-1.f,1.f})
        {
            Box(World,FVector(Lanes[I],Side*505,17),FVector(240,26,28),Teal,false);
            Box(World,FVector(Lanes[I],Side*520,85),FVector(18,18,130),Ink,false);
            Shape(World,TEXT("/Engine/BasicShapes/Sphere.Sphere"),FVector(Lanes[I],Side*520,165),FVector(35),Teal);
        }
    }
    // A restrained campus kit: plaster, brick, recessed windows, trees and seating.
    for(int32 I=0;I<8;++I)
    {
        const float Side=I%2?1.f:-1.f;
        const float X=I*1020.f+250;
        const float Y=Side*1500;
        const float Height=I%3==0?610.f:470.f;
        Box(World,FVector(X,Y,Height*.5f-20),FVector(640,570,Height),I%2?Cream:Swatch(TEXT("BB886F")));
        Box(World,FVector(X,Y,Height-6),FVector(700,620,35),Ink,false);
        Box(World,FVector(X,Y-Side*292,58),FVector(650,20,115),Swatch(TEXT("C4B399")),false);
        for(float DX:{-210.f,0.f,210.f})
        {
            Box(World,FVector(X+DX,Y-Side*293,Height*.58f),FVector(105,18,150),Ink,false);
            Box(World,FVector(X+DX,Y-Side*305,Height*.58f),FVector(75,10,115),Swatch(TEXT("9DBCB5")),false);
        }
        Tree(World,X+300,Side*875,400+I%3*55);
        if(I%2==0)Bench(World,X-120,Side*770);
    }
    // The cinema clocktower is a distant orientation landmark, outside the playable path.
    Box(World,FVector(9800,1800,550),FVector(430,430,1100),Cream,false);
    Box(World,FVector(9800,1800,1120),FVector(520,520,55),Orange,false);
    Shape(World,TEXT("/Engine/BasicShapes/Cone.Cone"),FVector(9800,1800,1300),FVector(550,550,330),Ink);
    auto* Clock=Shape(World,TEXT("/Engine/BasicShapes/Cylinder.Cylinder"),FVector(9578,1800,930),FVector(190,190,12),Cream);
    Clock->SetActorRotation(FRotator(90,0,0));
    Box(World,FVector(9568,1800,957),FVector(15,12,75),Ink,false);
    Box(World,FVector(9566,1827,930),FVector(15,75,12),Ink,false);

    auto* Sky=Shape(World,TEXT("/Engine/BasicShapes/Sphere.Sphere"),FVector(4000,0,0),FVector(100000),Swatch(TEXT("A9C9CD")));
    auto* SkyMesh=Sky->GetStaticMeshComponent();
    auto* SkyMaterial=UMaterialInstanceDynamic::Create(LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Materials/M_CourierSky.M_CourierSky")),SkyMesh);
    if(SkyMaterial)
    {
        SkyMaterial->SetVectorParameterValue(TEXT("Color"),Swatch(TEXT("A9C9CD")));
        SkyMesh->SetMaterial(0,SkyMaterial);
    }
    SkyMesh->SetCastShadow(false);
    auto* Sun = World->SpawnActor<ADirectionalLight>(FVector(0,0,1200), FRotator(-48,-25,0));
    if (auto* Light = Sun->FindComponentByClass<UDirectionalLightComponent>())
    {
        Light->SetIntensity(2.5f);
        Light->SetLightColor(Swatch(TEXT("FFF0D4")));
        Light->SetForwardShadingPriority(1);
    }
    auto* Fill = World->SpawnActor<ADirectionalLight>(FVector(0,0,1200), FRotator(-35,150,0));
    if (auto* Light = Fill->FindComponentByClass<UDirectionalLightComponent>())
    {
        Light->SetIntensity(1.1f);
        Light->SetLightColor(Swatch(TEXT("C7DCE2")));
        Light->SetForwardShadingPriority(0);
        Light->SetCastShadows(false);
    }
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
    PrimaryActorTick.bCanEverTick=true;
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
    Camera->SetFieldOfView(90.f);
    Arm->bEnableCameraLag = false;
    Arm->bEnableCameraRotationLag = false;
    Arm->ProbeSize = 18.f;
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GreyboxCourier"));
    Body->SetupAttachment(RootComponent);
    Body->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
    Body->SetRelativeLocation(FVector(0,0,5));
    Body->SetRelativeScale3D(FVector(.5f,.44f,.72f));
    Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Head = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CourierHead"));
    Head->SetupAttachment(RootComponent);
    Head->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")));
    Head->SetRelativeLocation(FVector(0,0,62));
    Head->SetRelativeScale3D(FVector(.42f));
    Head->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Bag = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Parcel"));
    Bag->SetupAttachment(RootComponent);
    Bag->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
    Bag->SetRelativeLocation(FVector(-35,0,15));
    Bag->SetRelativeScale3D(FVector(.35f,.5f,.5f));
    Bag->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Bag->SetVisibility(false);
}

void ACourierCharacter::BeginPlay()
{
    Super::BeginPlay();
    Paint(Body,Swatch(TEXT("D0794F")));
    Paint(Head,Swatch(TEXT("D9AE8A")));
    Paint(Bag,Swatch(TEXT("438C95")));
    const auto Part=[this](const FVector& Location,const FVector& Size,const FLinearColor& Color,const TCHAR* MeshPath)
    {
        auto* Mesh=NewObject<UStaticMeshComponent>(this);
        Mesh->SetupAttachment(RootComponent);
        Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,MeshPath));
        Mesh->SetRelativeLocation(Location);
        Mesh->SetRelativeScale3D(Size/100.f);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Mesh->RegisterComponent();
        Paint(Mesh,Color);
        return Mesh;
    };
    for(float Side:{-1.f,1.f})
    {
        Limbs.Add(Part(FVector(0,Side*14,-52),FVector(21,21,48),Swatch(TEXT("354F53")),TEXT("/Engine/BasicShapes/Cube.Cube")));
        Part(FVector(6,Side*14,-79),FVector(34,24,17),Swatch(TEXT("F0E3C8")),TEXT("/Engine/BasicShapes/Cube.Cube"));
        Part(FVector(0,Side*32,0),FVector(16,16,52),Swatch(TEXT("D0794F")),TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
        Part(FVector(0,Side*32,-28),FVector(19),Swatch(TEXT("D9AE8A")),TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    }
    Part(FVector(0,0,76),FVector(47,47,21),Swatch(TEXT("354F53")),TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Part(FVector(17,0,77),FVector(34,48,7),Swatch(TEXT("D0794F")),TEXT("/Engine/BasicShapes/Cube.Cube"));
    Part(FVector(-23,0,19),FVector(8,46,12),Swatch(TEXT("F0E3C8")),TEXT("/Engine/BasicShapes/Cube.Cube"));
}

void ACourierCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const float Speed=GetVelocity().Size2D();
    WalkCycle+=DeltaSeconds*Speed*.025f;
    for(int32 I=0;I<Limbs.Num();++I)
        Limbs[I]->SetRelativeRotation(FRotator(FMath::Sin(WalkCycle+I*PI)*12.f*FMath::Clamp(Speed/500.f,0.f,1.f),0,0));
}

void ACourierCharacter::OnJumped_Implementation()
{
    Super::OnJumped_Implementation();
    if(auto* Mode=Game(this);Mode && Mode->Phase==ECourierPhase::Practice && Mode->Tutorial==1)
    {
        const FVector Position=GetActorLocation();
        if(Position.X>600 && Position.X<1100 && FMath::Abs(Position.Y)<550)Mode->bTrainingJumped=true;
    }
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
void ACourierCharacter::Yaw(float Value)
{
    if (auto* Mode=Game(this); Mode && Mode->CanMove() && !Mode->bLockCamera)
    {
        const float Limit=180.f*GetWorld()->GetDeltaSeconds();
        AddControllerYawInput(FMath::Clamp(Value*Mode->MouseSensitivity,-Limit,Limit));
    }
}
void ACourierCharacter::Pitch(float Value)
{
    if (auto* Mode=Game(this); Mode && Mode->CanMove() && !Mode->bLockCamera)
    {
        const float Limit=120.f*GetWorld()->GetDeltaSeconds();
        AddControllerPitchInput(FMath::Clamp(Value*Mode->MouseSensitivity,-Limit,Limit));
    }
}
void ACourierCharacter::SetCameraFov(float Value) { Camera->SetFieldOfView(FMath::Clamp(Value,80.f,100.f)); }
void ACourierCharacter::JumpPressed() { if (Game(this) && Game(this)->CanMove()) Jump(); }
void ACourierCharacter::JumpReleased() { StopJumping(); }

void ACourierController::BeginPlay()
{
    Super::BeginPlay();
    if (PlayerCameraManager)
    {
        PlayerCameraManager->ViewPitchMin=-60.f;
        PlayerCameraManager->ViewPitchMax=20.f;
    }
}

void ACourierController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindAction(TEXT("Continue"),IE_Pressed,this,&ACourierController::Next).bExecuteWhenPaused=true;
    InputComponent->BindAction(TEXT("Back"),IE_Pressed,this,&ACourierController::Previous).bExecuteWhenPaused=true;
    InputComponent->BindAction(TEXT("Pause"),IE_Pressed,this,&ACourierController::PausePressed).bExecuteWhenPaused=true;
    InputComponent->BindAction(TEXT("Retry"),IE_Pressed,this,&ACourierController::RetryPressed).bExecuteWhenPaused=true;
    InputComponent->BindAction(TEXT("Help"),IE_Pressed,this,&ACourierController::HelpPressed).bExecuteWhenPaused=true;
    InputComponent->BindAction(TEXT("Language"),IE_Pressed,this,&ACourierController::LanguagePressed).bExecuteWhenPaused=true;
    InputComponent->BindAction(TEXT("Recenter"),IE_Pressed,this,&ACourierController::RecenterPressed).bExecuteWhenPaused=true;
    InputComponent->BindAction(TEXT("Interact"),IE_Pressed,this,&ACourierController::Interact);
    InputComponent->BindAction(TEXT("Slow"),IE_Pressed,this,&ACourierController::SlowPressed);
    InputComponent->BindAction(TEXT("Slow"),IE_Released,this,&ACourierController::SlowReleased).bExecuteWhenPaused=true;
}
void ACourierController::Next() { if(Game(this))Game(this)->Continue(); }
void ACourierController::Previous() { if(Game(this))Game(this)->Back(); }
void ACourierController::Interact() { if(Game(this))Game(this)->Interact(); }
void ACourierController::SlowPressed() { if(Game(this))Game(this)->StartSlow(); }
void ACourierController::SlowReleased() { if(Game(this) && Game(this)->IsPlaying())Game(this)->EndSlow(); }
void ACourierController::PausePressed() { if(Game(this))Game(this)->TogglePause(); }
void ACourierController::RetryPressed() { if(Game(this))Game(this)->Retry(); }
void ACourierController::HelpPressed() { if(Game(this))Game(this)->ToggleHelp(); }
void ACourierController::LanguagePressed() { if(Game(this))Game(this)->ToggleLanguage(); }
void ACourierController::RecenterPressed() { if(Game(this))Game(this)->RecenterCamera(); }

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
void ACourierCart::BeginPlay()
{
    Super::BeginPlay();
    const FVector RootScale(1.7f,2.2f,1.1f);
    const auto Detail=[this,RootScale](const TCHAR* Path,const FVector& Offset,const FVector& Size,const FLinearColor& Color)
    {
        auto* Part=NewObject<UStaticMeshComponent>(this);
        Part->SetupAttachment(Mesh);
        Part->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,Path));
        Part->SetRelativeLocation(Offset/RootScale);
        Part->SetRelativeScale3D(Size/(RootScale*100.f));
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Part->RegisterComponent();
        Paint(Part,Color);
    };
    for(float X:{-83.f,83.f})for(float Y:{-76.f,76.f})
        Detail(TEXT("/Engine/BasicShapes/Sphere.Sphere"),FVector(X,Y,-40),FVector(32),Swatch(TEXT("354F53")));
    Detail(TEXT("/Engine/BasicShapes/Cube.Cube"),FVector(-87,0,3),FVector(6,73,57),Swatch(TEXT("F0E3C8")));
    // The same clock mark on every device supports recognition beyond its blue colour.
    Detail(TEXT("/Engine/BasicShapes/Cube.Cube"),FVector(-91,0,13),FVector(4,7,25),Swatch(TEXT("354F53")));
    Detail(TEXT("/Engine/BasicShapes/Cube.Cube"),FVector(-92,9,1),FVector(4,25,7),Swatch(TEXT("354F53")));
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
    if(Mode->bSlowing)
        if(const auto* Player=UGameplayStatics::GetPlayerCharacter(this,0))
            if(FMath::Abs(Player->GetActorLocation().X-Origin.X)<600.f)bSlowedOnce=true;
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
