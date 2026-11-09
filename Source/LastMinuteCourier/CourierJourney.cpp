#include "CourierPrototype.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Dom/JsonObject.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Sound/SoundBase.h"

void ACourierGameMode::ConfigureOrder(int32 Index)
{
    if(Orders.Num()!=3)
    {
        Orders.SetNum(3);
        FString Json;
        TSharedPtr<FJsonObject> Root;
        if(FFileHelper::LoadFileToString(Json,*(FPaths::ProjectDir()/TEXT("DesignData/game_content.json"))) &&
           FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json),Root) && Root)
        {
            const TArray<TSharedPtr<FJsonValue>>* Rows=nullptr;
            if(Root->TryGetArrayField(TEXT("orders"),Rows) && Rows->Num()==3)
            {
                for(int32 I=0;I<3;++I)
                {
                    const auto Row=(*Rows)[I]->AsObject();
                    if(!Row)continue;
                    auto& Order=Orders[I];
                    const auto Pair=[&Row](const TCHAR* Key,FString& En,FString& Zh)
                    {
                        const TSharedPtr<FJsonObject>* Text=nullptr;
                        if(Row->TryGetObjectField(Key,Text))
                        {(*Text)->TryGetStringField(TEXT("en"),En);(*Text)->TryGetStringField(TEXT("zh"),Zh);}
                    };
                    Pair(TEXT("title"),Order.Title,Order.TitleZh);
                    Pair(TEXT("package"),Order.Package,Order.PackageZh);
                    Pair(TEXT("destination"),Order.Destination,Order.DestinationZh);
                    Pair(TEXT("brief"),Order.Brief,Order.BriefZh);
                    Pair(TEXT("label"),Order.Label,Order.LabelZh);
                    Pair(TEXT("delivered"),Order.Delivered,Order.DeliveredZh);
                    const double Defaults[]={40,120,130};double Seconds=Defaults[I];
                    Row->TryGetNumberField(TEXT("seconds"),Seconds);
                    Order.Seconds=FMath::Clamp(float(Seconds),20.f,600.f);
                }
            }
        }
        // English remains usable if staged JSON is unavailable, but the release audit must flag it.
        const TCHAR* Titles[]={TEXT("A Little Warmth"),TEXT("Light Up the Screen"),TEXT("The Final Reel")};
        for(int32 I=0;I<3;++I)if(Orders[I].Title.IsEmpty())
        {
            Orders[I].Title=Titles[I];Orders[I].TitleZh=Titles[I];
            Orders[I].Destination=Titles[I];Orders[I].Seconds=I==0?40.f:(I==1?120.f:130.f);
            Orders[I].Brief=TEXT("Follow the orange route markers. E collects and delivers; Q slows blue-tagged devices.");
            UE_LOG(LogTemp,Warning,TEXT("Courier order data missing: release data check required."));
        }
    }
    CurrentOrder=FMath::Clamp(Index,0,2);
    const FVector Stations[]={FVector(2400,0,100),FVector(8000,0,100),FVector(26000,0,500),FVector(46000,-950,520)};
    Pickup=Stations[CurrentOrder];Destination=Stations[CurrentOrder+1];
    Remaining=OrderBudget();OrderElapsed=0.f;RouteNode=0;RecoverySeconds=0.f;
    Checkpoint=Pickup+FVector(0,0,20);
    if(CurrentOrder==0)
        Route={FVector(4500,0,100),FVector(6500,0,100),Destination};
    else if(CurrentOrder==1)
        Route={FVector(8000,1100,100),FVector(9200,1100,100),FVector(10500,1100,100),FVector(11700,0,100),FVector(14000,0,420),
               FVector(16400,0,420),FVector(18100,-1400,100),FVector(18300,0,100),
               FVector(21100,0,100),FVector(22500,0,100),FVector(24800,0,500),Destination};
    else
        Route={FVector(26000,1100,500),FVector(27400,1100,500),FVector(28900,1100,100),FVector(30000,0,100),FVector(32700,0,100),
               FVector(33900,0,100),FVector(36300,0,100),FVector(38900,0,100),
               FVector(39600,-1100,100),FVector(39800,0,100),FVector(42000,0,520),FVector(46100,0,520),Destination};
    if(auto* Player=Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(this,0)))Player->SetParcelStyle(CurrentOrder);
}

