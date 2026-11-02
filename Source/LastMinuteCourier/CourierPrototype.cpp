#include "CourierPrototype.h"
#include "CourierCampusScene.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
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
    ConfigureOrder(0);
    Checkpoint=FVector(100,0,120);
    BuildPracticeRoute();
    BuildDeliveryDistricts();
    GetWorld()->SpawnActor<ACourierCampusScene>();
    UpdateCelebration();
    if(auto* Track=LoadObject<USoundBase>(nullptr,TEXT("/Game/Audio/CampusLoop.CampusLoop")))
        Music=UGameplayStatics::SpawnSound2D(this,Track,.30f);
    for(const TCHAR* Name:{TEXT("CampusStrings"),TEXT("CampusBells")})
    {
        const FString Path=FString::Printf(TEXT("/Game/Audio/%s.%s"),Name,Name);
        if(auto* Track=LoadObject<USoundBase>(nullptr,*Path))
            MusicLayers.Add(UGameplayStatics::SpawnSound2D(this,Track,0.f));
    }
    if (auto* Player = UGameplayStatics::GetPlayerCharacter(this, 0))
    {
        Player->SetActorLocation(Checkpoint);
        if (auto* Controller = UGameplayStatics::GetPlayerController(this, 0))
            Controller->SetControlRotation(FRotator(-22, 0, 0));
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
    return IsPlaying() && RecoverySeconds<=0.f ? (bSlowing ? .25f : 1.f) : 0.f;
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
    // Optional, read-only QA trace. It never changes play state or appears in UI.
    if(FParse::Param(FCommandLine::Get(),TEXT("CourierTelemetry")))
    {
        static double NextSample=0.;
        const double Now=FPlatformTime::Seconds();
        if(Now>=NextSample)
        {
            NextSample=Now+1.;
            const auto* P=UGameplayStatics::GetPlayerCharacter(this,0);
            const auto* PC=UGameplayStatics::GetPlayerController(this,0);
            if(P && PC)UE_LOG(LogTemp,Display,TEXT("CourierQA phase=%d order=%d tutorial=%d node=%d position=%s yaw=%.1f remaining=%.2f elapsed=%.2f"),
                int32(Phase),CurrentOrder,Tutorial,RouteNode,*P->GetActorLocation().ToString(),PC->GetControlRotation().Yaw,Remaining,OrderElapsed);
        }
    }
    // AwaitPickup still allows movement, so falling must recover in that state too.
    if (!CanMove()) return;
    auto* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!Player) return;
    CollisionProtection = FMath::Max(0.f, CollisionProtection - DeltaSeconds);
    NoticeSeconds=FMath::Max(0.f,NoticeSeconds-DeltaSeconds);
    SessionSeconds+=DeltaSeconds;
    if(RecoverySeconds>0.f)
    {
        RecoverySeconds=FMath::Max(0.f,RecoverySeconds-DeltaSeconds);
        return;
    }
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
        while(Route.IsValidIndex(RouteNode))
        {
            // Stay in the centre aisle until beyond the cinema seats, then turn left.
            const float Radius=CurrentOrder==2 && RouteNode==Route.Num()-2?100.f:280.f;
            if(!(Position.X>Route[RouteNode].X+120.f ||
                (FVector::Dist2D(Position,Route[RouteNode])<Radius && FMath::Abs(Position.Z-Route[RouteNode].Z)<140.f)))break;
            ++RouteNode;
        }
        if(Player->GetCharacterMovement()->IsMovingOnGround())
        {
            for(const FVector& Node:Route)
                if(FVector::Dist2D(Position,Node)<420.f && FMath::Abs(Position.Z-Node.Z)<130.f && Node.X>Checkpoint.X)
                    Checkpoint=Node+FVector(0,0,20);
            // Side routes earn the same progress as precision routes. Save their
            // safe landings before the next traffic lane, never on moving devices.
            TArray<FVector> Landings;
            if(CurrentOrder==1)Landings={FVector(14800,1900,420),FVector(21600,0,100),FVector(25700,2100,500)};
            if(CurrentOrder==2)Landings={FVector(32700,0,100),FVector(36300,0,100),FVector(38900,0,100),FVector(43000,2200,520)};
            for(const FVector& Landing:Landings)
                if(Landing.X>Checkpoint.X && FVector::Dist2D(Position,Landing)<240.f && FMath::Abs(Position.Z-Landing.Z)<130.f)
                    Checkpoint=Landing+FVector(0,0,20);
        }
        if (Remaining <= 0.f)
        {
            EndSlow();
            Phase = ECourierPhase::Failed;
            RefreshInput();
            return;
        }
    }
    const bool UnderRaisedRoute=(CurrentOrder==1 && ((Position.X>12900 && Position.X<16400 && Position.Z<140) ||
        (Position.X>23700 && Position.Z<220))) || (CurrentOrder==2 && ((Position.X<27400 && Position.Z<220) ||
        (Position.X>40850 && Position.Z<240)));
    if (Position.Z < -180.f || UnderRaisedRoute || FMath::Abs(Position.Y) > (CurrentOrder==0?1200.f:3400.f) || Position.X>(CurrentOrder==0?8650.f:(CurrentOrder==1?26800.f:47000.f)) || Position.X<(CurrentOrder==0?-250.f:(CurrentOrder==1?7600.f:25500.f)))
    {
        if(FParse::Param(FCommandLine::Get(),TEXT("CourierTelemetry")))UE_LOG(LogTemp,Display,TEXT("CourierQA recovery position=%s checkpoint=%s underRaised=%d"),*Position.ToString(),*Checkpoint.ToString(),UnderRaisedRoute);
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
    if(Phase==ECourierPhase::Opening)
    {
        if(++Card>=CardTitles.Num()){Phase=ECourierPhase::Practice;Card=0;}
    }
    else if(Phase==ECourierPhase::Briefing)AcceptOrder();
    else if(Phase==ECourierPhase::Delivered)NextOrder();
    else if(Phase==ECourierPhase::Complete)RestartRun();
    else if(Phase==ECourierPhase::Failed)Retry();
    else if(Phase==ECourierPhase::Paused)TogglePause();
    RefreshInput();
}

