#include "CourierPrototype.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/PointLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
    FLinearColor Color(const TCHAR* Hex){return FLinearColor::FromSRGBColor(FColor::FromHex(Hex));}
    UMaterialInstanceDynamic* Finish(UStaticMeshComponent* Mesh,const TCHAR* Hex)
    {
        auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Materials/M_CourierMatte.M_CourierMatte"));
        if(!Base)Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
        auto* M=UMaterialInstanceDynamic::Create(Base,Mesh);
        if(M){M->SetVectorParameterValue(TEXT("Color"),Color(Hex));Mesh->SetMaterial(0,M);}
        return M;
    }
    AStaticMeshActor* Block(UWorld* W,FVector At,FVector Size,const TCHAR* Hex,bool Solid=false,const TCHAR* Shape=TEXT("Cube"))
    {
        auto* A=W->SpawnActor<AStaticMeshActor>(At,FRotator::ZeroRotator);
        auto* M=A->GetStaticMeshComponent();M->SetMobility(EComponentMobility::Movable);
        M->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"),Shape,Shape)));
        M->SetWorldScale3D(Size/100.f);M->SetCollisionProfileName(Solid?TEXT("BlockAll"):TEXT("NoCollision"));
        Finish(M,Hex);return A;
    }
    struct FRailSpan {FVector A,B;float Width;bool Guard;};
    void Deck(UWorld* W,FVector At,FVector2D Size,const TCHAR* Hex=TEXT("C5BBA4"))
    {auto* Floor=Block(W,At-FVector(0,0,25),FVector(Size.X,Size.Y,50),Hex,true);Floor->Tags.Add(TEXT("CourierWalkable"));}
    void Road(UWorld* W,TArray<FRailSpan>& Spans,FVector A,FVector B,float Width=760,bool Rails=false)
    {
        const FVector D=B-A;
        auto* Floor=Block(W,(A+B)*.5f-FVector(0,0,25),FVector(D.Size()+80,Width,50),TEXT("C5BBA4"),true);
        Floor->SetActorRotation(D.Rotation());Floor->Tags.Add(TEXT("CourierWalkable"));
        Spans.Add({A,B,Width,Rails});
    }
    void Way(UWorld* W,TArray<FRailSpan>& Spans,std::initializer_list<FVector> Points,float Width=760,bool Rails=false)
    {
        TArray<FVector> Nodes;
        for(const auto& P:Points)Nodes.Add(P);
        for(int32 I=0;I<Nodes.Num();++I)
        {
            if(I>0)Road(W,Spans,Nodes[I-1],Nodes[I],Width,Rails);
            const bool RampBefore=I>0 && !FMath::IsNearlyEqual(Nodes[I-1].Z,Nodes[I].Z);
            const bool RampAfter=I+1<Nodes.Num() && !FMath::IsNearlyEqual(Nodes[I+1].Z,Nodes[I].Z);
            // A flat square over a sloped junction creates an unintended vertical lip.
            if(!RampBefore && !RampAfter)Deck(W,Nodes[I],FVector2D(Width,Width));
        }
    }
    void AddGuardrails(UWorld* W,const TArray<FRailSpan>& Spans)
    {
        // Evaluate the finished walkways before adding rails. A continuous rail on
        // each segment would put a gate across corners and side-path junctions.
        TArray<FTransform> Pieces,Curbs;
        FCollisionQueryParams Params;
        if(auto* Player=UGameplayStatics::GetPlayerCharacter(W,0))Params.AddIgnoredActor(Player);
        const auto Supported=[W,&Params](FVector P)
        {
            FHitResult Hit;
            return W->LineTraceSingleByChannel(Hit,P+FVector(0,0,120),P-FVector(0,0,100),ECC_Visibility,Params) &&
                Hit.GetActor() && Hit.GetActor()->ActorHasTag(TEXT("CourierWalkable")) && FMath::Abs(Hit.ImpactPoint.Z-P.Z)<45.f;
        };
        for(const auto& Span:Spans)
        {
            const FVector D=Span.B-Span.A;
            const FVector Side=FVector::CrossProduct(FVector::UpVector,D.GetSafeNormal2D());
            const int32 Count=FMath::Max(1,FMath::CeilToInt(D.Size()/240.f));
            for(float Sign:{-1.f,1.f})for(int32 I=0;I<Count;++I)
            {
                const FVector At=FMath::Lerp(Span.A,Span.B,(I+.5f)/Count)+Side*Sign*(Span.Width*.5f-15.f);
                if(Supported(At+Side*85.f) && Supported(At-Side*85.f))continue;
                Curbs.Add(FTransform(D.Rotation().Quaternion(),At+FVector(0,0,12),FVector(D.Size()/Count,22,24)/100.f));
                if(Span.Guard)Pieces.Add(FTransform(D.Rotation().Quaternion(),At+FVector(0,0,55),FVector(D.Size()/Count,20,30)/100.f));
            }
        }
        auto* Actor=W->SpawnActor<AActor>();Actor->Tags.Add(TEXT("CourierGuardrails"));
        auto* Mesh=NewObject<UInstancedStaticMeshComponent>(Actor);Actor->SetRootComponent(Mesh);
        Mesh->SetMobility(EComponentMobility::Movable);Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
        Mesh->SetCollisionProfileName(TEXT("BlockAll"));Mesh->SetCastShadow(false);Finish(Mesh,TEXT("438C95"));Mesh->RegisterComponent();
        for(const auto& Piece:Pieces)Mesh->AddInstance(Piece,true);
        auto* EdgeActor=W->SpawnActor<AActor>();
        auto* Edge=NewObject<UInstancedStaticMeshComponent>(EdgeActor);EdgeActor->SetRootComponent(Edge);
        Edge->SetMobility(EComponentMobility::Movable);Edge->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
        Edge->SetCollisionProfileName(TEXT("NoCollision"));Edge->SetCastShadow(false);Finish(Edge,TEXT("E5D7BB"));Edge->RegisterComponent();
        for(const auto& Curb:Curbs)Edge->AddInstance(Curb,true);
    }
    AActor* Plaque(UWorld* W,FVector At,const FString& Text,float Size=32)
    {
        Block(W,At+FVector(8,0,8),FVector(10,Text.Len()*Size*.62f+40,Size*1.9f),TEXT("243D43"));
        auto* A=W->SpawnActor<AActor>(At,FRotator(0,180,0));
        auto* T=NewObject<UTextRenderComponent>(A);A->SetRootComponent(T);T->RegisterComponent();
        T->SetWorldLocationAndRotation(At,FRotator(0,180,0));T->SetText(FText::FromString(Text));
        T->SetWorldSize(Size);T->SetHorizontalAlignment(EHTA_Center);T->SetTextRenderColor(FColor(245,235,207));
        T->SetTextMaterial(LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/EngineMaterials/UnlitText.UnlitText")));
        return A;
    }
    void Tree(UWorld* W,FVector At,float Scale=1)
    {
        Block(W,At+FVector(0,0,150*Scale),FVector(45,45,300)*Scale,TEXT("81634B"),false,TEXT("Cylinder"));
        Block(W,At+FVector(0,0,365*Scale),FVector(300,270,340)*Scale,TEXT("63856A"),false,TEXT("Sphere"));
        Block(W,At+FVector(-65,30,310)*Scale,FVector(210,230,260)*Scale,TEXT("9BAA7A"),false,TEXT("Sphere"));
    }
    void Pavilion(UWorld* W,FVector At,const FString& Name)
    {
        Block(W,At+FVector(280,0,125),FVector(150,650,250),TEXT("E5D7BB"),true);
        Block(W,At+FVector(170,0,85),FVector(65,650,170),TEXT("438C95"),true);
        Block(W,At+FVector(200,0,300),FVector(440,800,35),TEXT("C86C43"));
        for(float Y:{-340.f,340.f})Block(W,At+FVector(0,Y,140),FVector(20,20,280),TEXT("354F53"));
        Plaque(W,At+FVector(120,0,245),Name,30);
        Block(W,At+FVector(0,0,5),FVector(250,250,10),TEXT("C86C43"));
    }
}

