#pragma once

#include "CoreMinimal.h"

class ACourierGameMode;
class ACourierCharacter;
class ACourierLift;
enum class ECourierPhase : uint8;

// QA-only playthrough driver, enabled with -CourierAutoplay. It plays with the
// same verbs a player has (move, look, jump, hold Q, E, Enter) at real speed,
// never teleports, and records events plus in-engine screenshots. Normal play
// never constructs it.
class FCourierAutopilot
{
public:
    FCourierAutopilot();
    void Tick(ACourierGameMode& Mode,float DeltaSeconds);

private:
    enum class ELiftStep : uint8 {None,Approach,Wait,Board,Ride,Exit};
    FString RunDir;
    double WallStart=0.;
    float Clock=0.f;
    float PhaseClock=0.f;
    float ShotClock=0.f;
    float BlockedClock=0.f;
    float StuckClock=0.f;
    float SidestepClock=0.f;
    float SidestepSign=1.f;
    float LastTargetDistance=TNumericLimits<float>::Max();
    float OrderStart=0.f;
    float PracticeStart=-1.f;
    float ExitClock=-1.f;
    int32 Shots=0;
    int32 Recoveries=0;
    int32 QUses=0;
    int32 Jumps=0;
    int32 Stucks=0;
    int32 Timeouts=0;
    int32 LastCurrentOrder=-1;
    uint8 LastPhase=255;
    FVector LastPosition=FVector::ZeroVector;
    TWeakObjectPtr<ACourierLift> Lift;
    TArray<TWeakObjectPtr<ACourierLift>> LiftsDone;
    ELiftStep LiftStep=ELiftStep::None;
    bool bReleasedSlowAfterBoard=false;
    bool bTour=false;
    int32 TourStep=0;
    float TourClock=0.f;
    bool Tour(ACourierGameMode& Mode,float DeltaSeconds);
    void Log(const FString& Event) const;
    void Shot(const FString& Name);
    void Drive(ACourierGameMode& Mode,ACourierCharacter& Player,float DeltaSeconds);
    bool DriveLift(ACourierGameMode& Mode,ACourierCharacter& Player,FVector& Direction,bool& bHold);
    bool CartGate(ACourierGameMode& Mode,const FVector& Position,FVector& Direction,bool& bHold);
    void Face(ACourierCharacter& Player,const FVector& Direction,float DeltaSeconds);
    static FString PhaseName(uint8 Phase);
};
