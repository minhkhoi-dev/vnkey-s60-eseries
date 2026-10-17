#include <e32keys.h>
#include <e32property.h>
#include <utf.h>
#include "VnShared.h"

_LIT(KModCtrl, "ctrl");
_LIT(KModShift, "shift");
_LIT(KModFn, "fn");
_LIT(KModAlt, "alt");
_LIT(KKeySpace, "space");
_LIT(KKeyEnter, "enter");
_LIT(KKeyTab, "tab");
_LIT(KKeyBksp, "bksp");
_LIT(KHexPrefix, "0x");

const TUint KVnAllMods = EModifierCtrl | EModifierShift | EModifierFunc | EModifierAlt;

const TUint KVnFlagsKey = 1;

_LIT(KCardFolder, "E:\\VnKey\\");
_LIT(KPhoneFolder, "C:\\Data\\VnKey\\");
_LIT(KCardOff, "E:\\VnKey\\off");
_LIT(KPhoneOff, "C:\\Data\\VnKey\\off");

const TInt KMaxMacroFile = 16 * 1024;

TInt TVnShared::Define()
    {
    const TSecurityPolicy pass(TSecurityPolicy::EAlwaysPass);
    return RProperty::Define(KVnAppUid, KVnFlagsKey, RProperty::EInt, pass, pass);
    }

TBool TVnShared::Get(TInt& aFlags)
    {
    return RProperty::Get(KVnAppUid, KVnFlagsKey, aFlags) == KErrNone;
    }

TInt TVnShared::Set(TInt aFlags)
    {
    return RProperty::Set(KVnAppUid, KVnFlagsKey, aFlags);
    }

TInt TVnShared::Encode(TBool aVietnamese, const TVnOptions& aOptions, TBool aLogKeys, TInt aMethod)
    {
    TInt flags = (aMethod << EMethodShift) & EMethodMask;
    if (aVietnamese) flags |= EVietnamese;
    if (aOptions.iModernTone) flags |= EModernTone;
    if (aOptions.iWAlone) flags |= EWAlone;
    if (aOptions.iSpellCheck) flags |= ESpellCheck;
    if (aOptions.iRestore) flags |= ERestore;
    if (aOptions.iMacros) flags |= EMacros;
    if (aLogKeys) flags |= ELogKeys;
    return flags;
    }

void TVnShared::Decode(TInt aFlags, TBool& aVietnamese, TVnOptions& aOptions, TBool& aLogKeys,
                       TInt& aMethod)
    {
    aMethod = (aFlags & EMethodMask) >> EMethodShift;
    aVietnamese = (aFlags & EVietnamese) != 0;
    aOptions.iModernTone = (aFlags & EModernTone) != 0;
    aOptions.iWAlone = (aFlags & EWAlone) != 0;
    aOptions.iSpellCheck = (aFlags & ESpellCheck) != 0;
    aOptions.iRestore = (aFlags & ERestore) != 0;
    aOptions.iMacros = (aFlags & EMacros) != 0;
    aLogKeys = (aFlags & ELogKeys) != 0;
    }

void VnFiles::Folder(RFs& aFs, TDes& aFolder)
    {
    // the memory card when there is one
    TDriveInfo drive;
    if (aFs.Drive(drive, EDriveE) == KErrNone && drive.iType != EMediaNotPresent
        && aFs.MkDirAll(KCardFolder) != KErrNotReady)
        {
        aFolder.Copy(KCardFolder);
        }
    else
        {
        aFolder.Copy(KPhoneFolder);
        }
    aFs.MkDirAll(aFolder);
    }

TBool VnFiles::KillSwitch(RFs& aFs)
    {
    TEntry entry;
    return aFs.Entry(KCardOff, entry) == KErrNone || aFs.Entry(KPhoneOff, entry) == KErrNone;
    }

void VnFiles::LoadMacrosL(RFs& aFs, const TDesC& aName, TVnMacros& aMacros, TInt& aBadLine)
    {
    aBadLine = 0;
    RFile file;
    User::LeaveIfError(file.Open(aFs, aName, EFileRead | EFileShareReadersOnly));
    CleanupClosePushL(file);
    TInt size = 0;
    User::LeaveIfError(file.Size(size));
    HBufC8* raw = HBufC8::NewLC(Min(size, KMaxMacroFile));
    TPtr8 rawPtr(raw->Des());
    User::LeaveIfError(file.Read(rawPtr));
    HBufC* text = HBufC::NewLC(rawPtr.Length());
    TPtr textPtr(text->Des());
    CnvUtfConverter::ConvertToUnicodeFromUtf8(textPtr, rawPtr);

    aMacros.Reset();
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
        if (!aMacros.Add(key.Ptr(), key.Length(), val.Ptr(), val.Length()) && aBadLine == 0)
            {
            aBadLine = lineNo;
            }
        }
    CleanupStack::PopAndDestroy(3, &file); // text, raw, file
    }


TVnTriggers::TVnTriggers()
    {
    SetDefault();
    }

void TVnTriggers::SetDefault()
    {
    iCount = 0;
    Add(EModifierCtrl, EStdKeySpace); // Ctrl+Space
    }

void TVnTriggers::Clear()
    {
    iCount = 0;
    }