void ACourierGameMode::BuildDeliveryDistricts()
{
    UWorld* W=GetWorld();
    TArray<FRailSpan> RailSpans;
    // Grass is real ground. Only visibly marked ponds are holes; instances keep
    // this broad campus terrain to one component instead of thousands of actors.
    auto* Terrain=W->SpawnActor<AActor>();
    auto* Ground=NewObject<UInstancedStaticMeshComponent>(Terrain);
    Terrain->SetRootComponent(Ground);
    Ground->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
    Ground->SetMobility(EComponentMobility::Movable);Ground->SetCollisionProfileName(TEXT("BlockAll"));Ground->SetCastShadow(false);
    Ground->SetCanEverAffectNavigation(false);Ground->RegisterComponent();Finish(Ground,TEXT("8FA17B"));
    const FBox2D Ponds[]={FBox2D(FVector2D(11500,-1600),FVector2D(13800,1600)),
        FBox2D(FVector2D(18500,-1400),FVector2D(20850,1400)),FBox2D(FVector2D(22900,-1600),FVector2D(23850,1600)),
        FBox2D(FVector2D(34300,-1500),FVector2D(35200,1500)),FBox2D(FVector2D(40050,-1700),FVector2D(41050,1700))};
    for(float X=-200;X<48000;X+=400)for(float Y=-3800;Y<=3800;Y+=400)
    {
        bool Water=false;const FBox2D Cell(FVector2D(X-200,Y-200),FVector2D(X+200,Y+200));
        for(const auto& Pond:Ponds)if(Cell.Intersect(Pond)){Water=true;break;}
        if(!Water)Ground->AddInstance(FTransform(FRotator::ZeroRotator,FVector(X,Y,-28),FVector(4,4,.3f)),true);
    }
    // Routes share stations. All side routes are walkable alternatives, not dead ends.
    Way(W,RailSpans,{FVector(2800,0,0),FVector(3050,850,0),FVector(7400,850,0),FVector(7700,0,0)},450);
    Plaque(W,FVector(2860,440,160),TEXT("ARCADE / WALK"),25);
    Block(W,FVector(4600,0,25),FVector(110,380,50),TEXT("BB886F"),true);
    Block(W,FVector(4700,0,42.5f),FVector(90,300,85),TEXT("BB886F"),true);

    // Garden: approach, low boarding dock, upper garden, stepping stones, final lift.
    Block(W,FVector(17000,0,-100),FVector(19000,10500,60),TEXT("8FA17B"));
    Way(W,RailSpans,{FVector(8000,0,0),FVector(8000,1100,0),FVector(9200,1100,0),FVector(10500,1100,0),FVector(11700,0,0),FVector(12200,0,0)},850);
    Deck(W,FVector(12200,0,0),FVector2D(500,950));
    Way(W,RailSpans,{FVector(13300,0,320),FVector(14000,0,320),FVector(16400,0,320)},900,true);
    // The alternative is longer, but broad, continuous and never requires precise jumps.
    Way(W,RailSpans,{FVector(12000,0,0),FVector(12200,1900,0),FVector(14100,1900,320),FVector(14800,1900,320),FVector(14800,0,320)},650,true);
    Plaque(W,FVector(11900,-550,180),TEXT("LIFT / Q"));
    Plaque(W,FVector(12000,650,180),TEXT("RAMP / WALK"));
    Block(W,FVector(12700,0,-65),FVector(2600,3800,30),TEXT("709EA4"));
    Way(W,RailSpans,{FVector(16400,0,320),FVector(18100,-1400,0),FVector(18300,0,0)},850);
    Deck(W,FVector(18300,0,0),FVector2D(600,1000));
    for(float X:{19000.f,19700.f,20400.f})
    {
        Deck(W,FVector(X,0,0),FVector2D(450,800),TEXT("E5D7BB"));
        Block(W,FVector(X,0,3),FVector(75,75,6),TEXT("C86C43"));
    }
    Way(W,RailSpans,{FVector(21100,0,0),FVector(22500,0,0),FVector(22900,0,0)},900);
    Way(W,RailSpans,{FVector(18200,0,0),FVector(18400,-1900,0),FVector(21300,-1900,0),FVector(21600,0,0)},700,true);
    Block(W,FVector(19700,0,-70),FVector(3500,5000,30),TEXT("709EA4"));
    Plaque(W,FVector(18250,520,170),TEXT("STONES / JUMP"),28);
    Plaque(W,FVector(18150,-650,170),TEXT("GARDEN WALK"),28);
    Way(W,RailSpans,{FVector(24100,0,400),FVector(24800,0,400),FVector(26000,0,400)},1000,true);
    Way(W,RailSpans,{FVector(22500,0,0),FVector(22800,2100,0),FVector(24900,2100,400),FVector(25700,2100,400),FVector(25700,0,400)},700,true);
    Block(W,FVector(23450,0,-70),FVector(2200,4500,30),TEXT("709EA4"));
    Pavilion(W,FVector(26000,0,400),TEXT("PROJECTION BOOTH"));
    Plaque(W,FVector(23900,-600,540),TEXT("RIDE UP / STEP OFF"),26);

    const auto Lift=[this,W](FVector Origin,FVector Travel,int32 Order,float Period=9.f)
    {
        auto* P=W->SpawnActor<ACourierLift>(Origin,FRotator::ZeroRotator);
        P->Origin=Origin;P->Travel=Travel;P->OrderIndex=Order;P->Period=Period;P->Reset();Platforms.Add(P);
        if(auto* Player=UGameplayStatics::GetPlayerCharacter(this,0))
            Player->GetCharacterMovement()->AddTickPrerequisiteActor(P);
    };
    Lift(FVector(12600,0,-20),FVector(0,0,320),1,9);
    Lift(FVector(23300,0,-20),FVector(0,0,400),1,10);

    // Last reel: market -> horizontal shuttle -> cooldown choices -> final terrace.
    Block(W,FVector(37000,0,-110),FVector(23000,10500,60),TEXT("A8AB86"));
    Way(W,RailSpans,{FVector(26000,0,400),FVector(26000,1100,400),FVector(27400,1100,400),FVector(28900,1100,0),FVector(30000,0,0),FVector(32700,0,0),FVector(33900,0,0)},1050);
    Way(W,RailSpans,{FVector(30100,0,0),FVector(30200,1450,0),FVector(32300,1450,0),FVector(32700,0,0)},650);
    Plaque(W,FVector(30050,650,180),TEXT("MARKET ARCADE"),28);
    Deck(W,FVector(34000,0,0),FVector2D(600,1050));
    Way(W,RailSpans,{FVector(35700,0,0),FVector(36300,0,0),FVector(38900,0,0)},1050);
    Lift(FVector(34450,0,-20),FVector(900,0,0),2,10);
    Way(W,RailSpans,{FVector(33900,0,0),FVector(34100,-1900,0),FVector(36000,-1900,0),FVector(36300,0,0)},700,true);
    Block(W,FVector(34750,0,-70),FVector(2400,5000,30),TEXT("709EA4"));
    Plaque(W,FVector(33950,600,170),TEXT("SHUTTLE / Q"));
    Way(W,RailSpans,{FVector(36900,0,0),FVector(37000,1450,0),FVector(38700,1450,0),FVector(38900,0,0)},650);
    Way(W,RailSpans,{FVector(38900,0,0),FVector(39600,-1100,0),FVector(39800,0,0)},900);
    Deck(W,FVector(39900,0,0),FVector2D(650,1050));
    Lift(FVector(40300,0,-20),FVector(0,0,420),2,10);
    Way(W,RailSpans,{FVector(41400,0,420),FVector(42000,0,420),FVector(43200,0,420),FVector(46000,0,420)},1100,true);
    Way(W,RailSpans,{FVector(39700,0,0),FVector(39900,2200,0),FVector(42200,2200,420),FVector(43000,2200,420),FVector(43000,0,420)},750,true);
    Block(W,FVector(40400,0,-70),FVector(2000,5000,30),TEXT("709EA4"));
    Plaque(W,FVector(39700,650,180),TEXT("TERRACE RAMP"),28);
    // One small sequence of static planters teaches directional movement on the upper route.
    for(int32 I=0;I<3;++I)
        Block(W,FVector(43200+I*580,(I%2?1:-1)*280,470),FVector(160,530,100),TEXT("BB886F"),true);
    Deck(W,FVector(45900,0,420),FVector2D(2400,3000));
    // A roofless console preserves the screen sightline above the left aisle.
    Block(W,FVector(46220,-950,505),FVector(150,650,170),TEXT("354F53"),true);
    Block(W,FVector(46135,-950,590),FVector(40,610,25),TEXT("E5D7BB"),true);
    Plaque(W,FVector(46110,-950,690),TEXT("CLOCKTOWER CONSOLE"),26);
    Block(W,FVector(46000,-950,425),FVector(250,250,10),TEXT("C86C43"));

    const auto Cart=[this,W](FVector At,int32 Order,float CartPhase,float Period)
    {
        auto* C=W->SpawnActor<ACourierCart>(At,FRotator::ZeroRotator);C->Origin=At;
        C->OrderIndex=Order;C->InitialPhase=CartPhase;C->Period=Period;C->Reset();Carts.Add(C);
        Block(W,At-FVector(0,0,52),FVector(250,1000,6),TEXT("829C99"));
    };
    Cart(FVector(14800,0,375),1,.4f,6);
    Cart(FVector(15700,0,375),1,2.2f,7);
    Cart(FVector(30700,0,55),2,.2f,6);
    Cart(FVector(31600,0,55),2,2.f,7);
    Cart(FVector(37400,0,55),2,.7f,5);
    Cart(FVector(38300,0,55),2,2.5f,6);

    // Distinct districts share a quiet, restrained material palette.
    for(int32 I=0;I<16;++I)
    {
        const float X=9800+I*2200.f;
        const float Y=I%2?2850.f:-2850.f;
        const float Z=X>41000?420.f:0.f;
        Tree(W,FVector(X,Y,Z),.95f+(I%3)*.12f);
        if(I%3==0)
        {
            Block(W,FVector(X,Y*1.3f,Z+250),FVector(700,700,500),I%2?TEXT("BB886F"):TEXT("E5D7BB"));
            Block(W,FVector(X,Y*1.3f,Z+510),FVector(760,750,40),TEXT("354F53"));
            for(float Offset:{-210.f,0.f,210.f})
                Block(W,FVector(X+Offset,Y*1.3f+(Y>0?-360:360),Z+290),FVector(110,20,170),TEXT("9DBCB5"));
        }
    }
    Plaque(W,FVector(16900,700,260),TEXT("NO BORROWING TIME IN EXAMS"),24);

    // Movie night: a quiet screen, projector and rows of seats. Geometry is original.
    Block(W,FVector(47400,0,1350),FVector(100,2500,1500),TEXT("354F53"));
    Block(W,FVector(47340,0,1350),FVector(18,2290,1290),TEXT("C2C6B7"));
    Block(W,FVector(44800,-1800,650),FVector(200,200,170),TEXT("354F53"));
    for(int32 Row=0;Row<3;++Row)for(int32 Seat=0;Seat<6;++Seat)
    {
        const FVector At(45100+Row*380,-1000+Seat*400,420);
        Block(W,At+FVector(0,0,55),FVector(90,110,15),TEXT("B17B50"));
        Block(W,At+FVector(-40,0,100),FVector(15,110,95),TEXT("B17B50"));
        for(float Y:{-40.f,40.f})Block(W,At+FVector(0,Y,25),FVector(15,15,50),TEXT("354F53"));
    }
    auto* ProjectorLamp=W->SpawnActor<APointLight>(FVector(44800,-1800,800),FRotator::ZeroRotator);
    if(auto* Light=ProjectorLamp->FindComponentByClass<UPointLightComponent>())
    {Light->SetIntensity(18000);Light->SetAttenuationRadius(1100);Light->SetLightColor(Color(TEXT("FFF0D4")));}
    ProjectionActors.Add(ProjectorLamp);
    ProjectionActors.Add(Block(W,FVector(47310,0,1350),FVector(14,2220,1220),TEXT("243D43")));
    // The final frame is a little moonlit landscape, made with flat low-poly shapes.
    FinaleActors.Add(Block(W,FVector(47290,550,1650),FVector(20,330,330),TEXT("F1D9A0"),false,TEXT("Sphere")));
    for(int32 I=0;I<9;++I)
        FinaleActors.Add(Block(W,FVector(47280,-940+I*230,1500+(I%3)*170),FVector(12,14,14),TEXT("F4EBD9"),false,TEXT("Sphere")));
    for(int32 I=0;I<5;++I)
    {
        auto* Hill=Block(W,FVector(47280,-800+I*400,950),FVector(18,600,400+(I%3)*90),I%2?TEXT("438C95"):TEXT("63856A"),false,TEXT("Sphere"));
        FinaleActors.Add(Hill);
    }
    for(float Y:{-1400.f,1400.f})for(float X:{44400.f,45800.f,47100.f})
    {
        Block(W,FVector(X,Y,750),FVector(24,24,660),TEXT("354F53"));
        auto* Bulb=Block(W,FVector(X,Y,1100),FVector(85),TEXT("F1D9A0"),false,TEXT("Sphere"));
        FinaleActors.Add(Bulb);
    }
    for(const auto& Actor:ProjectionActors)if(Actor)Actor->Tags.Add(TEXT("CourierProjection"));
    for(const auto& Actor:FinaleActors)if(Actor)Actor->Tags.Add(TEXT("CourierFinale"));

    AddGuardrails(W,RailSpans);
    for(const auto& Span:RailSpans)
    {
        const float X=(Span.A.X+Span.B.X)*.5f;
        AddRouteGuideSpan(X<7900.f?0:(X<25999.f?1:2),Span.A,Span.B);
    }

    const auto Atmosphere=[W](FVector At,FVector Size,const TCHAR* MaterialName,const TCHAR* Tint=nullptr)
    {
        auto* Cloud=Block(W,At,Size,TEXT("FFF1DE"),false,TEXT("Sphere"));
        auto* M=Cloud->GetStaticMeshComponent();M->SetCastShadow(false);
        const FString Path=FString::Printf(TEXT("/Game/Materials/%s.%s"),MaterialName,MaterialName);
        if(auto* Base=LoadObject<UMaterialInterface>(nullptr,*Path))
        {
            auto* Instance=UMaterialInstanceDynamic::Create(Base,M);
            if(Tint)Instance->SetVectorParameterValue(TEXT("Color"),Color(Tint));
            M->SetMaterial(0,Instance);
        }
        return Cloud;
    };
    Atmosphere(FVector(40000,-18000,8000),FVector(3700),TEXT("M_CourierSun"),TEXT("FFE4AF"));
    const FVector Clouds[]={FVector(7000,-13000,7300),FVector(17000,15000,9500),FVector(29000,-21000,10800),
        FVector(38000,17000,11500),FVector(48000,-15000,8500),FVector(58000,14000,10500)};
    for(const auto& At:Clouds)
    {
        Atmosphere(At,FVector(2600,4800,1700),TEXT("M_CourierCloud"));
        Atmosphere(At+FVector(100,2100,330),FVector(2100,3100,1500),TEXT("M_CourierCloud"));
        Atmosphere(At+FVector(-50,-1700,200),FVector(2100,3300,1400),TEXT("M_CourierCloud"));
    }
    for(int32 I=0;I<8;++I)
        Atmosphere(FVector(61000+(I%2)*9000,-31000+I*9500,1100),FVector(14000,17000,5500+(I%3)*1500),TEXT("M_CourierSun"),I%2?TEXT("B6B4C0"):TEXT("9CAAB9"));
    auto* Haze=W->SpawnActor<AExponentialHeightFog>();
    if(auto* Fog=Haze->FindComponentByClass<UExponentialHeightFogComponent>())
    {
        Fog->SetVolumetricFog(false);Fog->SetFogDensity(.0008f);Fog->SetStartDistance(8000.f);
        Fog->SetFogMaxOpacity(.18f);Fog->SetFogInscatteringColor(Color(TEXT("DBC7BF")));
    }
}

