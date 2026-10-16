#ifndef VNFEPPROBE_H
#define VNFEPPROBE_H

#include <e32base.h>

class CCoeEnv;
class CVnLog;

// Checks for roadmap 2, the system-wide plugin that loads the phone's own FEP (AknFep)
// and sits above it in every application (CLAUDE.md "Design"). Each writes what it finds
// to the log; none of them changes anything outside VnKey's own process.
class VnFepProbe
    {
public:
    // The FEP plugins that ECOM knows and the one loaded in this application.
    static void ListL(CCoeEnv& aEnv, CVnLog& aLog);
    // Loads the plugin object of the FEP this application runs, then frees it.
    static void LoadPluginL(CCoeEnv& aEnv, CVnLog& aLog);
    // Has that plugin make a second FEP object, as VnKey's plugin will have to, then
    // deletes it. Risky: the second FEP lives next to the real one for a moment.
    static void CreateFepL(CCoeEnv& aEnv, CVnLog& aLog);
    };

#endif // VNFEPPROBE_H
