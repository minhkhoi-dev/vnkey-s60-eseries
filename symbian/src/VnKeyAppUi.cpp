#include <akntitle.h>
#include <aknnotewrappers.h>
#include <aknquerydialog.h>
#include <apgtask.h>
#include <avkon.hrh>
#include <bautils.h>
#include <eikapp.h>
#include <eikbtgpc.h>
#include <eikmenup.h>
#include <eikspane.h>
#include <utf.h>
#include <vnkey.rsg>
#include "VnKey.hrh"
#include "VnKeyAppUi.h"
#include "VnKeyAppView.h"
#include "VnFepProbe.h"
#include "VnLog.h"
#include "VnShared.h"
#include "VnKeyVersion.h"

_LIT(KStartFmt, "VnKey %S start, log in %S");
_LIT(KOptionsFmt, "options: vietnamese %d modern %d walone %d spell %d restore %d macros %d (%d) method %d logkeys %d");
// What VnKey shows on its screens is in the resource file (vnkey_vi.loc, vnkey_en.loc); the
// texts here are the log's, which stays in English.
_LIT(KTitleFmt, "VnKey %S %S");
_LIT(KModeV, "[V]");
_LIT(KModeE, "[E]");
_LIT(KMacrosFmt, "macros: %d from %S, line %d not taken");
_LIT(KTriggersFmt, "triggers: %d from %S, line %d not taken");
_LIT(KSharedNewFmt, "shared state: defined, flags %03x");
_LIT(KSharedOldFmt, "shared state: taken, flags %03x");
_LIT(KSharedErrFmt, "shared state: error %d, VnKey alone");
_LIT(KSwitchFmt, "system FEP: installing %08x");
_LIT(KSwitchErrFmt, "system FEP: error %d");
_LIT(KFepErrFmt, "fep check %d: error %d");
_LIT(KLoadedFepFmt, "fep: now %08x in VnKey");

// The resource files of the two languages, on the drive VnKey is installed on. The
// framework loads vnkey.rsc at the start; UseLanguageL swaps it for the other.
_LIT(KResourceVi, "\\resource\\apps\\vnkey.rsc");
_LIT(KResourceEn, "\\resource\\apps\\vnkey_en.rsc");
_LIT(KLanguageFmt, "language: %S, error %d");
_LIT(KLangViName, "vi");
_LIT(KLangEnName, "en");

// While VnKey turns Telex in all apps back on by itself, this file is next to the log. If
// the phone restarts before it is removed again, the next start does not try again.
_LIT(KGuardFile, "restoring.txt");
_LIT8(KGuardText, "VnKey is turning Telex in all apps back on. If the phone restarts before this file is gone, VnKey leaves it off.\r\n");
const TInt KGuardTime = 60000000; // a minute after the switch the guard goes

// Written to macros.txt when there is none (UTF-8): ko không, dc được, vn Việt Nam.
_LIT8(KSampleMacros,
      "# VnKey typing shortcuts: a key, a space, then the text it becomes. One per line.\r\n"
      "ko kh\xC3\xB4" "ng\r\n"
      "dc \xC4\x91\xC6\xB0\xE1\xBB\xA3" "c\r\n"
      "vn Vi\xE1\xBB\x87" "t Nam\r\n");

// Written to triggers.txt when there is none (ASCII).
_LIT8(KSampleTriggers,
      "# VnKey trigger keys: the key that switches Vietnamese on and off. One per line.\r\n"
      "# Join modifiers and a key with +.  Modifiers: ctrl shift fn alt.\r\n"
      "# Keys: space enter tab bksp, or a scan code like 0x5f.\r\n"
      "# Turn on 'Log every key' to read a key's scan code from the log.\r\n"
      "ctrl+space\r\n"
      "# Add more lines for more trigger keys, for example:\r\n"
      "# shift+space\r\n"
      "# fn+space\r\n");

const TInt KMaxLogSent = 1024 * 1024; // the last megabyte of the log at most

_LIT(KSendFmt, "send log: to %S, %d bytes");
_LIT(KSentFmt, "send log: error %d, %S");
const TInt KMaxReplyShown = 100; // of the stub's reply, in the note
_LIT(KSettingsFile, "settings.txt");
// SaveSettings writes this first and then puts it in settings.txt's place, so that a
// restart of the phone in the middle of a save cannot leave settings.txt half written (the
// live off of 2026-10-17 15:36 restarted the phone and left it empty: every option off).
_LIT(KSettingsTemp, "settings.tmp");
_LIT(KSettingsLoaded, "settings: flags %d, place %d, system %d, boot %d, lang %d, server %S");
_LIT(KSettingsNoFlags, "settings: no flags in %S, the options keep their defaults");
_LIT8(KFlagsKey, "flags");
_LIT8(KPlaceKey, "place");
_LIT8(KSystemKey, "system");
_LIT8(KBootKey, "boot");      // vnkeyboot.exe reads it too: boot=0 is the only value it heeds
_LIT8(KLangKey, "lang");      // vi (also when absent) or en
_LIT8(KLangVi8, "vi");
_LIT8(KLangEn8, "en");
_LIT8(KServerKey, "logserver");

