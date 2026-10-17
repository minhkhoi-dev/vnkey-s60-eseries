#include <akntitle.h>
#include <aknnotewrappers.h>
#include <aknquerydialog.h>
#include <avkon.hrh>
#include <eikmenup.h>
#include <eikspane.h>
#include <utf.h>
#include <vnkey.rsg>
#include "VnKey.hrh"
#include "VnKeyAppUi.h"
#include "VnKeyAppView.h"
#include "VnIndicator.h"
#include "VnFepProbe.h"
#include "VnLog.h"
#include "VnShared.h"
#include "VnKeyVersion.h"

_LIT(KStartFmt, "VnKey %S start, log in %S");
_LIT(KOptionsFmt, "options: vietnamese %d modern %d walone %d spell %d restore %d macros %d (%d) method %d logkeys %d");
_LIT(KTitleFmt, "VnKey %S %S%S");
_LIT(KTitleAll, " all");
_LIT(KModeV, "[V]");
_LIT(KModeE, "[E]");
_LIT(KMacrosFmt, "macros: %d from %S, line %d not taken");
_LIT(KTriggersFmt, "triggers: %d from %S, line %d not taken");
_LIT(KReloaded, "Reloaded macros and triggers");
_LIT(KSharedNewFmt, "shared state: defined, flags %02x");
_LIT(KSharedOldFmt, "shared state: taken, flags %02x");
_LIT(KSharedErrFmt, "shared state: error %d, VnKey alone");
_LIT(KSwitchFmt, "system FEP: installing %08x");
_LIT(KSwitchErrFmt, "system FEP: error %d");
_LIT(KAllOn, "Telex in all apps: on. To turn it off, restart the phone, or use this menu again.");
_LIT(KAllOff, "Telex in all apps: off.");
_LIT(KAllErrFmt, "Not switched: error %d. See the log.");
_LIT(KOffWarn, "Turning Telex off here may restart the phone. Restarting the phone turns it off safely. Turn off now anyway?");
_LIT(KMenuAll, "Telex in all apps: %S");
_LIT(KFepErrFmt, "fep check %d: error %d");
_LIT(KLoadedFepFmt, "fep: now %08x in VnKey");
_LIT(KOn, "on");
_LIT(KOff, "off");

_LIT(KMenuVietnamese, "Vietnamese: %S");
_LIT(KMenuTone, "Tone: %S");
_LIT(KToneModern, "modern (thu\x00FD)");
_LIT(KToneTraditional, "traditional (th\x00FAy)");
_LIT(KMenuWAlone, "w alone types \x01B0: %S");
_LIT(KMenuSpell, "Spell check: %S");
_LIT(KMenuRestore, "Restore English: %S");
_LIT(KMenuMacros, "Shortcuts: %S");
_LIT(KMenuMethod, "Rewrite method: %d");
_LIT(KMenuPlace, "Indicator place: %d");
_LIT(KMenuLogKeys, "Log every key: %S");

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
_LIT(KSentOkFmt, "Log sent: %S");
_LIT(KSentBadFmt, "Log not sent: error %d %S");
_LIT(KSending, "Sending the log");
_LIT(KBusy, "Still sending the log");
_LIT(KLogCleared, "Log cleared");

void CVnKeyAppUi::ConstructL()
    {
    BaseConstructL(EAknEnableSkin);
    iLog = CVnLog::NewL();
    TBuf<200> line;
    line.Format(KStartFmt, &KVnVersion, &iLog->Folder());
    iLog->Write(line);

    iView = CVnKeyAppView::NewL(ClientRect());
    AddToStackL(iView);
    iCatcher = CVnCatcher::NewL(*this);
    iIndicator = CVnIndicator::NewL();
    iSender = CVnLogSender::NewL(*this);
    iLogServer.Copy(KVnLogServer);
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
    ShareState();
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
    delete iSender;
    delete iCatcher; // takes itself off the control stack
    delete iIndicator;
    if (iView)
        {
        RemoveFromStack(iView);
        delete iView;
        }
    if (iLog)
        {
        iLog->Write(_L("VnKey exit"));
        }
    delete iLog;
    }

