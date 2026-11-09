#include "CourierPrototype.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

// Parkour set pieces (v0.9). The courier sprints, slides and climbs; the blue
// clock-tagged shutters and sweepers are new devices for the same Q rule, and
// gold sparks laid in lines reward the bolder line through each one.
namespace
{
    const FLinearColor DeviceBlue(.02f,.42f,.75f),DeviceSlow(.15f,.95f,1.f);
    FLinearColor Hex(const TCHAR* Value){return FLinearColor::FromSRGBColor(FColor::FromHex(Value));}

    UMaterialInstanceDynamic* Tint(UStaticMeshComponent* Mesh,const FLinearColor& Color)
    {
        auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Materials/M_CourierMatte.M_CourierMatte"));
        if(!Base)Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
        auto* M=UMaterialInstanceDynamic::Create(Base,Mesh);
        if(M){M->SetVectorParameterValue(TEXT("Color"),Color);Mesh->SetMaterial(0,M);}
        return M;
    }

    UStaticMeshComponent* Piece(AActor* Owner,USceneComponent* Parent,const TCHAR* Shape,const FVector& At,const FVector& Size,
        const FLinearColor& Color,bool bSolid=false,const FRotator& Rotation=FRotator::ZeroRotator)
    {
        auto* Mesh=NewObject<UStaticMeshComponent>(Owner);
        Mesh->SetupAttachment(Parent);Mesh->SetMobility(EComponentMobility::Movable);
        Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"),Shape,Shape)));
        Mesh->SetRelativeLocationAndRotation(At,Rotation);Mesh->SetRelativeScale3D(Size/100.f);
        if(bSolid)Mesh->SetCollisionProfileName(TEXT("BlockAll"));
        else Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Mesh->SetCastShadow(Size.GetMax()>60.f);
        Mesh->RegisterComponent();Tint(Mesh,Color);
        return Mesh;
    }

    // A solid, climbable box (crate, roof deck). Tagged walkable so the ledge
    // climb accepts it; hedges and sheds are never tagged.
    AStaticMeshActor* Solid(UWorld* W,const FVector& Center,const FVector& Size,const TCHAR* Color)
    {
        auto* A=W->SpawnActor<AStaticMeshActor>(Center,FRotator::ZeroRotator);
        auto* M=A->GetStaticMeshComponent();M->SetMobility(EComponentMobility::Movable);
        M->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
        M->SetWorldScale3D(Size/100.f);M->SetCollisionProfileName(TEXT("BlockAll"));
        Tint(M,Hex(Color));A->Tags.Add(TEXT("CourierWalkable"));
        return A;
    }

    AStaticMeshActor* Trim(UWorld* W,const FVector& Center,const FVector& Size,const TCHAR* Color,const TCHAR* Shape=TEXT("Cube"))
    {
        auto* A=W->SpawnActor<AStaticMeshActor>(Center,FRotator::ZeroRotator);
        auto* M=A->GetStaticMeshComponent();M->SetMobility(EComponentMobility::Movable);
        M->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"),Shape,Shape)));
        M->SetWorldScale3D(Size/100.f);CourierVisualCollision(M,Size);M->SetCastShadow(false);
        Tint(M,Hex(Color));
        return A;
    }

    // Parcel crates across a path: brown boxes with tape bands. Climb them with
    // a jump, or go around.
    void CrateWall(UWorld* W,float X,float Y,float Width,float Height,float Floor=0.f)
    {
        const int32 Count=FMath::Max(1,FMath::RoundToInt(Width/380.f));
        const float Each=Width/Count;
        for(int32 I=0;I<Count;++I)
        {
            const float CY=Y-Width*.5f+Each*(I+.5f);
            const float H=Height-(I%2)*12.f;
            Solid(W,FVector(X,CY,Floor+H*.5f),FVector(130,Each-8.f,H),I%2?TEXT("B98A5A"):TEXT("C99B66"));
            Trim(W,FVector(X,CY,Floor+H*.62f),FVector(134,Each-4.f,14),TEXT("E5D7BB"));
            Trim(W,FVector(X,CY,Floor+H+1.f),FVector(40,Each-12.f,3),TEXT("E5D7BB"));
        }
        // A low step to one side for anyone who prefers two small jumps.
        Solid(W,FVector(X-150.f,Y+Width*.25f,Floor+40.f),FVector(120,260,80),TEXT("C99B66"));
    }