void ACourierGameMode::BuildRouteGuide()
{
    for(int32 I=0;I<4;++I)
    {
        auto* Group=GetWorld()->SpawnActor<AActor>();
        Group->Tags.Add(TEXT("CourierRouteGuide"));
        Group->Tags.Add(I==0?FName(TEXT("CourierRoutePractice")):FName(*FString::Printf(TEXT("CourierRoute%d"),I-1)));
        for(int32 Layer=0;Layer<2;++Layer)
        {
            auto* Mesh=NewObject<UInstancedStaticMeshComponent>(Group);
            if(Layer==0)Group->SetRootComponent(Mesh);else Mesh->SetupAttachment(Group->GetRootComponent());
            Mesh->ComponentTags.Add(Layer==0?TEXT("RouteOutline"):TEXT("RoutePaint"));
            Mesh->SetMobility(EComponentMobility::Movable);
            Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
            Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Mesh->SetCanEverAffectNavigation(false);Mesh->SetCastShadow(false);Mesh->RegisterComponent();
            Finish(Mesh,Layer==0?TEXT("FFF0CB"):TEXT("D45B20"));
        }
        RouteGuideGroups.Add(Group);
    }
    RouteBeacon=GetWorld()->SpawnActor<AActor>();RouteBeacon->Tags.Add(TEXT("CourierNextMarker"));
    auto* Root=NewObject<USceneComponent>(RouteBeacon);RouteBeacon->SetRootComponent(Root);Root->RegisterComponent();
    const auto Part=[this](FVector At,FVector Size,const TCHAR* Hex,const TCHAR* Shape)
    {
        auto* A=Block(GetWorld(),At,Size,Hex,false,Shape);
        A->GetStaticMeshComponent()->SetCastShadow(false);
        A->AttachToActor(RouteBeacon,FAttachmentTransformRules::KeepRelativeTransform);return A;
    };
    // A stationary high-contrast ring and downward pointer identify the next waypoint.
    // Every piece is visual only, so it cannot introduce another invisible roadblock.
    Part(FVector(0,0,-94),FVector(190,190,4),TEXT("FFF0CB"),TEXT("Cylinder"));
    Part(FVector(0,0,-91),FVector(158,158,4),TEXT("D45B20"),TEXT("Cylinder"));
    Part(FVector(0,0,-88),FVector(105,105,4),TEXT("FFF0CB"),TEXT("Cylinder"));
    Part(FVector(0,0,95),FVector(80,80,100),TEXT("ED762C"),TEXT("Cone"))->SetActorRelativeRotation(FRotator(180,0,0));
    Part(FVector(0,0,155),FVector(18,18,60),TEXT("FFF0CB"),TEXT("Cylinder"));
}

