#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CourierCampusScene.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UInstancedStaticMeshComponent;

struct FCourierGuestPose
{
    TWeakObjectPtr<USceneComponent> Root, Arm, Cup;
    int32 AppearsAfter=0;
    float Offset=0.f;
};

// Cosmetic scenery never blocks a route or changes device/mission time.
UCLASS()
class LASTMINUTECOURIER_API ACourierCampusScene : public AActor
{
    GENERATED_BODY()
public:
    ACourierCampusScene();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
private:
    UPROPERTY() TArray<TObjectPtr<USceneComponent>> Parts;
    UPROPERTY() TArray<TObjectPtr<USceneComponent>> CoffeeService;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Steam;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> FilmStar;
    TMap<FString,UInstancedStaticMeshComponent*> Batches;
    TArray<FCourierGuestPose> Guests;
    float SceneSeconds=0.f;
    USceneComponent* Group(FVector At,FRotator Rotation=FRotator::ZeroRotator);
    UStaticMeshComponent* Part(USceneComponent* Parent,const TCHAR* Shape,FVector At,FVector Size,const TCHAR* Hex,FRotator Rotation=FRotator::ZeroRotator);
    void Decor(const TCHAR* Shape,FVector At,FVector Size,const TCHAR* Hex,FRotator Rotation=FRotator::ZeroRotator);
    void Line(FVector A,FVector B,float Width,const TCHAR* Hex);
    void Cup(FVector At);
    void FlowerPot(FVector At,float Scale=1.f);
    void Guest(FVector At,float Yaw,const TCHAR* Shirt,const TCHAR* Skin,int32 Stage,bool Seated=false);
};
