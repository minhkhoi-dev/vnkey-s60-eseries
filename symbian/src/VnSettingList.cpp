#include <eikfrlb.h>
#include "VnKey.hrh"
#include "VnSettingList.h"

TVnSettingData::TVnSettingData()
    : iVietnamese(ETrue), iAllApps(EFalse), iBoot(ETrue), iLanguage(EVnLangVietnamese),
      iModernTone(EFalse), iWAlone(ETrue),
      iSpellCheck(ETrue), iRestore(ETrue), iMacros(ETrue), iMethod(0), iPlace(0), iLogKeys(EFalse)
    {
    }

CVnSettingList::CVnSettingList(MVnSettingObserver& aObserver)
    : iObserver(aObserver)
    {
    }

// Each row is bound to its value in iData; the resource (r_vnkey_setting_list) gives the
// names, the pages and the texts of the values.
CAknSettingItem* CVnSettingList::CreateSettingItemL(TInt aId)
    {
    switch (aId)
        {
        case EVnSetVietnamese:
            return new (ELeave) CAknBinaryPopupSettingItem(aId, iData.iVietnamese);
        case EVnSetAllApps:
            return new (ELeave) CAknBinaryPopupSettingItem(aId, iData.iAllApps);
        case EVnSetBoot:
            return new (ELeave) CAknBinaryPopupSettingItem(aId, iData.iBoot);
        case EVnSetLanguage:
            return new (ELeave) CAknEnumeratedTextPopupSettingItem(aId, iData.iLanguage);
        case EVnSetTone:
            return new (ELeave) CAknBinaryPopupSettingItem(aId, iData.iModernTone);
        case EVnSetWAlone:
            return new (ELeave) CAknBinaryPopupSettingItem(aId, iData.iWAlone);
        case EVnSetSpell:
            return new (ELeave) CAknBinaryPopupSettingItem(aId, iData.iSpellCheck);
        case EVnSetRestore:
            return new (ELeave) CAknBinaryPopupSettingItem(aId, iData.iRestore);
        case EVnSetMacros:
            return new (ELeave) CAknBinaryPopupSettingItem(aId, iData.iMacros);
        case EVnSetMethod:
            return new (ELeave) CAknEnumeratedTextPopupSettingItem(aId, iData.iMethod);
        case EVnSetPlace:
            return new (ELeave) CAknEnumeratedTextPopupSettingItem(aId, iData.iPlace);
        case EVnSetLogKeys:
            return new (ELeave) CAknBinaryPopupSettingItem(aId, iData.iLogKeys);
        case EVnSetServer:
            return new (ELeave) CAknTextSettingItem(aId, iData.iServer);
        default:
            return NULL;
        }
    }

// The selection key on a row (aCalledFromMenu EFalse: an on/off row just toggles), or
// Options > Change. The new value goes to iData and to the app UI.
void CVnSettingList::EditItemL(TInt aIndex, TBool aCalledFromMenu)
    {
    CAknSettingItemArray* items = SettingItemArray();
    const TInt index = items->ItemIndexFromVisibleIndex(aIndex);
    if (index < 0 || index >= items->Count())
        {
        return;
        }
    iEditing = ETrue;
    TRAPD(err, CAknSettingItemList::EditItemL(aIndex, aCalledFromMenu));
    iEditing = EFalse;
    User::LeaveIfError(err);
    CAknSettingItem* item = items->At(index);
    item->StoreL();
    iObserver.SettingChangedL(item->Identifier());
    }

void CVnSettingList::EditCurrentL()
    {
    const TInt current = ListBox()->CurrentItemIndex();
    if (current >= 0)
        {
        EditItemL(current, ETrue);
        }
    }

TInt CVnSettingList::CurrentRow()
    {
    return ListBox() ? ListBox()->CurrentItemIndex() : -1;
    }

void CVnSettingList::SetCurrentRow(TInt aRow)
    {
    if (ListBox() && aRow >= 0 && aRow < SettingItemArray()->Count())
        {
        ListBox()->SetCurrentItemIndex(aRow);
        ListBox()->ScrollToMakeItemVisible(aRow); // the rows are two lines: a few a screen
        }
    }

void CVnSettingList::RefreshL()
    {
    if (iEditing)
        {
        return; // the page that is open has the row's value; it comes back through EditItemL
        }
    CAknSettingItemArray* items = SettingItemArray();
    for (TInt i = 0; i < items->Count(); ++i)
        {
        CAknSettingItem* item = items->At(i);
        item->LoadL();
        item->UpdateListBoxTextL();
        }
    ListBox()->DrawDeferred();
    }

void CVnSettingList::SizeChanged()
    {
    if (ListBox())
        {
        ListBox()->SetRect(Rect());
        }
    }