void ACourierGameMode::AddRouteGuideSpan(int32 Order,const FVector& A,const FVector& B)
{
    if(!RouteGuideGroups.IsValidIndex(Order+1))return;
    auto* Group=RouteGuideGroups[Order+1].Get();
    UInstancedStaticMeshComponent* Outline=nullptr;UInstancedStaticMeshComponent* Paint=nullptr;
    TArray<UInstancedStaticMeshComponent*> Meshes;Group->GetComponents(Meshes);
    for(auto* M:Meshes){if(M->ComponentHasTag(TEXT("RoutePaint")))Paint=M;else Outline=M;}
    if(!Outline || !Paint)return;
    const FVector Delta=B-A;const float Length=Delta.Size();if(Length<1.f)return;
    const FQuat Rotation=Delta.Rotation().Quaternion();
    const auto Piece=[&](FVector At,FVector Size,float Yaw=0.f)
    {
        const FQuat R=Rotation*FRotator(0,Yaw,0).Quaternion();
        Outline->AddInstance(FTransform(R,At+FVector(0,0,7),FVector(Size.X+10,Size.Y+12,3)/100.f),true);
        Paint->AddInstance(FTransform(R,At+FVector(0,0,10),FVector(Size.X,Size.Y,3)/100.f),true);
    };
    Piece((A+B)*.5f,FVector(Length,18,3));
    // Broad arrows at a calm five-metre spacing also show the intended direction.
    const int32 Count=FMath::Max(1,FMath::FloorToInt(Length/500.f));
    for(int32 I=0;I<Count;++I)
    {
        const FVector At=FMath::Lerp(A,B,(I+.5f)/Count);
        Piece(At+Rotation.RotateVector(FVector(-53,0,0)),FVector(85,18,3));
        Piece(At+Rotation.RotateVector(FVector(-10,22,0)),FVector(70,18,3),-40.f);
        Piece(At+Rotation.RotateVector(FVector(-10,-22,0)),FVector(70,18,3),40.f);
    }
}

