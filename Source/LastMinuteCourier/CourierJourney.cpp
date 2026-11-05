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
                    double Seconds=140+I*30;
                    Row->TryGetNumberField(TEXT("seconds"),Seconds);
                    Order.Seconds=FMath::Clamp(float(Seconds),60.f,600.f);
                }
            }
        }
        // English remains usable if staged JSON is unavailable, but the release audit must flag it.
        const TCHAR* Titles[]={TEXT("A Little Warmth"),TEXT("Light Up the Screen"),TEXT("The Final Reel")};
        for(int32 I=0;I<3;++I)if(Orders[I].Title.IsEmpty())
        {
            Orders[I].Title=Titles[I];Orders[I].TitleZh=Titles[I];
            Orders[I].Destination=Titles[I];Orders[I].Seconds=140+I*30;
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
        Route={FVector(8000,1100,100),FVector(9200,1100,100),FVector(11700,0,100),FVector(14000,0,420),
               FVector(16400,0,420),FVector(18100,-1400,100),FVector(18300,0,100),
               FVector(21100,0,100),FVector(22500,0,100),FVector(24800,0,500),Destination};
    else
        Route={FVector(26000,1100,500),FVector(27400,1100,500),FVector(30000,0,100),FVector(32700,0,100),
               FVector(33900,0,100),FVector(36300,0,100),FVector(38900,0,100),
               FVector(39600,-1100,100),FVector(39800,0,100),FVector(42000,0,520),FVector(46100,0,520),Destination};
    if(auto* Player=Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(this,0)))Player->SetParcelStyle(CurrentOrder);
}

float ACourierGameMode::OrderBudget() const {return Orders.IsValidIndex(CurrentOrder)?Orders[CurrentOrder].Seconds:140.f;}
FString ACourierGameMode::OrderTitle() const {const auto& O=Orders[CurrentOrder];return Copy(*O.Title,*O.TitleZh);}
FString ACourierGameMode::OrderDestination() const {const auto& O=Orders[CurrentOrder];return Copy(*O.Destination,*O.DestinationZh);}
FString ACourierGameMode::OrderOutcome() const {const auto& O=Orders[CurrentOrder];return Copy(*O.Delivered,*O.DeliveredZh);}
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
    if(CurrentOrder==0)return Copy(TEXT("Centre: shorter cart crossings. Right arcade: a wider, safer walk. Both routes reach the coffee stand."),TEXT("中央路线穿过车道，路程更短；右侧长廊更宽、更稳妥。两条路都能到咖啡摊。"));
    if(CurrentOrder==1)return Copy(TEXT("Garden lifts: jump aboard low, ride up, then leave at the landing. Q gives you more boarding time. Wide side ramps bypass every lift and the stepping stones."),TEXT("花园升降台：低处跳上，乘台升高，到站离开。Q 能延长登台窗口。宽阔的侧坡道可绕过升降台和跳石。"));
    return Copy(TEXT("The last route combines cart lanes, a shuttle and a terrace lift. Q lasts 3 seconds, then needs 4 to recharge. Side paths are always available; save a charge for your next crossing."),TEXT("最后一程组合了车道、摆渡台和露台升降机。Q 最长 3 秒，之后充能 4 秒。始终有侧路可选，为下一次通行安排好能量。"));
}