void ACourierGameMode::Back()
{
    if (Phase == ECourierPhase::Opening) Card = FMath::Max(0, Card - 1);
    else if(Phase==ECourierPhase::Briefing){Phase=ECourierPhase::AwaitPickup;RefreshInput();}
    else if (Phase == ECourierPhase::Paused) TogglePause();
}

void ACourierGameMode::Interact()
{
    auto* Player=Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(this,0));
    if(!Player)return;
    if(Phase==ECourierPhase::AwaitPickup && CanInteractAt(Pickup))
    {
        Phase=ECourierPhase::Briefing;
        Sound(TEXT("Paper"),.22f);
    }
    else if(Phase==ECourierPhase::Delivery && CanInteractAt(Destination))
    {
        EndSlow();
        DeliveryTimes[CurrentOrder]=OrderElapsed;
        CompletedOrders=FMath::Max(CompletedOrders,CurrentOrder+1);
        Player->ShowPackage(false);
        Phase=CurrentOrder==2?ECourierPhase::Complete:ECourierPhase::Delivered;
        UpdateCelebration();
        Sound(CurrentOrder==2?TEXT("Finale"):TEXT("Stamp"),.4f);
        if(CurrentOrder==2)Player->RecenterView((FVector(47400,0,1350)-Player->GetActorLocation()).Rotation().Yaw,5.f);
    }
    RefreshInput();
}

void ACourierGameMode::AcceptOrder()
{
    if(Phase!=ECourierPhase::Briefing || !CanInteractAt(Pickup))return;
    Phase=ECourierPhase::Delivery;
    Remaining=OrderBudget();OrderElapsed=0.f;RecoverySeconds=0.f;
    Checkpoint=Pickup+FVector(0,0,20);
    ResetDevices();
    if(auto* Player=Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(this,0)))Player->ShowPackage(true);
    Sound(TEXT("Pickup"),.28f);
}

void ACourierGameMode::NextOrder()
{
    if(Phase!=ECourierPhase::Delivered || CurrentOrder>=2)return;
    ConfigureOrder(CurrentOrder+1);
    // The next parcel is already at this counter. Reading never starts its clock.
    Phase=CanInteractAt(Pickup)?ECourierPhase::Briefing:ECourierPhase::AwaitPickup;
    ResetDevices();NoticeSeconds=0.f;
    RefreshInput();
}