void ACourierGameMode::UpdateRouteGuide()
{
    const ECourierPhase ViewPhase=Phase==ECourierPhase::Paused?BeforePause:Phase;
    const bool bShow=ViewPhase==ECourierPhase::Practice || ViewPhase==ECourierPhase::AwaitPickup ||
        ViewPhase==ECourierPhase::Briefing || ViewPhase==ECourierPhase::Delivery;
    const int32 Active=ViewPhase==ECourierPhase::Practice?0:CurrentOrder+1;
    for(int32 I=0;I<RouteGuideGroups.Num();++I)if(RouteGuideGroups[I])
        RouteGuideGroups[I]->SetActorHiddenInGame(!bShow || I!=Active);
    if(RouteBeacon)
    {
        RouteBeacon->SetActorHiddenInGame(!bShow);
        // Pause freezes the existing waypoint instead of resolving a different route.
        if(bShow && Phase!=ECourierPhase::Paused)RouteBeacon->SetActorLocation(NavigationTarget());
        TArray<AActor*> Parts;RouteBeacon->GetAttachedActors(Parts);
        for(auto* Part:Parts)Part->SetActorHiddenInGame(!bShow);
    }
}

ACourierLift::ACourierLift()
{
    PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickGroup=TG_PrePhysics;
    Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TimePlatform"));SetRootComponent(Mesh);
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
    Mesh->SetRelativeScale3D(FVector(6.5f,8.f,.4f));Mesh->SetCollisionProfileName(TEXT("BlockAll"));
    Mesh->CanCharacterStepUpOn=ECB_Yes;
}

