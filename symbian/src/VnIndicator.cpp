#include <aknutils.h>
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

const TInt KPadding = 2;
const TInt KGap = 2;
// Macros, not TRgb constants: nothing here depends on static constructors being run.
#define KColourV    TRgb(0x00, 0x60, 0xC0)
#define KColourE    TRgb(0x60, 0x60, 0x60)
#define KColourText TRgb(0xFF, 0xFF, 0xFF)

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

void CVnIndicator::Draw(const TRect& /*aRect*/) const
    {
    CWindowGc& gc = SystemGc();
    const TRect rect(Rect());
    gc.SetPenStyle(CGraphicsContext::ENullPen);
    gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
    gc.SetBrushColor(iVietnamese ? KColourV : KColourE);
    gc.DrawRect(rect);
    gc.SetBrushStyle(CGraphicsContext::ENullBrush);
    gc.SetPenStyle(CGraphicsContext::ESolidPen);
    gc.SetPenColor(KColourText);
    const CFont* font = CEikonEnv::Static()->DenseFont();
    gc.UseFont(font);
    const TInt baseline = (rect.Height() - font->HeightInPixels()) / 2 + font->AscentInPixels();
    gc.DrawText(iVietnamese ? KMarkV : KMarkE, rect, baseline, CGraphicsContext::ECenter, 0);
    gc.DiscardFont();
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
    }
