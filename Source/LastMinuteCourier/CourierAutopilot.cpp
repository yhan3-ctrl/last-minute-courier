#include "CourierAutopilot.h"
#include "CourierPrototype.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/PlatformMisc.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
    constexpr float LiftHalfLength=325.f;
    constexpr float WalkSpeed=430.f;
}

FCourierAutopilot::FCourierAutopilot()
{
    RunDir=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutoPlay")/FDateTime::Now().ToString(TEXT("%Y%m%d-%H%M%S")));
    WallStart=FPlatformTime::Seconds();
    bTour=FParse::Param(FCommandLine::Get(),TEXT("CourierUITour"));
    Log(bTour?TEXT("autoplay started with UI tour"):TEXT("autoplay started"));
}

FString FCourierAutopilot::PhaseName(uint8 Phase)
{
    static const TCHAR* Names[]={TEXT("Opening"),TEXT("Practice"),TEXT("AwaitPickup"),TEXT("Briefing"),TEXT("Delivery"),
        TEXT("Delivered"),TEXT("Complete"),TEXT("Failed"),TEXT("Paused")};
    return Phase<UE_ARRAY_COUNT(Names)?Names[Phase]:TEXT("?");
}

void FCourierAutopilot::Log(const FString& Event) const
{
    const FString Line=FString::Printf(TEXT("[%7.2f] %s\n"),Clock,*Event);
    UE_LOG(LogTemp,Display,TEXT("CourierAuto %s"),*Line.TrimEnd());
    FFileHelper::SaveStringToFile(Line,*(RunDir/TEXT("events.log")),FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
        &IFileManager::Get(),FILEWRITE_Append);
}

void FCourierAutopilot::Shot(const FString& Name)
{
    const FString File=RunDir/FString::Printf(TEXT("%03d-%s.png"),++Shots,*Name);
    FScreenshotRequest::RequestScreenshot(File,true,false);
    Log(FString::Printf(TEXT("screenshot %s"),*FPaths::GetCleanFilename(File)));
}

void FCourierAutopilot::Face(ACourierCharacter& Player,const FVector& Direction,float DeltaSeconds)
{
    auto* Controller=Player.GetController();
    if(!Controller || Direction.IsNearlyZero())return;
    FRotator Current=Controller->GetControlRotation();Current.Normalize();
    const float Delta=FMath::FindDeltaAngleDegrees(Current.Yaw,Direction.Rotation().Yaw);
    const float Step=FMath::Clamp(Delta,-140.f*DeltaSeconds,140.f*DeltaSeconds);
    const float Pitch=FMath::FInterpTo(Current.Pitch,-18.f,DeltaSeconds,3.f);
    Controller->SetControlRotation(FRotator(Pitch,Current.Yaw+Step,0));
}