void CVnKeyAppUi::ConstructL()
    {
    BaseConstructL(EAknEnableSkin);
    iLog = CVnLog::NewL();
    TBuf<200> line;
    line.Format(KStartFmt, &KVnVersion, &iLog->Folder());
    iLog->Write(line);

    iCatcher = CVnCatcher::NewL(*this);
    iIndicator = CVnIndicator::NewL(this);
    iSender = CVnLogSender::NewL(*this);
    iGuardTimer = CPeriodic::NewL(CActive::EPriorityIdle);
    iNoteCallBack = new (ELeave) CAsyncCallBack(TCallBack(ShowSkippedNote, this),
                                                CActive::EPriorityStandard);
    iLanguageCallBack = new (ELeave) CAsyncCallBack(TCallBack(LanguageChanged, this),
                                                    CActive::EPriorityStandard);
    iLogServer.Copy(KVnLogServer);
    iBootStart = ETrue;
    TRAPD(err, LoadMacrosL());
    if (err != KErrNone)
        {
        line.Format(_L("macros: error %d"), err);
        iLog->Write(line);
        }
    TRAP(err, LoadTriggersL());
    if (err != KErrNone)
        {
        line.Format(_L("triggers: error %d"), err);
        iLog->Write(line);
        }
    TRAP(err, LoadSettingsL());
    if (err != KErrNone)
        {
        line.Format(_L("settings: error %d"), err);
        iLog->Write(line);
        }
    ShareState();

    // The texts in the saved language, before anything shows them: the framework loaded
    // vnkey.rsc (Vietnamese) and made the softkeys from it.
    if (iLanguage != iUiLanguage)
        {
        TRAP(err, UseLanguageL(iLanguage));
        line.Format(KLanguageFmt, iUiLanguage == EVnLangEnglish ? &KLangEnName : &KLangViName, err);
        iLog->Write(line);
        ShowSoftkeysL();
        }

    // The typing test, behind until Options > Typing test, and the settings list in front.
    iView = CVnKeyAppView::NewL(ClientRect());
    iView->MakeVisible(EFalse);
    iSettings = NewSettingListL();
    AddToStackL(iSettings);
    iSettings->SetFocus(ETrue);

    iCoeEnv->AddObserverOfLoadedFepL(*this);
    ShowModeL();
    LogOptions();
    LogIndicator();
    // the FEP list costs nothing and is wanted in every log
    RunFepCheck(EVnCmdFepList);
    }

CVnKeyAppUi::~CVnKeyAppUi()
    {
    iCoeEnv->RemoveObserverOfLoadedFep(*this);
    if (iGuardTimer && iGuardTimer->IsActive() && iLog)
        {
        RemoveGuard(); // VnKey closed with no restart of the phone since the restore
        }
    delete iGuardTimer;
    delete iNoteCallBack;
    delete iLanguageCallBack;
    delete iSender;
    if (iSettings)
        {
        RemoveFromStack(iSettings);
        delete iSettings;
        }
    if (iView)
        {
        RemoveFromStack(iView);
        delete iView;
        }
    delete iCatcher; // takes itself off the control stack
    delete iIndicator;
    if (iLog)
        {
        iLog->Write(_L("VnKey exit"));
        }
    delete iLog;
    }

// Started by vnkeyboot.exe at boot (EApaCommandBackground): VnKey stays behind the other
// applications; a start from the menu comes with the run command and is in front.
TBool CVnKeyAppUi::ProcessCommandParametersL(TApaCommand aCommand, TFileName& aDocumentName,
                                             const TDesC8& aTail)
    {
    if (aCommand == EApaCommandBackground)
        {
        if (iLog)
            {
            iLog->Write(_L("started at boot, in the background"));
            }
        TApaTask task(iEikonEnv->WsSession());
        task.SetWgId(iEikonEnv->RootWin().Identifier());
        task.SendToBackground();
        }
    return CAknAppUi::ProcessCommandParametersL(aCommand, aDocumentName, aTail);
    }

void CVnKeyAppUi::HandleCommandL(TInt aCommand)
    {
    switch (aCommand)
        {
        case EEikCmdExit:
        case EAknSoftkeyExit:
            Exit();
            return;
        case EAknSoftkeyBack:
        case EVnCmdSettings:
            ShowTypingL(EFalse);
            return;
        case EVnCmdTyping:
            ShowTypingL(ETrue);
            return;
        case EVnCmdChange:
            iSettings->EditCurrentL();
            return;
        case EVnCmdFepList:
        case EVnCmdFepLoad:
        case EVnCmdFepCreate:
            RunFepCheck(aCommand);
            return;
        case EVnCmdClear:
            iView->ClearL();
            iLog->Write(_L("--- text cleared"));
            return;
        case EVnCmdReload:
            {
            TRAPD(rerr, LoadMacrosL());
            TRAP(rerr, LoadTriggersL());
            NoteResL(ENoteConfirm, R_VNKEY_NOTE_RELOADED, EFalse);
            return;
            }
        case EVnCmdSendLog:
            SendLogL();
            return;
        case EVnCmdClearLog:
            {
            iLog->Clear();
            TBuf<40> line;
            line.Format(_L("VnKey %S log cleared"), &KVnVersion);
            iLog->Write(line);
            NoteResL(ENoteConfirm, R_VNKEY_NOTE_LOG_CLEARED, EFalse);
            return;
            }
        default:
            return;
        }
    }

