#ifndef VNKEYAPPUI_H
#define VNKEYAPPUI_H

#include <aknappui.h>
#include <coemain.h>
#include "VnCatcher.h"
#include "VnLogSender.h"

class CVnKeyAppView;
class CVnIndicator;
class CVnLog;

// The probe (roadmap 1, CLAUDE.md): Telex in the app's own editor, with every key event,
// every rewrite and the FEP checks in the log, and the options of the menu. Roadmap 2: it
// shares the state and the options with the plugin in every application (TVnShared) and
// switches the system's FEP to the plugin and back (Telex in all apps).
class CVnKeyAppUi : public CAknAppUi, public MVnCatcherObserver, public MVnLogSenderObserver,
                    public MCoeObserverOfLoadedFep
    {
public:
    void ConstructL();
    ~CVnKeyAppUi();

private:
    // From CAknAppUi
    void HandleCommandL(TInt aCommand);
    void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);
    void HandleStatusPaneSizeChange();
    void HandleForegroundEventL(TBool aForeground);
    // From MCoeObserverOfLoadedFep
    void HandleChangeInLoadedFep();
    // From MVnCatcherObserver
    void CatcherLog(const TDesC& aLine);
    void CatcherModeChanged(TBool aVietnamese);
    // From MVnLogSenderObserver
    void LogSent(TInt aError, const TDesC& aReply);

    void ShowModeL();
    void ShareState();
    void PublishState();
    void LoadSettingsL();           // the saved options, state and FEP preference, at start
    void SaveSettings();            // write them after any change (does not leave)
    TBool SystemFepIsVnKey() const;
    // aAnnounce: show the confirmation note (a menu switch); EFalse restores it silently.
    void SwitchSystemFepL(TBool aOn, TBool aAnnounce);
    void LoadMacrosL();
    void WriteSampleMacrosL(const TDesC& aName);
    void LoadTriggersL();
    void WriteSampleTriggersL(const TDesC& aName);
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
    TBool iSystemFepOn;     // Telex in all apps: the plugin is the system FEP now
    TBool iFepDesired;      // the saved preference: Telex in all apps on or off
    TBool iFepRestoreTried; // the start-up restore of the FEP has been attempted
    };

#endif // VNKEYAPPUI_H
