#include "CourierPrototype.h"
#include "Components/AudioComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

// Borrower cells: spare batteries for the Borrower. Each gives Q one more use
// in the current delivery. Most sit on the riskier line (in a cart lane, at a
// lift's top, over the shuttle's water, on a planter or stepping stone), so
// playing the short route well funds the next crossing. Stars rate each
// delivery by its own concern: coffee warmth, lens condition, reel time.
namespace
{
    UStaticMeshComponent* TokenPart(AActor* Owner,USceneComponent* Parent,const TCHAR* Shape,const FVector& At,const FVector& Size,
        const TCHAR* Hex,const FRotator& Rotation=FRotator::ZeroRotator)
    {
        auto* Mesh=NewObject<UStaticMeshComponent>(Owner);
        Mesh->SetupAttachment(Parent);Mesh->SetMobility(EComponentMobility::Movable);
        Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"),Shape,Shape)));
        Mesh->SetRelativeLocationAndRotation(At,Rotation);Mesh->SetRelativeScale3D(Size/100.f);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);Mesh->SetCastShadow(false);Mesh->RegisterComponent();
        auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Materials/M_CourierMatte.M_CourierMatte"));
        if(auto* Material=UMaterialInstanceDynamic::Create(Base,Mesh))
        {
            Material->SetVectorParameterValue(TEXT("Color"),FLinearColor::FromSRGBColor(FColor::FromHex(Hex)));
            Mesh->SetMaterial(0,Material);
        }
        return Mesh;
    }
}

void ACourierGameMode::SpawnTokens()
{
    struct FSpot {int32 Order;FVector Floor;};
    // Floor height of each spot; the clock floats about a body height above it.
    const FSpot Spots[]={
        {-1,FVector(1800,0,0)},                                   // practice: learn that clocks exist
        {0,FVector(3300,0,0)},{0,FVector(4700,0,85)},{0,FVector(5475,0,0)},{0,FVector(5200,850,0)},
        {1,FVector(12600,0,300)},{1,FVector(15650,0,320)},{1,FVector(19700,0,0)},{1,FVector(23300,0,400)},{1,FVector(19900,-1900,0)},
        {2,FVector(31150,0,0)},{2,FVector(34900,0,0)},{2,FVector(37900,0,0)},{2,FVector(40300,0,420)},{2,FVector(43780,280,520)},{2,FVector(31300,1450,0)}};
    for(const FSpot& Spot:Spots)
    {
        auto* Actor=GetWorld()->SpawnActor<AActor>();
        auto* Root=NewObject<USceneComponent>(Actor);Actor->SetRootComponent(Root);Root->SetMobility(EComponentMobility::Movable);Root->RegisterComponent();
        // An upright battery: cyan body, cream label band with the Borrower's
        // clock mark, dark terminal cap. It matches the blue clock-tagged devices.
        TokenPart(Actor,Root,TEXT("Cylinder"),FVector::ZeroVector,FVector(46,46,74),TEXT("3CC4D8"));
        TokenPart(Actor,Root,TEXT("Cylinder"),FVector(0,0,4),FVector(48,48,22),TEXT("FFF3D6"));
        TokenPart(Actor,Root,TEXT("Cube"),FVector(24,0,6),FVector(2,3,14),TEXT("243D43"));
        TokenPart(Actor,Root,TEXT("Cube"),FVector(24,4,3),FVector(2,9,3),TEXT("243D43"));
        TokenPart(Actor,Root,TEXT("Cylinder"),FVector(0,0,42),FVector(20,20,10),TEXT("243D43"));
        auto* Glow=NewObject<UPointLightComponent>(Actor);Glow->SetupAttachment(Root);Glow->RegisterComponent();
        Glow->SetIntensity(2200.f);Glow->SetAttenuationRadius(300.f);Glow->SetLightColor(FLinearColor(.45f,.9f,1.f));Glow->SetCastShadows(false);
        Actor->Tags.Add(TEXT("CourierBorrowerCell"));
        const FVector Base=Spot.Floor+FVector(0,0,110);
        Actor->SetActorLocation(Base);
        Tokens.Add({Actor,Base,Spot.Order,false});
    }
}

bool ACourierGameMode::TokenVisible(const FCourierToken& Token) const
{
    if(Token.bTaken)return false;
    const ECourierPhase View=Phase==ECourierPhase::Paused?BeforePause:Phase;
    if(Token.Order<0)return View==ECourierPhase::Practice;
    return Token.Order==CurrentOrder && (View==ECourierPhase::Briefing || View==ECourierPhase::Delivery);
}

int32 ACourierGameMode::TokensTotal(int32 Order) const
{
    int32 Count=0;for(const auto& Token:Tokens)if(Token.Order==Order)++Count;return Count;
}

int32 ACourierGameMode::TokensTaken(int32 Order) const
{
    int32 Count=0;for(const auto& Token:Tokens)if(Token.Order==Order && Token.bTaken)++Count;return Count;
}

void ACourierGameMode::ResetTokens(int32 Order)
{
    for(auto& Token:Tokens)if(Order==-2 || Token.Order==Order)Token.bTaken=false;
}

void ACourierGameMode::UpdateTokens(float DeltaSeconds)
{
    TokenSpin+=DeltaSeconds;
    for(int32 I=0;I<Tokens.Num();++I)
    {
        auto& Token=Tokens[I];
        AActor* Actor=Token.Actor.Get();if(!Actor)continue;
        const bool bVisible=TokenVisible(Token);
        if(Actor->IsHidden()==bVisible)Actor->SetActorHiddenInGame(!bVisible);
        if(!bVisible)continue;
        Actor->SetActorLocationAndRotation(Token.Base+FVector(0,0,FMath::Sin(TokenSpin*2.2f+I)*10.f),FRotator(0,TokenSpin*95.f+I*40.f,0));
    }
}