FVector ACourierGameMode::NavigationTarget() const
{
    if(Phase==ECourierPhase::Practice)
    {
        const FVector Markers[]={FVector(650,0,100),FVector(1200,0,100),FVector(2000,0,100),Pickup};
        return Markers[FMath::Clamp(Tutorial,0,3)];
    }
    if(Phase==ECourierPhase::AwaitPickup || Phase==ECourierPhase::Briefing)return Pickup;
    // Follow a chosen side route to its exit instead of pointing across the water.
    if(const auto* Player=UGameplayStatics::GetPlayerCharacter(this,0))
    {
        const FVector P=Player->GetActorLocation();FVector DetourTarget;
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
            if(Segment<0 || Best>800.f*800.f)return false;
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

void ACourierGameMode::Sound(const TCHAR* Name,float Volume)
{
    if(!bAudioEnabled)return;
    const FString Path=FString::Printf(TEXT("/Game/Audio/%s.%s"),Name,Name);
    if(auto* Cue=LoadObject<USoundBase>(nullptr,*Path))UGameplayStatics::PlaySound2D(this,Cue,Volume);
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
    const FString Prompt=InteractionPrompt();if(!Prompt.IsEmpty())return Prompt;
    if(Phase==ECourierPhase::Practice)
    {
        if(Tutorial==2 && Carts.Num()>0 && Carts[0]->bSlowedOnce)
            return Copy(TEXT("Lane cleared. Release Q and follow the marker to the parcel desk."),TEXT("车道通过！松开 Q，沿标记走到取件台。"));
        if(Tutorial==2 && Cooldown>0.f)
            return Copy(TEXT("Q is recharging. Wait for READY, then hold Q while crossing the blue lane."),TEXT("Q 正在充能。等到就绪后，按住 Q 走过蓝色车道。"));
        const int32 I=FMath::Clamp(Tutorial,0,3);return Copy(*TutorialText[I],*TutorialTextZh[I]);
    }
    if(Phase==ECourierPhase::AwaitPickup)
        return Copy(TEXT("E reads the brief. Enter accepts. The clock waits for you."),TEXT("E 阅读简报，Enter 确认接单。现在尚未计时。"));
    const auto* Player=UGameplayStatics::GetPlayerCharacter(this,0);if(!Player)return FString();
    const FVector P=Player->GetActorLocation();
    for(const auto& Lift:Platforms)if(Lift && Player->GetMovementBaseObject()==Lift->Mesh.Get())
    {
        const FVector End=Lift->Origin+Lift->Travel;
        if(FVector::Dist(Lift->GetActorLocation(),End)<45.f)
            return Copy(TEXT("At the landing. Walk forward to step off."),TEXT("到站了。向前走，离开平台。"));
        return Copy(TEXT("You are aboard. Release Q to ride at normal speed; step off at the landing."),TEXT("已经上台。松开 Q 恢复正常速度，到达平台后再走下去。"));
    }
    if(CurrentOrder==0 && P.X>4260 && P.X<4840 && FMath::Abs(P.Y)<340)
        return Copy(TEXT("Space jumps over the planters. You can also walk around them."),TEXT("按空格跳过花箱，也可以从旁边绕过。"));
    const ACourierCart* NextCart=nullptr;
    for(const auto& Cart:Carts)if(Cart && !Cart->bTraining && Cart->OrderIndex==CurrentOrder &&
        Cart->Origin.X>=P.X-120.f && Cart->Origin.X-P.X<900.f && FMath::Abs(P.Y)<570.f &&
        (!NextCart || Cart->Origin.X<NextCart->Origin.X))NextCart=Cart;
    if(NextCart)
        return Copy(TEXT("Wait until the cart moves aside. Hold Q to keep the gap open, then cross."),TEXT("先等运输车让开，再按住 Q 延长空隙，趁机通过。"));
    for(const auto& Lift:Platforms)if(Lift && Lift->OrderIndex==CurrentOrder &&
        FVector::Dist2D(P,Lift->Origin)<1100.f && P.X<Lift->Origin.X+700.f)
    {
        if(Lift->Travel.Z>0.f && P.Z<Lift->Origin.Z+160.f && Lift->GetActorLocation().Z>P.Z+20.f)
            return Copy(TEXT("The lift is above you. Release Q and wait here for it to come down."),TEXT("升降台还在上方。松开 Q，在这里等它降下来。"));
        return Copy(TEXT("When the blue platform reaches your side, hold Q and press Space to jump aboard."),TEXT("等蓝色平台来到你这一侧，按住 Q，再按空格跳上去。"));
    }
    if(bSlowing)return Copy(TEXT("Blue devices are slower. You and the delivery clock move normally."),TEXT("蓝色装置正在减速。你的步伐和配送倒计时照常进行。"));
    if(FVector::Dist2D(P,Destination)<1800.f)
        return Copy(TEXT("Walk onto the orange square. Press E to deliver."),TEXT("走到橙色方形标记上，按 E 交付。"));
    return Copy(TEXT("Follow the orange route markers. C gently recenters your view."),TEXT("沿橙色路线标记前进。C 可平缓地将镜头回正。"));
}