float ACourierGameMode::OrderBudget() const {return Orders.IsValidIndex(CurrentOrder)?Orders[CurrentOrder].Seconds:60.f;}
FString ACourierGameMode::OrderTitle() const {const auto& O=Orders[CurrentOrder];return Copy(*O.Title,*O.TitleZh);}
FString ACourierGameMode::OrderDestination() const {const auto& O=Orders[CurrentOrder];return Copy(*O.Destination,*O.DestinationZh);}
FString ACourierGameMode::OrderOutcome() const {const auto& O=Orders[CurrentOrder];return Copy(*O.Delivered,*O.DeliveredZh);}
FString ACourierGameMode::OrderPackage() const {const auto& O=Orders[CurrentOrder];return Copy(*O.Package,*O.PackageZh);}
FString ACourierGameMode::OrderLabel() const {const auto& O=Orders[CurrentOrder];return Copy(*O.Label,*O.LabelZh);}
FString ACourierGameMode::OrderBrief() const
{
    const auto& O=Orders[CurrentOrder];
    return Copy(*O.Brief,*O.BriefZh)+TEXT("\n\n")+RouteHint()+TEXT("\n\n")+
        Copy(TEXT("TIME LIMIT"),TEXT("配送时限"))+FString::Printf(TEXT("  %d s"),FMath::RoundToInt(OrderBudget()))+TEXT("\n")+
        Copy(TEXT("Read at your own pace. The clock starts only after you accept."),TEXT("慢慢阅读。确认接单后才开始计时。"));
}

FString ACourierGameMode::RouteHint() const
{
    if(CurrentOrder==0)return Copy(TEXT("Climb the crates (jump at them) or take the right arcade, cross the cart lanes, beat the rolling shutter (slide under before it closes, or wait for it to open) and pass the turnstile (jump the low arm, slide under the high one). 3 uses of borrowed time this delivery."),TEXT("朝箱子跳上去翻过，或走右侧长廊；穿过车道，闯过卷帘门（趁没落下滑铲钻过去，或等它升起），再过旋转栏杆（低的跳、高的滑铲）。这一单可借 3 次时间。"));
    if(CurrentOrder==1)return Copy(TEXT("Garden lifts: step aboard while the platform is level, ride up, step off at the top; wide side ramps avoid them. Then the stepping stones, a turnstile (jump the low arm, slide under the high one) and the booth shutter. 4 uses this delivery."),TEXT("花园升降台：平台齐平时走上去，到顶再走下来；也可以走两侧宽坡道绕开。之后是跳石、旋转栏杆（低的跳、高的滑铲）和放映室前的卷帘门。这一单可借 4 次。"));
    return Copy(TEXT("Market lanes and a shutter, a roof run for the bold, the shuttle, a turnstile and two more lanes. Then up to the terrace: ride the lift or run over the springboard. The clocktower gate rolls down on a timer. 5 uses this delivery."),TEXT("市场车道和卷帘门、给大胆的人准备的屋顶跑酷、摆渡台、旋转栏杆和两条车道。然后上露台：坐升降台，或者跑过弹跳板直接弹上去。钟楼大门会定时落下。这一单可借 5 次。"));
}