// A row of the settings list was changed (the selection key or Options > Change).
void CVnKeyAppUi::SettingChangedL(TInt aId)
    {
    const TVnSettingData& d = iSettings->Data();
    TVnOptions options(iCatcher->Options());
    switch (aId)
        {
        case EVnSetVietnamese:
            if (d.iVietnamese == iCatcher->Vietnamese())
                {
                return;
                }
            iCatcher->SetVietnamese(d.iVietnamese);
            ShowModeL();
            break;
        case EVnSetAllApps:
            SetAllAppsL(d.iAllApps);
            return;
        case EVnSetBoot:
            if (d.iBoot == iBootStart)
                {
                return;
                }
            iBootStart = d.iBoot;
            SaveSettings();
            iLog->Write(iBootStart ? _L("settings: start at boot on") : _L("settings: start at boot off"));
            if (iBootStart)
                {
                NoteResL(ENoteInfo, R_VNKEY_NOTE_BOOT_ON, ETrue);
                }
            return;
        case EVnSetLanguage:
            if (d.iLanguage == iLanguage)
                {
                return;
                }
            iLanguage = d.iLanguage;
            SaveSettings();
            // Not here: the list is inside its EditItemL, and it is made again.
            iLanguageCallBack->CallBack();
            return;
        case EVnSetTone:
            options.iModernTone = d.iModernTone;
            break;
        case EVnSetWAlone:
            options.iWAlone = d.iWAlone;
            break;
        case EVnSetSpell:
            options.iSpellCheck = d.iSpellCheck;
            break;
        case EVnSetRestore:
            options.iRestore = d.iRestore;
            break;
        case EVnSetMacros:
            options.iMacros = d.iMacros;
            break;
        case EVnSetMethod:
            if (d.iMethod >= 0 && d.iMethod < CVnCatcher::EMethodCount)
                {
                iCatcher->SetMethod(static_cast<CVnCatcher::TMethod>(d.iMethod));
                }
            ShowModeL();
            break;
        case EVnSetPlace:
            iIndicator->SetPlace(d.iPlace);
            LogIndicator();
            SaveSettings();
            return;
        case EVnSetLogKeys:
            iCatcher->SetLogKeys(d.iLogKeys);
            break;
        case EVnSetServer:
            iLogServer.Copy(d.iServer);
            iLogServer.TrimAll();
            SaveSettings();
            RefreshSettingsL();
            return;
        default:
            return;
        }
    iCatcher->SetOptions(options);
    PublishState();
    SaveSettings();
    LogOptions();
    }

// The row shows the saved preference; the title ends in "all" while the plugin is the FEP.
// On: the switch now. Off: saved first, so that the next start leaves it off whatever
// happens, then the switch now only if asked for (the live off may restart the phone).
void CVnKeyAppUi::SetAllAppsL(TBool aOn)
    {
    if (aOn == iFepDesired)
        {
        RefreshSettingsL();
        return;
        }
    if (aOn)
        {
        if (SystemFepIsVnKey())
            {
            iFepDesired = ETrue; // never went off: it was to go at the next restart
            SaveSettings();
            NoteResL(ENoteInfo, R_VNKEY_NOTE_ALL_ON, ETrue);
            }
        else
            {
            SwitchSystemFepL(ETrue, ETrue);
            }
        }
    else
        {
        iFepDesired = EFalse;
        SaveSettings();
        iLog->Write(_L("settings: Telex in all apps off from the next start"));
        if (SystemFepIsVnKey())
            {
            CAknQueryDialog* dlg = CAknQueryDialog::NewL(); // the question is in the resource
            if (dlg->ExecuteLD(R_VNKEY_OFF_QUERY))
                {
                SwitchSystemFepL(EFalse, ETrue);
                }
            else
                {
                NoteResL(ENoteInfo, R_VNKEY_NOTE_OFF_LATER, ETrue);
                }
            }
        }
    RefreshSettingsL();
    ShowModeL();
    }

// The typing test (the editor) in front, with Back on the right softkey, or the settings
// list with Exit. The one in front is on the control stack, the other is hidden.
void CVnKeyAppUi::ShowTypingL(TBool aTyping)
    {
    if (aTyping == iTyping)
        {
        return;
        }
    CCoeControl* hide = aTyping ? static_cast<CCoeControl*>(iSettings) : iView;
    CCoeControl* show = aTyping ? static_cast<CCoeControl*>(iView) : iSettings;
    RemoveFromStack(hide);
    hide->MakeVisible(EFalse);
    hide->SetFocus(EFalse);
    show->SetRect(ClientRect());
    show->MakeVisible(ETrue);
    AddToStackL(show);
    show->SetFocus(ETrue);
    iTyping = aTyping;
    ShowSoftkeysL();
    if (!aTyping)
        {
        RefreshSettingsL();
        }
    show->DrawDeferred();
    iLog->Write(aTyping ? _L("--- typing test") : _L("--- settings"));
    }

// Options and Exit on the settings list, Options and Back in the typing test, in the
// language of the texts (VnKey's own softkeys, not Avkon's).
void CVnKeyAppUi::ShowSoftkeysL()
    {
    CEikButtonGroupContainer* cba = CEikButtonGroupContainer::Current();
    if (cba)
        {
        cba->SetCommandSetL(iTyping ? R_VNKEY_SOFTKEYS_OPTIONS_BACK : R_VNKEY_SOFTKEYS_OPTIONS_EXIT);
        cba->DrawDeferred();
        }
    }

