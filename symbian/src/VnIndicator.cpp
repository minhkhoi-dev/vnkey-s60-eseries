#include <aknutils.h>
#include <aknsconstants.h>
#include <aknsdrawutils.h>
#include <aknsutils.h>
#include <avkon.hrh>
#include <eikappui.h>
#include <eikbtgpc.h>
#include <eikenv.h>
#include <eikspane.h>
#include <gdi.h>
#include "VnIndicator.h"

_LIT(KPlaceFmt, "indicator place %d at %d,%d %dx%d");
_LIT(KRectFmt, " | %S %d,%d-%d,%d");
_LIT(KNameStatus, "status");
_LIT(KNameUniversal, "indic");
_LIT(KNameBattery, "battery");
_LIT(KNameSignal, "signal");
_LIT(KNameNavi, "navi");
_LIT(KNameTitle, "title");
_LIT(KNameContext, "context");
_LIT(KBarFmt, " | bar %d,%d-%d,%d");
_LIT(KNoBar, " | no bar");
_LIT(KStateFmt, " | %S");
_LIT(KNotChecked, "not checked yet");
_LIT(KShowFmt, "indicator: %S");
_LIT(KHiddenFmt, " | mark %d,%d-%d,%d | client %d,%d-%d,%d | focused %d,%d-%d,%d");

// Why the mark is shown or hidden (Check), for the log.
_LIT(KWhyShown, "shown");
_LIT(KWhyNoPane, "hidden, no status pane");
_LIT(KWhyPaneHidden, "hidden, status pane hidden");
_LIT(KWhyNotInLayout, "hidden, its pane is not in the layout");
_LIT(KWhyClientArea, "hidden, the app's area reaches it");
_LIT(KWhyNoBar, "hidden, softkeys hidden");
_LIT(KWhyCovered, "hidden, the focused window is in front of the bar");

// The checks after a key, a focus change or the foreground: 50 ms after it, then 0.75 s
// and 1.45 s after it, for applications that hide the bar a moment later.
const TInt KFirstCheck = 50000;
const TInt KCheckInterval = 700000;
const TInt KChecks = 3;

// The letters, drawn pixel by pixel as the phone's bitmap icons are: 5 x 7 pixels, a byte
// per row from the top, bit 4 the left column.
const TInt KGlyphWidth = 5;
const TInt KGlyphHeight = 7;
static const TUint8 KGlyphV[KGlyphHeight] = { 0x11, 0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04 };
static const TUint8 KGlyphE[KGlyphHeight] = { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F };
const TInt KPadding = 2; // pixels between the frame and the letter (Khoi, 2026-10-17)
const TInt KFrame = 1;   // the frame's line
const TInt KBoxWidth = KGlyphWidth + 2 * (KPadding + KFrame);   // 11
const TInt KBoxHeight = KGlyphHeight + 2 * (KPadding + KFrame); // 13
const TInt KGap = 2;     // between the box and the pane's edge

// The bar at the bottom of the screen (softkeys, and on the E72 the indicators and the
// clock), or NULL.
static CCoeControl* BottomBar()
    {
    CEikButtonGroupContainer* cba = CEikButtonGroupContainer::Current();
    return cba && cba->ButtonGroup() ? cba->ButtonGroup()->AsControl() : NULL;
    }