FVector ACourierGameMode::NavigationTarget() const
{
    if(Phase==ECourierPhase::Practice)
    {
        const FVector Markers[]={FVector(650,0,100),FVector(1150,0,100),FVector(1420,0,100),FVector(2000,0,100),Pickup};
        return Markers[FMath::Clamp(Tutorial,0,4)];
    }
    if(Phase==ECourierPhase::AwaitPickup || Phase==ECourierPhase::Briefing)return Pickup;
    // Follow a chosen side route to its exit instead of pointing across the water.
    if(const auto* Player=UGameplayStatics::GetPlayerCharacter(this,0))
    {
        const FVector P=Player->GetActorLocation();FVector DetourTarget;
        // Distance to the painted recommended line: a side path only takes over the
        // marker when the courier is clearly on it, not merely beside the main line.
        float MainBest=TNumericLimits<float>::Max();
        for(const auto& Leg:GuideLegs(CurrentOrder))for(int32 I=0;I+1<Leg.Num();++I)
        {
            const FVector A(Leg[I].X,Leg[I].Y,0),B(Leg[I+1].X,Leg[I+1].Y,0),Flat(P.X,P.Y,0);
            const float T=FMath::Clamp(FVector::DotProduct(Flat-A,B-A)/FMath::Max(1.f,(B-A).SizeSquared()),0.f,1.f);
            MainBest=FMath::Min(MainBest,FVector::DistSquared(Flat,FMath::Lerp(A,B,T)));
        }
        const auto Detour=[&](std::initializer_list<FVector> Points,float Side)
        {
            if(P.Y*Side<650)return false;
            TArray<FVector> Nodes;for(const auto& N:Points)Nodes.Add(N);
            if(P.X<Nodes[0].X-400 || P.X>Nodes.Last().X+300)return false;
            float Best=TNumericLimits<float>::Max();int32 Segment=-1;
            for(int32 I=0;I+1<Nodes.Num();++I)
            {
                const FVector A(Nodes[I].X,Nodes[I].Y,0),B(Nodes[I+1].X,Nodes[I+1].Y,0),Flat(P.X,P.Y,0);
                const float T=FMath::Clamp(FVector::DotProduct(Flat-A,B-A)/(B-A).SizeSquared(),0.f,1.f);
                const float Distance=FVector::DistSquared(Flat,FMath::Lerp(A,B,T));
                if(Distance<Best){Best=Distance;Segment=I;}
            }
            if(Segment<0 || Best>800.f*800.f || Best>MainBest*.5f)return false;
            int32 Next=Segment+1;
            if(FVector::Dist2D(P,Nodes[Next])<240.f && Next+1<Nodes.Num())++Next;
            DetourTarget=Nodes[Next]+FVector(0,0,100);return true;
        };
        if(CurrentOrder==0 && Detour({FVector(2800,0,0),FVector(3050,850,0),FVector(7400,850,0),FVector(7700,0,0)},1))return DetourTarget;
        if(CurrentOrder==1)
        {
            if(Detour({FVector(12000,0,0),FVector(12200,1900,0),FVector(14100,1900,320),FVector(14800,1900,320),FVector(14800,0,320)},1))return DetourTarget;
            if(Detour({FVector(18200,0,0),FVector(18400,-1900,0),FVector(21300,-1900,0),FVector(21600,0,0)},-1))return DetourTarget;
            if(Detour({FVector(22500,0,0),FVector(22800,2100,0),FVector(24900,2100,400),FVector(25700,2100,400),FVector(25700,0,400)},1))return DetourTarget;
        }
        if(CurrentOrder==2)
        {
            if(Detour({FVector(30100,0,0),FVector(30200,1450,0),FVector(32300,1450,0),FVector(32700,0,0)},1))return DetourTarget;
            if(Detour({FVector(33900,0,0),FVector(34100,-1900,0),FVector(36000,-1900,0),FVector(36300,0,0)},-1))return DetourTarget;
            if(Detour({FVector(36900,0,0),FVector(37000,1450,0),FVector(38700,1450,0),FVector(38900,0,0)},1))return DetourTarget;
            if(Detour({FVector(39700,0,0),FVector(39900,2200,0),FVector(42200,2200,420),FVector(43000,2200,420),FVector(43000,0,420)},1))return DetourTarget;
        }
    }
    return Route.IsValidIndex(RouteNode)?Route[RouteNode]:Destination;
}

FString ACourierGameMode::NavigationText() const
{
    if((Phase==ECourierPhase::AwaitPickup && CanInteractAt(Pickup)) ||
       (Phase==ECourierPhase::Delivery && CanInteractAt(Destination)))
        return Copy(TEXT("ARRIVED"),TEXT("已到达"));
    const auto* Player=UGameplayStatics::GetPlayerCharacter(this,0);
    const auto* PC=UGameplayStatics::GetPlayerController(this,0);
    if(!Player || !PC)return FString();
    const FVector Delta=NavigationTarget()-Player->GetActorLocation();
    const float Angle=FMath::FindDeltaAngleDegrees(PC->GetControlRotation().Yaw,Delta.Rotation().Yaw);
    const FString Direction=FMath::Abs(Angle)>135?Copy(TEXT("BEHIND"),TEXT("后方")):
        (Angle>25?Copy(TEXT("RIGHT"),TEXT("右方")):(Angle<-25?Copy(TEXT("LEFT"),TEXT("左方")):Copy(TEXT("AHEAD"),TEXT("前方"))));
    return Direction+FString::Printf(TEXT("   %.0f m"),FMath::CeilToFloat(Delta.Size2D()/100.f));
}

