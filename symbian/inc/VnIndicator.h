#ifndef VNINDICATOR_H
#define VNINDICATOR_H

#include <coecntrl.h>

// The [V] / [E] mark in the status bar (CLAUDE.md "Requirements"): a small window of the
// application's own, kept in front of its other windows over the status pane, since an
// application cannot add an icon to the system's own indicators. The probe puts it in one
// of a few places of the status pane (Place()), to find the one that looks right on the E72.
class CVnIndicator : public CCoeControl
    {
public:
    enum { KPlaces = 4 };

    static CVnIndicator* NewL();
    ~CVnIndicator();

    void SetVietnamese(TBool aOn);
    // 0: left end of the universal indicator pane, 1: left of the battery pane, 2: right
    // end of the navi pane, 3: right end of the title pane.
    void SetPlace(TInt aPlace);
    TInt Place() const { return iPlace; }
    // "place 1 at x,y wxh" plus the rectangles of the status pane's parts, for the log.
    void Describe(TDes& aLine) const;

private:
    CVnIndicator();
    void ConstructL();
    void Layout();
    void Draw(const TRect& aRect) const;

private:
    TBool iVietnamese;
    TInt iPlace;
    };

#endif // VNINDICATOR_H
