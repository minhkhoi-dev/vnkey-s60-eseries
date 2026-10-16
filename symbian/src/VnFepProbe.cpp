#include <badesca.h>
#include <coemain.h>
#include <ecom/ecom.h>
#include <fepbase.h>
#include <fepplugin.h>
#include "VnFepProbe.h"
#include "VnLog.h"

_LIT(KLoadedFmt, "fep: loaded uid %08x, object %S");
_LIT(KImplFmt, "fep impl %08x v%d drive %c rom %d: %S");
_LIT(KCountFmt, "fep: %d implementations of %08x");
_LIT(KAvailFmt, "fep available %08x: %S");
_LIT(KYes, "yes");
_LIT(KNo, "none");

static void ResetAndDestroy(TAny* aArray)
    {
    static_cast<RImplInfoPtrArray*>(aArray)->ResetAndDestroy();
    }

void VnFepProbe::ListL(CCoeEnv& aEnv, CVnLog& aLog)
    {
    TBuf<200> line;
    line.Format(KLoadedFmt, aEnv.FepUid().iUid, aEnv.Fep() ? &KYes : &KNo);
    aLog.Write(line);

    RImplInfoPtrArray impls;
    CleanupStack::PushL(TCleanupItem(ResetAndDestroy, &impls));
    REComSession::ListImplementationsL(KUidFepPluginInterface, impls);
    line.Format(KCountFmt, impls.Count(), KUidFepPluginInterface.iUid);
    aLog.Write(line);
    for (TInt i = 0; i < impls.Count(); ++i)
        {
        const CImplementationInformation* info = impls[i];
        TChar drive;
        if (RFs::DriveToChar(info->Drive(), drive) != KErrNone)
            {
            drive = '?';
            }
        line.Format(KImplFmt, info->ImplementationUid().iUid, info->Version(), TUint(drive),
                    info->RomBased(), &info->DisplayName());
        aLog.Write(line);
        }
    CleanupStack::PopAndDestroy(&impls);

    RArray<TUid> uids;
    CleanupClosePushL(uids);
    CDesCArrayFlat* names = new (ELeave) CDesCArrayFlat(4);
    CleanupStack::PushL(names);
    aEnv.AvailableFepsL(uids, names);
    for (TInt i = 0; i < uids.Count(); ++i)
        {
        const TPtrC name(i < names->Count() ? (*names)[i] : KNullDesC());
        line.Format(KAvailFmt, uids[i].iUid, &name);
        aLog.Write(line);
        }
    CleanupStack::PopAndDestroy(2, &uids); // names, uids
    }

// The UID of the FEP this application runs, or, while CONE has not loaded it yet (it does
// so from an active object after the app's start: 0.0.1 saw uid 0 there), the first FEP
// plugin ECOM knows: on the E72 the only one, AknFEP 0x101FD65A.
static TUid FepUidL(CCoeEnv& aEnv, CVnLog& aLog)
    {
    if (aEnv.FepUid() != KNullUid)
        {
        return aEnv.FepUid();
        }
    RImplInfoPtrArray impls;
    CleanupStack::PushL(TCleanupItem(ResetAndDestroy, &impls));
    REComSession::ListImplementationsL(KUidFepPluginInterface, impls);
    if (impls.Count() == 0)
        {
        User::Leave(KErrNotFound);
        }
    const TUid uid = impls[0]->ImplementationUid();
    CleanupStack::PopAndDestroy(&impls);
    aLog.Write(_L("fep: none loaded yet, taking the first plugin"));
    return uid;
    }

void VnFepProbe::LoadPluginL(CCoeEnv& aEnv, CVnLog& aLog)
    {
    const TUid uid = FepUidL(aEnv, aLog);
    TBuf<80> line;
    line.Format(_L("fep: loading plugin %08x"), uid.iUid);
    aLog.Write(line);
    CCoeFepPlugIn* plugin = CCoeFepPlugIn::NewL(uid);
    aLog.Write(_L("fep: plugin loaded"));
    delete plugin;
    aLog.Write(_L("fep: plugin freed"));
    }

// CCoeFepParameters has no data and no constructor an application can call (its NewLC is
// not exported); FEPs only pass it on to CCoeFep::BaseConstructL. An empty CBase object
// stands in for it here. VnKey's own plugin will pass on the real one CONE gives it.
class CVnFakeFepParameters : public CBase
    {
    };

void VnFepProbe::CreateFepL(CCoeEnv& aEnv, CVnLog& aLog)
    {
    const TUid uid = FepUidL(aEnv, aLog);
    TBuf<80> line;
    line.Format(_L("fep: creating a second FEP from %08x"), uid.iUid);
    aLog.Write(line);
    CCoeFepPlugIn* plugin = CCoeFepPlugIn::NewL(uid);
    CleanupStack::PushL(plugin);
    CVnFakeFepParameters* params = new (ELeave) CVnFakeFepParameters;
    CleanupStack::PushL(params);
    CCoeFep* fep = plugin->NewFepL(aEnv, *reinterpret_cast<const CCoeFepParameters*>(params));
    aLog.Write(_L("fep: second FEP created"));
    delete fep;
    aLog.Write(_L("fep: second FEP deleted"));
    CleanupStack::PopAndDestroy(2, plugin); // params, plugin
    aLog.Write(_L("fep: plugin freed"));
    }