    // Smoothstep for door travel.
    float Ease(float A){A=FMath::Clamp(A,0.f,1.f);return A*A*(3.f-2.f*A);}
}

ACourierObstacle::ACourierObstacle()
{
    PrimaryActorTick.bCanEverTick=true;
    Root=CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    Root->SetMobility(EComponentMobility::Movable);
    SetRootComponent(Root);
}

void ACourierObstacle::Build()
{
    SetActorLocation(Origin);
    Moving=NewObject<USceneComponent>(this);Moving->SetupAttachment(Root);Moving->SetMobility(EComponentMobility::Movable);Moving->RegisterComponent();
    const FLinearColor Ink=Hex(TEXT("354F53")),Cream=Hex(TEXT("F4EBD9")),Gold=Hex(TEXT("E9A93A")),Orange=Hex(TEXT("E2622C"));
    const auto Blue=[this](USceneComponent* Parent,const TCHAR* Shape,const FVector& At,const FVector& Size,const FRotator& R=FRotator::ZeroRotator)
    {
        auto* Mesh=Piece(this,Parent,Shape,At,Size,DeviceBlue,false,R);
        if(auto* M=Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0)))Tinted.Add(M);
        return Mesh;
    };
    const auto ClockFace=[&](USceneComponent* Parent,const FVector& At,float Yaw)
    {
        // The same clock mark every blue device carries.
        Piece(this,Parent,TEXT("Cylinder"),At,FVector(70,70,4),Cream,false,FRotator(90,Yaw,0));
        const FVector Out=FRotator(0,Yaw,0).RotateVector(FVector(-4,0,0));
        ClockHand=Piece(this,Parent,TEXT("Cube"),At+Out+FVector(0,0,12),FVector(4,6,26),Ink,false,FRotator(0,Yaw,0));
        Piece(this,Parent,TEXT("Cube"),At+Out+FVector(0,0,0)+FRotator(0,Yaw,0).RotateVector(FVector(0,9,0)),FVector(4,20,6),Ink,false,FRotator(0,Yaw,0));
    };
    switch(Kind)
    {
    case ECourierObstacleKind::Shutter:
    {
        // A roll-down shutter: posts, a housing and a slatted blue door.
        for(float Side:{-1.f,1.f})
        {
            Piece(this,Root,TEXT("Cube"),FVector(0,Side*(Width*.5f+28.f),175),FVector(56,56,350),Ink,true);
            Piece(this,Root,TEXT("Cube"),FVector(-34,Side*(Width*.5f+28.f),175),FVector(10,30,330),Gold);
        }
        Piece(this,Root,TEXT("Cube"),FVector(0,0,365),FVector(110,Width+112.f,70),Ink);
        auto* DoorFace=Blue(Moving,TEXT("Cube"),FVector(0,0,150),FVector(26,Width,300));
        // A follow camera behind the courier must stop before the opaque door.
        // Pawn collision remains controlled by the separate moving-height blocker.
        DoorFace->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        DoorFace->SetCollisionResponseToAllChannels(ECR_Ignore);
        DoorFace->SetCollisionResponseToChannel(ECC_Camera,ECR_Block);
        for(float Z:{25.f,95.f,165.f,235.f})
            Piece(this,Moving,TEXT("Cube"),FVector(0,0,Z),FVector(30,Width-6.f,9),Hex(TEXT("1E5F8A")));
        Piece(this,Moving,TEXT("Cube"),FVector(0,0,4),FVector(32,Width,8),Gold);
        ClockFace(Moving,FVector(-17,0,150),180.f);
        // Warning stripes either side of the threshold.
        for(float DX:{-90.f,90.f})Piece(this,Root,TEXT("Cube"),FVector(DX,0,1),FVector(30,Width,3),Gold);
        Blocker=Piece(this,Root,TEXT("Cube"),FVector(0,0,150),FVector(36,Width,300),DeviceBlue);
        Blocker->SetVisibility(false);Blocker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Blocker->SetCollisionResponseToAllChannels(ECR_Ignore);Blocker->SetCollisionResponseToChannel(ECC_Pawn,ECR_Block);
        break;
    }
    case ECourierObstacleKind::Sweeper:
    {
        // A turnstile with two arms: a low one to jump, a high one to slide under.
        Piece(this,Root,TEXT("Cylinder"),FVector(0,0,1),FVector(ArmLength*2.f+60.f,ArmLength*2.f+60.f,2),Hex(TEXT("B7AC93")));
        Piece(this,Root,TEXT("Cylinder"),FVector(0,0,90),FVector(64,64,180),Ink,true);
        Blue(Moving,TEXT("Cylinder"),FVector(0,0,186),FVector(90,90,24));
        Blue(Moving,TEXT("Cube"),FVector(ArmLength*.5f+20.f,0,55),FVector(ArmLength,24,40));
        Piece(this,Moving,TEXT("Cube"),FVector(ArmLength+10.f,0,55),FVector(26,28,44),Gold);
        Blue(Moving,TEXT("Cube"),FVector(-ArmLength*.5f-20.f,0,140),FVector(ArmLength,24,40));
        Piece(this,Moving,TEXT("Cube"),FVector(-ArmLength-10.f,0,140),FVector(26,28,44),Gold);
        for(int32 I=1;I<4;++I)Piece(this,Moving,TEXT("Cube"),FVector(-ArmLength*I/4.f-20.f,0,118),FVector(6,6,40),Ink);
        // Stripes on the low arm say "over"; the fringe under the high arm says "under".
        for(int32 I=1;I<5;++I)Piece(this,Moving,TEXT("Cube"),FVector(ArmLength*I/5.f+20.f,0,55),FVector(16,26,42),Gold);
        ClockFace(Moving,FVector(0,0,200),0.f);
        if(ClockHand)ClockHand->SetRelativeRotation(FRotator(90,0,0));
        break;
    }
    case ECourierObstacleKind::Spring:
    {
        // A springboard: orange base, gold pad, arrow pointing the way to leap.
        Piece(this,Root,TEXT("Cylinder"),FVector(0,0,6),FVector(210,210,12),Ink);
        Piece(this,Moving,TEXT("Cylinder"),FVector(0,0,16),FVector(180,180,12),Orange);
        Piece(this,Moving,TEXT("Cylinder"),FVector(0,0,23),FVector(140,140,4),Gold);
        Piece(this,Moving,TEXT("Cube"),FVector(-10,0,26),FVector(70,18,3),Hex(TEXT("FFF3D6")));
        Piece(this,Moving,TEXT("Cube"),FVector(25,18,26),FVector(40,16,3),Hex(TEXT("FFF3D6")),false,FRotator(0,-45,0));
        Piece(this,Moving,TEXT("Cube"),FVector(25,-18,26),FVector(40,16,3),Hex(TEXT("FFF3D6")),false,FRotator(0,45,0));
        break;
    }
    case ECourierObstacleKind::Beam:
    {
        // Practice: a striped beam hedge to hedge. Standing height will not fit.
        // 100 cm clear underneath, solid to 210 cm: too tall to hop, too low to walk.
        for(float Side:{-1.f,1.f})Piece(this,Root,TEXT("Cube"),FVector(0,Side*(Width*.5f-30.f),110),FVector(40,40,220),Ink,true);
        const int32 Stripes=FMath::Max(2,FMath::RoundToInt(Width/150.f));
        for(int32 I=0;I<Stripes;++I)
        {
            auto* Bar=Piece(this,Root,TEXT("Cube"),FVector(0,-Width*.5f+Width*(I+.5f)/Stripes,155),FVector(44,Width/Stripes,110),I%2?Cream:Orange,true);
            Bar->CanCharacterStepUpOn=ECB_No;
        }
        Piece(this,Root,TEXT("Cube"),FVector(-24,0,104),FVector(6,Width,8),Hex(TEXT("1E3036")));
        break;
    }
    }
}

