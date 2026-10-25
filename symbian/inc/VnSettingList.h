#ifndef VNSETTINGLIST_H
#define VNSETTINGLIST_H

#include <aknsettingitemlist.h>

// The values the settings list shows and edits (the rows of TVnSettingId). The app UI
// fills them from its state before a refresh and applies one after it was changed.
class TVnSettingData
    {
public:
    TVnSettingData();
public:
    TBool iVietnamese;
    TBool iAllApps;     // the saved preference; the title says whether it is on right now
    TBool iBoot;
    TInt iLanguage;     // TVnLanguage
    TBool iModernTone;
    TBool iWAlone;
    TBool iSpellCheck;
    TBool iRestore;
    TBool iMacros;
    TInt iMethod;       // 0..2, CVnCatcher::TMethod
    TInt iPlace;        // 0..3, CVnIndicator::Place()
    TBool iLogKeys;
    TBuf<60> iServer;
    };

class MVnSettingObserver
    {
public:
    // aId (TVnSettingId) was changed in the list; its new value is in the data.
    virtual void SettingChangedL(TInt aId) = 0;
    };

// VnKey's main screen (CLAUDE.md "Design"): a standard S60 settings list, as in the phone's
// own settings. The selection key toggles an on/off row, or opens the page of a row with
// more values; Options > Change opens any row's page.
class CVnSettingList : public CAknSettingItemList
    {
public:
    explicit CVnSettingList(MVnSettingObserver& aObserver);
    TVnSettingData& Data() { return iData; }
    // Shows the data again (after the app UI changed it). Does nothing while a page is open.
    void RefreshL();
    // Options > Change: the page of the focused row.
    void EditCurrentL();
    // The focused row (-1 when none), and the focus put on a row (when the list is made
    // again in another language).
    TInt CurrentRow();
    void SetCurrentRow(TInt aRow);

    // From CAknSettingItemList
    void EditItemL(TInt aIndex, TBool aCalledFromMenu);

private:
    CAknSettingItem* CreateSettingItemL(TInt aId);
    void SizeChanged();

private:
    MVnSettingObserver& iObserver;
    TVnSettingData iData;
    TBool iEditing;
    };

#endif // VNSETTINGLIST_H