void ACourierGameMode::RestartRun()
{
    if(Phase!=ECourierPhase::Complete)return;
    CurrentOrder=0;CompletedOrders=0;Retries=0;Tutorial=0;Card=0;SessionSeconds=0.f;
    DeliveryTimes={0.f,0.f,0.f};bTrainingJumped=false;
    ConfigureOrder(0);Checkpoint=FVector(100,0,120);
    Phase=ECourierPhase::Opening;NoticeSeconds=0.f;
    ResetDevices();RestoreCheckpoint();RecoverySeconds=0.f;
    if(auto* Player=Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(this,0)))Player->ShowPackage(false);
    UpdateCelebration();RecenterCamera();
    if(auto* PC=UGameplayStatics::GetPlayerController(this,0))PC->SetControlRotation(FRotator(-22,0,0));
    if(auto* Player=Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(this,0)))Player->RecenterView(0.f);
    RefreshInput();
}

void ACourierGameMode::StartSlow()
{
    if (IsPlaying() && RecoverySeconds<=0.f && Cooldown <= 0.f && SlowRemaining > 0.f && !bSlowing)
    {bSlowing=true;Sound(TEXT("Borrow"),.18f);}
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
    if (!Controller || Phase==ECourierPhase::Opening || Phase==ECourierPhase::Briefing || Phase==ECourierPhase::Complete || Phase==ECourierPhase::Delivered || Phase==ECourierPhase::Failed)return;
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
    if(Phase==ECourierPhase::Failed){bHelp=!bHelp;return;}
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
        const auto* Player=UGameplayStatics::GetPlayerCharacter(this,0);
        const FVector Offset=Player?NavigationTarget()-Player->GetActorLocation():FVector::ForwardVector;
        if(auto* Courier=Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(this,0)))
            Courier->RecenterView(Offset.Size2D()>200.f?Offset.Rotation().Yaw:0.f);
    }
}

void ACourierGameMode::RestoreCheckpoint()
{
    if (auto* Player = UGameplayStatics::GetPlayerCharacter(this, 0))
    {
        Player->SetBase(static_cast<FMovementBaseInterfaceData*>(nullptr));
        Player->GetCharacterMovement()->StopMovementImmediately();
        Player->StopJumping();
        Player->ConsumeMovementInputVector();
        Player->SetActorLocation(Checkpoint, false, nullptr, ETeleportType::TeleportPhysics);
        // Rewind stale route progress together with the player. Detour guidance
        // can then lead from the saved landing instead of across a pond.
        if(Phase==ECourierPhase::Delivery)
        {
            RouteNode=0;
            while(Route.IsValidIndex(RouteNode) && Route[RouteNode].X<Checkpoint.X-120.f)++RouteNode;
        }
        if (auto* PC=UGameplayStatics::GetPlayerController(this,0); PC && PC->PlayerCameraManager)
            PC->PlayerCameraManager->StartCameraFade(1.f,0.f,.3f,FLinearColor::Black,false,false);
        CollisionProtection = 1.f;
        RecoverySeconds=.3f;
    }
}

void ACourierGameMode::ResetDevices()
{
    for (const auto& Cart : Carts) if (Cart && Cart->OrderIndex==CurrentOrder) Cart->Reset();
    for(const auto& Platform:Platforms)if(Platform && Platform->OrderIndex==CurrentOrder)Platform->Reset();
    bSlowing = false;
    SlowRemaining = 3.f;
    Cooldown = 0.f;
}