void ACourierGameMode::CollectTokens(const FVector& Position)
{
    for(auto& Token:Tokens)
    {
        if(!TokenVisible(Token) || FVector::Dist2D(Position,Token.Base)>130.f || FMath::Abs(Position.Z-Token.Base.Z)>120.f)continue;
        Token.bTaken=true;
        if(AActor* Actor=Token.Actor.Get())Actor->SetActorHiddenInGame(true);
        Sound(TEXT("Pickup"),.38f,1.7f);
        if(Phase==ECourierPhase::Delivery)
        {
            ++Charges;
            Notify(TEXT("Borrower cell: +1 use of borrowed time."),TEXT("借时器电池：多一次借时间。"));
        }
        else Notify(TEXT("A Borrower cell! During a delivery, each one gives Q one more use."),TEXT("借时器电池！配送时，每拿到一个，Q 就多一次使用机会。"));
        TokenFlash=.5f;
    }
}

int32 ACourierGameMode::RateDelivery() const
{
    // Each parcel is judged by what its customer cares about.
    if(CurrentOrder==1)return FMath::Clamp(LensCondition,1,3);       // handle with care
    const float Left=FMath::Clamp(Remaining/FMath::Max(1.f,OrderBudget()),0.f,1.f);
    return Left>=.45f?3:(Left>=.2f?2:1);                             // serve warm / start on time
}

int32 ACourierGameMode::TotalStars() const
{
    int32 Total=0;for(int32 Value:Stars)Total+=Value;return Total;
}

FString ACourierGameMode::RankTitle() const
{
    const int32 Total=TotalStars();
    if(Total>=9)return Copy(TEXT("Legend of the Last Minute"),TEXT("最后一刻传奇"));
    if(Total>=7)return Copy(TEXT("Star Courier"),TEXT("明星快递员"));
    if(Total>=4)return Copy(TEXT("Reliable Courier"),TEXT("可靠的快递员"));
    return Copy(TEXT("Courier in Training"),TEXT("见习快递员"));
}

void ACourierGameMode::Celebrate(const FVector& At,int32 Count)
{
    // Paper confetti from basic shapes: a short physical burst, gone in seconds.
    static const TCHAR* Colors[]={TEXT("E2622C"),TEXT("E9A93A"),TEXT("3CC4D8"),TEXT("FFF3D6"),TEXT("2E8C95")};
    for(int32 I=0;I<Count;++I)
    {
        auto* Piece=GetWorld()->SpawnActor<AStaticMeshActor>(At+FVector(FMath::FRandRange(-60.f,60.f),FMath::FRandRange(-60.f,60.f),FMath::FRandRange(0.f,40.f)),FRotator(FMath::FRandRange(0.f,360.f),FMath::FRandRange(0.f,360.f),0));
        if(!Piece)continue;
        auto* Mesh=Piece->GetStaticMeshComponent();Mesh->SetMobility(EComponentMobility::Movable);
        Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
        Mesh->SetWorldScale3D(FVector(.18f,.12f,.02f));Mesh->SetCastShadow(false);
        Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
        Mesh->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);Mesh->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
        Mesh->SetCollisionResponseToChannel(ECC_Visibility,ECR_Ignore);
        auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Materials/M_CourierMatte.M_CourierMatte"));
        if(auto* Material=UMaterialInstanceDynamic::Create(Base,Mesh))
        {
            Material->SetVectorParameterValue(TEXT("Color"),FLinearColor::FromSRGBColor(FColor::FromHex(Colors[I%5])));
            Mesh->SetMaterial(0,Material);
        }
        Mesh->SetSimulatePhysics(true);Mesh->SetLinearDamping(1.2f);Mesh->SetAngularDamping(.4f);
        Mesh->SetPhysicsLinearVelocity(FVector(FMath::FRandRange(-320.f,320.f),FMath::FRandRange(-320.f,320.f),FMath::FRandRange(520.f,820.f)));
        Mesh->SetPhysicsAngularVelocityInDegrees(FVector(FMath::FRandRange(-720.f,720.f),FMath::FRandRange(-720.f,720.f),FMath::FRandRange(-720.f,720.f)));
        Piece->SetLifeSpan(3.5f);
    }
}

void ACourierGameMode::UpdateFeel(float DeltaSeconds)
{
    // Borrowed time is felt as well as seen: the music drops in pitch with the
    // blue screen tint (applied by the character's camera from SlowFx).
    SlowFx=FMath::FInterpTo(SlowFx,bSlowing?1.f:0.f,DeltaSeconds,9.f);
    const float Pitch=FMath::Lerp(1.f,.8f,SlowFx);
    if(Music)Music->SetPitchMultiplier(Pitch);
    for(const auto& Layer:MusicLayers)if(Layer)Layer->SetPitchMultiplier(Pitch);
    HitFlash=FMath::Max(0.f,HitFlash-DeltaSeconds);
    TokenFlash=FMath::Max(0.f,TokenFlash-DeltaSeconds);
    // The last ten seconds of a delivery tick audibly.
    if(Phase==ECourierPhase::Delivery && Remaining<=10.f && Remaining>0.f)
    {
        const int32 Second=FMath::CeilToInt(Remaining);
        if(Second!=LastTickSecond){LastTickSecond=Second;Sound(TEXT("Footstep"),.42f,2.3f);}
    }
    else LastTickSecond=-1;
}