void ACourierLift::BeginPlay()
{
    Super::BeginPlay();Material=Finish(Mesh,TEXT("438C95"));
    // A visible clock face identifies the device without relying on colour alone.
    auto* Mark=NewObject<UStaticMeshComponent>(this);Mark->SetupAttachment(Mesh);
    Mark->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
    Mark->SetRelativeLocation(FVector(0,0,58));Mark->SetRelativeScale3D(FVector(.24f,.195f,.2f));
    Mark->SetCollisionProfileName(TEXT("NoCollision"));Mark->RegisterComponent();Finish(Mark,TEXT("F4EBD9"));
    for(int32 I=0;I<2;++I)
    {
        auto* Hand=NewObject<UStaticMeshComponent>(this);Hand->SetupAttachment(Mesh);
        Hand->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
        Hand->SetRelativeLocation(FVector(I?3.f:0.f,I?0.f:3.f,80));
        Hand->SetRelativeScale3D(I?FVector(.09f,.012f,.1f):FVector(.014f,.075f,.1f));
        Hand->SetCollisionProfileName(TEXT("NoCollision"));Hand->RegisterComponent();Finish(Hand,TEXT("243D43"));
        if(I==0)ClockHand=Hand;
    }
}

void ACourierLift::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const auto* Mode=Cast<ACourierGameMode>(UGameplayStatics::GetGameMode(this));
    if(!Mode)return;
    const bool Active=Mode->IsPlaying() && Mode->CurrentOrder==OrderIndex && Mode->RecoverySeconds<=0;
    if(Material)Material->SetVectorParameterValue(TEXT("Color"),Color(Active && Mode->bSlowing?TEXT("8EC8CD"):TEXT("438C95")));
    if(!Active)return;
    Clock+=DeltaSeconds*Mode->DeviceRate();
    if(ClockHand)ClockHand->SetRelativeRotation(FRotator(0,Clock*360.f/Period,0));
    const float Phase=.5f-.5f*FMath::Cos(Clock*2.f*PI/Period);
    SetActorLocation(Origin+Travel*Phase,false,nullptr,ETeleportType::None);
    if(Material)Material->SetVectorParameterValue(TEXT("Color"),Color(Mode->bSlowing?TEXT("8EC8CD"):TEXT("438C95")));
}

void ACourierLift::Reset(){Clock=0.f;SetActorLocation(Origin,false,nullptr,ETeleportType::TeleportPhysics);}