void CVnKeyAppUi::HandleCommandL(TInt aCommand)
    {
    TVnOptions options(iCatcher->Options());
    switch (aCommand)
        {
        case EEikCmdExit:
        case EAknSoftkeyExit:
            Exit();
            return;
        case EVnCmdToggle:
            iCatcher->SetVietnamese(!iCatcher->Vietnamese());
            ShowModeL();
            break;
        case EVnCmdSystemFep:
            {
            const TBool turnOn = !SystemFepIsVnKey();
            if (!turnOn)
                {
                CAknQueryDialog* dlg = CAknQueryDialog::NewL();
                dlg->SetPromptL(KOffWarn);
                if (!dlg->ExecuteLD(R_VNKEY_OFF_QUERY))
                    {
                    return;
                    }
                }
            SwitchSystemFepL(turnOn);
            return;
            }
        case EVnCmdTone:
            options.iModernTone = !options.iModernTone;
            break;
        case EVnCmdWAlone:
            options.iWAlone = !options.iWAlone;
            break;
        case EVnCmdSpell:
            options.iSpellCheck = !options.iSpellCheck;
            break;
        case EVnCmdRestore:
            options.iRestore = !options.iRestore;
            break;
        case EVnCmdMacros:
            options.iMacros = !options.iMacros;
            break;
        case EVnCmdMethod:
            iCatcher->SetMethod(static_cast<CVnCatcher::TMethod>((iCatcher->Method() + 1) % CVnCatcher::EMethodCount));
            ShowModeL();
            break;
        case EVnCmdIndicatorPlace:
            iIndicator->SetPlace(iIndicator->Place() + 1);
            LogIndicator();
            return;
        case EVnCmdLogKeys:
            iCatcher->SetLogKeys(!iCatcher->LogKeys());
            break;
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
            CAknConfirmationNote* note = new (ELeave) CAknConfirmationNote;
            note->ExecuteLD(KReloaded);
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
            CAknConfirmationNote* note = new (ELeave) CAknConfirmationNote;
            note->ExecuteLD(KLogCleared);
            return;
            }
        default:
            return;
        }
    iCatcher->SetOptions(options);
    PublishState();
    LogOptions();
    }

static void SetItemL(CEikMenuPane* aMenuPane, TInt aCommand, TRefByValue<const TDesC> aFmt, ...)
    {
    VA_LIST list;
    VA_START(list, aFmt);
    TBuf<60> text;
    text.FormatList(aFmt, list);
    VA_END(list);
    aMenuPane->SetItemTextL(aCommand, text);
    }

void CVnKeyAppUi::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
    {
    if (aResourceId != R_VNKEY_MENU)
        {
        return;
        }
    const TVnOptions& o = iCatcher->Options();
    SetItemL(aMenuPane, EVnCmdToggle, KMenuVietnamese, iCatcher->Vietnamese() ? &KOn : &KOff);
    SetItemL(aMenuPane, EVnCmdSystemFep, KMenuAll, SystemFepIsVnKey() ? &KOn : &KOff);
    SetItemL(aMenuPane, EVnCmdTone, KMenuTone, o.iModernTone ? &KToneModern : &KToneTraditional);
    SetItemL(aMenuPane, EVnCmdWAlone, KMenuWAlone, o.iWAlone ? &KOn : &KOff);
    SetItemL(aMenuPane, EVnCmdSpell, KMenuSpell, o.iSpellCheck ? &KOn : &KOff);
    SetItemL(aMenuPane, EVnCmdRestore, KMenuRestore, o.iRestore ? &KOn : &KOff);
    SetItemL(aMenuPane, EVnCmdMacros, KMenuMacros, o.iMacros ? &KOn : &KOff);
    SetItemL(aMenuPane, EVnCmdMethod, KMenuMethod, iCatcher->Method() + 1);
    SetItemL(aMenuPane, EVnCmdIndicatorPlace, KMenuPlace, iIndicator->Place() + 1);
    SetItemL(aMenuPane, EVnCmdLogKeys, KMenuLogKeys, iCatcher->LogKeys() ? &KOn : &KOff);
    }

