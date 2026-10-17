#include <aknutils.h>
#include <eikedwin.h>
#include <gdi.h>
#include <txtfrmat.h>
#include <txtglobl.h>
#include "VnKeyAppView.h"

const TInt KMargin = 3;

CVnKeyAppView* CVnKeyAppView::NewL(const TRect& aRect)
    {
    CVnKeyAppView* self = new (ELeave) CVnKeyAppView;
    CleanupStack::PushL(self);
    self->ConstructL(aRect);
    CleanupStack::Pop(self);
    return self;
    }

void CVnKeyAppView::ConstructL(const TRect& aRect)
    {
    CreateWindowL();
    // The editor must exist before SetRect: SizeChanged lays it out.
    iEditor = new (ELeave) CEikEdwin;
    iEditor->SetContainerWindowL(*this);
    iEditor->ConstructL(CEikEdwin::ENoAutoSelection | CEikEdwin::EJustAutoCurEnd, 0, 0, 0);
    iEditor->SetBorder(TGulBorder::ENone);
    // White box with black text whatever the skin does, as in OttPilot's chat view.
    iEditor->SetSkinBackgroundControlContextL(NULL);
    iEditor->OverrideColorL(EColorControlBackground, KRgbWhite);
    TCharFormat format;
    TCharFormatMask mask;
    format.iFontSpec = AknLayoutUtils::FontFromId(EAknLogicalFontPrimaryFont)->FontSpecInTwips();
    mask.SetAttrib(EAttFontTypeface);
    mask.SetAttrib(EAttFontHeight);
    format.iFontPresentation.iTextColor = KRgbBlack;
    mask.SetAttrib(EAttColor);
    CCharFormatLayer* layer = CCharFormatLayer::NewL(format, mask);
    iEditor->SetCharFormatLayer(layer); // the editor owns it
    SetRect(aRect);
    iEditor->SetBackgroundColorL(KRgbWhite); // after sizing
    ActivateL(); // the editor takes the focus with the view (FocusChanged)
    }

CVnKeyAppView::~CVnKeyAppView()
    {
    delete iEditor;
    }

void CVnKeyAppView::ClearL()
    {
    iEditor->SetTextL(&KNullDesC);
    iEditor->HandleTextChangedL();
    }

TKeyResponse CVnKeyAppView::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
    {
    return iEditor->OfferKeyEventL(aKeyEvent, aType);
    }

TInt CVnKeyAppView::CountComponentControls() const
    {
    return 1;
    }

CCoeControl* CVnKeyAppView::ComponentControl(TInt /*aIndex*/) const
    {
    return iEditor;
    }

void CVnKeyAppView::SizeChanged()
    {
    TRect rect(Rect());
    rect.Shrink(KMargin, KMargin);
    iEditor->SetRect(rect);
    }

void CVnKeyAppView::FocusChanged(TDrawNow aDrawNow)
    {
    iEditor->SetFocus(IsFocused(), aDrawNow);
    }

void CVnKeyAppView::Draw(const TRect& /*aRect*/) const
    {
    CWindowGc& gc = SystemGc();
    gc.SetBrushColor(KRgbWhite);
    gc.Clear(Rect());
    }
