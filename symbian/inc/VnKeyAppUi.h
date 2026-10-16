#ifndef VNKEYAPPUI_H
#define VNKEYAPPUI_H

#include <aknappui.h>
#include "VnCatcher.h"
#include "VnLogSender.h"

class CVnKeyAppView;
class CVnIndicator;
class CVnLog;

// The probe (roadmap 1, CLAUDE.md): Telex in the app's own editor, with every key event,
// every rewrite and the FEP checks in the log, and the options of the menu.
class CVnKeyAppUi : public CAknAppUi, public MVnCatcherObserver, public MVnLogSenderObserver
    {
public:
    void ConstructL();
    ~CVnKeyAppUi();

private:
    // From CAknAppUi
    void HandleCommandL(TInt aCommand);
    void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);
    void HandleStatusPaneSizeChange();
    // From MVnCatcherObserver
    void CatcherLog(const TDesC& aLine);
    void CatcherModeChanged(TBool aVietnamese);
    // From MVnLogSenderObserver
    void LogSent(TInt aError, const TDesC& aReply);

    void ShowModeL();
    void LoadMacrosL();
    void WriteSampleMacrosL(const TDesC& aName);
    void LogOptions();
    void LogIndicator();
    void RunFepCheck(TInt aCommand);
    void SendLogL();

private:
    CVnLog* iLog;
    CVnKeyAppView* iView;
    CVnCatcher* iCatcher;
    CVnIndicator* iIndicator;
    CVnLogSender* iSender;
    TBuf<60> iLogServer;    // address:port of the web stub, last used
    };

#endif // VNKEYAPPUI_H