void ACourierGameMode::Retry()
{
    if (Phase == ECourierPhase::Opening || Phase == ECourierPhase::Practice || Phase==ECourierPhase::Complete || Phase==ECourierPhase::Delivered ||
        (Phase==ECourierPhase::Paused && BeforePause==ECourierPhase::Practice)) return;
    if (Phase == ECourierPhase::Paused)
        if (auto* PC = UGameplayStatics::GetPlayerController(this, 0)) PC->SetPause(false);
    Phase = ECourierPhase::AwaitPickup;
    bHelp = false;
    NoticeSeconds=0;
    ++Retries;
    Remaining = OrderBudget();
    OrderElapsed = 0.f;
    ConfigureOrder(CurrentOrder);
    Checkpoint = Pickup+FVector(0,0,20);
    ResetDevices();
    UpdateCelebration();
    RestoreCheckpoint();
    if (auto* Player = Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
        Player->ShowPackage(false);
    RefreshInput();
}

FString ACourierGameMode::InteractionPrompt() const
{
    const auto* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!Player) return FString();
    if (Phase == ECourierPhase::AwaitPickup && CanInteractAt(Pickup))
        return Copy(TEXT("E  Read delivery brief"),TEXT("E 查看配送简报"));
    if (Phase == ECourierPhase::Delivery && CanInteractAt(Destination))
        return Copy(TEXT("E  Deliver package"),TEXT("E 交付包裹"));
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
            // A release can arrive after UI takes focus. Never resume a stale held movement key.
            Controller->FlushPressedKeys();
            FInputModeGameAndUI Mode;
            Mode.SetHideCursorDuringCapture(false);
            Controller->SetInputMode(Mode);
        }
        if (auto* Player = UGameplayStatics::GetPlayerCharacter(this, 0))
            if (!CanMove())
            {
                Player->ConsumeMovementInputVector();
                Player->StopJumping();
                Player->GetCharacterMovement()->StopMovementImmediately();
            }
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
                    TEXT("Wait until the cart moves aside. Hold Q to keep the gap open, then cross."), TEXT("Reach the parcel desk. E reads the brief; Enter accepts and starts the clock.")};
    TutorialTextZh = {TEXT("WASD 移动，鼠标转动视角。走到标记位置。"), TEXT("按空格跳过低台阶。"),
                      TEXT("先等运输车让开，再按住 Q 延长空隙，趁机通过。"), TEXT("走到取件台。E 阅读简报，Enter 确认接单后才开始计时。")};
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *(FPaths::ProjectDir() / TEXT("DesignData/game_content.json"))))
    {
        UE_LOG(LogTemp, Warning, TEXT("Courier copy file unavailable; using fallback copy."));
        return;
    }
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root) return;
    const TArray<TSharedPtr<FJsonValue>>* Cards = nullptr;
    if (Root->TryGetArrayField(TEXT("opening_cards"), Cards) && Cards->Num()>=3 && Cards->Num()<=6)
    {
        CardTitles.SetNum(Cards->Num());CardTitlesZh.SetNum(Cards->Num());
        CardBodies.SetNum(Cards->Num());CardBodiesZh.SetNum(Cards->Num());
        for (int32 I=0;I<Cards->Num();++I)
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
    const TArray<TSharedPtr<FJsonValue>>* Lessons=nullptr;
    if(Root->TryGetArrayField(TEXT("tutorial"),Lessons) && Lessons->Num()==4)
        for(int32 I=0;I<4;++I)
        {
            const auto Text=(*Lessons)[I]->AsObject()->GetObjectField(TEXT("text"));
            TutorialText[I]=Localized(Text,TEXT("en"));TutorialTextZh[I]=Localized(Text,TEXT("zh"));
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
    Box(World,FVector(8440,0,170),FVector(100,860,340),Cream);
    Box(World,FVector(8180,0,45),FVector(70,740,90),Swatch(TEXT("B87952")));
    Box(World,FVector(8170,0,100),FVector(120,780,20),Cream,true);
    Box(World,FVector(8350,0,350),FVector(500,1000,30),Swatch(TEXT("D7835D")),false);
    for(float Y:{-445.f,445.f})Box(World,FVector(8120,Y,170),FVector(20,20,340),Ink,false);
    Box(World,FVector(8135,0,280),FVector(12,670,70),Cream,false);
    Sign(World,FVector(8126,0,295),TEXT("COFFEE / MOVIE NIGHT"));
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
    const float Lanes[] = {1550.f,3300.f,5300.f,5650.f};
    for (int32 I=0;I<4;++I)
    {
        auto* Cart = World->SpawnActor<ACourierCart>(FVector(Lanes[I],0,55), FRotator::ZeroRotator);
        Cart->Origin = FVector(Lanes[I],0,55);
        Cart->bTraining = I == 0;
        const float Periods[]={5.8f,5.8f,6.4f,6.4f};
        const float Phases[]={1.25f,.9f,.45f,2.5f};
        Cart->Period=Periods[I];Cart->InitialPhase=Phases[I];
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
        Tree(World,X+300,Side*1250,400+I%3*55);
        if(I%2==0)Bench(World,X-120,Side*1160);
    }
    // The cinema clocktower is a distant orientation landmark, outside the playable path.
    Box(World,FVector(47200,2000,970),FVector(430,430,1100),Cream,false);
    Box(World,FVector(47200,2000,1540),FVector(520,520,55),Orange,false);
    Shape(World,TEXT("/Engine/BasicShapes/Cone.Cone"),FVector(47200,2000,1720),FVector(550,550,330),Ink);
    auto* Clock=Shape(World,TEXT("/Engine/BasicShapes/Cylinder.Cylinder"),FVector(46978,2000,1350),FVector(190,190,12),Cream);
    Clock->SetActorRotation(FRotator(90,0,0));
    Box(World,FVector(46968,2000,1377),FVector(15,12,75),Ink,false);
    Box(World,FVector(46966,2027,1350),FVector(15,75,12),Ink,false);

    Box(World,FVector(47200,2000,200),FVector(800,800,440),Cream,false);

    auto* Sky=Shape(World,TEXT("/Engine/BasicShapes/Sphere.Sphere"),FVector(25000,0,0),FVector(220000),Swatch(TEXT("A9C9CD")));
    auto* SkyMesh=Sky->GetStaticMeshComponent();
    auto* SkyMaterial=UMaterialInstanceDynamic::Create(LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Materials/M_CourierSkyGradient.M_CourierSkyGradient")),SkyMesh);
    if(SkyMaterial)
    {
        SkyMaterial->SetVectorParameterValue(TEXT("Zenith"),Swatch(TEXT("83B6D5")));
        SkyMaterial->SetVectorParameterValue(TEXT("Horizon"),Swatch(TEXT("F8CEB1")));
        SkyMesh->SetMaterial(0,SkyMaterial);
    }
    SkyMesh->SetCastShadow(false);
    auto* Sun = World->SpawnActor<ADirectionalLight>(FVector(0,0,1200), FRotator(-14,156,0));
    if (auto* Light = Sun->FindComponentByClass<UDirectionalLightComponent>())
    {
        Light->SetIntensity(2.6f);
        Light->LightSourceAngle=4.f;
        Light->SetLightColor(Swatch(TEXT("FFF0D4")));
        Light->SetForwardShadingPriority(1);
    }
    auto* Fill = World->SpawnActor<ADirectionalLight>(FVector(0,0,1200), FRotator(-35,-30,0));
    if (auto* Light = Fill->FindComponentByClass<UDirectionalLightComponent>())
    {
        Light->SetIntensity(1.8f);
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
    GetCharacterMovement()->AirControl = .55f;
    GetCharacterMovement()->bImpartBaseVelocityX=false;
    GetCharacterMovement()->bImpartBaseVelocityY=false;
    GetCharacterMovement()->bImpartBaseVelocityZ=false;
    Arm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
    Arm->SetupAttachment(RootComponent);
    Arm->TargetArmLength = 620.f;
    Arm->SocketOffset = FVector(0,0,100);
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
    PackageMark=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ParcelSeal"));
    PackageMark->SetupAttachment(RootComponent);
    PackageMark->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
    PackageMark->SetRelativeRotation(FRotator(90,0,0));
    PackageMark->SetRelativeScale3D(FVector(.18f,.18f,.03f));
    PackageMark->SetRelativeLocation(FVector(-54,0,18));
    PackageMark->SetCollisionEnabled(ECollisionEnabled::NoCollision);PackageMark->SetVisibility(false);
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
    for(float Side:{-1.f,1.f})
    {
        Part(FVector(20,Side*8,65),FVector(4.5f),Swatch(TEXT("243D43")),TEXT("/Engine/BasicShapes/Sphere.Sphere"));
        Part(FVector(23,Side*15,4),FVector(4,6,48),Swatch(TEXT("F0E3C8")),TEXT("/Engine/BasicShapes/Cube.Cube"));
    }
    Part(FVector(20,0,54),FVector(2,9,2),Swatch(TEXT("81634B")),TEXT("/Engine/BasicShapes/Cube.Cube"));
    SetParcelStyle(Game(this)?Game(this)->CurrentOrder:0);
    FootstepCue=LoadObject<USoundBase>(nullptr,TEXT("/Game/Audio/Footstep.Footstep"));
}

void ACourierCharacter::SetParcelStyle(int32 Order)
{
    Bag->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,Order==2?TEXT("/Engine/BasicShapes/Cylinder.Cylinder"):TEXT("/Engine/BasicShapes/Cube.Cube")));
    Bag->SetRelativeRotation(Order==2?FRotator(90,0,0):FRotator::ZeroRotator);
    Bag->SetRelativeScale3D(Order==2?FVector(.6f,.6f,.22f):(Order==1?FVector(.3f,.58f,.42f):FVector(.35f,.5f,.5f)));
    Paint(Bag,Swatch(Order==2?TEXT("354F53"):(Order==1?TEXT("C86C43"):TEXT("438C95"))));
    Paint(PackageMark,Swatch(Order==2?TEXT("D6AB73"):TEXT("F0E3C8")));
    PackageMark->SetRelativeLocation(FVector(Order==2?-48.f:(Order==1?-52.f:-54.f),0,18));
}

void ACourierCharacter::ShowPackage(bool bVisible)
{
    Bag->SetVisibility(bVisible);PackageMark->SetVisibility(bVisible);
}

void ACourierCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const bool Finale=Game(this) && Game(this)->Phase==ECourierPhase::Complete;
    // A seated audience behind the courier must not collapse the ending pullback.
    Arm->bDoCollisionTest=!Finale;
    if(!Finale){Arm->TargetArmLength=620.f;Arm->SocketOffset.Z=100.f;}
    if(RecenterTime>=0.f && Controller)
    {
        const float Minimum=Finale?2.4f:.45f;
        const float Duration=FMath::Max3<float>(Minimum,FMath::Abs(FMath::FindDeltaAngleDegrees(RecenterFrom.Yaw,RecenterTo.Yaw))/100.f,FMath::Abs(RecenterFrom.Pitch-RecenterTo.Pitch)/80.f);
        RecenterTime=FMath::Min(1.f,RecenterTime+DeltaSeconds/Duration);
        const float Alpha=RecenterTime*RecenterTime*(3.f-2.f*RecenterTime);
        if(Finale)
        {
            // Pull back above the chair backs. The gameplay camera and FOV stay
            // unchanged; a fresh run restores the normal arm under its menu.
            Arm->TargetArmLength=FMath::Lerp(620.f,1800.f,Alpha);
            Arm->SocketOffset.Z=FMath::Lerp(100.f,400.f,Alpha);
        }
        Controller->SetControlRotation(FRotator(FMath::Lerp(RecenterFrom.Pitch,RecenterTo.Pitch,Alpha),
            RecenterFrom.Yaw+FMath::FindDeltaAngleDegrees(RecenterFrom.Yaw,RecenterTo.Yaw)*Alpha,0));
        if(RecenterTime>=1.f)RecenterTime=-1.f;
    }
    const float Speed=GetVelocity().Size2D();
    WalkCycle+=DeltaSeconds*Speed*.025f;
    if(auto* Mode=Game(this);Mode && Mode->CanMove() && GetCharacterMovement()->IsMovingOnGround() && Speed>120.f)
    {
        FootstepDistance+=Speed*DeltaSeconds;
        if(FootstepDistance>155.f)
        {
            FootstepDistance=0.f;
            if(Mode->bAudioEnabled && FootstepCue)UGameplayStatics::PlaySound2D(this,FootstepCue,.16f,FMath::FRandRange(.95f,1.05f));
        }
    }
    for(int32 I=0;I<Limbs.Num();++I)
        Limbs[I]->SetRelativeRotation(FRotator(FMath::Sin(WalkCycle+I*PI)*12.f*FMath::Clamp(Speed/500.f,0.f,1.f),0,0));
}

