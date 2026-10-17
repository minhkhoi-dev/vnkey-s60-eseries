#ifndef VNKEYAPPVIEW_H
#define VNKEYAPPVIEW_H

#include <coecntrl.h>

class CEikEdwin;

// The typing test (Options > Typing test): an editor that fills the view, with the phone's
// own FEP in it as in any other application, to type Telex into (docs/TESTING.md). The
// editor has the focus while the view has it, so that it shows no cursor while hidden.
class CVnKeyAppView : public CCoeControl
    {
public:
    static CVnKeyAppView* NewL(const TRect& aRect);
    ~CVnKeyAppView();
    void ClearL();

    TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);
    TInt CountComponentControls() const;
    CCoeControl* ComponentControl(TInt aIndex) const;

private:
    void ConstructL(const TRect& aRect);
    void SizeChanged();
    void FocusChanged(TDrawNow aDrawNow);
    void Draw(const TRect& aRect) const;

private:
    CEikEdwin* iEditor;
    };

#endif // VNKEYAPPVIEW_H