void ACourierObstacle::Reset()
{
    Clock=0.f;SideBefore=0.f;CloseCallCooldown=0.f;
    if(Blocker)Blocker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Pose();
}

bool ACourierObstacle::Relevant() const
{
    const auto* Mode=Cast<ACourierGameMode>(UGameplayStatics::GetGameMode(this));
    if(!Mode)return false;
    if(OrderIndex<0)return Mode->Phase==ECourierPhase::Practice;
    return Mode->CurrentOrder==OrderIndex && Mode->Phase!=ECourierPhase::Practice;
}

float ACourierObstacle::DoorBottom(float ClockValue) const
{
    // Open, roll down, stay shut, roll up. Fractions of one period.
    float F=FMath::Fmod(ClockValue/Period+InitialPhase,1.f);if(F<0.f)F+=1.f;
    constexpr float Open=290.f;
    if(F<.40f)return Open;
    if(F<.62f)return Open*(1.f-Ease((F-.40f)/.22f));
    if(F<.80f)return 0.f;
    return Open*Ease((F-.80f)/.20f);
}

bool ACourierObstacle::Closing(float ClockValue) const
{
    float F=FMath::Fmod(ClockValue/Period+InitialPhase,1.f);if(F<0.f)F+=1.f;
    return F>=.40f && F<.62f;
}

