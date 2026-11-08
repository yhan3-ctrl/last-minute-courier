#include "CourierCampusScene.h"
#include "CourierPrototype.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    void PaintPart(UStaticMeshComponent* Mesh,const TCHAR* Hex)
    {
        auto* Base=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Materials/M_CourierMatte.M_CourierMatte"));
        if(auto* Material=UMaterialInstanceDynamic::Create(Base,Mesh))
        {
            Material->SetVectorParameterValue(TEXT("Color"),FLinearColor::FromSRGBColor(FColor::FromHex(Hex)));
            Mesh->SetMaterial(0,Material);
        }
    }
    UStaticMesh* Basic(const TCHAR* Shape)
    {return LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"),Shape,Shape));}
}

ACourierCampusScene::ACourierCampusScene()
{
    PrimaryActorTick.bCanEverTick=true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("CampusLife")));
    Tags.Add(TEXT("CourierCampusPresentation"));
}

USceneComponent* ACourierCampusScene::Group(FVector At,FRotator Rotation)
{
    auto* Node=NewObject<USceneComponent>(this);Parts.Add(Node);
    Node->SetupAttachment(RootComponent);Node->SetMobility(EComponentMobility::Movable);
    Node->SetRelativeLocationAndRotation(At,Rotation);Node->RegisterComponent();return Node;
}

UStaticMeshComponent* ACourierCampusScene::Part(USceneComponent* Parent,const TCHAR* Shape,FVector At,FVector Size,const TCHAR* Hex,FRotator Rotation)
{
    auto* Mesh=NewObject<UStaticMeshComponent>(this);Parts.Add(Mesh);
    Mesh->SetupAttachment(Parent);Mesh->SetMobility(EComponentMobility::Movable);Mesh->SetStaticMesh(Basic(Shape));
    Mesh->SetRelativeLocationAndRotation(At,Rotation);Mesh->SetRelativeScale3D(Size/100.f);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);Mesh->SetCanEverAffectNavigation(false);
    Mesh->SetCullDistance(18000.f);Mesh->RegisterComponent();PaintPart(Mesh,Hex);return Mesh;
}

void ACourierCampusScene::Decor(const TCHAR* Shape,FVector At,FVector Size,const TCHAR* Hex,FRotator Rotation)
{
    const FString Key=FString(Shape)+Hex;
    auto*& Batch=Batches.FindOrAdd(Key);
    if(!Batch)
    {
        Batch=NewObject<UInstancedStaticMeshComponent>(this);Parts.Add(Batch);
        Batch->SetupAttachment(RootComponent);Batch->SetMobility(EComponentMobility::Movable);Batch->SetStaticMesh(Basic(Shape));
        Batch->SetCollisionEnabled(ECollisionEnabled::NoCollision);Batch->SetCanEverAffectNavigation(false);
        Batch->SetCullDistances(18000,22000);Batch->RegisterComponent();PaintPart(Batch,Hex);
    }
    Batch->AddInstance(FTransform(Rotation,At,Size/100.f));
}

void ACourierCampusScene::Line(FVector A,FVector B,float Width,const TCHAR* Hex)
{Decor(TEXT("Cylinder"),(A+B)*.5f,FVector(Width,Width,(B-A).Size()),Hex,FRotationMatrix::MakeFromZ(B-A).Rotator());}

void ACourierCampusScene::Cup(FVector At)
{
    auto* Node=Group(At);CoffeeService.Add(Node);
    Part(Node,TEXT("Cylinder"),FVector(0,0,13),FVector(22,22,26),TEXT("FFF0D4"));
    Part(Node,TEXT("Cylinder"),FVector(0,0,27),FVector(19,19,2),TEXT("674634"));
    Part(Node,TEXT("Cube"),FVector(13,0,14),FVector(9,5,14),TEXT("FFF0D4"));
    for(int32 I=0;I<3;++I)
        Steam.Add(Part(Node,TEXT("Sphere"),FVector(0,0,35+I*10),FVector(5),TEXT("FFF2DE")));
}

void ACourierCampusScene::FlowerPot(FVector At,float Scale)
{
    Decor(TEXT("Cylinder"),At+FVector(0,0,24*Scale),FVector(65,65,48)*Scale,TEXT("C97D58"));
    Decor(TEXT("Cylinder"),At+FVector(0,0,45*Scale),FVector(76,76,12)*Scale,TEXT("DE9C72"));
    Decor(TEXT("Sphere"),At+FVector(0,0,64*Scale),FVector(78,64,45)*Scale,TEXT("77966A"));
    for(int32 I=0;I<5;++I)
    {
        const float Angle=I*2.4f;
        const FVector Tip=At+FVector(FMath::Cos(Angle)*23,FMath::Sin(Angle)*23,85+(I%2)*13)*Scale;
        Line(At+FVector(0,0,48*Scale),Tip,3*Scale,TEXT("577B55"));
        Decor(TEXT("Sphere"),Tip,FVector(18,18,12)*Scale,I%2?TEXT("F1BD70"):TEXT("E99B91"));
    }
}