// The settings list from the resource file loaded now, its rows with the state, not yet
// on the control stack.
CVnSettingList* CVnKeyAppUi::NewSettingListL()
    {
    CVnSettingList* list = new (ELeave) CVnSettingList(*this);
    CleanupStack::PushL(list);
    FillSettingData(list->Data()); // the rows take their values when they are made
    list->SetMopParent(this);
    list->ConstructFromResourceL(R_VNKEY_SETTING_LIST);
    list->SetRect(ClientRect());
    list->ActivateL();
    CleanupStack::Pop(list);
    return list;
    }

void CVnKeyAppUi::FillSettingData(TVnSettingData& d)
    {
    const TVnOptions& o = iCatcher->Options();
    d.iVietnamese = iCatcher->Vietnamese();
    d.iAllApps = iFepDesired;
    d.iBoot = iBootStart;
    d.iLanguage = iLanguage;
    d.iModernTone = o.iModernTone;
    d.iWAlone = o.iWAlone;
    d.iSpellCheck = o.iSpellCheck;
    d.iRestore = o.iRestore;
    d.iMacros = o.iMacros;
    d.iMethod = iCatcher->Method();
    d.iPlace = iIndicator->Place();
    d.iLogKeys = iCatcher->LogKeys();
    d.iServer.Copy(iLogServer);
    }

void CVnKeyAppUi::RefreshSettingsL()
    {
    if (iSettings)
        {
        FillSettingData(iSettings->Data());
        iSettings->RefreshL();
        }
    }

// The Language row was changed (after the list's EditItemL has returned).
TInt CVnKeyAppUi::LanguageChanged(TAny* aSelf)
    {
    CVnKeyAppUi* self = static_cast<CVnKeyAppUi*>(aSelf);
    TRAPD(err, self->ApplyLanguageL());
    TBuf<40> line;
    line.Format(KLanguageFmt, self->iUiLanguage == EVnLangEnglish ? &KLangEnName : &KLangViName, err);
    self->iLog->Write(line);
    return 0;
    }

// The texts swapped, then the screens made again with them: the softkeys, the title, and
// the settings list, on the same row. The menu reads its texts each time it opens.
void CVnKeyAppUi::ApplyLanguageL()
    {
    UseLanguageL(iLanguage);
    ShowSoftkeysL();
    ShowModeL();
    // the new list first, so that a failure leaves the old one there
    CVnSettingList* list = NewSettingListL();
    const TInt row = iSettings->CurrentRow();
    if (!iTyping)
        {
        RemoveFromStack(iSettings);
        }
    delete iSettings;
    iSettings = list;
    iSettings->SetCurrentRow(row);
    if (iTyping)
        {
        iSettings->MakeVisible(EFalse);
        }
    else
        {
        AddToStackL(iSettings);
        iSettings->SetFocus(ETrue);
        iSettings->DrawDeferred();
        }
    }

// VnKey's resource file swapped for the one of aLanguage. Both are made from vnkey.rss with
// the same NAME, so they have the same resource IDs and the same offset: the application
// frees the one loaded now at the exit, as it would have the first.
void CVnKeyAppUi::UseLanguageL(TInt aLanguage)
    {
    if (aLanguage == iUiLanguage)
        {
        return;
        }
    TFileName name;
    ResourceFile(aLanguage, name);
    if (!BaflUtils::FileExists(iCoeEnv->FsSession(), name))
        {
        User::Leave(KErrNotFound); // the texts stay as they are
        }
    TFileName old;
    ResourceFile(iUiLanguage, old);
    iCoeEnv->DeleteResourceFile(Application()->ResourceFileOffset());
    TRAPD(err, iCoeEnv->AddResourceFileL(name));
    if (err != KErrNone)
        {
        iCoeEnv->AddResourceFileL(old); // the texts of before
        User::Leave(err);
        }
    iUiLanguage = aLanguage;
    }

void CVnKeyAppUi::ResourceFile(TInt aLanguage, TDes& aName) const
    {
    const TFileName exe(Application()->AppFullName());
    TParsePtrC app(exe);
    aName.Copy(app.Drive());
    aName.Append(aLanguage == EVnLangEnglish ? KResourceEn() : KResourceVi());
    }

void CVnKeyAppUi::NoteL(TNoteKind aKind, const TDesC& aText, TBool aWaiting)
    {
    CAknResourceNoteDialog* note = NULL;
    switch (aKind)
        {
        case ENoteConfirm:
            note = new (ELeave) CAknConfirmationNote(aWaiting);
            break;
        case ENoteError:
            note = new (ELeave) CAknErrorNote(aWaiting);
            break;
        default:
            note = new (ELeave) CAknInformationNote(aWaiting);
            break;
        }
    note->ExecuteLD(aText);
    }

void CVnKeyAppUi::NoteResL(TNoteKind aKind, TInt aResourceId, TBool aWaiting)
    {
    HBufC* text = iEikonEnv->AllocReadResourceLC(aResourceId);
    NoteL(aKind, *text, aWaiting);
    CleanupStack::PopAndDestroy(text);
    }