float ACourierObstacle::ArmYaw(float ClockValue) const
{
    return FMath::Fmod((ClockValue/Period+InitialPhase)*360.f,360.f);
}

void ACourierObstacle::Pose()
{
    if(!Moving)return;
    if(Kind==ECourierObstacleKind::Shutter)Moving->SetRelativeLocation(FVector(0,0,DoorBottom(Clock)));
    else if(Kind==ECourierObstacleKind::Sweeper)Moving->SetRelativeRotation(FRotator(0,ArmYaw(Clock),0));
}

void ACourierObstacle::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* Mode=Cast<ACourierGameMode>(UGameplayStatics::GetGameMode(this));
    if(!Mode)return;
    CloseCallCooldown=FMath::Max(0.f,CloseCallCooldown-DeltaSeconds);
    if(Kind==ECourierObstacleKind::Spring && Moving)
    {
        // The pad settles back after a launch.
        const FVector Rest=Moving->GetRelativeLocation();
        Moving->SetRelativeLocation(FVector(0,0,FMath::FInterpTo(Rest.Z,0.f,DeltaSeconds,8.f)));
        return;
    }
    if(!bSlowable())return;
    const bool bSlowed=Mode->CanMove() && Mode->bSlowing;
    for(const auto& M:Tinted)if(M)M->SetVectorParameterValue(TEXT("Color"),bSlowed?DeviceSlow:DeviceBlue);
    const float Rate=Mode->DeviceRate();
    if(Rate<=0.f)return;
    Clock+=DeltaSeconds*Rate;
    if(ClockHand && Kind==ECourierObstacleKind::Shutter)ClockHand->SetRelativeRotation(FRotator(0,180.f,Clock*360.f/Period));
    Pose();
}