void FCourierAutopilot::Tick(ACourierGameMode& Mode,float DeltaSeconds)
{
    Clock+=DeltaSeconds;PhaseClock+=DeltaSeconds;ShotClock+=DeltaSeconds;
    auto* Player=Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(&Mode,0));
    if(!Player)return;
    const uint8 Phase=uint8(Mode.Phase);
    if(Phase!=LastPhase || Mode.CurrentOrder!=LastCurrentOrder)
    {
        Log(FString::Printf(TEXT("phase %s order=%d session=%.1f pos=%s"),*PhaseName(Phase),Mode.CurrentOrder+1,Mode.SessionSeconds,*Player->GetActorLocation().ToCompactString()));
        if(Mode.Phase==ECourierPhase::Practice)PracticeStart=Clock;
        if(Mode.Phase==ECourierPhase::Delivery)OrderStart=Clock;
        if(Mode.Phase==ECourierPhase::Delivered || Mode.Phase==ECourierPhase::Complete)
            Log(FString::Printf(TEXT("order %d delivered: clock used %.1f s of %.0f, remaining %.1f"),Mode.CurrentOrder+1,
                Mode.DeliveryTimes[Mode.CurrentOrder],Mode.OrderBudget(),Mode.Remaining));
        if(Mode.Phase==ECourierPhase::Failed){++Timeouts;Shot(TEXT("timeout"));}
        LastPhase=Phase;LastCurrentOrder=Mode.CurrentOrder;PhaseClock=0.f;
        LiftStep=ELiftStep::None;Lift.Reset();
    }
    if(Clock>1200.f)
    {
        Log(TEXT("RESULT: gave up after 20 minutes"));Shot(TEXT("gave-up"));
        FPlatformMisc::RequestExit(false,TEXT("CourierAutoplay"));return;
    }
    if(bTour && Tour(Mode,DeltaSeconds))return;
    switch(Mode.Phase)
    {
    case ECourierPhase::Opening:
        if(PhaseClock>1.6f && PhaseClock-DeltaSeconds<=1.6f)Shot(FString::Printf(TEXT("card-%d"),Mode.Card+1));
        if(PhaseClock>3.f){Mode.Continue();PhaseClock=0.f;}
        return;
    case ECourierPhase::Briefing:
        if(PhaseClock>1.6f && PhaseClock-DeltaSeconds<=1.6f)Shot(FString::Printf(TEXT("brief-%d"),Mode.CurrentOrder+1));
        if(PhaseClock>5.f)Mode.Continue();
        return;
    case ECourierPhase::Delivered:
        if(PhaseClock>1.6f && PhaseClock-DeltaSeconds<=1.6f)Shot(FString::Printf(TEXT("delivered-%d"),Mode.CurrentOrder+1));
        if(PhaseClock>4.f)Mode.Continue();
        return;
    case ECourierPhase::Failed:
        if(PhaseClock>3.f)Mode.Continue();
        return;
    case ECourierPhase::Paused:
        Mode.Continue();return;
    case ECourierPhase::Complete:
        if(ExitClock<0.f)
        {
            ExitClock=Clock;
            const float Wall=float(FPlatformTime::Seconds()-WallStart);
            Log(FString::Printf(TEXT("RESULT: complete. wall %.1f s (%d:%02d), session %.1f s, practice started %.1f s, orders %.1f / %.1f / %.1f s"),
                Wall,int32(Wall)/60,int32(Wall)%60,Mode.SessionSeconds,PracticeStart,Mode.DeliveryTimes[0],Mode.DeliveryTimes[1],Mode.DeliveryTimes[2]));
            Log(FString::Printf(TEXT("RESULT: recoveries %d, timeouts %d, retries %d, Q uses %d, jumps %d, stuck %d"),
                Recoveries,Timeouts,Mode.Retries,QUses,Jumps,Stucks));
        }
        if(Clock-ExitClock>1.f && Clock-ExitClock-DeltaSeconds<=1.f)Shot(TEXT("finale-a"));
        if(Clock-ExitClock>4.f && Clock-ExitClock-DeltaSeconds<=4.f)Shot(TEXT("finale-b"));
        if(Clock-ExitClock>6.f)FPlatformMisc::RequestExit(false,TEXT("CourierAutoplay"));
        return;
    default:break;
    }
    // Recovery is the game's own fall/collision response; record why it happened.
    static float PreviousRecovery=0.f;
    if(Mode.RecoverySeconds>0.f && PreviousRecovery<=0.f)
    {
        ++Recoveries;
        Log(FString::Printf(TEXT("RECOVERY #%d from %s to %s: %s"),Recoveries,*LastPosition.ToCompactString(),
            *Mode.Checkpoint.ToCompactString(),*Mode.StatusNotice()));
        if(Recoveries<=10)Shot(FString::Printf(TEXT("recovery-%d"),Recoveries));
        LiftStep=ELiftStep::None;Lift.Reset();
    }
    PreviousRecovery=Mode.RecoverySeconds;
    LastPosition=Player->GetActorLocation();
    if(Mode.RecoverySeconds>0.f || Mode.StunSeconds>0.f)return; // staggered like a player
    if(ShotClock>15.f){ShotClock=0.f;Shot(FString::Printf(TEXT("play-%s-%d"),*PhaseName(Phase),Mode.CurrentOrder+1));}
    if(SlowShotAt>0.f && Clock>=SlowShotAt){SlowShotAt=-1.f;Shot(TEXT("borrowed-time"));}
    Drive(Mode,*Player,DeltaSeconds);
}