void CVnKeyAppUi::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
    {
    if (aResourceId != R_VNKEY_MENU)
        {
        return;
        }
    aMenuPane->SetItemDimmed(EVnCmdChange, iTyping);
    aMenuPane->SetItemDimmed(EVnCmdTyping, iTyping);
    aMenuPane->SetItemDimmed(EVnCmdClear, !iTyping);
    aMenuPane->SetItemDimmed(EVnCmdSettings, !iTyping);
    }

void CVnKeyAppUi::HandleStatusPaneSizeChange()
    {
    if (iView)
        {
        iView->SetRect(ClientRect());
        }
    if (iSettings)
        {
        iSettings->SetRect(ClientRect());
        }
    }

void CVnKeyAppUi::HandleResourceChangeL(TInt aType)
    {
    CAknAppUi::HandleResourceChangeL(aType);
    if (aType == KEikDynamicLayoutVariantSwitch)
        {
        HandleStatusPaneSizeChange();
        }
    }

// Back in front: Ctrl+Space in another application may have changed the state.
void CVnKeyAppUi::HandleForegroundEventL(TBool aForeground)
    {
    CAknAppUi::HandleForegroundEventL(aForeground);
    if (aForeground && iCatcher)
        {
        iCatcher->SyncShared();
        ShowModeL();
        SaveSettings(); // a Ctrl+Space in another application may have changed the state
        RefreshSettingsL();
        if (iRestoreSkipped)
            {
            iNoteCallBack->CallBack();
            }
        }
    }

// CONE loaded a FEP in VnKey: at the start (some time after ConstructL) and after
// Telex in all apps switched the system's FEP.
void CVnKeyAppUi::HandleChangeInLoadedFep()
    {
    TBuf<40> line;
    line.Format(KLoadedFepFmt, iCoeEnv->FepUid().iUid);
    iLog->Write(line);
    iSystemFepOn = iCoeEnv->FepUid() == KVnFepImplUid;
    // At start, when the saved preference is on but the phone came back as AknFep (the FEP
    // setting is not kept over a reboot), put the plugin back as the system FEP, once.
    if (iFepDesired && !iSystemFepOn && !iFepRestoreTried)
        {
        iFepRestoreTried = ETrue;
        RestoreSystemFep();
        }
    TRAP_IGNORE(ShowModeL());
    TRAP_IGNORE(RefreshSettingsL());
    }

void CVnKeyAppUi::RestoreSystemFep()
    {
    if (VnFiles::KillSwitch(iLog->Fs()))
        {
        iLog->Write(_L("settings: kill switch (VnKey\\off), Telex in all apps not restored"));
        return;
        }
    if (GuardThere())
        {
        // the last restore was cut short by a restart of the phone: not again, and off
        iLog->Write(_L("settings: the phone restarted during the last restore; Telex in all apps left off"));
        RemoveGuard();
        iFepDesired = EFalse;
        SaveSettings();
        iRestoreSkipped = ETrue;
        if (IsForeground())
            {
            iNoteCallBack->CallBack();
            }
        return;
        }
    WriteGuard();
    iLog->Write(_L("settings: restoring Telex in all apps"));
    TRAP_IGNORE(SwitchSystemFepL(ETrue, EFalse));
    if (iSystemFepOn)
        {
        iGuardTimer->Cancel();
        iGuardTimer->Start(KGuardTime, KGuardTime, TCallBack(GuardDone, this));
        }
    else
        {
        RemoveGuard(); // not switched: nothing to guard
        }
    }

void CVnKeyAppUi::GuardFileName(TDes& aName) const
    {
    aName.Copy(iLog->Folder());
    aName.Append(KGuardFile);
    }

TBool CVnKeyAppUi::GuardThere()
    {
    TFileName name;
    GuardFileName(name);
    TEntry entry;
    return iLog->Fs().Entry(name, entry) == KErrNone;
    }

void CVnKeyAppUi::WriteGuard()
    {
    TFileName name;
    GuardFileName(name);
    RFile file;
    if (file.Replace(iLog->Fs(), name, EFileWrite) == KErrNone)
        {
        file.Write(KGuardText);
        file.Flush();
        file.Close();
        }
    }

void CVnKeyAppUi::RemoveGuard()
    {
    TFileName name;
    GuardFileName(name);
    iLog->Fs().Delete(name);
    }

TInt CVnKeyAppUi::GuardDone(TAny* aSelf)
    {
    CVnKeyAppUi* self = static_cast<CVnKeyAppUi*>(aSelf);
    self->iGuardTimer->Cancel();
    self->RemoveGuard();
    self->iLog->Write(_L("settings: Telex in all apps restored, the phone kept going"));
    return 0;
    }

TInt CVnKeyAppUi::ShowSkippedNote(TAny* aSelf)
    {
    CVnKeyAppUi* self = static_cast<CVnKeyAppUi*>(aSelf);
    if (!self->iRestoreSkipped)
        {
        return 0;
        }
    self->iRestoreSkipped = EFalse;
    TRAP_IGNORE(self->NoteResL(ENoteInfo, R_VNKEY_NOTE_SKIPPED, ETrue));
    return 0;
    }

void CVnKeyAppUi::CatcherLog(const TDesC& aLine)
    {
    iLog->Write(aLine);
    }

void CVnKeyAppUi::IndicatorLog(const TDesC& aLine)
    {
    iLog->Write(aLine);
    }