void ACourierCampusScene::Guest(FVector At,float Yaw,const TCHAR* Shirt,const TCHAR* Skin,int32 Stage,bool Seated)
{
    auto* Root=Group(At,FRotator(0,Yaw,0));
    const float Torso=Seated?80.f:92.f;
    const float Head=Seated?136.f:148.f;
    Part(Root,TEXT("Cylinder"),FVector(0,0,Torso),FVector(45,43,64),Shirt);
    Part(Root,TEXT("Sphere"),FVector(0,0,Head),FVector(43),Skin);
    Part(Root,TEXT("Sphere"),FVector(-3,0,Head+13),FVector(45,46,23),Stage==2?TEXT("564941"):TEXT("4A3E38"));
    for(float Side:{-1.f,1.f})
    {
        Part(Root,TEXT("Sphere"),FVector(20,Side*8,Head+2),FVector(4),TEXT("243D43"));
        if(Seated)
        {
            Part(Root,TEXT("Cube"),FVector(16,Side*12,54),FVector(42,18,18),TEXT("475467"));
            Part(Root,TEXT("Cylinder"),FVector(36,Side*12,28),FVector(18,18,39),TEXT("475467"));
        }
        else Part(Root,TEXT("Cylinder"),FVector(0,Side*12,32),FVector(18,18,53),TEXT("475467"));
        Part(Root,TEXT("Cube"),FVector(Seated?42.f:5.f,Side*12,8),FVector(30,22,15),TEXT("F2E5CE"));
    }
    Part(Root,TEXT("Cube"),FVector(21,0,Head-9),FVector(2,9,2),TEXT("825C48"));
    Part(Root,TEXT("Cylinder"),FVector(0,-28,Torso-10),FVector(14,14,45),Shirt);
    auto* Arm=NewObject<USceneComponent>(this);Parts.Add(Arm);Arm->SetupAttachment(Root);
    Arm->SetRelativeLocation(FVector(0,28,Torso+18));Arm->RegisterComponent();
    Part(Arm,TEXT("Cylinder"),FVector(0,0,-18),FVector(15,15,36),Shirt);
    Part(Arm,TEXT("Sphere"),FVector(0,0,-39),FVector(18),Skin);
    auto* Mug=Part(Arm,TEXT("Cylinder"),FVector(13,0,-38),FVector(18,18,24),TEXT("FFF0D4"));
    Guests.Add({Root,Arm,Mug,Stage,float(Guests.Num())*.8f});
    if(Stage==0 && At.X>8100 && At.X<8400)
    {
        Part(Root,TEXT("Cube"),FVector(23,0,79),FVector(4,32,52),TEXT("F3DEC1"));
        Part(Root,TEXT("Cylinder"),FVector(0,0,Head+23),FVector(49,49,12),TEXT("F3DEC1"));
    }
}