void ACourierObstacle::Check(ACourierGameMode& Mode,ACourierCharacter& Player,float DeltaSeconds)
{
    const FVector P=Player.GetActorLocation();
    const float Half=Player.GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    const float Feet=P.Z-Half-Origin.Z,Top=Feet+2.f*Half;
    auto* Move=Player.GetCharacterMovement();
    switch(Kind)
    {
    case ECourierObstacleKind::Shutter:
    {
        const float DX=P.X-Origin.X;
        const bool bAcross=FMath::Abs(P.Y-Origin.Y)<Width*.5f+20.f && Feet>-60.f && Feet<300.f;
        const float Bottom=DoorBottom(Clock);
        const bool bInPlane=bAcross && FMath::Abs(DX)<13.f+34.f;
        // The closed door is solid. It turns solid only while nobody stands in
        // the doorway, so it never traps the courier inside it.
        if(Blocker)
        {
            const bool bSolidNow=Blocker->GetCollisionEnabled()!=ECollisionEnabled::NoCollision;
            const bool bWantBlock=Bottom<100.f && (bSolidNow || !bInPlane);
            if(bSolidNow!=bWantBlock)Blocker->SetCollisionEnabled(bWantBlock?ECollisionEnabled::QueryAndPhysics:ECollisionEnabled::NoCollision);
        }
        if(bInPlane && Top>Bottom+2.f && Closing(Clock) && Mode.CollisionProtection<=0.f)
        {
            const float Away=SideBefore!=0.f?SideBefore:(DX<0.f?-1.f:1.f);
            Mode.KnockBack(FVector(Away*560.f,0,220.f),TEXT("The shutter came down on you! Slide under it sooner, or wait for it to open."),
                TEXT("卷帘门砸到你了！早点滑铲钻过去，或者等它升起来。"));
            return;
        }
        if(bAcross && FMath::Abs(DX)>55.f)
        {
            const float Side=DX<0.f?-1.f:1.f;
            // Through the doorway while it was low: a close call worth sparks.
            if(SideBefore!=0.f && Side!=SideBefore && Bottom<165.f && CloseCallCooldown<=0.f){Mode.CloseCall(3);CloseCallCooldown=2.f;}
            SideBefore=Side;
        }
        break;
    }
    case ECourierObstacleKind::Sweeper:
    {
        const FVector2D Rel(P.X-Origin.X,P.Y-Origin.Y);
        if(Rel.Size()>ArmLength+80.f || Feet>260.f || Feet<-60.f)break;
        const float Yaw=FMath::DegreesToRadians(ArmYaw(Clock));
        for(int32 Arm=0;Arm<2;++Arm)
        {
            const float A=Yaw+(Arm?PI:0.f);
            const FVector2D Dir(FMath::Cos(A),FMath::Sin(A));
            const float Along=FVector2D::DotProduct(Rel,Dir);
            const float Perp=FMath::Abs(Rel.X*Dir.Y-Rel.Y*Dir.X);
            if(Along<20.f || Along>ArmLength+30.f || Perp>34.f+6.f)continue;
            // Low arm spans 35-75 cm: clear it with a jump (a little grace at the
            // top). High arm spans 120-160 cm: clear it with a slide.
            const bool bHit=Arm==0?Feet<62.f:Top>128.f;
            if(bHit && Mode.CollisionProtection<=0.f)
            {
                const FVector2D Swing(-Dir.Y,Dir.X);
                Mode.KnockBack(FVector(Swing.X*520.f,Swing.Y*520.f,240.f),TEXT("The barrier arm swept you! Jump the low arm, slide under the high one."),
                    TEXT("旋转栏杆把你扫开了！低的跳过去，高的滑铲钻过去。"));
                return;
            }
            if(!bHit && CloseCallCooldown<=0.f){Mode.CloseCall(2);CloseCallCooldown=1.6f;}
        }
        break;
    }
    case ECourierObstacleKind::Spring:
    {
        if(FVector2D(P.X-Origin.X,P.Y-Origin.Y).Size()>105.f || Feet<10.f || Feet>60.f || Move->Velocity.Z>50.f || CloseCallCooldown>0.f)break;
        FVector Ahead=Move->Velocity.GetSafeNormal2D();
        if(Ahead.IsNearlyZero())Ahead=Player.GetActorForwardVector().GetSafeNormal2D();
        const float Speed=FMath::Max(Move->Velocity.Size2D(),650.f);
        Player.CancelMoves();
        Player.LaunchCharacter(Ahead*Speed+FVector(0,0,1080.f),true,true);
        if(Moving)Moving->SetRelativeLocation(FVector(0,0,-10));
        CloseCallCooldown=.6f;
        Mode.Sound(TEXT("Stamp"),.35f,1.8f);
        break;
    }
    default:break;
    }
}