// Ctrl+Space in VnKey, or the shared state changed in another application.
void CVnKeyAppUi::CatcherModeChanged(TBool /*aVietnamese*/)
    {
    TRAP_IGNORE(ShowModeL());
    TRAP_IGNORE(RefreshSettingsL());
    SaveSettings();
    }

// The shared state (TVnShared): defined with VnKey's own state at the first start since
// the phone started; taken from it when VnKey ran before.
void CVnKeyAppUi::ShareState()
    {
    TBuf<60> line;
    TInt err = TVnShared::Define();
    if (err == KErrNone)
        {
        PublishState();
        TInt flags = 0;
        TVnShared::Get(flags);
        line.Format(KSharedNewFmt, flags);
        }
    else if (err == KErrAlreadyExists)
        {
        iCatcher->SyncShared();
        TInt flags = 0;
        TVnShared::Get(flags);
        line.Format(KSharedOldFmt, flags);
        }
    else
        {
        line.Format(KSharedErrFmt, err);
        }
    iLog->Write(line);
    }

// VnKey's state and options to every application.
void CVnKeyAppUi::PublishState()
    {
    TVnShared::Set(TVnShared::Encode(iCatcher->Vietnamese(), iCatcher->Options(), iCatcher->LogKeys(),
                                     iCatcher->Method()));
    }

TBool CVnKeyAppUi::SystemFepIsVnKey() const
    {
    // Our own record: FepUid() stays AknFep's in VnKey's own process even after the
    // switch, and the setting does not survive a reboot (so this starts off, matching
    // the phone, which comes back as AknFep).
    return iSystemFepOn;
    }

// Telex in all apps: the system's FEP becomes VnKey's plugin, or AknFep again. CONE then
// loads the new one in every running application. The setting does not survive a reboot
// (it comes back as AknFep), which is the safe way to turn it off.
void CVnKeyAppUi::SwitchSystemFepL(TBool aOn, TBool aAnnounce)
    {
    TBuf<80> line;
    const TUid uid = aOn ? KVnFepImplUid : KVnAknFepUid;
    if (!aOn && iFepDesired)
        {
        iFepDesired = EFalse; // saved first: the live off may restart the phone
        SaveSettings();
        }
    line.Format(KSwitchFmt, uid.iUid);
    iLog->Write(line);
    TRAPD(err, iCoeEnv->InstallFepL(uid));
    if (err == KErrNone)
        {
        iSystemFepOn = aOn;
        if (iFepDesired != aOn)
            {
            // the saved preference, restored at the next start; an off was saved above,
            // before the swap, not here where the phone may be restarting
            iFepDesired = aOn;
            SaveSettings();
            }
        }
    else
        {
        line.Format(KSwitchErrFmt, err);
        iLog->Write(line);
        }
    RunFepCheck(EVnCmdFepList);
    ShowModeL();
    if (!aAnnounce)
        {
        return; // the start-up restore is silent
        }
    if (err == KErrNone)
        {
        NoteResL(ENoteInfo, aOn ? R_VNKEY_NOTE_ALL_ON : R_VNKEY_NOTE_ALL_OFF, ETrue);
        }
    else
        {
        HBufC* fmt = iEikonEnv->AllocReadResourceLC(R_VNKEY_NOTE_SWITCH_ERROR);
        HBufC* text = HBufC::NewLC(fmt->Length() + 12);
        text->Des().Format(*fmt, err);
        NoteL(ENoteError, *text, ETrue);
        CleanupStack::PopAndDestroy(2, fmt); // text, fmt
        }
    }