// Visits every menu state once so each can be inspected in a screenshot.
// Returns true while it is holding control of the run.
bool FCourierAutopilot::Tour(ACourierGameMode& Mode,float DeltaSeconds)
{
    TourClock+=DeltaSeconds;
    const auto Step=[&](float After,TFunction<void()> Action)
    {
        if(TourClock<After)return true;
        Action();++TourStep;TourClock=0.f;return true;
    };
    switch(TourStep)
    {
    // Every screenshot gets its own step; state changes happen a beat later so a
    // capture never races a widget rebuild.
    case 0:if(Mode.Phase!=ECourierPhase::Opening || PhaseClock<1.f)return false;
        return Step(0.f,[&](){Mode.SetLanguage(TEXT("zh"));});
    case 1:return Step(1.2f,[&](){Shot(TEXT("tour-card1-zh"));});
    case 2:return Step(.6f,[&](){Mode.SetLanguage(TEXT("de"));});
    case 3:return Step(1.2f,[&](){Shot(TEXT("tour-card1-de"));});
    case 4:return Step(.6f,[&](){Mode.SetLanguage(TEXT("en"));PhaseClock=0.f;});
    case 5:if(Mode.Phase!=ECourierPhase::Practice || PhaseClock<1.f)return false;
        return Step(0.f,[&](){Mode.TogglePause();Mode.SetLanguage(TEXT("de"));});
    case 6:return Step(1.2f,[&](){Shot(TEXT("tour-pause"));});
    case 7:return Step(.6f,[&](){Mode.ToggleHelp();});
    case 8:return Step(1.2f,[&](){Shot(TEXT("tour-help"));});
    case 9:return Step(.6f,[&](){Mode.TogglePause();Mode.SetLanguage(TEXT("en"));});
    case 10:if(Mode.Phase!=ECourierPhase::Delivery || PhaseClock<2.f)return false;
        return Step(0.f,[&](){Mode.RequestRetry();});
    case 11:return Step(1.2f,[&](){Shot(TEXT("tour-confirm-retry"));});
    case 12:return Step(.6f,[&](){Mode.TogglePause();});
    case 13:return Step(.3f,[&](){Mode.Remaining=.2f;});
    case 14:if(Mode.Phase!=ECourierPhase::Failed)return TourClock<3.f;
        return Step(1.2f,[&](){Shot(TEXT("tour-timeout"));});
    case 15:return Step(.6f,[&](){Mode.Continue();});
    case 16:return Step(1.6f,[&](){Shot(TEXT("tour-retry-brief"));});
    case 17:return Step(.6f,[&](){Mode.SetLanguage(TEXT("zh"));});
    case 18:return Step(1.2f,[&](){Shot(TEXT("tour-brief-zh"));});
    case 19:return Step(.6f,[&](){Mode.Continue();});
    case 20:return Step(3.f,[&](){Shot(TEXT("tour-hud-zh"));});
    case 21:return Step(.6f,[&](){Mode.SetLanguage(TEXT("en"));});
    default:return false;
    }
}

bool FCourierAutopilot::CartGate(ACourierGameMode& Mode,const FVector& P,FVector& Direction,bool& bHold)
{
    const ACourierCart* Next=nullptr;
    for(const auto& Cart:Mode.Carts)
    {
        if(!Cart)continue;
        const bool Active=Cart->bTraining?Mode.Phase==ECourierPhase::Practice:
            (Mode.Phase==ECourierPhase::Delivery && Mode.CurrentOrder==Cart->OrderIndex);
        if(!Active || FMath::Abs(P.Y-Cart->Origin.Y)>700.f || FMath::Abs((Cart->Origin.Z-55.f)-(P.Z-88.f))>150.f)continue;
        if(P.X>Cart->Origin.X+130.f || P.X<Cart->Origin.X-520.f)continue;
        if(!Next || Cart->Origin.X<Next->Origin.X)Next=Cart;
    }
    if(!Next)return false;
    const float Enter=Next->Origin.X-122.f,Exit=Next->Origin.X+122.f;
    Direction=FVector(1.f,FMath::Clamp(-(P.Y-Next->Origin.Y)*.003f,-.4f,.4f),0).GetSafeNormal();
    const auto Safe=[&](float SlowSeconds)
    {
        const float Begin=FMath::Max(0.f,(Enter-P.X)/WalkSpeed);
        const float End=(Exit-P.X)/WalkSpeed+.2f;
        for(float T=Begin;T<=End;T+=.04f)
        {
            const float Advance=FMath::Min(T,SlowSeconds)*.25f+FMath::Max(0.f,T-SlowSeconds);
            if(!Next->OnStreet(Next->Clock+Advance))continue;
            const float Y=Next->Origin.Y+Next->YAt(Next->Clock+Advance);
            if(FMath::Abs(Y-P.Y)<137.f+35.f)return false;
        }
        return true;
    };
    if(P.X>=Enter)return true; // Committed: keep walking and keep any Q already held.
    const bool bAllowQ=Mode.Phase==ECourierPhase::Delivery;
    if(Safe(Mode.bSlowing?Mode.SlowRemaining:0.f))return true;
    if(bAllowQ && !Mode.bSlowing && Mode.Cooldown<=0.f && Mode.Charges>0 && Safe(Mode.SlowRemaining))
    {
        Mode.StartSlow();
        if(Mode.bSlowing)
        {
            ++QUses;Log(FString::Printf(TEXT("hold Q for cart at x=%.0f"),Next->Origin.X));
            if(SlowShots<2){SlowShotAt=Clock+.45f;++SlowShots;}
        }
        return true;
    }
    if(Mode.bSlowing)Mode.EndSlow();
    bHold=P.X>Enter-75.f;
    return true;
}

