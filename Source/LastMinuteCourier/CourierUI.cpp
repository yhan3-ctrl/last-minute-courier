#include "CourierPrototype.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/Paths.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Fonts/CompositeFont.h"
#include "Rendering/SlateRenderTransform.h"
#include "Styling/SlateTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Text/STextBlock.h"

// Visual system (v0.5.4): a courier's paper slip for menus, compact dark plates
// for the in-play HUD, key caps for every control, Lilita One for display type
// and Nunito for interface text. CJK languages use the bundled Noto faces.
namespace CourierPalette
{
    inline FLinearColor Hex(const TCHAR* Value,float Alpha=1.f)
    {
        FLinearColor C=FLinearColor::FromSRGBColor(FColor::FromHex(Value));C.A=Alpha;return C;
    }
    const FLinearColor Ink=Hex(TEXT("1E3036"));
    const FLinearColor Plate=Hex(TEXT("142429"),.86f);
    const FLinearColor Paper=Hex(TEXT("F7EFDF"));
    const FLinearColor PaperDeep=Hex(TEXT("E9DCC2"));
    const FLinearColor Cream=Hex(TEXT("FFF6E6"));
    const FLinearColor CreamMuted=Hex(TEXT("FFF6E6"),.72f);
    const FLinearColor Muted=Hex(TEXT("7A7C72"));
    const FLinearColor Orange=Hex(TEXT("E2622C"));
    const FLinearColor OrangeHover=Hex(TEXT("F0763F"));
    const FLinearColor Teal=Hex(TEXT("2E8C95"));
    const FLinearColor Cyan=Hex(TEXT("3CC4D8"));
    const FLinearColor Red=Hex(TEXT("D9432F"));
    const FLinearColor Gold=Hex(TEXT("E2AE45"));
    const FLinearColor KeyEdge=Hex(TEXT("B7A88C"));
    const FLinearColor Shadow=FLinearColor(0.f,0.f,0.f,.32f);
}

enum class EFace : uint8 {Display,Black,Heavy,Bold,Digits,Symbol};

class SCourierOverlay final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCourierOverlay) {}
        SLATE_ARGUMENT(TWeakObjectPtr<ACourierGameMode>, Game)
    SLATE_END_ARGS()

    void Construct(const FArguments& Args)
    {
        using namespace CourierPalette;
        Mode=Args._Game;
        for(const FString& Code:ACourierGameMode::Languages())LanguageOptions.Add(MakeShared<FString>(Code));
        // This small live HUD must repaint as one unit. Cached static text was
        // intermittently omitted beside timer updates in the Mac Metal preview.
        ForceVolatile(true);
        // White brushes are tinted per use through BorderBackgroundColor.
        R4=FSlateRoundedBoxBrush(FLinearColor::White,4.f);
        R6=FSlateRoundedBoxBrush(FLinearColor::White,6.f);
        R12=FSlateRoundedBoxBrush(FLinearColor::White,12.f);
        R16=FSlateRoundedBoxBrush(FLinearColor::White,16.f);
        Pill=FSlateRoundedBoxBrush(FLinearColor::White,22.f);
        Top=FSlateRoundedBoxBrush(FLinearColor::White,FVector4(16,16,0,0));
        Bottom=FSlateRoundedBoxBrush(FLinearColor::White,FVector4(0,0,16,16));
        Left=FSlateRoundedBoxBrush(FLinearColor::White,FVector4(16,0,0,16));
        Right=FSlateRoundedBoxBrush(FLinearColor::White,FVector4(0,16,16,0));
        Shade=FSlateColorBrush(FLinearColor(.01f,.025f,.03f,.62f));
        Solid=FSlateColorBrush(FLinearColor::White);
        Stamp=FSlateRoundedBoxBrush(Hex(TEXT("FFF6E6"),.94f),12.f,Orange,5.f);
        StampSmall=FSlateRoundedBoxBrush(FLinearColor::Transparent,26.f,Orange,3.f);
        const auto Button=[](FButtonStyle& Style,const FLinearColor& Fill,const FLinearColor& Hover,const FLinearColor& Press,const FLinearColor* Outline)
        {
            if(Outline)
                Style.SetNormal(FSlateRoundedBoxBrush(Fill,10.f,*Outline,2.f)).SetHovered(FSlateRoundedBoxBrush(Hover,10.f,*Outline,2.f))
                    .SetPressed(FSlateRoundedBoxBrush(Press,10.f,*Outline,2.f));
            else
                Style.SetNormal(FSlateRoundedBoxBrush(Fill,10.f)).SetHovered(FSlateRoundedBoxBrush(Hover,10.f)).SetPressed(FSlateRoundedBoxBrush(Press,10.f));
            Style.SetDisabled(FSlateRoundedBoxBrush(FLinearColor(.5f,.5f,.5f,.25f),10.f))
                .SetNormalPadding(FMargin(16,10,16,11)).SetPressedPadding(FMargin(16,11,16,10));
        };
        Button(PrimaryStyle,Orange,OrangeHover,Hex(TEXT("B84A1D")),nullptr);
        Button(QuietStyle,Paper,PaperDeep,KeyEdge,&KeyEdge);
        const FLinearColor Glass(1,1,1,.12f),GlassHover(1,1,1,.22f),GlassPress(1,1,1,.06f),GlassLine(1,1,1,.35f);
        Button(DarkStyle,Glass,GlassHover,GlassPress,&GlassLine);
        ChargeStyle.SetBackgroundImage(FSlateRoundedBoxBrush(FLinearColor(1,1,1,.16f),4.f))
            .SetFillImage(FSlateRoundedBoxBrush(FLinearColor::White,4.f));
        LanguageRowStyle.SetEvenRowBackgroundBrush(FSlateColorBrush(Paper))
            .SetOddRowBackgroundBrush(FSlateColorBrush(Paper))
            .SetEvenRowBackgroundHoveredBrush(FSlateColorBrush(PaperDeep))
            .SetOddRowBackgroundHoveredBrush(FSlateColorBrush(PaperDeep))
            .SetActiveBrush(FSlateColorBrush(PaperDeep))
            .SetInactiveBrush(FSlateColorBrush(PaperDeep))
            .SetTextColor(Ink).SetSelectedTextColor(Ink);
        const FSlateRoundedBoxBrush Thumb(Orange,10.f,FVector2f(20.f,20.f));
        const FSlateRoundedBoxBrush ThumbHover(OrangeHover,10.f,FVector2f(20.f,20.f));
        const FSlateRoundedBoxBrush Bar(PaperDeep,3.f);
        SliderStyle.SetNormalBarImage(Bar).SetHoveredBarImage(Bar).SetDisabledBarImage(Bar)
            .SetNormalThumbImage(Thumb).SetHoveredThumbImage(ThumbHover).SetDisabledThumbImage(Thumb).SetBarThickness(6.f);
        const FSlateRoundedBoxBrush BoxOff(Paper,5.f,KeyEdge,2.f,FVector2f(22.f,22.f));
        const FSlateRoundedBoxBrush BoxOffHover(PaperDeep,5.f,KeyEdge,2.f,FVector2f(22.f,22.f));
        const FSlateRoundedBoxBrush BoxOn(Orange,5.f,Hex(TEXT("B84A1D")),2.f,FVector2f(22.f,22.f));
        CheckStyle.SetCheckBoxType(ESlateCheckBoxType::CheckBox)
            .SetUncheckedImage(BoxOff).SetUncheckedHoveredImage(BoxOffHover).SetUncheckedPressedImage(BoxOffHover)
            .SetCheckedImage(BoxOn).SetCheckedHoveredImage(BoxOn).SetCheckedPressedImage(BoxOn)
            .SetPadding(FMargin(10,0,0,0));
        ChildSlot[SAssignNew(Root,SBox)];
        Rebuild();
    }

    virtual void Tick(const FGeometry& Geometry,const double Time,const float Delta) override
    {
        SCompoundWidget::Tick(Geometry,Time,Delta);
        if (Mode.IsValid() && (LastPhase!=Mode->Phase || LastCard!=Mode->Card || LastLanguage!=Mode->Language ||
            LastHelp!=Mode->bHelp || LastOrder!=Mode->CurrentOrder || LastConfirm!=Mode->bConfirmRetry))
            Rebuild();
    }