CVnIndicator* CVnIndicator::NewL(MVnIndicatorObserver* aObserver)
    {
    CVnIndicator* self = new (ELeave) CVnIndicator(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CVnIndicator::CVnIndicator(MVnIndicatorObserver* aObserver)
    : iObserver(aObserver), iVietnamese(ETrue)
    {
    }

void CVnIndicator::ConstructL()
    {
    CreateWindowL();
    Window().SetOrdinalPosition(0, ECoeWinPriorityAlwaysAtFront);
    Window().SetNonFading(ETrue); // a menu in front must not dim it
    Layout();
    iTimer = CPeriodic::NewL(CActive::EPriorityLow);
    iCoeEnv->AddFocusObserverL(*this);
    iFocusObserving = ETrue;
    iCoeEnv->AddForegroundObserverL(*this);
    iForegroundObserving = ETrue;
    MakeVisible(EFalse); // until the first check has seen the bar on the screen
    ActivateL();
    CheckSoon();
    }

CVnIndicator::~CVnIndicator()
    {
    if (iTimer)
        {
        iTimer->Cancel();
        }
    delete iTimer;
    if (iFocusObserving)
        {
        iCoeEnv->RemoveFocusObserver(*this);
        }
    if (iForegroundObserving)
        {
        iCoeEnv->RemoveForegroundObserver(*this);
        }
    }

void CVnIndicator::SetVietnamese(TBool aOn)
    {
    iVietnamese = aOn;
    DrawNow();
    }

void CVnIndicator::SetPlace(TInt aPlace)
    {
    iPlace = ((aPlace % KPlaces) + KPlaces) % KPlaces; // a bad settings.txt gives no bad place
    Layout();
    DrawNow();
    CheckSoon();
    }

void CVnIndicator::CheckSoon()
    {
    if (!iTimer)
        {
        return;
        }
    iTimer->Cancel();
    iTicks = KChecks;
    iTimer->Start(KFirstCheck, KCheckInterval, TCallBack(Tick, this));
    }

TInt CVnIndicator::Tick(TAny* aSelf)
    {
    CVnIndicator* self = static_cast<CVnIndicator*>(aSelf);
    self->Check();
    if (--self->iTicks <= 0)
        {
        self->iTimer->Cancel(); // CPeriodic queues the next tick before the call
        }
    return 0;
    }

// Shows or hides the mark as ShouldShow says; a line in the log when that changes.
void CVnIndicator::Check()
    {
    TPtrC why;
    const TBool show = ShouldShow(why);
    iWhy.Set(why);
    if (show == iShown)
        {
        return;
        }
    iShown = show;
    if (show)
        {
        Layout();
        MakeVisible(ETrue);
        DrawDeferred();
        }
    else
        {
        MakeVisible(EFalse);
        }
    if (iObserver)
        {
        TBuf<200> line;
        line.Format(KShowFmt, &why);
        if (!show)
            {
            // what the decision saw, should the mark hide where the bar is on the screen
            const TRect mine(Position(), Size());
            CEikAppUi* appUi = static_cast<CEikAppUi*>(iCoeEnv->AppUi());
            const TRect client(appUi ? appUi->ClientRect() : TRect());
            TRect focused;
            CCoeControl* control = appUi ? appUi->TopFocusedControl() : NULL;
            RDrawableWindow* win = control ? control->DrawableWindow() : NULL;
            if (win)
                {
                focused = TRect(win->AbsPosition(), win->Size());
                }
            line.AppendFormat(KHiddenFmt, mine.iTl.iX, mine.iTl.iY, mine.iBr.iX, mine.iBr.iY,
                              client.iTl.iX, client.iTl.iY, client.iBr.iX, client.iBr.iY,
                              focused.iTl.iX, focused.iTl.iY, focused.iBr.iX, focused.iBr.iY);
            }
        iObserver->IndicatorLog(line);
        }
    }

// Whether the bar the mark sits in is on the screen now, and why not (CLAUDE.md "Design").
TBool CVnIndicator::ShouldShow(TPtrC& aWhy) const
    {
    CEikStatusPaneBase* sp = CEikStatusPaneBase::Current();
    if (!sp)
        {
        aWhy.Set(KWhyNoPane);
        return EFalse;
        }
    if (!sp->IsVisible())
        {
        aWhy.Set(KWhyPaneHidden);
        return EFalse;
        }
    TInt pane = EEikStatusPaneUidTitle;
    switch (iPlace)
        {
        case 0: pane = EEikStatusPaneUidIndic; break;
        case 1: pane = EEikStatusPaneUidBattery; break;
        case 2: pane = EEikStatusPaneUidNavi; break;
        default: break;
        }
    if (!sp->PaneCapabilities(TUid::Uid(pane)).IsInCurrentLayout())
        {
        aWhy.Set(KWhyNotInLayout);
        return EFalse;
        }
    const TRect mine(Position(), Size());
    CEikAppUi* appUi = static_cast<CEikAppUi*>(iCoeEnv->AppUi());
    if (appUi && appUi->ClientRect().Intersects(mine))
        {
        aWhy.Set(KWhyClientArea); // the status pane or the softkeys are not there
        return EFalse;
        }
    CCoeControl* bar = BottomBar();
    if (bar && TRect(bar->PositionRelativeToScreen(), bar->Size()).Intersects(mine))
        {
        CEikButtonGroupContainer* cba = CEikButtonGroupContainer::Current();
        if (!bar->IsVisible() || (cba && !cba->IsVisible()))
            {
            aWhy.Set(KWhyNoBar);
            return EFalse;
            }
        }
    if (appUi && CoveredByFocused(mine))
        {
        aWhy.Set(KWhyCovered); // a game or a video that draws over a status pane it left on
        return EFalse;
        }
    aWhy.Set(KWhyShown);
    return ETrue;
    }

// ETrue when the focused control's window lies over the mark's spot and is in front of the
// window of the bar there (the softkeys' bar, or else the title pane's). When the two are
// not siblings in the window tree their order cannot be told, and the answer is EFalse.
TBool CVnIndicator::CoveredByFocused(const TRect& aMine) const
    {
    CCoeControl* focused = static_cast<CEikAppUi*>(iCoeEnv->AppUi())->TopFocusedControl();
    if (!focused || focused == this)
        {
        return EFalse;
        }
    RDrawableWindow* win = focused->DrawableWindow();
    if (!win || !TRect(win->AbsPosition(), win->Size()).Intersects(aMine))
        {
        return EFalse;
        }
    RDrawableWindow* barWin = NULL;
    CCoeControl* bar = BottomBar();
    if (bar && TRect(bar->PositionRelativeToScreen(), bar->Size()).Intersects(aMine))
        {
        barWin = bar->DrawableWindow();
        }
    else
        {
        CEikStatusPaneBase* sp = CEikStatusPaneBase::Current();
        CCoeControl* title = NULL;
        if (sp)
            {
            TRAP_IGNORE(title = sp->ContainerControlL(TUid::Uid(EEikStatusPaneUidTitle)));
            }
        barWin = title ? title->DrawableWindow() : NULL;
        }
    if (!barWin || barWin == win || barWin->Parent() != win->Parent())
        {
        return EFalse;
        }
    return win->FullOrdinalPosition() < barWin->FullOrdinalPosition(); // 0 is the front
    }

// A box of KBoxWidth x KBoxHeight pixels (twice that and more on a bigger screen), in the
// middle of the pane's height, next to the pane's edge that the place names.
void CVnIndicator::Layout()
    {
    TRect pane;
    TBool leftEdge = EFalse;
    TBool outside = EFalse; // to the left of the pane, not inside it
    switch (iPlace)
        {
        case 0:
            AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EUniversalIndicatorPane, pane);
            leftEdge = ETrue;
            break;
        case 1:
            AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EBatteryPane, pane);
            leftEdge = ETrue;
            outside = ETrue;
            break;
        case 2:
            AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::ENaviPane, pane);
            break;
        default:
            AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::ETitlePane, pane);
            break;
        }
    if (pane.IsEmpty())
        {
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EStatusPane, pane);
        leftEdge = EFalse;
        outside = EFalse;
        }
    iScale = Max(1, pane.Height() / (KBoxHeight + 1));
    const TInt width = KBoxWidth * iScale;
    const TInt h = KBoxHeight * iScale;
    const TInt y = pane.iTl.iY + (pane.Height() - h + 1) / 2;
    TInt x;
    if (outside)
        {
        x = pane.iTl.iX - KGap - width;
        }
    else if (leftEdge)
        {
        x = pane.iTl.iX + KGap;
        }
    else
        {
        x = pane.iBr.iX - KGap - width;
        }
    SetExtent(TPoint(Max(x, 0), Max(y, 0)), TSize(width, h));
    }