bool FCourierAutopilot::DriveLift(ACourierGameMode& Mode,ACourierCharacter& Player,FVector& Direction,bool& bHold)
{
    const FVector P=Player.GetActorLocation();
    if(LiftStep==ELiftStep::None)
    {
        const FVector Target=Mode.NavigationTarget();
        for(const auto& Candidate:Mode.Platforms)
        {
            if(!Candidate || Candidate->OrderIndex!=Mode.CurrentOrder || LiftsDone.Contains(Candidate.Get()))continue;
            const FVector O=Candidate->Origin;
            if(P.X>O.X-1400.f && P.X<O.X-LiftHalfLength+20.f && FMath::Abs(P.Y-O.Y)<700.f && P.Z<O.Z+250.f && Target.X>O.X-LiftHalfLength)
            {
                Lift=Candidate.Get();LiftStep=ELiftStep::Approach;
                Log(FString::Printf(TEXT("lift at x=%.0f: approach"),O.X));Shot(FString::Printf(TEXT("lift-%.0f-approach"),O.X));
                break;
            }
        }
        if(LiftStep==ELiftStep::None)return false;
    }
    ACourierLift* L=Lift.Get();
    if(!L){LiftStep=ELiftStep::None;return false;}
    const FVector Location=L->GetActorLocation();
    const float Fraction=FVector::DotProduct(Location-L->Origin,L->Travel)/L->Travel.SizeSquared();
    const float Cycle=FMath::Fmod(L->Clock,L->Period)/L->Period;
    const bool bOn=Player.GetMovementBaseObject()==L->Mesh.Get();
    const FVector WaitAt(L->Origin.X-LiftHalfLength-80.f,L->Origin.Y,P.Z);
    switch(LiftStep)
    {
    case ELiftStep::Approach:
        Direction=(WaitAt-P).GetSafeNormal2D();
        if(FVector::Dist2D(P,WaitAt)<60.f){LiftStep=ELiftStep::Wait;PhaseClock=0.f;}
        if(Mode.bSlowing)Mode.EndSlow();
        return true;
    case ELiftStep::Wait:
        Direction=FVector(1,0,0);bHold=true;
        if(Mode.bSlowing)Mode.EndSlow();
        if(Fraction<.03f && (Cycle>.9f || Cycle<.02f) && (Mode.Cooldown<=0.f || Cycle>.94f))
        {
            LiftStep=ELiftStep::Board;PhaseClock=0.f;
            Mode.StartSlow();
            if(Mode.bSlowing)++QUses;
            Log(FString::Printf(TEXT("lift at x=%.0f: boarding (Q %s)"),L->Origin.X,Mode.bSlowing?TEXT("held"):TEXT("unavailable")));
        }
        return true;
    case ELiftStep::Board:
        Direction=(Location-P).GetSafeNormal2D();
        if(bOn && FVector::Dist2D(P,Location)<180.f)
        {
            LiftStep=ELiftStep::Ride;Mode.EndSlow();
            Log(FString::Printf(TEXT("lift at x=%.0f: aboard, released Q"),L->Origin.X));Shot(FString::Printf(TEXT("lift-%.0f-aboard"),L->Origin.X));
        }
        else if(PhaseClock>5.f){Log(TEXT("lift boarding timed out; waiting again"));LiftStep=ELiftStep::Approach;Mode.EndSlow();}
        return true;
    case ELiftStep::Ride:
        Direction=FVector(1,0,0);bHold=true;
        if(Fraction>.95f && Cycle<.5f)
        {
            LiftStep=ELiftStep::Exit;PhaseClock=0.f;
            Log(FString::Printf(TEXT("lift at x=%.0f: stepping off"),L->Origin.X));
        }
        if(!bOn && P.Z<L->Origin.Z+40.f && PhaseClock>1.f){Log(TEXT("fell off lift while riding"));LiftStep=ELiftStep::Approach;}
        return true;
    case ELiftStep::Exit:
        Direction=FVector(1,FMath::Clamp(-(P.Y-L->Origin.Y)*.003f,-.3f,.3f),0).GetSafeNormal();
        if(!bOn && P.X>L->Origin.X+FMath::Max(0.f,L->Travel.X)+LiftHalfLength+40.f && Player.GetCharacterMovement()->IsMovingOnGround())
        {
            LiftsDone.Add(L);LiftStep=ELiftStep::None;
            Log(FString::Printf(TEXT("lift at x=%.0f: done"),L->Origin.X));Shot(FString::Printf(TEXT("lift-%.0f-done"),L->Origin.X));
        }
        else if(bOn && Fraction<.8f){Log(TEXT("missed the landing; riding another cycle"));LiftStep=ELiftStep::Ride;}
        return true;
    default:return false;
    }
}