private:
    TWeakObjectPtr<ACourierGameMode> Mode;
    TSharedPtr<SBox> Root;
    ECourierPhase LastPhase=ECourierPhase::Opening;
    int32 LastCard=-1;
    FString LastLanguage;
    int32 LastOrder=-1;
    bool LastHelp=false;
    bool LastConfirm=false;
    FSlateBrush R4,R6,R12,R16,Pill,Top,Bottom,Left,Right,Shade,Stamp,StampSmall,Solid;
    FButtonStyle PrimaryStyle,QuietStyle,DarkStyle;
    FProgressBarStyle ChargeStyle;
    FTableRowStyle LanguageRowStyle;
    FSliderStyle SliderStyle;
    FCheckBoxStyle CheckStyle;
    TArray<TSharedPtr<FString>> LanguageOptions;

    enum class EButton : uint8 {Primary,Quiet,Dark};

    FText Copy(const TCHAR* En,const TCHAR* Zh) const
    {
        return FText::FromString(Mode->Copy(En,Zh));
    }

    FSlateFontInfo Typeface(int32 Size,EFace Face=EFace::Bold,const FString& FontLanguage=FString()) const
    {
        const FString Locale=FontLanguage.IsEmpty()?Mode->Language:FontLanguage;
        const bool Cjk=Locale==TEXT("zh") || Locale==TEXT("ja") || Locale==TEXT("ko");
        const TCHAR* Name=TEXT("Nunito-Bold.ttf");
        if(Face==EFace::Digits)Name=TEXT("LilitaOne-Regular.ttf");
        else if(Face==EFace::Symbol)Name=TEXT("NotoSansSC-Regular.otf");
        else if(Cjk)Name=Locale==TEXT("ja")?TEXT("NotoSansJP-Regular.otf"):(Locale==TEXT("ko")?TEXT("NotoSansKR-Regular.otf"):TEXT("NotoSansSC-Regular.otf"));
        else if(Face==EFace::Display)Name=TEXT("LilitaOne-Regular.ttf");
        else if(Face==EFace::Black)Name=TEXT("Nunito-Black.ttf");
        else if(Face==EFace::Heavy)Name=TEXT("Nunito-ExtraBold.ttf");
        static TMap<FString,TSharedPtr<const FCompositeFont>> Fonts;
        const FString Path=FPaths::ProjectContentDir()/TEXT("Fonts")/Name;
        auto& Font=Fonts.FindOrAdd(Path);
        if(!Font.IsValid())Font=MakeShared<FCompositeFont>(FName(TEXT("Regular")),Path,EFontHinting::Default,EFontLoadingPolicy::LazyLoad);
        return FSlateFontInfo(Font,Size);
    }

    TSharedRef<STextBlock> Text(const FText& Value,int32 Size,EFace Face,const FLinearColor& Color,float WrapWidth=0,ETextJustify::Type Justify=ETextJustify::Left) const
    {
        return SNew(STextBlock).Text(Value).Font(Typeface(Size,Face)).ColorAndOpacity(Color).Justification(Justify)
            .SimpleTextMode(WrapWidth<=0).WrapTextAt(WrapWidth)
            .WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping).LineHeightPercentage(1.22f);
    }

    TSharedRef<SWidget> Box(const FSlateBrush& Brush,const FLinearColor& Color,const FMargin& Padding,TSharedRef<SWidget> Content) const
    {
        return SNew(SBorder).BorderImage(&Brush).BorderBackgroundColor(Color).Padding(Padding)[Content];
    }

    // A physical-looking key cap: paper face on a darker edge.
    TSharedRef<SWidget> KeyCap(const FText& Label,int32 Size=15,TAttribute<FSlateColor> FaceColor=FSlateColor(CourierPalette::Paper),
        const FLinearColor& TextColor=CourierPalette::Ink) const
    {
        return SNew(SBox).MinDesiredWidth(Size*2.f)
            [SNew(SBorder).BorderImage(&R6).BorderBackgroundColor(CourierPalette::KeyEdge).Padding(FMargin(0,0,0,3))
                [SNew(SBorder).BorderImage(&R6).BorderBackgroundColor(FaceColor).Padding(FMargin(9,2,9,1))
                    .HAlign(HAlign_Center).VAlign(VAlign_Center)
                    [Text(Label,Size,EFace::Black,TextColor)]]];
    }

    TSharedRef<SWidget> Key(const TCHAR* Label,int32 Size=15) const {return KeyCap(FText::FromString(Label),Size);}

    TSharedRef<SWidget> Action(const FText& Caption,int32 Id,EButton Kind=EButton::Quiet,const TCHAR* KeyLabel=nullptr)
    {
        const FLinearColor Ink=Kind==EButton::Quiet?CourierPalette::Ink:CourierPalette::Cream;
        auto Content=SNew(SHorizontalBox)
            +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[Text(Caption,19,EFace::Black,Ink)];
        if(KeyLabel)
            Content->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(12,0,0,0)
                [Box(R4,Kind==EButton::Quiet?FLinearColor(.12f,.19f,.21f,.1f):FLinearColor(1,1,1,.22f),FMargin(7,1,7,2),
                    Text(FText::FromString(KeyLabel),13,EFace::Heavy,Ink))];
        return SNew(SButton).ButtonStyle(Kind==EButton::Primary?&PrimaryStyle:(Kind==EButton::Dark?&DarkStyle:&QuietStyle))
            .IsFocusable(true)
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
                    case 7:UKismetSystemLibrary::QuitGame(Mode.Get(),UGameplayStatics::GetPlayerController(Mode.Get(),0),EQuitPreference::Quit,false);break;
                    case 8:Mode->RecenterCamera();break;
                }
                return FReply::Handled();
            })
            [Content];
    }

    TSharedRef<SWidget> Languages()
    {
        TSharedPtr<FString> Selected;
        for(const auto& Option:LanguageOptions)if(*Option==Mode->Language)Selected=Option;
        return SNew(SBox).WidthOverride(230)
            [SNew(SComboBox<TSharedPtr<FString>>).OptionsSource(&LanguageOptions).InitiallySelectedItem(Selected)
                .ButtonStyle(&DarkStyle).ItemStyle(&LanguageRowStyle).ForegroundColor(CourierPalette::Cream)
                .Method(EPopupMethod::UseCurrentWindow)
                .MaxListHeight(400).EnableGamepadNavigationMode(true)
                .OnGenerateWidget_Lambda([this](TSharedPtr<FString> Code)->TSharedRef<SWidget>
                {
                    return SNew(SBox).Padding(FMargin(14,8))
                        [SNew(STextBlock).Text(FText::FromString(ACourierGameMode::LanguageName(*Code)))
                            .Font(Typeface(18,EFace::Bold,*Code)).ColorAndOpacity(CourierPalette::Ink).SimpleTextMode(true)];
                })
                .OnSelectionChanged_Lambda([this](TSharedPtr<FString> Code,ESelectInfo::Type)
                {if(Mode.IsValid() && Code.IsValid())Mode->SetLanguage(*Code);})
                [SNew(STextBlock).Text(FText::FromString(ACourierGameMode::LanguageName(Mode->Language)))
                    .Font(Typeface(17,EFace::Heavy)).ColorAndOpacity(CourierPalette::Cream).SimpleTextMode(true)]];
    }

    // Under-card strip for language, quitting and keyboard hints.
    TSharedRef<SWidget> UtilityBar(bool bLanguage=true)
    {
        auto Row=SNew(SHorizontalBox);
        if(bLanguage)
        {
            Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,10,0)
                [Text(Copy(TEXT("Language"),TEXT("语言")),15,EFace::Heavy,CourierPalette::CreamMuted)];
            Row->AddSlot().AutoWidth().VAlign(VAlign_Center)[Languages()];
            Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(8,0,0,0)[Key(TEXT("L"),12)];
        }
        Row->AddSlot().FillWidth(1)[SNew(SSpacer)];
        Row->AddSlot().AutoWidth().VAlign(VAlign_Center)[Action(Copy(TEXT("Quit game"),TEXT("退出游戏")),7,EButton::Dark)];
        return Row;
    }

    TSharedRef<SWidget> Perforation(int32 Count=38) const
    {
        auto Row=SNew(SHorizontalBox);
        for(int32 I=0;I<Count;++I)
            Row->AddSlot().FillWidth(1).HAlign(HAlign_Center)
                [SNew(SBox).WidthOverride(9).HeightOverride(3)[SNew(SBorder).BorderImage(&R4).BorderBackgroundColor(CourierPalette::PaperDeep)]];
        return Row;
    }

    TSharedRef<SWidget> Pips(int32 Index,int32 Count,bool bDark) const
    {
        auto Row=SNew(SHorizontalBox);
        for(int32 I=0;I<Count;++I)
        {
            const FLinearColor Color=I<Index?CourierPalette::Gold:(I==Index?CourierPalette::Orange:
                (bDark?FLinearColor(1,1,1,.22f):CourierPalette::PaperDeep));
            Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,I+1<Count?7:0,0)
                [SNew(SBox).WidthOverride(I==Index?24:11).HeightOverride(11)[SNew(SBorder).BorderImage(&R6).BorderBackgroundColor(Color)]];
        }
        return Row;
    }

    TSharedRef<SWidget> SlipHeader(const FText& RightLabel) const
    {
        return SNew(SHorizontalBox)
            +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [Text(FText::FromString(TEXT("LAST MINUTE COURIER")),22,EFace::Digits,CourierPalette::Cream)]
            +SHorizontalBox::Slot().FillWidth(1)[SNew(SSpacer)]
            +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [Text(RightLabel,14,EFace::Heavy,CourierPalette::Cream)];
    }

    // The menu card: coloured header band, paper body, perforation, footer.
    TSharedRef<SWidget> Slip(const FLinearColor& Band,const FText& HeaderRight,TSharedRef<SWidget> Body,TSharedRef<SWidget> Footer,float Width=780) const
    {
        return SNew(SBox).WidthOverride(Width)
            [SNew(SBorder).BorderImage(&R16).BorderBackgroundColor(CourierPalette::Shadow).Padding(FMargin(0,0,0,7))
                [SNew(SVerticalBox)
                    +SVerticalBox::Slot().AutoHeight()[Box(Top,Band,FMargin(34,13,34,13),SlipHeader(HeaderRight))]
                    +SVerticalBox::Slot().AutoHeight()
                        [Box(Bottom,CourierPalette::Paper,FMargin(38,30,38,24),
                            SNew(SVerticalBox)
                                +SVerticalBox::Slot().AutoHeight()[Body]
                                +SVerticalBox::Slot().AutoHeight().Padding(0,26,0,20)[Perforation()]
                                +SVerticalBox::Slot().AutoHeight()[Footer])]]];
    }

    TSharedRef<SWidget> Screen(TSharedRef<SWidget> Card,TSharedRef<SWidget> Under,float Width=780)
    {
        return SNew(SOverlay)
            +SOverlay::Slot()[SNew(SBorder).BorderImage(&Shade)]
            +SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(24)
                [SNew(SScaleBox).Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly)
                    [SNew(SVerticalBox)
                        +SVerticalBox::Slot().AutoHeight()[Card]
                        +SVerticalBox::Slot().AutoHeight().Padding(4,16,4,0)[SNew(SBox).WidthOverride(Width-8)[Under]]]];
    }

    TSharedRef<SWidget> Title(const FText& Value,int32 Size=46) const
    {
        return Text(Value,Size,EFace::Display,CourierPalette::Ink,700);
    }

    TSharedRef<SWidget> Body(const FText& Value,int32 Size=21,const FLinearColor& Color=CourierPalette::Ink,float Wrap=700.f) const
    {
        return SNew(STextBlock).Text(Value).Font(Typeface(Size,EFace::Bold)).ColorAndOpacity(Color)
            .WrapTextAt(Wrap).WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping).LineHeightPercentage(1.32f);
    }

    TSharedRef<SWidget> ControlRow(std::initializer_list<FText> Keys,const FText& Label,float KeyWidth) const
    {
        auto Caps=SNew(SHorizontalBox);
        for(const FText& K:Keys)Caps->AddSlot().AutoWidth().Padding(4,0,0,0)[KeyCap(K,15)];
        return SNew(SHorizontalBox)
            +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(SBox).WidthOverride(KeyWidth).HAlign(HAlign_Right)[Caps]]
            +SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center).Padding(12,0,0,0)[Text(Label,17,EFace::Heavy,CourierPalette::Ink,250)];
    }

    TSharedRef<SWidget> ControlsGrid() const
    {
        const auto S=[](const TCHAR* V){return FText::FromString(V);};
        const auto Column=[](std::initializer_list<TSharedRef<SWidget>> Rows)
        {
            auto C=SNew(SVerticalBox);
            for(const auto& R:Rows)C->AddSlot().AutoHeight().Padding(0,0,0,12)[R];
            return C;
        };
        return SNew(SHorizontalBox)
            +SHorizontalBox::Slot().FillWidth(1.12f)
                [Column({ControlRow({S(TEXT("W")),S(TEXT("A")),S(TEXT("S")),S(TEXT("D"))},Copy(TEXT("Move"),TEXT("移动")),156),
                    ControlRow({Copy(TEXT("Mouse"),TEXT("鼠标"))},Copy(TEXT("Look around"),TEXT("转动视角")),156),
                    ControlRow({Copy(TEXT("Space"),TEXT("空格"))},Copy(TEXT("Jump"),TEXT("跳跃")),156),
                    ControlRow({S(TEXT("Q"))},Copy(TEXT("Hold: slow blue devices"),TEXT("按住：让蓝色装置变慢")),156)})]
            +SHorizontalBox::Slot().FillWidth(.88f).Padding(14,0,0,0)
                [Column({ControlRow({S(TEXT("E"))},Copy(TEXT("Collect / deliver"),TEXT("取件 / 交付")),50),
                    ControlRow({S(TEXT("C"))},Copy(TEXT("Recenter camera"),TEXT("镜头回正")),50),
                    ControlRow({S(TEXT("H"))},Copy(TEXT("Route help"),TEXT("路线帮助")),50),
                    ControlRow({S(TEXT("Esc"))},Copy(TEXT("Pause & settings"),TEXT("暂停与设置")),50)})];
    }

    TSharedRef<SWidget> Settings()
    {
        const auto Row=[this](const FText& Label,TSharedRef<SWidget> Control)
        {
            return SNew(SHorizontalBox)
                +SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[Text(Label,17,EFace::Heavy,CourierPalette::Ink)]
                +SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[Control];
        };
        const auto Check=[this](const FText& Label,TFunction<bool()> Get,TFunction<void(bool)> Set)
        {
            return SNew(SCheckBox).Style(&CheckStyle)
                .IsChecked_Lambda([Get](){return Get()?ECheckBoxState::Checked:ECheckBoxState::Unchecked;})
                .OnCheckStateChanged_Lambda([Set](ECheckBoxState V){Set(V==ECheckBoxState::Checked);})
                [Text(Label,17,EFace::Heavy,CourierPalette::Ink)];
        };
        return SNew(SVerticalBox)
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,12)
                [Text(Copy(TEXT("Camera comfort"),TEXT("镜头舒适度")),24,EFace::Display,CourierPalette::Teal)]
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,10)
                [SNew(SHorizontalBox)
                    +SHorizontalBox::Slot().FillWidth(1)
                        [Check(Copy(TEXT("Music & sound"),TEXT("音乐与音效")),[this](){return Mode.IsValid() && Mode->bAudioEnabled;},
                            [this](bool V){if(Mode.IsValid())Mode->SetAudioEnabled(V);})]
                    +SHorizontalBox::Slot().FillWidth(1)
                        [Check(Copy(TEXT("Lock camera direction"),TEXT("锁定镜头方向")),[this](){return Mode.IsValid() && Mode->bLockCamera;},
                            [this](bool V){if(Mode.IsValid())Mode->bLockCamera=V;})]]
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,8)
                [Row(Copy(TEXT("Mouse sensitivity"),TEXT("鼠标灵敏度")),
                    SNew(SSlider).Style(&SliderStyle).Value((Mode->MouseSensitivity-.2f)/1.4f).StepSize(.05f)
                        .OnValueChanged_Lambda([this](float V){if(Mode.IsValid())Mode->MouseSensitivity=.2f+V*1.4f;}))]
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,8)
                [Row(Copy(TEXT("Field of view  (80–100)"),TEXT("视野范围（80–100）")),
                    SNew(SSlider).Style(&SliderStyle).Value((Mode->CameraFov-80.f)/20.f).StepSize(.05f)
                        .OnValueChanged_Lambda([this](float V)
                        {
                            if(!Mode.IsValid())return;
                            Mode->CameraFov=80.f+V*20.f;
                            if(auto* Player=Cast<ACourierCharacter>(UGameplayStatics::GetPlayerCharacter(Mode.Get(),0)))
                                Player->SetCameraFov(Mode->CameraFov);
                        }))]
            +SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)
                [Text(Copy(TEXT("Steady camera. No head bob, camera shake, speed zoom or motion blur."),TEXT("稳定镜头；无走路晃动、震屏、速度变焦或运动模糊。")),15,EFace::Bold,CourierPalette::Muted,700)];
    }

    TSharedRef<SWidget> InfoCell(const FText& Label,TSharedRef<SWidget> Value) const
    {
        return SNew(SVerticalBox)
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,3)[Text(Label,13,EFace::Heavy,CourierPalette::Muted)]
            +SVerticalBox::Slot().AutoHeight()[Value];
    }

    TSharedRef<SWidget> StarRow(int32 Count,int32 Size,const FLinearColor& Off) const
    {
        auto Row=SNew(SHorizontalBox);
        for(int32 I=0;I<3;++I)
            Row->AddSlot().AutoWidth().Padding(0,0,I<2?4:0,0)
                [Text(FText::FromString(I<Count?TEXT("★"):TEXT("☆")),Size,EFace::Symbol,I<Count?CourierPalette::Gold:Off)];
        return Row;
    }

    FString StarRule() const
    {
        if(Mode->CurrentOrder==0)return Mode->Copy(TEXT("Stars: how warm the coffee is when it arrives."),TEXT("星级：送到时咖啡还有多热。"));
        if(Mode->CurrentOrder==1)return Mode->Copy(TEXT("Stars: how few knocks the lens takes. Carts and falls knock it; the third knock cracks it."),TEXT("星级：镜头被磕的次数越少越好。被车撞、摔下去都会磕到，第三次就裂了。"));
        return Mode->Copy(TEXT("Stars: how much time is left before the screening."),TEXT("星级：开场前还剩多少时间。"));
    }

    FString ResultLine() const
    {
        FString Line;
        const float Share=FMath::Clamp(Mode->Remaining/FMath::Max(1.f,Mode->OrderBudget()),0.f,1.f);
        if(Mode->CurrentOrder==0)Line=Mode->Copy(TEXT("COFFEE WARMTH"),TEXT("咖啡温度"))+FString::Printf(TEXT("  %d%%"),FMath::RoundToInt(Share*100.f));
        else if(Mode->CurrentOrder==1)Line=Mode->Copy(TEXT("LENS CASE"),TEXT("镜头盒"))+FString::Printf(TEXT("  %d / 3"),Mode->LensCondition);
        else Line=Mode->Copy(TEXT("TIME LEFT"),TEXT("剩余时间"))+TEXT("  ")+Clock(Mode->Remaining);
        return Line+TEXT("     ")+Mode->Copy(TEXT("BORROWER CELLS"),TEXT("借时器电池"))+
            FString::Printf(TEXT("  %d / %d"),Mode->TokensTaken(Mode->CurrentOrder),Mode->TokensTotal(Mode->CurrentOrder));
    }

    static FString Clock(float Seconds)
    {
        const int32 S=FMath::Max(0,FMath::CeilToInt(Seconds));
        return FString::Printf(TEXT("%d:%02d"),S/60,S%60);
    }

    FLinearColor OrderColor() const
    {
        return Mode->CurrentOrder==0?CourierPalette::Teal:(Mode->CurrentOrder==1?CourierPalette::Orange:CourierPalette::Ink);
    }

    FText OrderCount() const
    {
        return FText::FromString(Mode->Copy(TEXT("DELIVERY"),TEXT("配送"))+FString::Printf(TEXT("  %d / 3"),Mode->CurrentOrder+1));
    }

    // ---------------------------------------------------------------- screens

    TSharedRef<SWidget> Opening()
    {
        const int32 Count=Mode->CardTitles.Num();
        const int32 I=FMath::Clamp(Mode->Card,0,Count-1);
        const bool bLast=I==Count-1;
        auto Content=SNew(SVerticalBox);
        Content->AddSlot().AutoHeight().Padding(0,0,0,18)
            [Title(FText::FromString(Mode->Copy(*Mode->CardTitles[I],*Mode->CardTitlesZh[I])))];
        if(bLast)
        {
            Content->AddSlot().AutoHeight().Padding(0,4,0,10)[ControlsGrid()];
            Content->AddSlot().AutoHeight()[Body(FText::FromString(Mode->Copy(*Mode->CardBodies[I],*Mode->CardBodiesZh[I])),19,CourierPalette::Teal)];
        }
        else Content->AddSlot().AutoHeight()[Body(FText::FromString(Mode->Copy(*Mode->CardBodies[I],*Mode->CardBodiesZh[I])),22)];
        auto Footer=SNew(SHorizontalBox)
            +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[Pips(I,Count,false)]
            +SHorizontalBox::Slot().FillWidth(1)[SNew(SSpacer)];
        if(I>0)Footer->AddSlot().AutoWidth().Padding(0,0,12,0)[Action(Copy(TEXT("Back"),TEXT("返回")),1,EButton::Quiet,TEXT("Backspace"))];
        Footer->AddSlot().AutoWidth()
            [Action(bLast?Copy(TEXT("Start practice"),TEXT("进入练习区")):Copy(TEXT("Continue"),TEXT("继续")),0,EButton::Primary,TEXT("Enter"))];
        return Screen(Slip(CourierPalette::Orange,FText::FromString(FString::Printf(TEXT("%02d / %02d"),I+1,Count)),Content,Footer),UtilityBar());
    }

    TSharedRef<SWidget> Briefing()
    {
        using namespace CourierPalette;
        auto Content=SNew(SVerticalBox)
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,4)[Text(FText::FromString(Mode->OrderLabel()),16,EFace::Heavy,OrderColor())]
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,20)[Title(FText::FromString(Mode->OrderTitle()),44)]
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,20)
                [SNew(SHorizontalBox)
                    +SHorizontalBox::Slot().FillWidth(1.f).Padding(0,0,12,0)
                        [InfoCell(Copy(TEXT("PACKAGE"),TEXT("包裹")),Text(FText::FromString(Mode->OrderPackage()),19,EFace::Black,Ink,185))]
                    +SHorizontalBox::Slot().FillWidth(1.f).Padding(0,0,12,0)
                        [InfoCell(Copy(TEXT("DELIVER TO"),TEXT("送往")),Text(FText::FromString(Mode->OrderDestination()),19,EFace::Black,Ink,185))]
                    +SHorizontalBox::Slot().FillWidth(.62f)
                        [InfoCell(Copy(TEXT("TIME LIMIT"),TEXT("配送时限")),Text(FText::FromString(Clock(Mode->OrderBudget())),38,EFace::Digits,Orange))]
                    +SHorizontalBox::Slot().FillWidth(.62f)
                        [InfoCell(Copy(TEXT("Q USES"),TEXT("Q 次数")),Text(FText::FromString(FString::Printf(TEXT("×%d"),Mode->OrderCharges())),38,EFace::Digits,Teal))]]
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,16)[Body(FText::FromString(Mode->Copy(*Mode->Orders[Mode->CurrentOrder].Brief,*Mode->Orders[Mode->CurrentOrder].BriefZh)),20)]
            +SVerticalBox::Slot().AutoHeight()
                [Box(R12,PaperDeep,FMargin(16,12),
                    SNew(SVerticalBox)
                        +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,4)[Text(Copy(TEXT("ROUTE"),TEXT("路线")),13,EFace::Heavy,Orange)]
                        +SVerticalBox::Slot().AutoHeight()[Body(FText::FromString(Mode->RouteHint()),17,CourierPalette::Ink,660.f)]
                        +SVerticalBox::Slot().AutoHeight().Padding(0,8,0,0)
                            [SNew(SHorizontalBox)
                                +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,8,0)
                                    [SNew(SBox).WidthOverride(12).HeightOverride(12)[SNew(SBorder).BorderImage(&R6).BorderBackgroundColor(Gold)]]
                                +SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
                                    [Body(FText::FromString(Mode->Copy(TEXT("Borrower cells on this route give Q one more use each."),TEXT("路线上的借时器电池每个让 Q 多用一次。"))+FString::Printf(TEXT("  (%d)  "),Mode->TokensTotal(Mode->CurrentOrder))+StarRule()),16,CourierPalette::Ink,640.f)]])]
            +SVerticalBox::Slot().AutoHeight().Padding(0,14,0,0)
                [Text(Copy(TEXT("The clock starts when you accept."),TEXT("确认接单后才开始计时。")),15,EFace::Heavy,Teal)];
        auto Footer=SNew(SHorizontalBox)
            +SHorizontalBox::Slot().FillWidth(1)[SNew(SSpacer)]
            +SHorizontalBox::Slot().AutoWidth().Padding(0,0,12,0)[Action(Copy(TEXT("Back"),TEXT("返回")),1,EButton::Quiet,TEXT("Esc"))]
            +SHorizontalBox::Slot().AutoWidth()[Action(Copy(TEXT("Accept delivery"),TEXT("确认接单")),0,EButton::Primary,TEXT("E / Enter"))];
        return Screen(Slip(OrderColor(),OrderCount(),Content,Footer),UtilityBar(false));
    }

    TSharedRef<SWidget> Paused()
    {
        using namespace CourierPalette;
        auto Content=SNew(SVerticalBox);
        auto Footer=SNew(SHorizontalBox)+SHorizontalBox::Slot().FillWidth(1)[SNew(SSpacer)];
        if(Mode->bConfirmRetry)
        {
            Content->AddSlot().AutoHeight().Padding(0,0,0,16)[Title(Copy(TEXT("Restart this order?"),TEXT("重新开始这一单？")))];
            Content->AddSlot().AutoHeight()[Body(Copy(TEXT("This order goes back to its desk and its clock resets. Earlier deliveries stay delivered."),
                TEXT("这一单会回到取件台，倒计时重置。之前送达的不受影响。")))];
            Footer->AddSlot().AutoWidth().Padding(0,0,12,0)[Action(Copy(TEXT("Keep going"),TEXT("继续配送")),2,EButton::Quiet,TEXT("Esc"))];
            Footer->AddSlot().AutoWidth()[Action(Copy(TEXT("Restart order"),TEXT("重新开始")),4,EButton::Primary,TEXT("Enter"))];
            return Screen(Slip(Red,Copy(TEXT("PAUSED"),TEXT("已暂停")),Content,Footer),UtilityBar(false));
        }
        if(Mode->bHelp)
        {
            Content->AddSlot().AutoHeight().Padding(0,0,0,14)[Title(Copy(TEXT("Story & controls"),TEXT("故事与操作")),40)];
            Content->AddSlot().AutoHeight().Padding(0,0,0,18)[Body(FText::FromString(Mode->RouteHint()),19)];
            Content->AddSlot().AutoHeight()[ControlsGrid()];
            Footer->AddSlot().AutoWidth()[Action(Copy(TEXT("Resume"),TEXT("继续")),2,EButton::Primary,TEXT("Esc"))];
            return Screen(Slip(Teal,Copy(TEXT("PAUSED"),TEXT("已暂停")),Content,Footer),UtilityBar());
        }
        Content->AddSlot().AutoHeight().Padding(0,0,0,6)[Title(Copy(TEXT("Take your time."),TEXT("慢慢来。")))];
        Content->AddSlot().AutoHeight().Padding(0,0,0,22)
            [Body(Copy(TEXT("A moment to breathe. The clock and devices are paused."),TEXT("慢慢来。倒计时和装置都已暂停。")),19,Muted)];
        Content->AddSlot().AutoHeight()[Settings()];
        // Wraps instead of overflowing when a language has longer button labels.
        auto Buttons=SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(10,10))
            +SWrapBox::Slot()[Action(Copy(TEXT("Resume"),TEXT("继续")),2,EButton::Primary,TEXT("Esc"))]
            +SWrapBox::Slot()[Action(Copy(TEXT("Story & controls"),TEXT("故事与操作")),3,EButton::Quiet,TEXT("H"))]
            +SWrapBox::Slot()[Action(Copy(TEXT("Retry order"),TEXT("重试本单")),4,EButton::Quiet,TEXT("R"))];
        return Screen(Slip(Ink,Copy(TEXT("PAUSED"),TEXT("已暂停")),Content,Buttons),UtilityBar());
    }

    TSharedRef<SWidget> Failed()
    {
        auto Content=SNew(SVerticalBox)
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,16)
                [Title(Mode->FailReason==1?Copy(TEXT("The lens cracked."),TEXT("镜头裂了。")):Copy(TEXT("Time is up."),TEXT("配送时间到了。")))]
            +SVerticalBox::Slot().AutoHeight()[Body(FText::FromString(Mode->bHelp?Mode->RouteHint():(Mode->FailReason==1?
                Mode->Copy(TEXT("Three knocks were too many for the lens case. Your earlier deliveries are safe. Try again and keep clear of the carts and the water."),
                    TEXT("镜头盒经不起第三次磕碰。之前的交付已经保留。再来一次，避开运输车和水面。")):
                Mode->Copy(TEXT("The delivery clock ran out. Your earlier deliveries are safe. Try this order again; the clock starts after you accept the brief."),
                    TEXT("本单配送超时。之前的交付已经保留。再试一次吧，确认简报后才开始计时。")))))];
        auto Footer=SNew(SHorizontalBox)
            +SHorizontalBox::Slot().FillWidth(1)[SNew(SSpacer)]
            +SHorizontalBox::Slot().AutoWidth().Padding(0,0,12,0)[Action(Copy(TEXT("Show route hint"),TEXT("查看路线提示")),3,EButton::Quiet,TEXT("H"))]
            +SHorizontalBox::Slot().AutoWidth()[Action(Copy(TEXT("Retry order"),TEXT("重试本单")),4,EButton::Primary,TEXT("Enter"))];
        return Screen(Slip(CourierPalette::Red,OrderCount(),Content,Footer),UtilityBar(false));
    }

    TSharedRef<SWidget> Delivered()
    {
        using namespace CourierPalette;
        auto StampMark=SNew(SBox).RenderTransformPivot(FVector2D(.5f,.5f))
            .RenderTransform(TOptional<FSlateRenderTransform>(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(-6.f)))))
            [SNew(SBorder).BorderImage(&Stamp).Padding(FMargin(30,4,30,8))
                [Text(Copy(TEXT("Delivered."),TEXT("送达了。")),58,EFace::Display,Orange)]];
        auto Panel=Box(R16,Plate,FMargin(28,20,28,22),
            SNew(SVerticalBox)
                +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,10)
                    [SNew(SHorizontalBox)
                        +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,12,0)[Pips(Mode->CompletedOrders,3,true)]
                        +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[Text(FText::FromString(Mode->ProgressText()),14,EFace::Heavy,Gold)]]
                +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,12)[StarRow(Mode->Stars[Mode->CurrentOrder],34,FLinearColor(1,1,1,.25f))]
                +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,14)
                    [Text(FText::FromString(ResultLine()),14,EFace::Heavy,CreamMuted,520)]
                +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,18)[Text(FText::FromString(Mode->OrderOutcome()),22,EFace::Black,Cream,520)]
                +SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left)
                    [Action(Copy(TEXT("Read next delivery"),TEXT("阅读下一单")),0,EButton::Primary,TEXT("Enter"))]);
        return SNew(SOverlay)
            +SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(24,24,24,56)
                [SNew(SScaleBox).Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly)
                    [SNew(SVerticalBox)
                        +SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0,0,0,18)[StampMark]
                        +SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[SNew(SBox).WidthOverride(580)[Panel]]]];
    }

    TSharedRef<SWidget> Complete()
    {
        using namespace CourierPalette;
        const FText Names[]={Copy(TEXT("COFFEE"),TEXT("咖啡")),Copy(TEXT("LENS"),TEXT("镜头")),Copy(TEXT("REEL"),TEXT("胶片"))};
        auto Stub=SNew(SVerticalBox);
        for(int32 I=0;I<3;++I)
            Stub->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0,0,0,I<2?14:0)
                [SNew(SBox).WidthOverride(92).HeightOverride(92).RenderTransformPivot(FVector2D(.5f,.5f))
                    .RenderTransform(TOptional<FSlateRenderTransform>(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(I==1?8.f:-8.f)))))
                    [SNew(SBorder).BorderImage(&StampSmall).HAlign(HAlign_Center).VAlign(VAlign_Center)
                        [SNew(SVerticalBox)
                            +SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[Text(FText::FromString(FString::Printf(TEXT("0%d"),I+1)),26,EFace::Digits,Orange)]
                            +SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[Text(Names[I],12,EFace::Black,Orange)]
                            +SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[StarRow(Mode->Stars[I],11,PaperDeep)]]]];
        const float Total=Mode->DeliveryTimes[0]+Mode->DeliveryTimes[1]+Mode->DeliveryTimes[2];
        auto Main=SNew(SVerticalBox)
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,8)[Text(Copy(TEXT("ADMIT ONE / CAMPUS CINEMA"),TEXT("一人入场 / 校园电影夜")),14,EFace::Heavy,Orange)]
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,12)[Text(Copy(TEXT("A seat for you."),TEXT("也有你的座位。")),42,EFace::Display,Ink,380)]
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,18)
                [Text(Copy(TEXT("The film begins. Your three delivery stamps make one ticket. Tonight, there is a seat for you too."),
                    TEXT("放映开始。三枚配送邮戳，拼成了你的电影票。今晚，也有一个座位属于你。")),18,EFace::Bold,Ink,380)]
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,14)
                [SNew(SHorizontalBox)
                    +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,10,0)
                        [Text(FText::FromString(TEXT("★")),30,EFace::Symbol,Gold)]
                    +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,14,0)
                        [Text(FText::FromString(FString::Printf(TEXT("%d / 9"),Mode->TotalStars())),34,EFace::Digits,Ink)]
                    +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                        [Text(FText::FromString(Mode->RankTitle()),20,EFace::Black,Orange,220)]]
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,20)
                [SNew(SHorizontalBox)
                    +SHorizontalBox::Slot().AutoWidth().Padding(0,0,34,0)
                        [InfoCell(Copy(TEXT("Delivery time"),TEXT("配送用时")),Text(FText::FromString(Clock(Total)),34,EFace::Digits,Ink))]
                    +SHorizontalBox::Slot().AutoWidth()
                        [InfoCell(Copy(TEXT("Retries"),TEXT("重试次数")),Text(FText::FromString(FString::FromInt(Mode->Retries)),34,EFace::Digits,Ink))]]
            +SVerticalBox::Slot().AutoHeight()
                [SNew(SHorizontalBox)
                    +SHorizontalBox::Slot().AutoWidth().Padding(0,0,12,0)[Action(Copy(TEXT("Play again"),TEXT("再玩一次")),0,EButton::Primary,TEXT("Enter"))]
                    +SHorizontalBox::Slot().AutoWidth()[Action(Copy(TEXT("Quit game"),TEXT("退出游戏")),7,EButton::Quiet)]];
        auto Dots=SNew(SVerticalBox);
        for(int32 I=0;I<18;++I)
            Dots->AddSlot().FillHeight(1).VAlign(VAlign_Center)
                [SNew(SBox).WidthOverride(3).HeightOverride(9)[SNew(SBorder).BorderImage(&R4).BorderBackgroundColor(PaperDeep)]];
        auto TicketCard=SNew(SBorder).BorderImage(&R16).BorderBackgroundColor(Shadow).Padding(FMargin(0,0,0,7))
            [SNew(SHorizontalBox)
                +SHorizontalBox::Slot().AutoWidth()[Box(Left,Paper,FMargin(30,26,26,26),SNew(SBox).WidthOverride(390)[Main])]
                +SHorizontalBox::Slot().AutoWidth()[Box(R4,Paper,FMargin(0,10),SNew(SBox).WidthOverride(3)[Dots])]
                +SHorizontalBox::Slot().AutoWidth()[Box(Right,Paper,FMargin(18,26,22,26),SNew(SBox).VAlign(VAlign_Center)[Stub])]];
        return SNew(SOverlay)
            +SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center).Padding(36)
                [SNew(SScaleBox).Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly)[TicketCard]];
    }

    // What this customer cares about, live: the coffee cools, the lens can crack,
    // the square waits for the reel.
    TSharedRef<SWidget> OrderMeter()
    {
        using namespace CourierPalette;
        if(Mode->CurrentOrder==1)
        {
            auto Row=SNew(SHorizontalBox)
                +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,10,0)
                    [Text(Copy(TEXT("LENS CASE"),TEXT("镜头盒")),14,EFace::Heavy,CreamMuted)];
            for(int32 I=0;I<3;++I)
                Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,6,0)
                    [SNew(SBox).WidthOverride(22).HeightOverride(14)
                        [SNew(SBorder).BorderImage(&R4).BorderBackgroundColor_Lambda([this,I]()->FSlateColor
                        {return Mode.IsValid() && I<Mode->LensCondition?CourierPalette::Cream:CourierPalette::Red;})]];
            return Row;
        }
        const bool bCoffee=Mode->CurrentOrder==0;
        return SNew(SVerticalBox)
            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,5)
                [Text(bCoffee?Copy(TEXT("COFFEE WARMTH"),TEXT("咖啡温度")):Copy(TEXT("THE SQUARE IS WAITING"),TEXT("全场都在等")),14,EFace::Heavy,CreamMuted)]
            +SVerticalBox::Slot().AutoHeight()
                [SNew(SBox).WidthOverride(240).HeightOverride(8)
                    [SNew(SProgressBar).Style(&ChargeStyle)
                        .FillColorAndOpacity_Lambda([this,bCoffee]()->FSlateColor
                        {
                            if(!Mode.IsValid())return CourierPalette::Orange;
                            const float Share=FMath::Clamp(Mode->Remaining/FMath::Max(1.f,Mode->OrderBudget()),0.f,1.f);
                            return bCoffee?FLinearColor::LerpUsingHSV(CourierPalette::Cyan,CourierPalette::Orange,Share):CourierPalette::Gold;
                        })
                        .Percent_Lambda([this]()->TOptional<float>
                        {return Mode.IsValid()?FMath::Clamp(Mode->Remaining/FMath::Max(1.f,Mode->OrderBudget()),0.f,1.f):0.f;})]];
    }

    // Remaining borrowed-time uses: one pip per use and a count. Practice is free.
    TSharedRef<SWidget> Uses()
    {
        using namespace CourierPalette;
        if(Mode->Phase==ECourierPhase::Practice)
            return Text(Copy(TEXT("FREE IN PRACTICE"),TEXT("练习中不限次数")),13,EFace::Heavy,Cyan,120);
        if(Mode->Phase!=ECourierPhase::Delivery)return SNullWidget::NullWidget;
        auto Pips=SNew(SHorizontalBox);
        for(int32 I=0;I<FMath::Max(Mode->OrderCharges(),Mode->Charges);++I)
            Pips->AddSlot().AutoWidth().Padding(0,0,5,0)
                [SNew(SBox).WidthOverride(12).HeightOverride(12)
                    [SNew(SBorder).BorderImage(&R6).BorderBackgroundColor_Lambda([this,I]()->FSlateColor
                    {return Mode.IsValid() && I<Mode->Charges?CourierPalette::Cyan:FLinearColor(1,1,1,.18f);})]];
        return SNew(SVerticalBox)
            +SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left)
                [SNew(STextBlock).Font(Typeface(26,EFace::Digits))
                    .ColorAndOpacity_Lambda([this]()->FSlateColor{return Mode.IsValid() && Mode->Charges<=0?CourierPalette::Red:CourierPalette::Cream;})
                    .Text_Lambda([this](){return FText::FromString(Mode.IsValid()?FString::Printf(TEXT("×%d"),Mode->Charges):FString());})]
            +SVerticalBox::Slot().AutoHeight().Padding(0,2,0,0)[Pips];
    }

    // ---------------------------------------------------------------- live HUD

    TSharedRef<SWidget> Gameplay()
    {
        using namespace CourierPalette;
        auto Overlay=SNew(SOverlay);
        Overlay->AddSlot()
            [SNew(SBorder).BorderImage(&Solid).Visibility(EVisibility::HitTestInvisible)
                .BorderBackgroundColor_Lambda([this]()->FSlateColor
                {return FLinearColor(1.f,.1f,.05f,Mode.IsValid()?FMath::Clamp(Mode->HitFlash,0.f,.6f)*.55f:0.f);})];
        // Borrowed time washes the screen with a faint cyan, on top of the camera grade.
        Overlay->AddSlot()
            [SNew(SBorder).BorderImage(&Solid).Visibility(EVisibility::HitTestInvisible)
                .BorderBackgroundColor_Lambda([this]()->FSlateColor
                {return FLinearColor(.25f,.85f,1.f,Mode.IsValid()?Mode->SlowFx*.1f:0.f);})];
        const bool bPractice=Mode->Phase==ECourierPhase::Practice;
        const bool bDelivery=Mode->Phase==ECourierPhase::Delivery;

        // Top left: what am I carrying and where to.
        Overlay->AddSlot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(24)
            [SNew(SBox).MaxDesiredWidth(400)
                [Box(R16,Plate,FMargin(20,14,22,16),
                    SNew(SVerticalBox)
                        +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,6)
                            [SNew(SHorizontalBox)
                                +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,12,0)
                                    [Text(bPractice?Copy(TEXT("PRACTICE"),TEXT("练习")):OrderCount(),13,EFace::Heavy,bPractice?Cyan:Orange)]
                                +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                                    [Pips(bPractice?-1:Mode->CurrentOrder,3,true)]]
                        +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,6)
                            [Text(bPractice?Copy(TEXT("Practice yard"),TEXT("练习区")):FText::FromString(Mode->OrderTitle()),26,EFace::Display,Cream,360)]
                        +SVerticalBox::Slot().AutoHeight()
                            [SNew(SHorizontalBox)
                                +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,9,0)
                                    [SNew(SBox).WidthOverride(9).HeightOverride(9)[SNew(SBorder).BorderImage(&R4).BorderBackgroundColor(Orange)]]
                                +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                                    [Text(bDelivery?FText::FromString(Mode->OrderDestination()):Copy(TEXT("Parcel desk"),TEXT("取件台")),17,EFace::Heavy,CreamMuted,330)]]
                        +SVerticalBox::Slot().AutoHeight().Padding(0,10,0,0)[bDelivery?OrderMeter():SNullWidget::NullWidget]
                        +SVerticalBox::Slot().AutoHeight().Padding(0,8,0,0)
                            [SNew(SHorizontalBox).Visibility(bDelivery?EVisibility::HitTestInvisible:EVisibility::Collapsed)
                                +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,8,0)
                                    [SNew(SBox).WidthOverride(10).HeightOverride(16)[SNew(SBorder).BorderImage(&R4).BorderBackgroundColor(Cyan)]]
                                +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                                    [SNew(STextBlock).Font(Typeface(14,EFace::Heavy)).ColorAndOpacity(Cyan)
                                        .Text_Lambda([this]()
                                        {
                                            if(!Mode.IsValid())return FText();
                                            return FText::FromString(Mode->Copy(TEXT("BORROWER CELLS"),TEXT("借时器电池"))+FString::Printf(TEXT("  %d / %d"),
                                                Mode->TokensTaken(Mode->CurrentOrder),Mode->TokensTotal(Mode->CurrentOrder)));
                                        })]])]];

        // Top centre: delivery clock, then a compass arrow toward the next marker.
        auto Centre=SNew(SVerticalBox);
        if(bDelivery)
            Centre->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0,0,0,8)
                [SNew(SBorder).BorderImage(&R16).Padding(FMargin(26,2,26,6))
                    .BorderBackgroundColor_Lambda([this]()->FSlateColor
                    {
                        if(!Mode.IsValid() || Mode->Remaining>20.f)return CourierPalette::Plate;
                        const float Pulse=.5f+.5f*FMath::Sin(float(FPlatformTime::Seconds())*7.f);
                        return FLinearColor::LerpUsingHSV(CourierPalette::Red,CourierPalette::Hex(TEXT("8E2416")),Pulse);
                    })
                    [SNew(STextBlock).Font(Typeface(50,EFace::Digits)).ColorAndOpacity(Cream)
                        .Text_Lambda([this](){return FText::FromString(Mode.IsValid()?Clock(Mode->Remaining):FString());})]];
        Centre->AddSlot().AutoHeight().HAlign(HAlign_Center)
            [Box(Pill,Plate,FMargin(16,5,20,6),
                SNew(SHorizontalBox)
                    +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,10,0)
                        [SNew(STextBlock).Text(FText::FromString(TEXT("▲"))).Font(Typeface(20,EFace::Symbol)).ColorAndOpacity(Orange)
                            .RenderTransformPivot(FVector2D(.5f,.5f))
                            .RenderTransform_Lambda([this]()->TOptional<FSlateRenderTransform>
                            {
                                const auto* P=Mode.IsValid()?UGameplayStatics::GetPlayerCharacter(Mode.Get(),0):nullptr;
                                const auto* PC=Mode.IsValid()?UGameplayStatics::GetPlayerController(Mode.Get(),0):nullptr;
                                if(!P || !PC)return TOptional<FSlateRenderTransform>();
                                const FVector D=Mode->NavigationTarget()-P->GetActorLocation();
                                const float A=FMath::FindDeltaAngleDegrees(PC->GetControlRotation().Yaw,D.Rotation().Yaw);
                                return TOptional<FSlateRenderTransform>(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(A))));
                            })]
                    +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                        [SNew(STextBlock).Font(Typeface(19,EFace::Black)).ColorAndOpacity(Cream)
                            .Text_Lambda([this]()
                            {
                                if(!Mode.IsValid())return FText();
                                const FString Arrived=Mode->Copy(TEXT("ARRIVED"),TEXT("已到达"));
                                if(Mode->NavigationText()==Arrived)return FText::FromString(Arrived);
                                const auto* P=UGameplayStatics::GetPlayerCharacter(Mode.Get(),0);
                                if(!P)return FText();
                                return FText::FromString(FString::Printf(TEXT("%.0f m"),FMath::CeilToFloat((Mode->NavigationTarget()-P->GetActorLocation()).Size2D()/100.f)));
                            })])];
        Overlay->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(24)[Centre];

        // Bottom centre: the one thing to do now, then a single-line hint.
        Overlay->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(24,24,24,112)
            [SNew(SVerticalBox)
                +SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0,0,0,12)
                    [SNew(SBox).Visibility_Lambda([this](){return Mode.IsValid() && !Mode->InteractionLabel().IsEmpty()?EVisibility::HitTestInvisible:EVisibility::Collapsed;})
                        [Box(Pill,Plate,FMargin(12,9,24,9),
                            SNew(SHorizontalBox)
                                +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,14,0)
                                    [KeyCap(FText::FromString(TEXT("E")),22,FSlateColor(Paper))]
                                +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                                    [SNew(STextBlock).Font(Typeface(23,EFace::Black)).ColorAndOpacity(Cream)
                                        .Text_Lambda([this](){return FText::FromString(Mode.IsValid()?Mode->InteractionLabel():FString());})])]]
                +SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
                    [SNew(SBox).MaxDesiredWidth(720)
                        .Visibility_Lambda([this](){return Mode.IsValid() && !Mode->ContextHint().IsEmpty()?EVisibility::HitTestInvisible:EVisibility::Collapsed;})
                        [SNew(SBorder).BorderImage(&R12).Padding(FMargin(22,10,22,11))
                            .BorderBackgroundColor_Lambda([this]()->FSlateColor
                            {return Mode.IsValid() && !Mode->StatusNotice().IsEmpty()?CourierPalette::Hex(TEXT("B8461F"),.94f):CourierPalette::Plate;})
                            [SNew(STextBlock).Font(Typeface(19,EFace::Heavy)).ColorAndOpacity(Cream).Justification(ETextJustify::Center)
                                .WrapTextAt(676).WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping).LineHeightPercentage(1.22f)
                                .Text_Lambda([this](){return FText::FromString(Mode.IsValid()?Mode->ContextHint():FString());})]]]];

        // Bottom left: the borrowed-time charge, as a key plus meter.
        Overlay->AddSlot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(24)
            [Box(R16,Plate,FMargin(14,12,18,13),
                SNew(SHorizontalBox)
                    +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,14,0)
                        [KeyCap(FText::FromString(TEXT("Q")),22,TAttribute<FSlateColor>::CreateLambda([this]()->FSlateColor
                        {
                            if(!Mode.IsValid())return CourierPalette::Paper;
                            if(Mode->QDenied>0.f)return CourierPalette::Red;
                            if(Mode->TokenFlash>0.f)return FLinearColor::White;
                            if(Mode->bSlowing)return FLinearColor(.55f,.95f,1.f);
                            return Mode->Cooldown>0.f?CourierPalette::KeyEdge:CourierPalette::Cyan;
                        }))]
                    +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                        [SNew(SVerticalBox)
                            +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,7)
                                [SNew(STextBlock).Font(Typeface(14,EFace::Heavy)).ColorAndOpacity(Cream)
                                    .Text_Lambda([this]()
                                    {
                                        if(!Mode.IsValid())return FText();
                                        if(Mode->bSlowing)return Copy(TEXT("SLOWING BLUE DEVICES"),TEXT("蓝色装置减速中"));
                                        if(Mode->Phase==ECourierPhase::Delivery && Mode->Charges<=0)return Copy(TEXT("NO USES LEFT THIS DELIVERY"),TEXT("这一单的次数已用完"));
                                        if(Mode->Cooldown>0.f)return Copy(TEXT("Recharging…"),TEXT("充能中…"));
                                        return Copy(TEXT("HOLD TO BORROW TIME"),TEXT("按住借时间"));
                                    })]
                            +SVerticalBox::Slot().AutoHeight()
                                [SNew(SBox).WidthOverride(190).HeightOverride(9)
                                    [SNew(SProgressBar).Style(&ChargeStyle)
                                        .FillColorAndOpacity_Lambda([this]()->FSlateColor
                                        {
                                            if(!Mode.IsValid())return CourierPalette::Cyan;
                                            if(Mode->QDenied>0.f)return CourierPalette::Red;
                                            return Mode->Cooldown>0.f?FLinearColor(1,1,1,.45f):CourierPalette::Cyan;
                                        })
                                        .Percent_Lambda([this]()->TOptional<float>
                                        {
                                            if(!Mode.IsValid())return 0.f;
                                            return Mode->Cooldown>0.f?1.f-Mode->Cooldown/ACourierGameMode::RechargeSeconds:Mode->SlowRemaining/3.f;
                                        })]]]
                    +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16,0,0,0)[Uses()])];

        // Bottom right: the only always-on control reminder.
        Overlay->AddSlot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(24)
            [Box(Pill,Plate,FMargin(10,7,16,7),
                SNew(SHorizontalBox)
                    +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,10,0)[Key(TEXT("Esc"),13)]
                    +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[Text(Copy(TEXT("Pause"),TEXT("暂停")),15,EFace::Heavy,CreamMuted)])];
        Overlay->SetVisibility(EVisibility::HitTestInvisible);
        return Overlay;
    }

    void Rebuild()
    {
        // Packaged actor BeginPlay order can construct the HUD before the game mode
        // has loaded its cards. Keep the empty root and retry on the next tick.
        // Do not cache LastCard until content exists, or the opening can stay blank.
        if(!Mode.IsValid() || Mode->CardTitles.IsEmpty() ||
            Mode->CardTitles.Num()!=Mode->CardBodies.Num() ||
            Mode->CardTitles.Num()!=Mode->CardTitlesZh.Num() ||
            Mode->CardTitles.Num()!=Mode->CardBodiesZh.Num() || Mode->Orders.Num()!=3)return;
        LastPhase=Mode->Phase;LastCard=Mode->Card;LastLanguage=Mode->Language;LastHelp=Mode->bHelp;
        LastOrder=Mode->CurrentOrder;LastConfirm=Mode->bConfirmRetry;
        TSharedRef<SWidget> Content=SNullWidget::NullWidget;
        // The HUD fills the real viewport so every plate sits in its corner;
        // a fixed 1280x800 frame left wide margins whenever DPI scale was below 1.
        if(Mode->CanMove())Content=Gameplay();
        else switch(Mode->Phase)
        {
            case ECourierPhase::Opening:Content=Opening();break;
            case ECourierPhase::Briefing:Content=Briefing();break;
            case ECourierPhase::Paused:Content=Paused();break;
            case ECourierPhase::Delivered:Content=Delivered();break;
            case ECourierPhase::Complete:Content=Complete();break;
            default:Content=Failed();break;
        }
        Root->SetContent(Content);
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