void ACourierGameMode::BuildParkour()
{
    UWorld* W=GetWorld();
    const auto Make=[this,W](ECourierObstacleKind Kind,int32 Order,const FVector& At,float Width,float Period,float StartPhase,float Arm=450.f)
    {
        auto* O=W->SpawnActor<ACourierObstacle>(At,FRotator::ZeroRotator);
        O->Kind=Kind;O->OrderIndex=Order;O->Origin=At;O->Width=Width;O->Period=Period;O->InitialPhase=StartPhase;O->ArmLength=Arm;
        O->Build();O->Reset();Obstacles.Add(O);
        return O;
    };
    // Practice: one low beam from hedge to hedge teaches the slide.
    Make(ECourierObstacleKind::Beam,-1,FVector(1250,0,0),2500.f,1.f,0.f);
    // Coffee: climb the crate wall (or take the arcade), then the first shutter.
    CrateWall(W,4650.f,0.f,1130.f,150.f);
    Make(ECourierObstacleKind::Shutter,0,FVector(6300,0,0),1130.f,6.5f,.05f);
    // The stand's turnstile gate: slow, and the only way to the counter.
    Make(ECourierObstacleKind::Sweeper,0,FVector(7300,0,0),1130.f,10.f,.5f,560.f);
    // Garden: a crate wall on the approach, a turnstile after the stones and a
    // shutter in front of the projection booth.
    CrateWall(W,9800.f,1100.f,850.f,160.f);
    Make(ECourierObstacleKind::Sweeper,1,FVector(22000,0,0),900.f,8.5f,0.f,460.f);
    Make(ECourierObstacleKind::Shutter,1,FVector(25200,0,400),1000.f,6.f,.3f);
    // Reel: a shutter after the market lanes, a market roof to run along, a
    // turnstile before the last lanes, a springboard up to the terrace and the
    // clocktower gate.
    Make(ECourierObstacleKind::Shutter,2,FVector(32300,0,0),1050.f,6.f,.5f);
    Solid(W,FVector(30380,-800,40),FVector(140,300,80),TEXT("C99B66"));
    Solid(W,FVector(30520,-800,82),FVector(140,300,164),TEXT("B98A5A"));
    Solid(W,FVector(31150,-800,295),FVector(1100,320,50),TEXT("D7835D"));
    for(float X:{30650.f,31150.f,31650.f})Trim(W,FVector(X,-930,135),FVector(24,24,270),TEXT("354F53"));
    for(int32 I=0;I<6;++I)Trim(W,FVector(30650+I*200.f,-640,312),FVector(100,12,12),I%2?TEXT("E5D7BB"):TEXT("C86C43"));
    Make(ECourierObstacleKind::Sweeper,2,FVector(36650,0,0),1050.f,7.5f,.25f,540.f);
    // Every turnstile stands in a wall from hedge to hedge: like a ticket gate,
    // the only way on is through it.
    for(const auto& O:Obstacles)if(O && O->Kind==ECourierObstacleKind::Sweeper)
    {
        const float Edge=O->OrderIndex==0?1250.f:3450.f;
        const float Inner=O->ArmLength+45.f;
        for(float Side:{-1.f,1.f})
        {
            const float Mid=Side*(Inner+Edge)*.5f,Length=Edge-Inner;
            Trim(W,FVector(O->Origin.X,Mid,O->Origin.Z+115.f),FVector(60,Length,230),TEXT("BB886F"))->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
            Trim(W,FVector(O->Origin.X,Mid,O->Origin.Z+238.f),FVector(76,Length+10.f,18),TEXT("E5D7BB"));
            Trim(W,FVector(O->Origin.X,Side*Inner,O->Origin.Z+130.f),FVector(70,70,260),TEXT("354F53"))->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
        }
    }
    Make(ECourierObstacleKind::Spring,2,FVector(39700,380,0),0.f,1.f,0.f);
    Make(ECourierObstacleKind::Shutter,2,FVector(42600,0,420),1100.f,5.5f,.6f);
}

void ACourierGameMode::KnockBack(const FVector& Push,const TCHAR* English,const TCHAR* Chinese)
{
    auto* Player=Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(this,0));
    if(!Player)return;
    Player->CancelMoves();
    Player->LaunchCharacter(Push,true,true);
    StunSeconds=.55f;CollisionProtection=1.2f;HitFlash=.55f;MoteChain=0;
    Sound(TEXT("Stamp"),.4f,.5f);
    if(Phase==ECourierPhase::Delivery && CurrentOrder==1){DamageLens(English,Chinese);return;}
    Notify(English,Chinese);
}

