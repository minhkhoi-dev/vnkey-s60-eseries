#ifndef VNSHARED_H
#define VNSHARED_H

#include <e32base.h>
#include <f32file.h>
#include "VnTelex.h"

// UIDs (CLAUDE.md "Hard constraints"): the app, the plugin DLL and its one implementation.
const TUid KVnAppUid = { 0xE7A10001 };
const TUid KVnFepDllUid = { 0xE7A10002 };
const TUid KVnFepImplUid = { 0xE7A10003 };
// The phone's own FEP, AknFEP in ROM (FEP check 1 on the E72, 2026-10-16).
const TUid KVnAknFepUid = { 0x101FD65A };

// What VnKey shares with its plugin in every application: one Publish and Subscribe
// property in VnKey's own category (its SID), the on/off state and the options as flags.
// VnKey defines it when it starts, and any process may read and write it, so that the
// options set in VnKey and Ctrl+Space in any application reach every application. It is
// gone when the phone restarts, until VnKey runs again; till then the plugin's catchers do
// nothing (CLAUDE.md "Design": a plugin that breaks applications cannot take the phone's
// start with it).
class TVnShared
    {
public:
    enum TFlag
        {
        EVietnamese = 0x01,
        EModernTone = 0x02,
        EWAlone = 0x04,
        ESpellCheck = 0x08,
        ERestore = 0x10,
        EMacros = 0x20,
        ELogKeys = 0x40,    // a log line for every key event, in every application
        EMethodMask = 0x180 // two bits: the rewrite method (0..2), for A/B on the phone
        };
    enum { EMethodShift = 7 };
    // VnKey only (the category is its SID). KErrAlreadyExists when VnKey ran before.
    static TInt Define();
    // EFalse when the property is not there.
    static TBool Get(TInt& aFlags);
    static TInt Set(TInt aFlags);
    static TInt Encode(TBool aVietnamese, const TVnOptions& aOptions, TBool aLogKeys, TInt aMethod);
    static void Decode(TInt aFlags, TBool& aVietnamese, TVnOptions& aOptions, TBool& aLogKeys,
                       TInt& aMethod);
    };

// The keys that switch Vietnamese on and off (CLAUDE.md "Requirements": configurable).
// Small and fixed, so it can be a member of the catcher in every application.
class TVnTriggers
    {
public:
    enum { KMax = 8 };
    TVnTriggers();
    void SetDefault();          // one trigger: Ctrl+Space
    void Clear();
    // aModMask is the EModifier* bits that must be held; aScanCode the key. EFalse when
    // the table is full.
    TBool Add(TUint aModMask, TInt aScanCode);
    TInt Count() const { return iCount; }
    // ETrue when the key event (its modifiers and scan code) is one of the triggers.
    TBool Matches(TUint aModifiers, TInt aScanCode) const;
private:
    TUint iMod[KMax];
    TInt iScan[KMax];
    TInt iCount;
    };

// VnKey's files: the log, macros.txt and the kill switch, in E:\VnKey\ on the memory card
// or C:\Data\VnKey\ without one.
class VnFiles
    {
public:
    // The folder, with a trailing backslash, made when it is not there.
    static void Folder(RFs& aFs, TDes& aFolder);
    // The kill switch: a file or a folder named "off" in E:\VnKey\ or C:\Data\VnKey\ (the
    // phone's File manager can make a folder). While it is there the plugin adds nothing
    // to applications and AknFep works alone.
    static TBool KillSwitch(RFs& aFs);
    // The typing shortcuts of aName (UTF-8: per line a key, a space or tab, and the text;
    // lines that start with # are comments) into aMacros. aBadLine is the first line that
    // was not taken, 0 when all were.
    static void LoadMacrosL(RFs& aFs, const TDesC& aName, TVnMacros& aMacros, TInt& aBadLine);
    // The trigger keys of aName (UTF-8: per line a key spec like "ctrl+space", "fn+space"
    // or "0x5f"; # comments). Falls back to Ctrl+Space when the file has no valid line.
    static void LoadTriggersL(RFs& aFs, const TDesC& aName, TVnTriggers& aTriggers, TInt& aBadLine);
    };

_LIT(KVnMacroFile, "macros.txt");
_LIT(KVnTriggerFile, "triggers.txt");

#endif // VNSHARED_H
