#ifndef VNKEYAPPVIEW_H
#define VNKEYAPPVIEW_H

#include <coecntrl.h>

class CEikEdwin;

// The probe's one view: an editor that fills it, with the phone's own FEP in it as in any
// other application, to type Telex into (docs/TESTING.md).
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
    void Draw(const TRect& aRect) const;

private:
    CEikEdwin* iEditor;
    };

#endif // VNKEYAPPVIEW_H