FString ACourierGameMode::ProgressText() const
{
    return Copy(TEXT("DELIVERIES"),TEXT("配送进度"))+FString::Printf(TEXT("  %d / 3"),CompletedOrders);
}

void ACourierGameMode::Sound(const TCHAR* Name,float Volume,float Pitch)
{
    if(!bAudioEnabled)return;
    const FString Path=FString::Printf(TEXT("/Game/Audio/%s.%s"),Name,Name);
    if(auto* Cue=LoadObject<USoundBase>(nullptr,*Path))UGameplayStatics::PlaySound2D(this,Cue,Volume,Pitch);
}

void ACourierGameMode::SetAudioEnabled(bool bEnabled)
{
    bAudioEnabled=bEnabled;
    if(Music)Music->SetPaused(!bEnabled);
    for(const auto& Layer:MusicLayers)if(Layer)Layer->SetPaused(!bEnabled);
}

void ACourierGameMode::UpdateCelebration()
{
    SetCoffeeLit(CompletedOrders>=1);
    for(int32 I=0;I<MusicLayers.Num();++I)if(MusicLayers[I])
        MusicLayers[I]->AdjustVolume(2.f,CompletedOrders>=I+1?(CompletedOrders>=3?.36f:.27f):0.f);
    for(const auto& Actor:ProjectionActors)if(Actor)Actor->SetActorHiddenInGame(CompletedOrders<2);
    for(const auto& Actor:FinaleActors)if(Actor)Actor->SetActorHiddenInGame(CompletedOrders<3);
}