// The part of the bar's skin background that this window covers, drawn with the bar's
// own control context, so that it matches whatever skin and layout the phone has. EFalse
// when the window is not over the bar or the bar has no skin background.
TBool CVnIndicator::DrawBarBackground(CWindowGc& aGc) const
    {
    MAknsSkinInstance* skin = AknsUtils::SkinInstance();
    CCoeControl* bar = BottomBar();
    if (!skin || !bar)
        {
        return EFalse;
        }
    MAknsControlContext* cc = AknsDrawUtils::ControlContext(bar);
    const TRect barRect(bar->PositionRelativeToScreen(), bar->Size());
    TRect mine(PositionRelativeToScreen(), Size());
    if (!cc || !barRect.Contains(mine.iTl) || !barRect.Contains(mine.iBr - TPoint(1, 1)))
        {
        return EFalse;
        }
    mine.Move(-barRect.iTl.iX, -barRect.iTl.iY); // in the bar's coordinates
    return AknsDrawUtils::DrawBackground(skin, cc, bar, aGc, TPoint(0, 0), mine,
                                         KAknsDrawParamDefault);
    }

// The frame, its corner pixels left out as the phone's rounded icon frames, and the letter
// in the middle, KPadding pixels from the frame on every side; all in one colour.
void CVnIndicator::Draw(const TRect& /*aRect*/) const
    {
    CWindowGc& gc = SystemGc();
    const TRect rect(Rect());
    if (!DrawBarBackground(gc))
        {
        gc.SetPenStyle(CGraphicsContext::ENullPen);
        gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
        gc.SetBrushColor(KRgbBlack);
        gc.DrawRect(rect);
        }
    // the colour of the universal indicator pane's icons, as the memory card's
    TRgb colour(KRgbWhite);
    AknsUtils::GetCachedColor(AknsUtils::SkinInstance(), colour, KAknsIIDQsnIconColors,
                              EAknsCIQsnIconColorsCG2);
    gc.SetPenStyle(CGraphicsContext::ENullPen);
    gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
    gc.SetBrushColor(colour);
    const TInt s = iScale;
    const TInt l = rect.iTl.iX;
    const TInt t = rect.iTl.iY;
    const TInt r = rect.iBr.iX;
    const TInt b = rect.iBr.iY;
    gc.DrawRect(TRect(l + s, t, r - s, t + s));     // top
    gc.DrawRect(TRect(l + s, b - s, r - s, b));     // bottom
    gc.DrawRect(TRect(l, t + s, l + s, b - s));     // left
    gc.DrawRect(TRect(r - s, t + s, r, b - s));     // right
    const TUint8* glyph = iVietnamese ? KGlyphV : KGlyphE;
    const TInt x0 = l + (rect.Width() - KGlyphWidth * s) / 2;
    const TInt y0 = t + (rect.Height() - KGlyphHeight * s) / 2;
    for (TInt row = 0; row < KGlyphHeight; row++)
        {
        for (TInt col = 0; col < KGlyphWidth; col++)
            {
            if (glyph[row] & (0x10 >> col))
                {
                gc.DrawRect(TRect(TPoint(x0 + col * s, y0 + row * s), TSize(s, s)));
                }
            }
        }
    }