void FCourierAutopilot::Drive(ACourierGameMode& Mode,ACourierCharacter& Player,float DeltaSeconds)
{
    const FVector P=Player.GetActorLocation();
    if(Mode.Phase==ECourierPhase::AwaitPickup && Mode.CanInteractAt(Mode.Pickup))
    {
        if(Mode.bSlowing)Mode.EndSlow();
        if(PhaseClock>.8f){Log(TEXT("press E at parcel desk"));Mode.Interact();}
        return;
    }
    if(Mode.Phase==ECourierPhase::Delivery && Mode.CanInteractAt(Mode.Destination))
    {
        Log(TEXT("press E to deliver"));Mode.Interact();return;
    }
    const FVector Target=Mode.NavigationTarget();
    FVector Direction=(Target-P).GetSafeNormal2D();
    bool bHold=false;
    const bool bLift=DriveLift(Mode,Player,Direction,bHold);
    if(!bLift)CartGate(Mode,P,Direction,bHold);
    if(SidestepClock>0.f)
    {
        SidestepClock-=DeltaSeconds;
        Direction=(Direction+FVector::CrossProduct(FVector::UpVector,Direction)*SidestepSign*1.4f).GetSafeNormal2D();
    }
    Face(Player,bHold?FVector(1,0,0):Direction,DeltaSeconds);
    if(bHold){BlockedClock=0.f;StuckClock=0.f;return;}
    Player.AddMovementInput(Direction,1.f);

    auto* Movement=Player.GetCharacterMovement();
    const bool bGround=Movement->IsMovingOnGround();
    const bool bRiding=LiftStep==ELiftStep::Ride || LiftStep==ELiftStep::Wait || LiftStep==ELiftStep::Board;
    if(bGround && !bRiding)
    {
        // Jump a visible gap: no floor near foot height a short stride ahead.
        FCollisionQueryParams Params;Params.AddIgnoredActor(&Player);
        for(const auto& Cart:Mode.Carts)if(Cart)Params.AddIgnoredActor(Cart.Get());
        const FVector Ahead=P+Direction*115.f;const float Feet=P.Z-88.f;
        FHitResult Hit;
        const bool bFloor=Player.GetWorld()->LineTraceSingleByChannel(Hit,FVector(Ahead.X,Ahead.Y,Feet+60.f),FVector(Ahead.X,Ahead.Y,Feet-200.f),ECC_Visibility,Params)
            && Hit.ImpactPoint.Z>Feet-70.f;
        const bool bBlocked=Player.GetVelocity().Size2D()<90.f;
        BlockedClock=bBlocked?BlockedClock+DeltaSeconds:0.f;
        if(!bFloor || BlockedClock>.3f)
        {
            Player.Jump();++Jumps;
            Log(FString::Printf(TEXT("jump (%s) at %s"),bFloor?TEXT("blocked"):TEXT("gap"),*P.ToCompactString()));
            if(BlockedClock>1.2f){SidestepClock=.9f;SidestepSign=-SidestepSign;BlockedClock=0.f;}
        }
    }
    else if(!bGround)Player.StopJumping();
    // Long-term progress check toward whatever the game currently points at.
    StuckClock+=DeltaSeconds;
    if(StuckClock>10.f)
    {
        static FVector Anchor=FVector::ZeroVector;
        if(FVector::Dist(P,Anchor)<200.f)
        {
            ++Stucks;Log(FString::Printf(TEXT("STUCK near %s, target %s"),*P.ToCompactString(),*Target.ToCompactString()));
            Shot(FString::Printf(TEXT("stuck-%d"),Stucks));SidestepClock=1.5f;SidestepSign=-SidestepSign;
        }
        Anchor=P;StuckClock=0.f;
    }
}
