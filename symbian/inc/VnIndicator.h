#ifndef VNINDICATOR_H
#define VNINDICATOR_H

#include <coecntrl.h>
#include <coemain.h>

class CPeriodic;

// What the mark tells its owner: a line for the log when it is shown or hidden.
class MVnIndicatorObserver
    {
public:
    virtual void IndicatorLog(const TDesC& aLine) = 0;
    };

// The [V] / [E] mark in the status bar (CLAUDE.md "Requirements"): a small window of the
// application's own, kept in front of its other windows over the status pane, since an
// application cannot add an icon to the system's own indicators. It is drawn as the
// phone's own indicator icons are (the memory card's): the letter, 5 x 7 pixels, in a thin
// frame with 2 pixels between them, in the skin's colour for universal indicator icons, on
// the background of the bar it sits in.
// The probe can put it in a few places (Place()); place 0, at the left end of the
// universal indicator pane in the E72's bottom bar, is the one Khoi chose (2026-10-16).
//
// It shows only while the bar it sits in is on the screen: not while the application is
// full screen (a game, a video), which an always-in-front window would otherwise cover
// (Khoi, 2026-10-17). Avkon tells no other control when the status pane or the softkeys
// are hidden, so it looks again (Check) shortly after every key, focus change and return
// to the foreground: hidden when the status pane is hidden, the pane it sits in is not in
// the status pane's layout, the application's client area or its softkeys' absence reach
// the mark, or the focused control's window lies over the spot, in front of the bar.
class CVnIndicator : public CCoeControl, public MCoeFocusObserver, public MCoeForegroundObserver
    {
public:
    enum { KPlaces = 4 };

    static CVnIndicator* NewL(MVnIndicatorObserver* aObserver = NULL);
    ~CVnIndicator();

    void SetVietnamese(TBool aOn);
    // 0: left end of the universal indicator pane, 1: left of the battery pane, 2: right
    // end of the navi pane, 3: right end of the title pane.
    void SetPlace(TInt aPlace);
    TInt Place() const { return iPlace; }
    // Looks again soon, and twice more within a second and a half, whether the bar is on
    // the screen (the application may hide it a moment after the key that asked for it).
    void CheckSoon();
    // "place 1 at x,y wxh shown|hidden (why)" plus the rectangles of the status pane's
    // parts, for the log.
    void Describe(TDes& aLine) const;

private:
    CVnIndicator(MVnIndicatorObserver* aObserver);
    void ConstructL();
    void Layout();
    void Check();
    TBool ShouldShow(TPtrC& aWhy) const;
    TBool CoveredByFocused(const TRect& aMine) const;
    static TInt Tick(TAny* aSelf);
    void Draw(const TRect& aRect) const;
    TBool DrawBarBackground(CWindowGc& aGc) const;
    void HandleResourceChange(TInt aType);
    // From MCoeFocusObserver
    void HandleChangeInFocus();
    void HandleDestructionOfFocusedItem();
    // From MCoeForegroundObserver
    void HandleGainingForeground();
    void HandleLosingForeground();

private:
    MVnIndicatorObserver* iObserver;
    TBool iVietnamese;
    TInt iPlace;
    TInt iScale;        // pixels per pixel of the letter and the frame
    TBool iShown;       // what the last Check decided
    TPtrC iWhy;         // and why (a literal)
    CPeriodic* iTimer;  // the checks after a key, a focus change, the foreground
    TInt iTicks;        // checks left
    TBool iFocusObserving;      // added to CONE's focus observers
    TBool iForegroundObserving; // and to its foreground observers
    };

#endif // VNINDICATOR_H