void ACourierCharacter::OnJumped_Implementation()
{
    Super::OnJumped_Implementation();
    if(auto* Mode=Game(this);Mode && Mode->Phase==ECourierPhase::Practice && Mode->Tutorial==1)
    {
        const FVector Position=GetActorLocation();
        if(Position.X>500 && Position.X<1100 && FMath::Abs(Position.Y)<550)Mode->bTrainingJumped=true;
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
    if (Controller && Game(this) && Game(this)->CanMove() && Game(this)->RecoverySeconds<=0.f)
        AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::X),Value);
}
void ACourierCharacter::Right(float Value)
{
    if (Controller && Game(this) && Game(this)->CanMove() && Game(this)->RecoverySeconds<=0.f)
        AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),Value);
}
void ACourierCharacter::Yaw(float Value)
{
    if (auto* Mode=Game(this); Mode && Mode->CanMove() && !Mode->bLockCamera && Mode->RecoverySeconds<=0.f)
    {
        const float Limit=180.f*FMath::Min(GetWorld()->GetDeltaSeconds(),.04f);
        if(!FMath::IsNearlyZero(Value))RecenterTime=-1.f;
        AddControllerYawInput(FMath::Clamp(Value*Mode->MouseSensitivity,-Limit,Limit));
    }
}
void ACourierCharacter::Pitch(float Value)
{
    if (auto* Mode=Game(this); Mode && Mode->CanMove() && !Mode->bLockCamera && Mode->RecoverySeconds<=0.f)
    {
        const float Limit=120.f*FMath::Min(GetWorld()->GetDeltaSeconds(),.04f);
        if(!FMath::IsNearlyZero(Value))RecenterTime=-1.f;
        AddControllerPitchInput(FMath::Clamp(Value*Mode->MouseSensitivity,-Limit,Limit));
    }
}
void ACourierCharacter::RecenterView(float Yaw,float Pitch)
{
    if(!Controller)return;
    RecenterFrom=Controller->GetControlRotation();RecenterFrom.Normalize();
    RecenterTo=FRotator(Pitch,Yaw,0);
    if(Game(this) && Game(this)->Phase==ECourierPhase::Paused)
    {
        // A paused world cannot advance a camera animation; apply beneath its menu.
        Controller->SetControlRotation(RecenterTo);RecenterTime=-1.f;return;
    }
    RecenterTime=0.f;
}