void ACourierGameMode::ShowPopup(const FString& Text)
{
    Popup=Text;PopupSeconds=1.2f;
}

void ACourierGameMode::CloseCall(int32 Bonus)
{
    Sound(TEXT("Borrow"),.3f,1.5f);
    if(Phase!=ECourierPhase::Delivery){ShowPopup(Copy(TEXT("CLOSE CALL!"),TEXT("好险！")));return;}
    ShowPopup(Copy(TEXT("CLOSE CALL!"),TEXT("好险！"))+FString::Printf(TEXT("  +%d"),Bonus));
    AwardMotes(Bonus);
}

void ACourierGameMode::AwardMotes(int32 Count)
{
    if(Phase!=ECourierPhase::Delivery)return;
    MoteBank+=Count;
    while(MoteBank>=MotesPerUse)
    {
        MoteBank-=MotesPerUse;++Charges;TokenFlash=.5f;
        ShowPopup(Copy(TEXT("+1 BORROWED TIME"),TEXT("+1 次借时间")));
        Sound(TEXT("Pickup"),.42f,1.9f);
    }
}

void ACourierGameMode::SpawnMotes()
{
    struct FLine {int32 Order;TArray<FVector> Points;bool bTrace;};
    TArray<FLine> Lines;
    Lines.Add({-1,{FVector(250,0,0),FVector(820,0,0)},true});
    for(int32 Order=0;Order<3;++Order)for(const auto& Leg:GuideLegs(Order))Lines.Add({Order,Leg,true});
    Lines.Add({2,{FVector(30650,-800,320),FVector(31700,-800,320)},true});   // market roof run
    {
        // The springboard's flight, drawn in sparks: launch, arc, terrace.
        TArray<FVector> Arc;
        for(int32 I=1;I<=9;++I)
        {
            const float T=I*.14f;
            Arc.Add(FVector(39700+820.f*T,380,88.f+1080.f*T-490.f*T*T-70.f));
        }
        Lines.Add({2,Arc,false});
    }
    FCollisionQueryParams Params(TEXT("CourierMotes"),false);
    if(auto* Player=UGameplayStatics::GetPlayerCharacter(this,0))Params.AddIgnoredActor(Player);
    for(const auto& Line:Lines)
    {
        TArray<FVector> Spots;
        if(!Line.bTrace)Spots=Line.Points;
        else
        {
            float Next=130.f;float Walked=0.f;
            for(int32 I=0;I+1<Line.Points.Num();++I)
            {
                const FVector A=Line.Points[I],B=Line.Points[I+1];
                const float Length=FVector::Dist(A,B);
                while(Next<=Walked+Length)
                {
                    Spots.Add(FMath::Lerp(A,B,(Next-Walked)/FMath::Max(1.f,Length)));
                    Next+=250.f;
                }
                Walked+=Length;
            }
        }
        for(FVector Spot:Spots)
        {
            bool bSkip=false;
            for(const auto& O:Obstacles)if(O && O->Kind!=ECourierObstacleKind::Shutter && FVector::Dist2D(O->Origin,Spot)<170.f)bSkip=true;
            if(bSkip)continue;
            if(Line.bTrace)
            {
                FHitResult Hit;
                if(!GetWorld()->LineTraceSingleByChannel(Hit,Spot+FVector(0,0,330),Spot-FVector(0,0,420),ECC_Visibility,Params) || Hit.ImpactNormal.Z<.7f)continue;
                Spot.Z=Hit.ImpactPoint.Z;
            }
            const FVector Base=Spot+FVector(0,0,75);
            auto* Actor=GetWorld()->SpawnActor<AActor>(Base,FRotator::ZeroRotator);
            auto* Root=NewObject<USceneComponent>(Actor);Actor->SetRootComponent(Root);Root->SetMobility(EComponentMobility::Movable);Root->RegisterComponent();
            Piece(Actor,Root,TEXT("Sphere"),FVector::ZeroVector,FVector(24,24,34),Hex(TEXT("F6C445")));
            Piece(Actor,Root,TEXT("Sphere"),FVector(0,0,0),FVector(30,10,14),Hex(TEXT("FFE9A6")));
            Actor->SetActorLocation(Base);Actor->SetActorHiddenInGame(true);
            Actor->Tags.Add(TEXT("CourierSpark"));
            Motes.Add({Actor,Base,Line.Order,false});
        }
    }
}

