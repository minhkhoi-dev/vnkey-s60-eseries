#ifndef VNINDICATOR_H
#define VNINDICATOR_H

#include <coecntrl.h>

// The [V] / [E] mark in the status bar (CLAUDE.md "Requirements"): a small window of the
// application's own, kept in front of its other windows over the status pane, since an
// application cannot add an icon to the system's own indicators. It is drawn as the
// phone's own indicator icons are (the memory card's): the letter, 5 x 7 pixels, in a thin
// frame with 2 pixels between them, in the skin's colour for universal indicator icons, on
// the background of the bar it sits in.
// The probe can put it in a few places (Place()); place 0, at the left end of the
// universal indicator pane in the E72's bottom bar, is the one Khoi chose (2026-10-16).
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
    TBool DrawBarBackground(CWindowGc& aGc) const;
    void HandleResourceChange(TInt aType);

private:
    TBool iVietnamese;
    TInt iPlace;
    TInt iScale; // pixels per pixel of the letter and the frame
    };

#endif // VNINDICATOR_H
