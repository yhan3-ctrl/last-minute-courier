#include "CourierPrototype.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Fonts/CompositeFont.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Text/STextBlock.h"

namespace CourierPalette
{
    const FLinearColor Paper=FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("F4EBD9")));
    const FLinearColor Ink=FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("243D43")));
    const FLinearColor Muted=FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("62716B")));
    const FLinearColor Orange=FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("C75A3B")));
    const FLinearColor OrangeHover=FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("DA6A48")));
    const FLinearColor Teal=FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("397D82")));
    const FLinearColor Rule=FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("D1C8B7")));
}

class SCourierOverlay final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCourierOverlay) {}
        SLATE_ARGUMENT(TWeakObjectPtr<ACourierGameMode>, Game)
    SLATE_END_ARGS()

    void Construct(const FArguments& Args)
    {
        Mode=Args._Game;
        for(const FString& Code:ACourierGameMode::Languages())LanguageOptions.Add(MakeShared<FString>(Code));
        // This small live HUD must repaint as one unit. Cached static text was
        // intermittently omitted beside timer updates in the Mac Metal preview.
        ForceVolatile(true);
        PaperBrush=FSlateRoundedBoxBrush(CourierPalette::Paper,8.f);
        ShadeBrush=FSlateColorBrush(FLinearColor(.018f,.03f,.032f,.68f));
        RuleBrush=FSlateColorBrush(CourierPalette::Rule);
        PrimaryStyle.SetNormal(FSlateRoundedBoxBrush(CourierPalette::Orange,4.f))
            .SetHovered(FSlateRoundedBoxBrush(CourierPalette::OrangeHover,4.f))
            .SetPressed(FSlateRoundedBoxBrush(CourierPalette::Ink,4.f))
            .SetNormalPadding(FMargin(22,12)).SetPressedPadding(FMargin(22,12));
        QuietStyle.SetNormal(FSlateRoundedBoxBrush(CourierPalette::Paper,4.f,CourierPalette::Rule,1.f))
            .SetHovered(FSlateRoundedBoxBrush(FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("E9DECA"))),4.f))
            .SetPressed(FSlateRoundedBoxBrush(CourierPalette::Rule,4.f))
            .SetNormalPadding(FMargin(18,12)).SetPressedPadding(FMargin(18,12));
        ChargeStyle.SetBackgroundImage(FSlateRoundedBoxBrush(CourierPalette::Rule,3.f))
            .SetFillImage(FSlateRoundedBoxBrush(CourierPalette::Teal,3.f));
        LanguageRowStyle.SetEvenRowBackgroundBrush(FSlateColorBrush(CourierPalette::Paper))
            .SetOddRowBackgroundBrush(FSlateColorBrush(CourierPalette::Paper))
            .SetEvenRowBackgroundHoveredBrush(FSlateColorBrush(CourierPalette::Rule))
            .SetOddRowBackgroundHoveredBrush(FSlateColorBrush(CourierPalette::Rule))
            .SetActiveBrush(FSlateColorBrush(CourierPalette::Rule))
            .SetInactiveBrush(FSlateColorBrush(CourierPalette::Rule))
            .SetTextColor(CourierPalette::Ink).SetSelectedTextColor(CourierPalette::Ink);
        ChildSlot[SAssignNew(Root,SBox)];
        Rebuild();
    }

    virtual void Tick(const FGeometry& Geometry,const double Time,const float Delta) override
    {
        SCompoundWidget::Tick(Geometry,Time,Delta);
        if (Mode.IsValid() && (LastPhase!=Mode->Phase || LastCard!=Mode->Card || LastLanguage!=Mode->Language || LastHelp!=Mode->bHelp))
            Rebuild();
    }