namespace
{
    bool MoteShown(const ACourierGameMode& Mode,const FCourierMote& Mote)
    {
        if(Mote.bTaken)return false;
        const ECourierPhase View=Mode.Phase==ECourierPhase::Paused?Mode.BeforePause:Mode.Phase;
        if(Mote.Order<0)return View==ECourierPhase::Practice;
        return Mote.Order==Mode.CurrentOrder && (View==ECourierPhase::Briefing || View==ECourierPhase::Delivery);
    }
}

void ACourierGameMode::UpdateMotes(float DeltaSeconds)
{
    PopupSeconds=FMath::Max(0.f,PopupSeconds-DeltaSeconds);
    MoteChainSeconds=FMath::Max(0.f,MoteChainSeconds-DeltaSeconds);
    if(MoteChainSeconds<=0.f)MoteChain=0;
    const auto* Player=UGameplayStatics::GetPlayerCharacter(this,0);
    const FVector P=Player?Player->GetActorLocation():FVector(1e9);
    const float Spin=GetWorld()->GetTimeSeconds()*180.f;
    for(int32 I=0;I<Motes.Num();++I)
    {
        auto& Mote=Motes[I];
        AActor* Actor=Mote.Actor.Get();if(!Actor)continue;
        const bool bShow=MoteShown(*this,Mote);
        if(Actor->IsHidden()==bShow)Actor->SetActorHiddenInGame(!bShow);
        if(!bShow)continue;
        FVector At=Actor->GetActorLocation();
        // Sparks drift to a courier who comes close, like coins to a magnet.
        if(CanMove() && FVector::Dist2D(At,P)<240.f && FMath::Abs(At.Z-P.Z)<190.f)
            At=FMath::VInterpConstantTo(At,P,DeltaSeconds,1500.f);
        else At=FVector(Mote.Base.X,Mote.Base.Y,Mote.Base.Z+FMath::Sin(GetWorld()->GetTimeSeconds()*3.f+I*.7f)*8.f);
        Actor->SetActorLocationAndRotation(At,FRotator(0,Spin+I*25.f,0));
    }
}

void ACourierGameMode::CollectMotes(const FVector& Position)
{
    for(auto& Mote:Motes)
    {
        AActor* Actor=Mote.Actor.Get();
        if(!Actor || !MoteShown(*this,Mote))continue;
        const FVector At=Actor->GetActorLocation();
        if(FVector::Dist2D(At,Position)>95.f || FMath::Abs(At.Z-Position.Z)>140.f)continue;
        Mote.bTaken=true;Actor->SetActorHiddenInGame(true);
        // A run of sparks climbs the scale, the way coin lines sing.
        MoteChain=MoteChainSeconds>0.f?MoteChain+1:0;MoteChainSeconds=.9f;
        Sound(TEXT("Pickup"),.17f,FMath::Min(2.4f,1.25f+MoteChain*.07f));
        AwardMotes(1);
    }
}

int32 ACourierGameMode::MotesTotal(int32 Order) const
{
    int32 Count=0;for(const auto& Mote:Motes)if(Mote.Order==Order)++Count;return Count;
}

int32 ACourierGameMode::MotesTaken(int32 Order) const
{
    int32 Count=0;for(const auto& Mote:Motes)if(Mote.Order==Order && Mote.bTaken)++Count;return Count;
}

void ACourierGameMode::ResetMotes(int32 Order)
{
    for(auto& Mote:Motes)if(Order==-2 || Mote.Order==Order)
    {
        Mote.bTaken=false;
        if(AActor* Actor=Mote.Actor.Get())Actor->SetActorLocation(Mote.Base);
    }
}
