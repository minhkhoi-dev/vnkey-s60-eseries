#include <aknutils.h>
#include <aknsconstants.h>
#include <aknsdrawutils.h>
#include <aknsutils.h>
#include <eikbtgpc.h>
#include <eikenv.h>
#include <gdi.h>
#include "VnIndicator.h"

_LIT(KMarkV, "V");
_LIT(KMarkE, "E");
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

const TInt KPadding = 2;
const TInt KGap = 2;

// The bar at the bottom of the screen (softkeys, and on the E72 the indicators and the
// clock), or NULL.
static CCoeControl* BottomBar()
    {
    CEikButtonGroupContainer* cba = CEikButtonGroupContainer::Current();
    return cba && cba->ButtonGroup() ? cba->ButtonGroup()->AsControl() : NULL;
    }

CVnIndicator* CVnIndicator::NewL()
    {
    CVnIndicator* self = new (ELeave) CVnIndicator;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CVnIndicator::CVnIndicator()
    : iVietnamese(ETrue)
    {
    }

void CVnIndicator::ConstructL()
    {
    CreateWindowL();
    Window().SetOrdinalPosition(0, ECoeWinPriorityAlwaysAtFront);
    Window().SetNonFading(ETrue); // a menu in front must not dim it
    Layout();
    ActivateL();
    }

CVnIndicator::~CVnIndicator()
    {
    }

void CVnIndicator::SetVietnamese(TBool aOn)
    {
    iVietnamese = aOn;
    DrawNow();
    }

void CVnIndicator::SetPlace(TInt aPlace)
    {
    iPlace = aPlace % KPlaces;
    Layout();
    DrawNow();
    }

// A box as high as the pane it sits in (at most the font and padding), next to the pane's
// edge that the place names.
void CVnIndicator::Layout()
    {
    const CFont* font = CEikonEnv::Static()->DenseFont();
    const TInt width = font->TextWidthInPixels(KMarkV) + 2 * KPadding + 2;
    const TInt height = font->HeightInPixels() + 2 * KPadding;
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
    const TInt h = Min(height, Max(pane.Height(), 8));
    const TInt y = pane.iTl.iY + (pane.Height() - h) / 2;
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
    gc.SetBrushStyle(CGraphicsContext::ENullBrush);
    gc.SetPenStyle(CGraphicsContext::ESolidPen);
    gc.SetPenColor(colour);
    gc.DrawRoundRect(rect, TSize(2, 2));
    const CFont* font = CEikonEnv::Static()->DenseFont();
    gc.UseFont(font);
    const TInt baseline = (rect.Height() - font->HeightInPixels()) / 2 + font->AscentInPixels();
    gc.DrawText(iVietnamese ? KMarkV : KMarkE, rect, baseline, CGraphicsContext::ECenter, 0);
    gc.DiscardFont();
    }

void CVnIndicator::HandleResourceChange(TInt aType)
    {
    CCoeControl::HandleResourceChange(aType);
    if (aType == KAknsMessageSkinChange)
        {
        DrawDeferred(); // a new skin: new background and colour
        }
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
    }