TBool TVnTriggers::Add(TUint aModMask, TInt aScanCode)
    {
    if (iCount >= KMax)
        {
        return EFalse;
        }
    iMod[iCount] = aModMask & KVnAllMods;
    iScan[iCount] = aScanCode;
    ++iCount;
    return ETrue;
    }

TBool TVnTriggers::Matches(TUint aModifiers, TInt aScanCode) const
    {
    for (TInt i = 0; i < iCount; ++i)
        {
        if ((aModifiers & KVnAllMods) == iMod[i] && aScanCode == iScan[i])
            {
            return ETrue;
            }
        }
    return EFalse;
    }

// One token of a trigger line ("ctrl", "space", "0x5f") to a modifier bit or a scan code.
// Returns ETrue and sets aMod (a bit, 0 when the token is a key) and aScan (-1 when the
// token is a modifier).
static TBool ParseTriggerToken(const TPtrC& aToken, TUint& aMod, TInt& aScan)
    {
    aMod = 0;
    aScan = -1;
    if (aToken.CompareF(KModCtrl) == 0) { aMod = EModifierCtrl; return ETrue; }
    if (aToken.CompareF(KModShift) == 0) { aMod = EModifierShift; return ETrue; }
    if (aToken.CompareF(KModFn) == 0) { aMod = EModifierFunc; return ETrue; }
    if (aToken.CompareF(KModAlt) == 0) { aMod = EModifierAlt; return ETrue; }
    if (aToken.CompareF(KKeySpace) == 0) { aScan = EStdKeySpace; return ETrue; }
    if (aToken.CompareF(KKeyEnter) == 0) { aScan = EStdKeyEnter; return ETrue; }
    if (aToken.CompareF(KKeyTab) == 0) { aScan = EStdKeyTab; return ETrue; }
    if (aToken.CompareF(KKeyBksp) == 0) { aScan = EStdKeyBackspace; return ETrue; }
    if (aToken.Length() > 2 && aToken.Left(2).CompareF(KHexPrefix) == 0)
        {
        TLex lex(aToken.Mid(2));
        TUint value = 0;
        if (lex.Val(value, EHex) == KErrNone && lex.Eos())
            {
            aScan = static_cast<TInt>(value);
            return ETrue;
            }
        }
    return EFalse;
    }

// One line ("ctrl+space") to a modifier mask and a scan code. EFalse when a token is not
// known or there is not exactly one key.
static TBool ParseTriggerLine(const TPtrC& aLine, TUint& aMod, TInt& aScan)
    {
    aMod = 0;
    aScan = -1;
    TPtrC rest(aLine);
    while (rest.Length() > 0)
        {
        TInt plus = rest.Locate('+');
        TPtrC token(plus < 0 ? rest : rest.Left(plus));
        rest.Set(plus < 0 ? TPtrC() : rest.Mid(plus + 1));
        while (token.Length() > 0 && (token[0] == ' ' || token[0] == '\t'))
            {
            token.Set(token.Mid(1));
            }
        while (token.Length() > 0 && (token[token.Length() - 1] == ' ' || token[token.Length() - 1] == '\t'))
            {
            token.Set(token.Left(token.Length() - 1));
            }
        if (token.Length() == 0)
            {
            continue;
            }
        TUint mod = 0;
        TInt scan = -1;
        if (!ParseTriggerToken(token, mod, scan))
            {
            return EFalse;
            }
        if (scan >= 0)
            {
            if (aScan >= 0)
                {
                return EFalse; // two keys in one line
                }
            aScan = scan;
            }
        else
            {
            aMod |= mod;
            }
        }
    return aScan >= 0;
    }

void VnFiles::LoadTriggersL(RFs& aFs, const TDesC& aName, TVnTriggers& aTriggers, TInt& aBadLine)
    {
    aBadLine = 0;
    aTriggers.Clear();
    RFile file;
    User::LeaveIfError(file.Open(aFs, aName, EFileRead | EFileShareReadersOnly));
    CleanupClosePushL(file);
    TInt size = 0;
    User::LeaveIfError(file.Size(size));
    HBufC8* raw = HBufC8::NewLC(Min(size, 8 * 1024));
    TPtr8 rawPtr(raw->Des());
    User::LeaveIfError(file.Read(rawPtr));
    HBufC* text = HBufC::NewLC(rawPtr.Length());
    TPtr textPtr(text->Des());
    CnvUtfConverter::ConvertToUnicodeFromUtf8(textPtr, rawPtr);

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
            line.Set(line.Mid(1));
            }
        while (line.Length() > 0 && (line[0] == ' ' || line[0] == '\t'))
            {
            line.Set(line.Mid(1));
            }
        if (line.Length() == 0 || line[0] == '#')
            {
            continue;
            }
        TUint mod = 0;
        TInt scan = -1;
        if (ParseTriggerLine(line, mod, scan) && aTriggers.Add(mod, scan))
            {
            // taken
            }
        else if (aBadLine == 0)
            {
            aBadLine = lineNo;
            }
        }
    CleanupStack::PopAndDestroy(3, &file); // text, raw, file
    if (aTriggers.Count() == 0)
        {
        aTriggers.SetDefault(); // never leave the user with no way to switch
        }
    }
