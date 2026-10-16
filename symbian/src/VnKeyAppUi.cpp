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
#include "VnKeyVersion.h"

_LIT(KStartFmt, "VnKey %S start, log in %S");
_LIT(KOptionsFmt, "options: vietnamese %d modern %d walone %d spell %d restore %d macros %d (%d) method %d logkeys %d");
_LIT(KTitleFmt, "VnKey %S %S");
_LIT(KModeV, "[V]");
_LIT(KModeE, "[E]");
_LIT(KMacroFile, "macros.txt");
_LIT(KMacrosFmt, "macros: %d from %S");
_LIT(KMacroBadFmt, "macros: line %d not taken");
_LIT(KFepErrFmt, "fep check %d: error %d");
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

const TInt KMaxMacroFile = 16 * 1024;
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
    ShowModeL();
    LogOptions();
    LogIndicator();
    // the FEP list costs nothing and is wanted in every log
    RunFepCheck(EVnCmdFepList);
    }

CVnKeyAppUi::~CVnKeyAppUi()
    {
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

void CVnKeyAppUi::CatcherLog(const TDesC& aLine)
    {
    iLog->Write(aLine);
    }

void CVnKeyAppUi::CatcherModeChanged(TBool aVietnamese)
    {
    iLog->Write(aVietnamese ? _L("Ctrl+Space: Vietnamese") : _L("Ctrl+Space: English"));
    TRAP_IGNORE(ShowModeL());
    }

// The indicator and the title: "VnKey [V] m1".
void CVnKeyAppUi::ShowModeL()
    {
    const TBool on = iCatcher->Vietnamese();
    iIndicator->SetVietnamese(on);
    TBuf<8> method;
    method.Format(_L("m%d"), iCatcher->Method() + 1);
    TBuf<40> title;
    title.Format(KTitleFmt, on ? &KModeV : &KModeE, &method);
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

// macros.txt next to the log, UTF-8: per line a key, a space or tab, and the text; lines
// that start with # are comments. A sample is written when there is no file.
void CVnKeyAppUi::LoadMacrosL()
    {
    TFileName name(iLog->Folder());
    name.Append(KMacroFile);
    RFs& fs = iLog->Fs();
    RFile file;
    TInt err = file.Open(fs, name, EFileRead | EFileShareReadersOnly);
    if (err == KErrNotFound)
        {
        WriteSampleMacrosL(name);
        err = file.Open(fs, name, EFileRead | EFileShareReadersOnly);
        }
    User::LeaveIfError(err);
    CleanupClosePushL(file);
    TInt size = 0;
    User::LeaveIfError(file.Size(size));
    HBufC8* raw = HBufC8::NewLC(Min(size, KMaxMacroFile));
    TPtr8 rawPtr(raw->Des());
    User::LeaveIfError(file.Read(rawPtr));
    HBufC* text = HBufC::NewLC(rawPtr.Length());
    TPtr textPtr(text->Des());
    CnvUtfConverter::ConvertToUnicodeFromUtf8(textPtr, rawPtr);

    TVnMacros& macros = iCatcher->Macros();
    macros.Reset();
    TPtrC rest(textPtr);
    TInt lineNo = 0;
    while (rest.Length() > 0)
        {
        ++lineNo;
        TInt end = rest.Locate('\n');
        TPtrC line(end < 0 ? rest : rest.Left(end));
        rest.Set(end < 0 ? TPtrC() : rest.Mid(end + 1));
        if (line.Length() > 0 && line[line.Length() - 1] == '\r')
            {
            line.Set(line.Left(line.Length() - 1));
            }
        if (line.Length() > 0 && line[0] == 0xFEFF)
            {
            line.Set(line.Mid(1)); // a byte order mark
            }
        if (line.Length() == 0 || line[0] == '#')
            {
            continue;
            }
        TInt gap = 0;
        while (gap < line.Length() && line[gap] != ' ' && line[gap] != '\t')
            {
            ++gap;
            }
        TInt value = gap;
        while (value < line.Length() && (line[value] == ' ' || line[value] == '\t'))
            {
            ++value;
            }
        TPtrC key(line.Left(gap));
        TPtrC val(line.Mid(value));
        if (!macros.Add(key.Ptr(), key.Length(), val.Ptr(), val.Length()))
            {
            TBuf<40> bad;
            bad.Format(KMacroBadFmt, lineNo);
            iLog->Write(bad);
            }
        }
    CleanupStack::PopAndDestroy(3, &file); // text, raw, file
    TBuf<200> line;
    line.Format(KMacrosFmt, macros.Count(), &name);
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
