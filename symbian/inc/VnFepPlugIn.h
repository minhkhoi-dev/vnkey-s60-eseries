#ifndef VNFEPPLUGIN_H
#define VNFEPPLUGIN_H

#include <fepplugin.h>
#include "VnCatcher.h"

class CVnLog;

// VnKey's FEP plugin, vnkeyfep.dll (roadmap 2, CLAUDE.md "Design"). VnKey installs it as
// the phone's FEP in place of AknFep, and CONE then makes one in every application. It
// loads AknFep's plugin and returns the FEP object AknFep makes, so that everything AknFep
// does goes on unchanged, and adds a CVnCatcher to the application, above AknFep on the
// control stack. Not in VnKey's own process (it has its own catcher), and not while the
// kill switch is there (VnFiles::KillSwitch).
//
// CONE keeps this object as long as the FEP it made and deletes the FEP first; this object
// keeps AknFep's plugin, so AknFep's DLL stays loaded as long as AknFep's FEP lives.
class CVnFepPlugIn : public CCoeFepPlugIn, public MVnCatcherObserver
    {
public:
    static CVnFepPlugIn* NewL(); // ECOM's factory
    ~CVnFepPlugIn();

    // From CCoeFepPlugIn
    CCoeFep* NewFepL(CCoeEnv& aConeEnvironment, const CCoeFepParameters& aFepParameters);
    void SynchronouslyExecuteSettingsDialogL(CCoeEnv& aConeEnvironment);

private:
    CVnFepPlugIn() {}
    void LoadAknFepL();
    void AddCatcherL(CCoeEnv& aEnv);
    void Log(TRefByValue<const TDesC> aFmt, ...);

    // From MVnCatcherObserver
    void CatcherLog(const TDesC& aLine);
    void CatcherModeChanged(TBool aVietnamese);
    void CatcherDestroyed();

private:
    CCoeFepPlugIn* iAknFep;     // AknFep's plugin, owned
    CVnCatcher* iCatcher;       // owned by the app UI's control stack (CVnCatcher::NewL)
    CVnLog* iLog;
    };

#endif // VNFEPPLUGIN_H
