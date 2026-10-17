#ifndef VNKEYAPPUI_H
#define VNKEYAPPUI_H

#include <aknappui.h>
#include <coemain.h>
#include "VnCatcher.h"
#include "VnIndicator.h"
#include "VnLogSender.h"
#include "VnSettingList.h"

class CVnKeyAppView;
class CVnLog;
class CPeriodic;

// VnKey's app: the settings list in front (CVnSettingList), and the typing test, an editor
// with the phone's FEP and VnKey's catcher in it (CVnKeyAppView, roadmap 1), behind
// Options > Typing test. It shares the state and the options with the plugin in every
// application (TVnShared), switches the system's FEP to the plugin and back (Telex in all
// apps), and keeps the settings in settings.txt. Started at boot by vnkeyboot.exe with the
// background command, it stays behind the other applications and turns Telex in all apps
// back on when it was on (roadmap 4).
class CVnKeyAppUi : public CAknAppUi, public MVnCatcherObserver, public MVnLogSenderObserver,
                    public MCoeObserverOfLoadedFep, public MVnSettingObserver,
                    public MVnIndicatorObserver
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
    void HandleResourceChangeL(TInt aType);
    TBool ProcessCommandParametersL(TApaCommand aCommand, TFileName& aDocumentName,
                                    const TDesC8& aTail);
    // From MCoeObserverOfLoadedFep
    void HandleChangeInLoadedFep();
    // From MVnCatcherObserver
    void CatcherLog(const TDesC& aLine);
    void CatcherModeChanged(TBool aVietnamese);
    // From MVnLogSenderObserver
    void LogSent(TInt aError, const TDesC& aReply);
    // From MVnSettingObserver
    void SettingChangedL(TInt aId);
    // From MVnIndicatorObserver
    void IndicatorLog(const TDesC& aLine);

    void ShowTypingL(TBool aTyping);    // the typing test in front, or the settings list
    void FillSettingData();             // the state into the list's data
    void RefreshSettingsL();            // and shown
    void SetAllAppsL(TBool aOn);        // the Telex in all apps row was changed
    void ShowModeL();
    void ShareState();
    void PublishState();
    void LoadSettingsL();           // the saved options, state and FEP preference, at start
    void SaveSettings();            // write them after any change (does not leave)
    TBool SystemFepIsVnKey() const;
    // aAnnounce: show the confirmation note (a switch by hand); EFalse restores it silently.
    void SwitchSystemFepL(TBool aOn, TBool aAnnounce);
    // At start: the plugin back as the system FEP when the saved preference says so,
    // unless the guard file says the last try was cut short by a restart of the phone.
    void RestoreSystemFep();
    void GuardFileName(TDes& aName) const;
    TBool GuardThere();
    void WriteGuard();
    void RemoveGuard();
    static TInt GuardDone(TAny* aSelf);
    static TInt ShowSkippedNote(TAny* aSelf);
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
    CVnSettingList* iSettings;
    CVnKeyAppView* iView;   // the typing test
    TBool iTyping;          // the typing test is in front
    CVnCatcher* iCatcher;
    CVnIndicator* iIndicator;
    CVnLogSender* iSender;
    TBuf<60> iLogServer;    // address:port of the web stub, last used
    TBool iSystemFepOn;     // Telex in all apps: the plugin is the system FEP now
    TBool iFepDesired;      // the saved preference: Telex in all apps on or off
    TBool iFepRestoreTried; // the start-up restore of the FEP has been attempted
    TBool iBootStart;       // the saved preference: start at boot (vnkeyboot.exe reads it)
    TBool iRestoreSkipped;  // the guard stopped the restore: a note when VnKey is in front
    CPeriodic* iGuardTimer; // removes the guard file a while after a restore
    CAsyncCallBack* iNoteCallBack;
    };

#endif // VNKEYAPPUI_H