void ACourierCharacter::SetCameraFov(float Value) { Camera->SetFieldOfView(FMath::Clamp(Value,80.f,100.f)); }
void ACourierCharacter::JumpPressed() { if (Game(this) && Game(this)->CanMove() && Game(this)->RecoverySeconds<=0.f) Jump(); }
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
        Paint(Part,Color);return Part;
    };
    for(float X:{-83.f,83.f})for(float Y:{-76.f,76.f})
        Detail(TEXT("/Engine/BasicShapes/Sphere.Sphere"),FVector(X,Y,-40),FVector(32),Swatch(TEXT("354F53")));
    Detail(TEXT("/Engine/BasicShapes/Sphere.Sphere"),FVector(-87,0,3),FVector(6,70,70),Swatch(TEXT("F0E3C8")));
    // The same clock mark on every device supports recognition beyond its blue colour.
    ClockHand=Detail(TEXT("/Engine/BasicShapes/Cube.Cube"),FVector(-91,0,13),FVector(4,7,25),Swatch(TEXT("354F53")));
    Detail(TEXT("/Engine/BasicShapes/Cube.Cube"),FVector(-92,9,1),FVector(4,25,7),Swatch(TEXT("354F53")));
}
void ACourierCart::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* Mode=Game(this);
    if(!Mode) return;
    const bool bEnabled=bTraining ? Mode->Phase==ECourierPhase::Practice : Mode->Phase==ECourierPhase::Delivery && Mode->CurrentOrder==OrderIndex;
    if(!Material) Material=Paint(Mesh,FLinearColor(.02f,.42f,.75f));
    Material->SetVectorParameterValue(TEXT("Color"),bEnabled && Mode->bSlowing?FLinearColor(.15f,.95f,1.f):FLinearColor(.02f,.42f,.75f));
    if(!bEnabled) return;
    Clock+=DeltaSeconds*Mode->DeviceRate();
    if(ClockHand)ClockHand->SetRelativeRotation(FRotator(0,0,Clock*360.f/Period));
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
    if(!Mode || (bTraining ? Mode->Phase!=ECourierPhase::Practice : (Mode->Phase!=ECourierPhase::Delivery || Mode->CurrentOrder!=OrderIndex))) return false;
    const FVector D=Player-GetActorLocation();
    return FMath::Abs(D.X)<112.f && FMath::Abs(D.Y)<137.f && D.Z<132.f && D.Z>-120.f;
}