// The saved settings in settings.txt next to the log (key=value, ASCII): the options and
// state packed as the shared flags, the indicator place, the Telex-in-all-apps preference
// and the log server. Absent at the first run (saved on the first change). Does not touch
// the catcher's shortcuts or triggers (their own files).
void CVnKeyAppUi::LoadSettingsL()
    {
    TFileName name(iLog->Folder());
    name.Append(KSettingsFile);
    RFile file;
    if (file.Open(iLog->Fs(), name, EFileRead | EFileShareReadersOnly) != KErrNone)
        {
        // a restart between the two steps of SaveSettings leaves only the new file
        name.Copy(iLog->Folder());
        name.Append(KSettingsTemp);
        if (file.Open(iLog->Fs(), name, EFileRead | EFileShareReadersOnly) != KErrNone)
            {
            return; // no file yet: the built-in defaults stand
            }
        }
    CleanupClosePushL(file);
    TInt size = 0;
    User::LeaveIfError(file.Size(size));
    HBufC8* raw = HBufC8::NewLC(Min(size, 1024));
    TPtr8 rawPtr(raw->Des());
    User::LeaveIfError(file.Read(rawPtr));

    TInt flags = 0;
    TBool haveFlags = EFalse;   // an empty or cut file must not turn every option off
    TInt place = 0;
    TBool system = EFalse;
    TBool boot = iBootStart;
    TInt language = EVnLangVietnamese;
    TBuf<60> server(iLogServer);
    TPtrC8 rest(rawPtr);
    while (rest.Length() > 0)
        {
        TInt end = rest.Locate('\n');
        TPtrC8 ln(end < 0 ? rest : rest.Left(end));
        rest.Set(end < 0 ? TPtrC8() : rest.Mid(end + 1));
        TInt eq = ln.Locate('=');
        if (eq < 0)
            {
            continue;
            }
        TPtrC8 key(ln.Left(eq));
        TPtrC8 val(ln.Mid(eq + 1));
        while (val.Length() > 0 && (val[val.Length() - 1] == '\r' || val[val.Length() - 1] == ' '))
            {
            val.Set(val.Left(val.Length() - 1));
            }
        TLex8 lex(val);
        if (key == KFlagsKey) { haveFlags = lex.Val(flags) == KErrNone; }
        else if (key == KPlaceKey) { lex.Val(place); }
        else if (key == KSystemKey) { TInt v = 0; lex.Val(v); system = v != 0; }
        else if (key == KBootKey) { TInt v = 1; lex.Val(v); boot = v != 0; }
        else if (key == KLangKey) { language = val == KLangEn8 ? EVnLangEnglish : EVnLangVietnamese; }
        else if (key == KServerKey && val.Length() <= server.MaxLength()) { server.Copy(val); }
        }
    CleanupStack::PopAndDestroy(2, &file); // raw, file

    if (haveFlags)
        {
        TBool vietnamese = ETrue;
        TBool logKeys = EFalse;
        TInt method = CVnCatcher::EMethodKeys;
        TVnOptions options;
        TVnShared::Decode(flags, vietnamese, options, logKeys, method);
        iCatcher->SetOptions(options);
        iCatcher->SetVietnamese(vietnamese);
        iCatcher->SetLogKeys(logKeys);
        if (method >= 0 && method < CVnCatcher::EMethodCount)
            {
            iCatcher->SetMethod(static_cast<CVnCatcher::TMethod>(method));
            }
        }
    else
        {
        TBuf<200> warn;
        warn.Format(KSettingsNoFlags, &name);
        iLog->Write(warn);
        }
    iIndicator->SetPlace(place);
    iFepDesired = system;
    iBootStart = boot;
    iLanguage = language;
    iLogServer.Copy(server);

    TBuf<200> line;
    line.Format(KSettingsLoaded, flags, place, system, boot, language, &iLogServer);
    iLog->Write(line);
    }

void CVnKeyAppUi::SaveSettings()
    {
    if (!iLog || !iCatcher || !iIndicator)
        {
        return;
        }
    TFileName name(iLog->Folder());
    name.Append(KSettingsFile);
    TFileName temp(iLog->Folder());
    temp.Append(KSettingsTemp);
    RFile file;
    if (file.Replace(iLog->Fs(), temp, EFileWrite) != KErrNone)
        {
        return; // best effort: a setting that cannot be written is lost
        }
    const TInt flags = TVnShared::Encode(iCatcher->Vietnamese(), iCatcher->Options(),
                                         iCatcher->LogKeys(), iCatcher->Method());
    TBuf8<200> out;
    out.AppendFormat(_L8("flags=%d\r\n"), flags);
    out.AppendFormat(_L8("place=%d\r\n"), iIndicator->Place());
    out.AppendFormat(_L8("system=%d\r\n"), iFepDesired ? 1 : 0);
    out.AppendFormat(_L8("boot=%d\r\n"), iBootStart ? 1 : 0);
    out.AppendFormat(_L8("lang=%S\r\n"), iLanguage == EVnLangEnglish ? &KLangEn8 : &KLangVi8);
    TBuf8<64> server;
    server.Copy(iLogServer.Left(server.MaxLength()));
    out.AppendFormat(_L8("logserver=%S\r\n"), &server);
    TInt err = file.Write(out);
    if (err == KErrNone)
        {
        err = file.Flush();
        }
    file.Close();
    if (err == KErrNone)
        {
        iLog->Fs().Replace(temp, name); // the old settings.txt stays whole until here
        }
    }

// The indicator and the title: "VnKey [V] m1", and "all" ("tất cả") while Telex in all
// apps is on.
void CVnKeyAppUi::ShowModeL()
    {
    const TBool on = iCatcher->Vietnamese();
    iIndicator->SetVietnamese(on);
    TBuf<8> method;
    method.Format(_L("m%d"), iCatcher->Method() + 1);
    TBuf<40> title;
    title.Format(KTitleFmt, on ? &KModeV : &KModeE, &method);
    if (SystemFepIsVnKey())
        {
        HBufC* all = iEikonEnv->AllocReadResourceLC(R_VNKEY_TITLE_ALL);
        title.Append(all->Left(title.MaxLength() - title.Length()));
        CleanupStack::PopAndDestroy(all);
        }
    CAknTitlePane* pane = static_cast<CAknTitlePane*>(StatusPane()->ControlL(TUid::Uid(EEikStatusPaneUidTitle)));
    pane->SetTextL(title);
    }

void CVnKeyAppUi::LogOptions()
    {
    const TVnOptions& o = iCatcher->Options();
    TBuf<200> line;
    line.Format(KOptionsFmt, iCatcher->Vietnamese(), o.iModernTone, o.iWAlone, o.iSpellCheck,
                o.iRestore, o.iMacros, iCatcher->Macros().Count(), iCatcher->Method() + 1,
                iCatcher->LogKeys());
    iLog->Write(line);
    }

void CVnKeyAppUi::LogIndicator()
    {
    TBuf<400> line;
    iIndicator->Describe(line);
    iLog->Write(line);
    }