private:
    TWeakObjectPtr<ACourierGameMode> Mode;
    TSharedPtr<SBox> Root;
    ECourierPhase LastPhase=ECourierPhase::Opening;
    int32 LastCard=-1;
    FString LastLanguage;
    bool LastHelp=false;
    FSlateBrush PaperBrush,ShadeBrush,RuleBrush;
    FButtonStyle PrimaryStyle,QuietStyle;
    FProgressBarStyle ChargeStyle;
    FTableRowStyle LanguageRowStyle;
    TArray<TSharedPtr<FString>> LanguageOptions;

    FText Copy(const TCHAR* En,const TCHAR* Zh) const
    {
        return FText::FromString(Mode->Copy(En,Zh));
    }

    FSlateFontInfo Typeface(int32 Size,bool Title=false,bool ForceChinese=false) const
    {
        const bool Chinese=ForceChinese || (Mode.IsValid() && Mode->UsesCjkFont());
        const TCHAR* Name=Chinese?
            (Mode->Language==TEXT("ja")?TEXT("NotoSansJP-Regular.otf"):(Mode->Language==TEXT("ko")?TEXT("NotoSansKR-Regular.otf"):TEXT("NotoSansSC-Regular.otf"))):
            (Title?TEXT("BreeSerif-Regular.ttf"):TEXT("AtkinsonHyperlegible-Regular.ttf"));
        static TMap<FString,TSharedPtr<const FCompositeFont>> Fonts;
        const FString Path=FPaths::ProjectContentDir()/TEXT("Fonts")/Name;
        auto& Font=Fonts.FindOrAdd(Path);
        if(!Font.IsValid())Font=MakeShared<FCompositeFont>(FName(TEXT("Regular")),Path,EFontHinting::Default,EFontLoadingPolicy::LazyLoad);
        return FSlateFontInfo(Font,Size);
    }

    TSharedRef<STextBlock> Text(const FText& Value,int32 Size=21,bool Title=false,FLinearColor Color=CourierPalette::Ink,float WrapWidth=0) const
    {
        return SNew(STextBlock).Text(Value).Font(Typeface(Size,Title)).ColorAndOpacity(Color)
            .SimpleTextMode(WrapWidth<=0).WrapTextAt(WrapWidth).LineHeightPercentage(1.25f);
    }

    TSharedRef<SWidget> Action(const FText& Caption,int32 Id,bool Primary=false,bool ChineseLabel=false)
    {
        return SNew(SButton).ButtonStyle(Primary?&PrimaryStyle:&QuietStyle).IsFocusable(false)
            .IsEnabled(Id!=4 || Mode->Phase!=ECourierPhase::Paused || Mode->BeforePause!=ECourierPhase::Practice)
            .OnClicked_Lambda([this,Id]()
            {
                if(!Mode.IsValid()) return FReply::Unhandled();
                switch(Id)
                {
                    case 0:Mode->Continue();break;
                    case 1:Mode->Back();break;
                    case 2:Mode->TogglePause();break;
                    case 3:Mode->ToggleHelp();break;
                    case 4:Mode->Retry();break;
                }
                return FReply::Handled();
            })
            [SNew(STextBlock).Text(Caption).Font(Typeface(18,false,ChineseLabel))
                .ColorAndOpacity(Primary?CourierPalette::Paper:CourierPalette::Ink)];
    }

    TSharedRef<SWidget> Languages()
    {
        TSharedPtr<FString> Selected;
        for(const auto& Option:LanguageOptions)if(*Option==Mode->Language)Selected=Option;
        return SNew(SHorizontalBox)
            +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,12,0)
                [Text(Copy(TEXT("Language"),TEXT("语言")),16,false,CourierPalette::Muted)]
            +SHorizontalBox::Slot().AutoWidth()
                [SNew(SBox).WidthOverride(300)
                    [SNew(SComboBox<TSharedPtr<FString>>).OptionsSource(&LanguageOptions).InitiallySelectedItem(Selected)
                        .ButtonStyle(&QuietStyle).ItemStyle(&LanguageRowStyle).ForegroundColor(CourierPalette::Ink)
                        .MaxListHeight(400).EnableGamepadNavigationMode(true)
                        .OnGenerateWidget_Lambda([this](TSharedPtr<FString> Code)->TSharedRef<SWidget>
                        {
                            const bool Cjk=*Code==TEXT("zh") || *Code==TEXT("ja") || *Code==TEXT("ko");
                            return SNew(SBox).Padding(FMargin(14,8))
                                [SNew(STextBlock).Text(FText::FromString(ACourierGameMode::LanguageName(*Code)))
                                    .Font(Typeface(18,false,Cjk)).ColorAndOpacity(CourierPalette::Ink).SimpleTextMode(true)];
                        })
                        .OnSelectionChanged_Lambda([this](TSharedPtr<FString> Code,ESelectInfo::Type)
                        {if(Mode.IsValid() && Code.IsValid())Mode->SetLanguage(*Code);})
                        [SNew(STextBlock).Text(FText::FromString(ACourierGameMode::LanguageName(Mode->Language)))
                            .Font(Typeface(18)).ColorAndOpacity(CourierPalette::Ink).SimpleTextMode(true)]]];
    }

    TSharedRef<SWidget> Rule()
    {
        return SNew(SBox).HeightOverride(1)[SNew(SBorder).BorderImage(&RuleBrush).Padding(0)];
    }

    TSharedRef<SWidget> Menu(const FText& Title,const FText& Body,TSharedRef<SWidget> Actions,bool Comfort=false)
    {
        auto Column=SNew(SVerticalBox);
        Column->AddSlot().AutoHeight()
            [SNew(SHorizontalBox)
                +SHorizontalBox::Slot().FillWidth(1)[Text(Copy(TEXT("CAMPUS DELIVERY SERVICE"),TEXT("校园配送服务")),14,false,CourierPalette::Muted)]
                +SHorizontalBox::Slot().AutoWidth()[Text(FText::FromString(TEXT("LAST MINUTE COURIER")),14,false,CourierPalette::Orange)]];
        Column->AddSlot().AutoHeight().Padding(0,18,0,20)[Rule()];
        Column->AddSlot().AutoHeight().Padding(0,0,0,20)
            [SNew(STextBlock).Text(Title).Font(Typeface(42,true)).ColorAndOpacity(CourierPalette::Ink).WrapTextAt(820).LineHeightPercentage(1.25f)];
        Column->AddSlot().AutoHeight().Padding(0,0,0,28)
            [SNew(STextBlock).Text(Body).Font(Typeface(22)).ColorAndOpacity(CourierPalette::Ink).WrapTextAt(820).LineHeightPercentage(1.25f)];
        if(Comfort) Column->AddSlot().AutoHeight().Padding(0,0,0,22)[CameraSettings()];
        Column->AddSlot().AutoHeight().Padding(0,0,0,24)[Actions];
        Column->AddSlot().AutoHeight().Padding(0,0,0,16)[Rule()];
        Column->AddSlot().AutoHeight()[Languages()];
        Column->AddSlot().AutoHeight().Padding(0,15,0,0)
            [Text(Copy(TEXT("Enter to continue  /  Backspace to go back  /  L to switch language"),TEXT("Enter 继续 / Backspace 返回 / L 切换语言")),14,false,CourierPalette::Muted,820)];
        return SNew(SOverlay)
            +SOverlay::Slot()[SNew(SBorder).BorderImage(&ShadeBrush)]
            +SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(30)
                [SNew(SScaleBox).Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly)
                    [SNew(SBox).WidthOverride(900)
                        [SNew(SBorder).BorderImage(&PaperBrush).Padding(FMargin(40,30))[Column]]]];
    }

    TSharedRef<SWidget> CameraSettings()
    {
        return SNew(SVerticalBox)
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,8)
                [Text(Copy(TEXT("Camera comfort"),TEXT("镜头舒适度")),20,true)]
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,10)
                [SNew(SCheckBox)
                    .IsChecked_Lambda([this](){return Mode->bLockCamera?ECheckBoxState::Checked:ECheckBoxState::Unchecked;})
                    .OnCheckStateChanged_Lambda([this](ECheckBoxState Value){if(Mode.IsValid())Mode->bLockCamera=Value==ECheckBoxState::Checked;})
                    [Text(Copy(TEXT("Lock camera direction"),TEXT("锁定镜头方向")),18)]]
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,8)
                [SNew(SHorizontalBox)
                    +SHorizontalBox::Slot().FillWidth(1)[Text(Copy(TEXT("Mouse sensitivity"),TEXT("鼠标灵敏度")),18)]
                    +SHorizontalBox::Slot().FillWidth(1)
                        [SNew(SSlider).Value((Mode->MouseSensitivity-.2f)/1.4f).StepSize(.05f)
                            .OnValueChanged_Lambda([this](float V){if(Mode.IsValid())Mode->MouseSensitivity=.2f+V*1.4f;})]]
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,8)
                [SNew(SHorizontalBox)
                    +SHorizontalBox::Slot().FillWidth(1)[Text(Copy(TEXT("Field of view  (80–100)"),TEXT("视野范围（80–100）")),18)]
                    +SHorizontalBox::Slot().FillWidth(1)
                        [SNew(SSlider).Value((Mode->CameraFov-80.f)/20.f).StepSize(.05f)
                            .OnValueChanged_Lambda([this](float V)
                            {
                                if(!Mode.IsValid())return;
                                Mode->CameraFov=80.f+V*20.f;
                                if(auto* Player=Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(Mode.Get(),0)))
                                    Player->SetCameraFov(Mode->CameraFov);
                            })]]
            +SVerticalBox::Slot().AutoHeight()
                [Text(Copy(TEXT("Steady camera. No head bob, camera shake, speed zoom or motion blur."),TEXT("稳定镜头；无走路晃动、震屏、速度变焦或运动模糊。")),15,false,CourierPalette::Muted,820)]
            +SVerticalBox::Slot().AutoHeight().Padding(0,6,0,0)
                [Text(Copy(TEXT("Keyboard: Tab selects controls; Space toggles camera lock; Enter + arrows adjust sliders."),TEXT("键盘：Tab 选择设置；空格切换镜头锁定；Enter + 方向键调整滑条。")),14,false,CourierPalette::Muted,820)];
    }

    TSharedRef<SWidget> Gameplay()
    {
        const auto CardBox=[this](TSharedRef<SWidget> Content)
        {
            return SNew(SBorder).BorderImage(&PaperBrush).Padding(FMargin(20,14))[Content];
        };
        auto Overlay=SNew(SOverlay);
        Overlay->AddSlot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(24)
            [SNew(SBox).WidthOverride(350)
                [CardBox(SNew(SVerticalBox)
                    +SVerticalBox::Slot().AutoHeight()[Text(Copy(TEXT("DELIVERY / 01"),TEXT("配送 / 01")),14,false,CourierPalette::Orange)]
                    +SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)
                        [Text(Mode->Phase==ECourierPhase::Delivery?Copy(TEXT("A little warmth"),TEXT("把暖意送到")):Copy(TEXT("Practice yard"),TEXT("练习区")),27,true)]
                    +SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)
                        [SNew(STextBlock).Text(Mode->Phase==ECourierPhase::Delivery?Copy(TEXT("Destination: Coffee Stand"),TEXT("目的地：咖啡摊")):Copy(TEXT("Follow the marked path to the parcel desk."),TEXT("沿标记路线前往取件台。")))
                            .Font(Typeface(17)).ColorAndOpacity(CourierPalette::Ink).WrapTextAt(310).LineHeightPercentage(1.25f)])]];
        if(Mode->Phase==ECourierPhase::Delivery)
        {
            Overlay->AddSlot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(24)
                [SNew(SBox).WidthOverride(170)
                    [CardBox(SNew(SVerticalBox)
                        +SVerticalBox::Slot().AutoHeight()[Text(Copy(TEXT("TIME LEFT"),TEXT("剩余时间")),14,false,CourierPalette::Muted)]
                        +SVerticalBox::Slot().AutoHeight()
                            [SNew(STextBlock).Font(Typeface(38)).ColorAndOpacity(CourierPalette::Ink)
                                .Text_Lambda([this]()
                                {
                                    const int32 Sec=Mode.IsValid()?FMath::CeilToInt(Mode->Remaining):0;
                                    return FText::FromString(FString::Printf(TEXT("%02d:%02d"),Sec/60,Sec%60));
                                })])]];
        }
        Overlay->AddSlot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(24)
            [SNew(SBox).WidthOverride(270)
                [CardBox(SNew(SVerticalBox)
                    +SVerticalBox::Slot().AutoHeight()
                        [SNew(STextBlock).Font(Typeface(17)).ColorAndOpacity(CourierPalette::Ink).SimpleTextMode(true)
                            .Text_Lambda([this]()
                            {
                                return Mode->Cooldown>0.f?Copy(TEXT("Recharging…"),TEXT("充能中…")):Copy(TEXT("Hold Q / Borrow time"),TEXT("按住 Q / 借秒器"));
                            })]
                    +SVerticalBox::Slot().AutoHeight().Padding(0,10,0,0)
                        [SNew(SBox).HeightOverride(8)[SNew(SProgressBar).Style(&ChargeStyle).FillColorAndOpacity(FLinearColor::White)
                            .Percent_Lambda([this]()->TOptional<float>
                            {
                                if(!Mode.IsValid())return 0.f;
                                return Mode->Cooldown>0.f?1.f-Mode->Cooldown/4.f:Mode->SlowRemaining/3.f;
                            })]])]];
        Overlay->AddSlot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(24)
            [CardBox(Text(Copy(TEXT("Esc  Pause / camera settings"),TEXT("Esc 暂停 / 镜头设置")),16))];
        Overlay->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(FMargin(35,0,35,125))
            [SNew(SBox).MaxDesiredWidth(720)
                [CardBox(SNew(STextBlock).Font(Typeface(21)).ColorAndOpacity(CourierPalette::Ink)
                    .WrapTextAt(650).LineHeightPercentage(1.2f)
                    .Text_Lambda([this]()
                    {
                        const FString Prompt=Mode->InteractionPrompt();
                        if(!Prompt.IsEmpty())return FText::FromString(Prompt);
                        if(Mode->Phase==ECourierPhase::Practice)
                        {
                            const int32 I=FMath::Clamp(Mode->Tutorial,0,3);
                            return FText::FromString(Mode->Copy(*Mode->TutorialText[I],*Mode->TutorialTextZh[I]));
                        }
                        if(Mode->Phase==ECourierPhase::AwaitPickup)
                            return Copy(TEXT("Find the parcel desk. Your delivery clock has not started."),TEXT("找到取件台。订单倒计时尚未开始。"));
                        return Copy(TEXT("Coffee Stand ahead. Pick a gap, then go."),TEXT("咖啡摊就在前面。找准空隙再出发。"));
                    }))]];
        Overlay->SetVisibility(EVisibility::HitTestInvisible);
        return Overlay;
    }

    void Rebuild()
    {
        if(!Mode.IsValid())return;
        LastPhase=Mode->Phase;LastCard=Mode->Card;LastLanguage=Mode->Language;LastHelp=Mode->bHelp;
        if(Mode->CanMove())
        {
            Root->SetContent(Gameplay());
            return;
        }
        auto Actions=SNew(SHorizontalBox);
        if(Mode->Phase==ECourierPhase::Opening)
        {
            const int32 I=FMath::Clamp(Mode->Card,0,2);
            Actions->AddSlot().AutoWidth().Padding(0,0,12,0)
                [Action(I==2?Copy(TEXT("Start practice"),TEXT("进入练习区")):Copy(TEXT("Continue"),TEXT("继续")),0,true)];
            if(I>0)Actions->AddSlot().AutoWidth()[Action(Copy(TEXT("Back"),TEXT("返回")),1)];
            Actions->AddSlot().FillWidth(1).HAlign(HAlign_Right).VAlign(VAlign_Center)
                [Text(FText::FromString(FString::Printf(TEXT("%02d / 03"),I+1)),16,false,CourierPalette::Muted)];
            Root->SetContent(Menu(FText::FromString(Mode->Copy(*Mode->CardTitles[I],*Mode->CardTitlesZh[I])),
                FText::FromString(Mode->Copy(*Mode->CardBodies[I],*Mode->CardBodiesZh[I])),Actions));
        }
        else if(Mode->Phase==ECourierPhase::Paused)
        {
            Actions->AddSlot().AutoWidth().Padding(0,0,12,0)[Action(Copy(TEXT("Resume delivery"),TEXT("继续配送")),2,true)];
            Actions->AddSlot().AutoWidth().Padding(0,0,12,0)[Action(Copy(TEXT("Story & controls"),TEXT("故事与操作")),3)];
            Actions->AddSlot().AutoWidth()[Action(Copy(TEXT("Retry order"),TEXT("重试本单")),4)];
            const FText Body=Mode->bHelp?
                Copy(TEXT("Movie night needs three deliveries. Hold Q to slow blue-tagged devices for up to 3 seconds; recharge takes 4 seconds. Your delivery clock keeps ticking. WASD moves, Space jumps, E collects and delivers. Progress lasts for this session."),TEXT("电影夜还缺三份急件。按住 Q 减慢蓝色标签装置，最长 3 秒，结束后充能 4 秒；订单倒计时照常进行。WASD 移动，空格跳跃，E 接单和交付。进度在本次运行中保留。")):
                Copy(TEXT("A moment to breathe. The clock and devices are paused."),TEXT("慢慢来。倒计时和装置都已暂停。"));
            Root->SetContent(Menu(Copy(TEXT("Take your time."),TEXT("慢慢来。")),Body,Actions,true));
        }
        else
        {
            const bool Success=Mode->Phase==ECourierPhase::Delivered;
            Actions->AddSlot().AutoWidth()[Action(Copy(TEXT("Try this delivery again"),TEXT("再配送一次")),4,true)];
            Root->SetContent(Menu(Success?Copy(TEXT("Coffee is ready."),TEXT("咖啡准备好了。")):Copy(TEXT("Almost there."),TEXT("差一点。")),
                Success?Copy(TEXT("The stand is lit and the first delivery is complete. This preview contains the first route; the projector and film-reel deliveries are still being built."),TEXT("咖啡摊亮起来了，第一单完成。当前预览包含第一条路线，镜头和胶片配送仍在制作中。")):
                Copy(TEXT("Give it another go. The clock starts when you collect the package."),TEXT("再试一次吧。领取包裹后才开始计时。")),Actions));
        }
    }
};

void ACourierHUD::BeginPlay()
{
    Super::BeginPlay();
    auto* Mode=Cast<ACourierGameMode>(UGameplayStatics::GetGameMode(this));
    if(Mode && GEngine && GEngine->GameViewport)
    {
        Interface=SNew(SCourierOverlay).Game(TWeakObjectPtr<ACourierGameMode>(Mode));
        GEngine->GameViewport->AddViewportWidgetContent(Interface.ToSharedRef(),10);
    }
}

void ACourierHUD::EndPlay(const EEndPlayReason::Type Reason)
{
    if(Interface.IsValid() && GEngine && GEngine->GameViewport)
        GEngine->GameViewport->RemoveViewportWidgetContent(Interface.ToSharedRef());
    Interface.Reset();
    Super::EndPlay(Reason);
}