void ACourierCampusScene::BeginPlay()
{
    Super::BeginPlay();
    // Short, distinct waiting pads indicate a safe place to read each crossing.
    for(float X:{650.f,1240.f,2000.f,2940.f,4890.f})
    {
        Decor(TEXT("Cylinder"),FVector(X,0,4),FVector(200,200,5),TEXT("EBCB91"));
        // A pair of shoe prints (sole + heel) pointing down the route. Two plain
        // ovals read as a giant face from the chase camera.
        for(float Y:{-19.f,19.f})
        {
            Decor(TEXT("Sphere"),FVector(X+12,Y,8),FVector(34,15,2),TEXT("B4805C"));
            Decor(TEXT("Sphere"),FVector(X-19,Y,8),FVector(17,13,2),TEXT("B4805C"));
        }
    }
    // A striped awning and a cup silhouette identify the destination before text is readable.
    for(int32 I=0;I<9;++I)
        Decor(TEXT("Cube"),FVector(8350,-445+I*111.f,370),FVector(500,53,8),TEXT("FFF0D4"));
    Decor(TEXT("Cylinder"),FVector(8300,0,443),FVector(105,105,116),TEXT("FFF0D4"));
    Decor(TEXT("Cylinder"),FVector(8300,0,503),FVector(93,93,4),TEXT("674634"));
    Decor(TEXT("Cube"),FVector(8300,67,445),FVector(18,45,56),TEXT("FFF0D4"));
    Decor(TEXT("Cube"),FVector(8300,78,445),FVector(22,17,27),TEXT("C97D58"));
    Guest(FVector(8280,-175,0),180,TEXT("B66F4B"),TEXT("D4A17B"),0);
    Guest(FVector(7440,-1040,-13),60,TEXT("7C8E9E"),TEXT("BE875D"),0);
    Guest(FVector(7690,-1040,-13),115,TEXT("AA7B8A"),TEXT("E5BB96"),0);
    for(float X:{7500.f,7630.f})
    {
        Decor(TEXT("Cylinder"),FVector(X,-930,62),FVector(115,115,12),TEXT("E8CDA5"));
        Decor(TEXT("Cylinder"),FVector(X,-930,22),FVector(12,12,80),TEXT("6C6658"));
        Cup(FVector(X,-930,69));
    }
    Cup(FVector(8150,-230,110));Cup(FVector(8150,10,110));
    for(const FVector& At:{FVector(7840,-1080,-13),FVector(8480,-650,-13),FVector(8480,690,-13),FVector(6970,1110,0)})FlowerPot(At,1.5f);
    for(float X:{6950.f,7610.f})
    {
        for(float Y:{-550.f,550.f})Decor(TEXT("Cylinder"),FVector(X,Y,205),FVector(13,13,410),TEXT("6B6653"));
        for(int32 I=0;I<8;++I)
        {
            const float A=-550+I*110.f,B=A+110;
            const auto Height=[](float Y){return 402.f-50.f*(1.f-FMath::Square(Y/550.f));};
            Line(FVector(X,A,Height(A)),FVector(X,B,Height(B)),5,TEXT("6B6653"));
            Decor(TEXT("Cube"),FVector(X,A+55,Height(A+55)-27),FVector(4,38,46),I%2?TEXT("D88C63"):TEXT("F0C875"),FRotator(0,0,8));
        }
    }
    // Flower arches give the raised garden a distinct silhouette without obscuring the lifts.
    for(float X:{13900.f,15350.f,16300.f})
    {
        for(float Y:{-690.f,690.f})Line(FVector(X,Y,320),FVector(X,Y,810),15,TEXT("EAD8B9"));
        for(int32 I=0;I<12;++I)
        {
            const float A=I*PI/12.f,B=(I+1)*PI/12.f;
            Line(FVector(X,FMath::Cos(A)*690,810+FMath::Sin(A)*230),FVector(X,FMath::Cos(B)*690,810+FMath::Sin(B)*230),15,TEXT("EAD8B9"));
        }
        FlowerPot(FVector(X,-690,320));FlowerPot(FVector(X,690,320));
    }
    Guest(FVector(25800,670,400),-90,TEXT("789A84"),TEXT("BD835D"),0);
    Guest(FVector(45100,-1000,420),0,TEXT("B48B60"),TEXT("E4B895"),2,true);
    Guest(FVector(45480,-600,420),0,TEXT("7C8E9E"),TEXT("BD835D"),2,true);
    Guest(FVector(45100,1000,420),0,TEXT("AA7B8A"),TEXT("E4B895"),2,true);
    Guest(FVector(45480,600,420),0,TEXT("789A84"),TEXT("C79673"),2,true);
    // The central aisle and the screen remain clear; the console sits off to the left.
    for(int32 I=0;I<6;++I)
    {
        const FVector At=FMath::Lerp(FVector(46050,0,425),FVector(46000,-850,425),(I+.5f)/6.f);
        Decor(TEXT("Cube"),At,FVector(70,20,4),TEXT("C86C43"),FRotator(0,-93,0));
    }
    FilmStar=Part(RootComponent,TEXT("Sphere"),FVector(47260,-850,1680),FVector(10,24,15),TEXT("F7DEAC"));
    Tick(0.f);
}

void ACourierCampusScene::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const auto* Mode=Cast<ACourierGameMode>(UGameplayStatics::GetGameMode(this));if(!Mode)return;
    SceneSeconds+=DeltaSeconds;
    for(const auto& Guest:Guests)
    {
        if(!Guest.Root.IsValid())continue;
        const bool Visible=Mode->CompletedOrders>=Guest.AppearsAfter;
        if(Guest.Root->IsVisible()!=Visible)Guest.Root->SetVisibility(Visible,true);
        if(Guest.Cup.IsValid())Guest.Cup->SetVisibility(Visible && Mode->CompletedOrders>=1);
        const float ArmPitch=Mode->CompletedOrders>=1?55.f+FMath::Sin(SceneSeconds*.9f+Guest.Offset)*3.f:0.f;
        if(Guest.Arm.IsValid())Guest.Arm->SetRelativeRotation(FRotator(ArmPitch,0,0));
        if(Guest.Cup.IsValid())Guest.Cup->SetRelativeRotation(FRotator(-ArmPitch,0,0));
    }
    for(const auto& Node:CoffeeService)Node->SetVisibility(Mode->CompletedOrders>=1,true);
    for(int32 I=0;I<Steam.Num();++I)
    {
        const float T=FMath::Fmod(SceneSeconds*.28f+(I%3)/3.f,1.f);
        Steam[I]->SetRelativeLocation(FVector(FMath::Sin(T*PI)*4,0,33+T*43));
        Steam[I]->SetRelativeScale3D(FVector(.055f*(.2f+FMath::Sin(T*PI))));
    }
    if(FilmStar)
    {
        FilmStar->SetVisibility(Mode->CompletedOrders>=3);
        FilmStar->SetRelativeLocation(FVector(47260,FMath::Sin(SceneSeconds*.12f)*850,1680+FMath::Sin(SceneSeconds*.24f)*90));
    }
}