void CVnKeyAppUi::HandleStatusPaneSizeChange()
    {
    if (iView)
        {
        iView->SetRect(ClientRect());
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
        }
    }

// CONE loaded a FEP in VnKey: at the start (some time after ConstructL) and after
// Telex in all apps switched the system's FEP.
void CVnKeyAppUi::HandleChangeInLoadedFep()
    {
    TBuf<40> line;
    line.Format(KLoadedFepFmt, iCoeEnv->FepUid().iUid);
    iLog->Write(line);
    TRAP_IGNORE(ShowModeL());
    }

void CVnKeyAppUi::CatcherLog(const TDesC& aLine)
    {
    iLog->Write(aLine);
    }

void CVnKeyAppUi::CatcherModeChanged(TBool /*aVietnamese*/)
    {
    TRAP_IGNORE(ShowModeL());
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
void CVnKeyAppUi::SwitchSystemFepL(TBool aOn)
    {
    TBuf<80> line;
    const TUid uid = aOn ? KVnFepImplUid : KVnAknFepUid;
    line.Format(KSwitchFmt, uid.iUid);
    iLog->Write(line);
    TRAPD(err, iCoeEnv->InstallFepL(uid));
    if (err == KErrNone)
        {
        iSystemFepOn = aOn;
        }
    else
        {
        line.Format(KSwitchErrFmt, err);
        iLog->Write(line);
        }
    RunFepCheck(EVnCmdFepList);
    ShowModeL();
    if (err == KErrNone)
        {
        CAknInformationNote* note = new (ELeave) CAknInformationNote(ETrue);
        note->ExecuteLD(aOn ? KAllOn() : KAllOff());
        }
    else
        {
        TBuf<60> text;
        text.Format(KAllErrFmt, err);
        CAknErrorNote* note = new (ELeave) CAknErrorNote(ETrue);
        note->ExecuteLD(text);
        }
    }

// The indicator and the title: "VnKey [V] m1", and "all" while Telex in all apps is on.
void CVnKeyAppUi::ShowModeL()
    {
    const TBool on = iCatcher->Vietnamese();
    iIndicator->SetVietnamese(on);
    TBuf<8> method;
    method.Format(_L("m%d"), iCatcher->Method() + 1);
    TBuf<40> title;
    title.Format(KTitleFmt, on ? &KModeV : &KModeE, &method, SystemFepIsVnKey() ? &KTitleAll : &KNullDesC);
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
        CAknInformationNote* note = new (ELeave) CAknInformationNote;
        note->ExecuteLD(KBusy);
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
    CAknInformationNote* note = new (ELeave) CAknInformationNote;
    note->ExecuteLD(KSending);
    }

void CVnKeyAppUi::LogSent(TInt aError, const TDesC& aReply)
    {
    TBuf<200> line;
    line.Format(KSentFmt, aError, &aReply);
    iLog->Write(line);
    TBuf<160> text;
    if (aError == KErrNone)
        {
        text.Format(KSentOkFmt, &aReply);
        CAknConfirmationNote* note = new CAknConfirmationNote;
        if (note)
            {
            TRAP_IGNORE(note->ExecuteLD(text));
            }
        }
    else
        {
        text.Format(KSentBadFmt, aError, &aReply);
        CAknErrorNote* note = new CAknErrorNote;
        if (note)
            {
            TRAP_IGNORE(note->ExecuteLD(text));
            }
        }
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