// macros.txt next to the log (VnFiles::LoadMacrosL); a sample is written when there is
// none. The plugin reads the same file in every application.
void CVnKeyAppUi::LoadMacrosL()
    {
    TFileName name(iLog->Folder());
    name.Append(KVnMacroFile);
    TEntry entry;
    if (iLog->Fs().Entry(name, entry) == KErrNotFound)
        {
        WriteSampleMacrosL(name);
        }
    TInt bad = 0;
    VnFiles::LoadMacrosL(iLog->Fs(), name, iCatcher->Macros(), bad);
    TBuf<200> line;
    line.Format(KMacrosFmt, iCatcher->Macros().Count(), &name, bad);
    iLog->Write(line);
    }

void CVnKeyAppUi::WriteSampleMacrosL(const TDesC& aName)
    {
    RFile file;
    User::LeaveIfError(file.Create(iLog->Fs(), aName, EFileWrite));
    CleanupClosePushL(file);
    User::LeaveIfError(file.Write(KSampleMacros));
    CleanupStack::PopAndDestroy(&file);
    }

// triggers.txt next to the log (VnFiles::LoadTriggersL); a sample is written when there is
// none. The plugin reads the same file in every application.
void CVnKeyAppUi::LoadTriggersL()
    {
    TFileName name(iLog->Folder());
    name.Append(KVnTriggerFile);
    TEntry entry;
    if (iLog->Fs().Entry(name, entry) == KErrNotFound)
        {
        WriteSampleTriggersL(name);
        }
    TInt bad = 0;
    VnFiles::LoadTriggersL(iLog->Fs(), name, iCatcher->Triggers(), bad);
    TBuf<200> line;
    line.Format(KTriggersFmt, iCatcher->Triggers().Count(), &name, bad);
    iLog->Write(line);
    }

void CVnKeyAppUi::WriteSampleTriggersL(const TDesC& aName)
    {
    RFile file;
    User::LeaveIfError(file.Create(iLog->Fs(), aName, EFileWrite));
    CleanupClosePushL(file);
    User::LeaveIfError(file.Write(KSampleTriggers));
    CleanupStack::PopAndDestroy(&file);
    }

// Asks for the stub's address (the last one, at first the one the build baked in) and
// sends the log there; LogSent() says how it went.
void CVnKeyAppUi::SendLogL()
    {
    if (iSender->IsSending())
        {
        NoteResL(ENoteInfo, R_VNKEY_NOTE_BUSY, EFalse);
        return;
        }
    TBuf<60> server(iLogServer);
    CAknTextQueryDialog* query = CAknTextQueryDialog::NewL(server);
    if (!query->ExecuteLD(R_VNKEY_SERVER_QUERY))
        {
        return;
        }
    server.TrimAll();
    iLogServer.Copy(server);
    SaveSettings(); // remember the web stub's address
    RefreshSettingsL();
    HBufC8* content = iLog->ContentL(KMaxLogSent);
    CleanupStack::PushL(content);
    TBuf<100> line;
    line.Format(KSendFmt, &iLogServer, content->Length());
    iLog->Write(line); // in the next log sent, not this one
    TRAPD(err, iSender->SendL(iLogServer, *content));
    CleanupStack::PopAndDestroy(content);
    if (err != KErrNone)
        {
        LogSent(err, KNullDesC);
        return;
        }
    NoteResL(ENoteInfo, R_VNKEY_NOTE_SENDING, EFalse);
    }

void CVnKeyAppUi::LogSent(TInt aError, const TDesC& aReply)
    {
    TBuf<200> line;
    const TPtrC logged(aReply.Left(line.MaxLength() - 40));
    line.Format(KSentFmt, aError, &logged);
    iLog->Write(line);
    TRAP_IGNORE(SentNoteL(aError, aReply));
    }

// "Log sent: <the stub's reply>", or "Log not sent: error <n> <reply>".
void CVnKeyAppUi::SentNoteL(TInt aError, const TDesC& aReply)
    {
    const TPtrC reply(aReply.Left(KMaxReplyShown));
    HBufC* fmt = iEikonEnv->AllocReadResourceLC(aError == KErrNone ? R_VNKEY_NOTE_SENT
                                                                     : R_VNKEY_NOTE_NOT_SENT);
    HBufC* text = HBufC::NewLC(fmt->Length() + reply.Length() + 12);
    if (aError == KErrNone)
        {
        text->Des().Format(*fmt, &reply);
        }
    else
        {
        text->Des().Format(*fmt, aError, &reply);
        }
    NoteL(aError == KErrNone ? ENoteConfirm : ENoteError, *text, EFalse);
    CleanupStack::PopAndDestroy(2, fmt); // text, fmt
    }

void CVnKeyAppUi::RunFepCheck(TInt aCommand)
    {
    TInt err = KErrNone;
    switch (aCommand)
        {
        case EVnCmdFepList:
            TRAP(err, VnFepProbe::ListL(*iCoeEnv, *iLog));
            break;
        case EVnCmdFepLoad:
            TRAP(err, VnFepProbe::LoadPluginL(*iCoeEnv, *iLog));
            break;
        default:
            TRAP(err, VnFepProbe::CreateFepL(*iCoeEnv, *iLog));
            break;
        }
    if (err != KErrNone)
        {
        TBuf<40> line;
        line.Format(KFepErrFmt, aCommand - EVnCmdFepList + 1, err);
        iLog->Write(line);
        }
    }
