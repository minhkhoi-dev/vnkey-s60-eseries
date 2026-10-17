#include <coeaui.h>
#include <coemain.h>
#include <ecom/ecom.h>
#include <ecom/implementationproxy.h>
#include "VnFepPlugIn.h"
#include "VnLog.h"
#include "VnShared.h"
#include "VnKeyVersion.h"

_LIT(KPrefixFmt, "%S: ");
_LIT(KStartFmt, "plugin %S: sid %08x");
_LIT(KAknFepErrFmt, "AknFep %08x: error %d, looking for another FEP");
_LIT(KOtherFepFmt, "FEP %08x loaded instead");
_LIT(KCatcherFmt, "catcher added; macros %d, triggers %d (macros err %d line %d, triggers err %d line %d)");
_LIT(KCatcherErrFmt, "catcher: error %d, AknFep alone");

// Lines are cut where they do not fit, never a panic.
class TVnPlugInOverflow : public TDes16Overflow
    {
public:
    void Overflow(TDes16& /*aDes*/) {}
    };

static void ResetAndDestroy(TAny* aArray)
    {
    static_cast<RImplInfoPtrArray*>(aArray)->ResetAndDestroy();
    }

CVnFepPlugIn* CVnFepPlugIn::NewL()
    {
    return new (ELeave) CVnFepPlugIn;
    }

CVnFepPlugIn::~CVnFepPlugIn()
    {
    CVnCatcher* catcher = iCatcher;
    iCatcher = NULL;
    if (catcher)
        {
        catcher->Unstack();
        delete catcher;
        }
    delete iAknFep; // CONE has deleted AknFep's FEP before this
    Log(_L("plugin freed"));
    delete iLog;
    }

CCoeFep* CVnFepPlugIn::NewFepL(CCoeEnv& aConeEnvironment, const CCoeFepParameters& aFepParameters)
    {
    RProcess process;
    if (!iLog)
        {
        TRAP_IGNORE(iLog = CVnLog::NewL());
        if (iLog)
            {
            const TFileName exe(process.FileName());
            TParsePtrC parse(exe);
            const TPtrC name(parse.Name());
            TBuf<40> prefix;
            prefix.Format(KPrefixFmt, &name);
            iLog->SetPrefix(prefix);
            }
        }
    Log(KStartFmt, &KVnVersion, process.SecureId().iId);

    LoadAknFepL();
    CCoeFep* fep = iAknFep->NewFepL(aConeEnvironment, aFepParameters);
    if (process.SecureId().iId == static_cast<TUint32>(KVnAppUid.iUid))
        {
        Log(_L("VnKey's own process: AknFep alone"));
        return fep;
        }
    if (VnFiles::KillSwitch(aConeEnvironment.FsSession()))
        {
        Log(_L("kill switch (VnKey\\off): AknFep alone"));
        return fep;
        }
    if (!iCatcher)
        {
        TRAPD(err, AddCatcherL(aConeEnvironment));
        if (err != KErrNone)
            {
            Log(KCatcherErrFmt, err);
            }
        }
    return fep;
    }

void CVnFepPlugIn::SynchronouslyExecuteSettingsDialogL(CCoeEnv& aConeEnvironment)
    {
    LoadAknFepL();
    iAknFep->SynchronouslyExecuteSettingsDialogL(aConeEnvironment);
    }

// AknFep's plugin, or, should it not be there, the first other FEP plugin ECOM knows.
void CVnFepPlugIn::LoadAknFepL()
    {
    if (iAknFep)
        {
        return;
        }
    TRAPD(err, iAknFep = CCoeFepPlugIn::NewL(KVnAknFepUid));
    if (err == KErrNone)
        {
        return;
        }
    Log(KAknFepErrFmt, KVnAknFepUid.iUid, err);
    RImplInfoPtrArray impls;
    CleanupStack::PushL(TCleanupItem(ResetAndDestroy, &impls));
    REComSession::ListImplementationsL(KUidFepPluginInterface, impls);
    TUid uid = KNullUid;
    for (TInt i = 0; i < impls.Count() && uid == KNullUid; ++i)
        {
        if (impls[i]->ImplementationUid() != KVnFepImplUid)
            {
            uid = impls[i]->ImplementationUid();
            }
        }
    CleanupStack::PopAndDestroy(&impls);
    if (uid == KNullUid)
        {
        User::Leave(err);
        }
    iAknFep = CCoeFepPlugIn::NewL(uid);
    Log(KOtherFepFmt, uid.iUid);
    }

void CVnFepPlugIn::AddCatcherL(CCoeEnv& aEnv)
    {
    if (!aEnv.AppUi())
        {
        Log(_L("no app UI: AknFep alone"));
        return;
        }
    iCatcher = CVnCatcher::NewL(*this, ETrue);
    RFs& fs = aEnv.FsSession();
    TFileName folder;
    VnFiles::Folder(fs, folder);
    TFileName mname(folder);
    mname.Append(KVnMacroFile);
    TInt mbad = 0;
    TRAPD(merr, VnFiles::LoadMacrosL(fs, mname, iCatcher->Macros(), mbad));
    TFileName tname(folder);
    tname.Append(KVnTriggerFile);
    TInt tbad = 0;
    TRAPD(terr, VnFiles::LoadTriggersL(fs, tname, iCatcher->Triggers(), tbad));
    Log(KCatcherFmt, iCatcher->Macros().Count(), iCatcher->Triggers().Count(),
        merr, mbad, terr, tbad);
    }

void CVnFepPlugIn::Log(TRefByValue<const TDesC> aFmt, ...)
    {
    if (!iLog)
        {
        return;
        }
    VA_LIST list;
    VA_START(list, aFmt);
    TVnPlugInOverflow overflow;
    TBuf<200> line;
    line.AppendFormatList(aFmt, list, &overflow);
    VA_END(list);
    iLog->Write(line);
    }

void CVnFepPlugIn::CatcherLog(const TDesC& aLine)
    {
    if (iLog)
        {
        iLog->Write(aLine);
        }
    }

void CVnFepPlugIn::CatcherModeChanged(TBool aVietnamese)
    {
    Log(aVietnamese ? _L("mode: Vietnamese") : _L("mode: English"));
    }

void CVnFepPlugIn::CatcherDestroyed()
    {
    if (iCatcher)
        {
        iCatcher = NULL; // the app UI deleted it, at the app's exit
        Log(_L("catcher deleted with the app UI"));
        }
    }

const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY(0xE7A10003, CVnFepPlugIn::NewL)
    };

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    aTableCount = sizeof(ImplementationTable) / sizeof(ImplementationTable[0]);
    return ImplementationTable;
    }