FString ACourierGameMode::ContextHint() const
{
    const FString Notice=StatusNotice();if(!Notice.IsEmpty())return Notice;
    if(!InteractionPrompt().IsEmpty())return FString();
    if(Phase==ECourierPhase::Practice)
    {
        if(Tutorial==3 && Cooldown>0.f)
            return Copy(TEXT("Q is recharging. You can still cross once the cart has passed."),TEXT("Q 正在充能。运输车开过去后，你仍然可以直接通过。"));
        const int32 I=FMath::Clamp(Tutorial,0,TutorialText.Num()-1);return Copy(*TutorialText[I],*TutorialTextZh[I]);
    }
    if(Phase==ECourierPhase::AwaitPickup)
        return Copy(TEXT("Your next parcel waits at the marker. Walk there to read the brief; the clock has not started."),TEXT("下一份包裹在标记处。走过去查看简报，倒计时还没开始。"));
    const auto* Player=UGameplayStatics::GetPlayerCharacter(this,0);if(!Player)return FString();
    const FVector P=Player->GetActorLocation();
    for(const auto& Lift:Platforms)if(Lift && Player->GetMovementBaseObject()==Lift->Mesh.Get())
    {
        const FVector End=Lift->Origin+Lift->Travel;
        if(FVector::Dist(Lift->GetActorLocation(),End)<45.f)
            return Copy(TEXT("At the landing. Walk forward to step off."),TEXT("到站了。向前走，离开平台。"));
        return Copy(TEXT("You are aboard. Release Q to ride at normal speed; step off at the landing."),TEXT("已经上台。松开 Q 恢复正常速度，到达平台后再走下去。"));
    }
    if((CurrentOrder==0 && P.X>4100 && P.X<4800 && FMath::Abs(P.Y)<600) || (CurrentOrder==1 && P.X>9250 && P.X<9950 && FMath::Abs(P.Y-1100)<520))
        return Copy(TEXT("Crates! Jump at them to climb up and over, or walk around."),TEXT("箱子挡路！朝它跳就能爬上去翻过去，也可以绕开。"));
    if(CurrentOrder==2 && P.X>42900 && P.X<44700 && FMath::Abs(P.Y)<600 && P.Z>400)
        return Copy(TEXT("Space jumps over the planters. You can also walk around them."),TEXT("按空格跳过花箱，也可以从旁边绕过。"));
    for(const auto& Obstacle:Obstacles)
    {
        if(!Obstacle || Obstacle->OrderIndex!=CurrentOrder)continue;
        const FVector D=P-Obstacle->Origin;
        if(FMath::Abs(D.Z)>300.f)continue;
        if(Obstacle->Kind==ECourierObstacleKind::Shutter && D.X>-1000.f && D.X<-60.f && FMath::Abs(D.Y)<Obstacle->Width*.5f+200.f)
            return Copy(TEXT("Rolling shutter! Sprint and slide under (Ctrl) while it is still up, or wait for it to open. Q slows it."),TEXT("卷帘门！趁它还没落下，冲刺后按 Ctrl 滑铲钻过去，或者等它再升起来。Q 能让它变慢。"));
        if(Obstacle->Kind==ECourierObstacleKind::Sweeper && FVector2D(D.X,D.Y).Size()<Obstacle->ArmLength+700.f && D.X<Obstacle->ArmLength)
            return Copy(TEXT("Turnstile! Jump the low arm, slide under the high arm. Q slows it."),TEXT("旋转栏杆！低的一侧跳过去，高的一侧滑铲钻过去。Q 能让它变慢。"));
        if(Obstacle->Kind==ECourierObstacleKind::Spring && FVector2D(D.X,D.Y).Size()<700.f && D.X<0.f)
            return Copy(TEXT("Springboard: run across it to leap straight up to the terrace, no lift needed."),TEXT("弹跳板：跑过去就能直接弹上露台，不用等升降台。"));
    }
    const ACourierCart* NextCart=nullptr;
    for(const auto& Cart:Carts)if(Cart && !Cart->bTraining && Cart->OrderIndex==CurrentOrder &&
        Cart->Origin.X>=P.X-120.f && Cart->Origin.X-P.X<900.f && FMath::Abs(P.Y)<570.f &&
        (!NextCart || Cart->Origin.X<NextCart->Origin.X))NextCart=Cart;
    const bool bOutOfTime=Phase==ECourierPhase::Delivery && Charges<=0;
    if(NextCart)
        return bOutOfTime?Copy(TEXT("No borrowed time left. Wait for the cart to pass, then cross."),TEXT("借时间已用完。等运输车开过去再通过。")):
            Copy(TEXT("Wait for the cart to pass. Hold Q to keep the gap open, then cross."),TEXT("先等运输车开过去，再按住 Q 延长空隙，趁机通过。"));
    for(const auto& Lift:Platforms)if(Lift && Lift->OrderIndex==CurrentOrder &&
        FVector::Dist2D(P,Lift->Origin)<1100.f && P.X<Lift->Origin.X+700.f)
    {
        if(Lift->Travel.Z>0.f && P.Z<Lift->Origin.Z+160.f && Lift->GetActorLocation().Z>P.Z+20.f)
            return Copy(TEXT("The lift is above you. Release Q and wait here for it to come down."),TEXT("升降台还在上方。松开 Q，在这里等它降下来。"));
        return bOutOfTime?Copy(TEXT("No borrowed time left. Step aboard as soon as the platform is level with you, or take the side path."),TEXT("借时间已用完。平台和你齐平时立刻走上去，或者走侧路。")):
            Copy(TEXT("When the blue platform reaches your side, hold Q and walk aboard. Space also works."),TEXT("等蓝色平台来到你这一侧，按住 Q 走上去；也可以按空格跳上去。"));
    }
    if(bSlowing)return Copy(TEXT("Blue devices are slower. You and the delivery clock move normally."),TEXT("蓝色装置正在减速。你的步伐和配送倒计时照常进行。"));
    if(FVector::Dist2D(P,Destination)<1800.f)
        return Copy(TEXT("Walk onto the orange square. Press E to deliver."),TEXT("走到橙色方形标记上，按 E 交付。"));
    // A quiet screen once the player is underway; the reminder shows briefly per order.
    if(Phase==ECourierPhase::Delivery && OrderElapsed>7.f)return FString();
    return Copy(TEXT("Follow the orange route markers. C gently recenters your view."),TEXT("沿橙色路线标记前进。C 可平缓地将镜头回正。"));
}