void CVnIndicator::HandleResourceChange(TInt aType)
    {
    CCoeControl::HandleResourceChange(aType);
    if (aType == KAknsMessageSkinChange)
        {
        DrawDeferred(); // a new skin: new background and colour
        }
    else if (aType == KEikDynamicLayoutVariantSwitch)
        {
        Layout();
        CheckSoon();
        }
    }

void CVnIndicator::HandleChangeInFocus()
    {
    CheckSoon(); // a new view, a dialog, a full-screen control
    }

void CVnIndicator::HandleDestructionOfFocusedItem()
    {
    CheckSoon();
    }

void CVnIndicator::HandleGainingForeground()
    {
    CheckSoon();
    }

void CVnIndicator::HandleLosingForeground()
    {
    }

static void AppendRect(TDes& aLine, const TDesC& aName, AknLayoutUtils::TAknLayoutMetrics aPane)
    {
    TRect r;
    AknLayoutUtils::LayoutMetricsRect(aPane, r);
    if (aLine.Length() + 40 < aLine.MaxLength())
        {
        aLine.AppendFormat(KRectFmt, &aName, r.iTl.iX, r.iTl.iY, r.iBr.iX, r.iBr.iY);
        }
    }

void CVnIndicator::Describe(TDes& aLine) const
    {
    const TRect r(Position(), Size());
    aLine.Format(KPlaceFmt, iPlace, r.iTl.iX, r.iTl.iY, r.Width(), r.Height());
    AppendRect(aLine, KNameStatus, AknLayoutUtils::EStatusPane);
    AppendRect(aLine, KNameUniversal, AknLayoutUtils::EUniversalIndicatorPane);
    AppendRect(aLine, KNameBattery, AknLayoutUtils::EBatteryPane);
    AppendRect(aLine, KNameSignal, AknLayoutUtils::ESignalPane);
    AppendRect(aLine, KNameNavi, AknLayoutUtils::ENaviPane);
    AppendRect(aLine, KNameTitle, AknLayoutUtils::ETitlePane);
    AppendRect(aLine, KNameContext, AknLayoutUtils::EContextPane);
    const CCoeControl* bar = BottomBar();
    if (!bar)
        {
        aLine.Append(KNoBar);
        }
    else if (aLine.Length() + 40 < aLine.MaxLength())
        {
        const TRect r(bar->PositionRelativeToScreen(), bar->Size());
        aLine.AppendFormat(KBarFmt, r.iTl.iX, r.iTl.iY, r.iBr.iX, r.iBr.iY);
        }
    const TDesC& state = iWhy.Length() > 0 ? static_cast<const TDesC&>(iWhy) : KNotChecked();
    if (aLine.Length() + state.Length() + 3 < aLine.MaxLength())
        {
        aLine.AppendFormat(KStateFmt, &state);
        }
    }
