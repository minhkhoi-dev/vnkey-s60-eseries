#include <coeaui.h>
#include <coeinput.h>
#include <coemain.h>
#include <e32keys.h>
#include <frmtlay.h>
#include <txtfrmat.h>
#include <aknedsts.h>
#include "VnCatcher.h"

// Just above the phone's FEP (ECoeStackPriorityFep), below the environment filter.
const TInt KStackPriority = ECoeStackPriorityFep + 1;

_LIT(KTypeDown, "down");
_LIT(KTypeKey, "key ");
_LIT(KTypeUp, "up  ");
_LIT(KKeyFmt, "%S code %04x scan %02x mod %05x rep %d");
_LIT(KEditorFmt, " | cur %d anc %d len %d");
_LIT(KStateFmt, " | mode %x case %x flags %x");
_LIT(KNoEditor, " | no editor");
_LIT(KOk, "ok");
_LIT(KBad, "BAD");

// CCoeFep::SimulateKeyEventsL is how a FEP hands characters to the editor so that it does not
// see them again itself, but it is protected. This class only makes it reachable: the call
// goes to the phone's own FEP object, never one of this type (the function is not virtual).
class CVnFepAccess : public CCoeFep
    {
public:
    CCoeFep::SimulateKeyEventsL;
private:
    CVnFepAccess(CCoeEnv& aEnv); // never made, never defined
    };

// Lines are cut where they do not fit, never a panic.
class TIgnoreOverflow : public TDes16Overflow
    {
public:
    void Overflow(TDes16& /*aDes*/) {}
    };

static TBool IsAsciiLetter(TUint aCode)
    {
    return (aCode >= 'a' && aCode <= 'z') || (aCode >= 'A' && aCode <= 'Z');
    }

// Space, digits, punctuation, Enter, Tab: the word before the cursor is over.
static TBool EndsWord(TUint aCode)
    {
    if (aCode == EKeyEnter || aCode == EKeyLineFeed || aCode == EKeyTab || aCode == 0x2029)
        {
        return ETrue;
        }
    return aCode >= ' ' && aCode < 0x7F && !IsAsciiLetter(aCode);
    }

CVnCatcher* CVnCatcher::NewL(MVnCatcherObserver& aObserver)
    {
    CVnCatcher* self = new (ELeave) CVnCatcher(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CVnCatcher::CVnCatcher(MVnCatcherObserver& aObserver)
    : iObserver(aObserver), iVietnamese(ETrue), iLogKeys(ETrue), iMethod(EMethodInline)
    {
    }

void CVnCatcher::ConstructL()
    {
    iAsyncFlush = new (ELeave) CAsyncCallBack(TCallBack(AsyncFlush, this), CActive::EPriorityStandard);
    iCoeEnv->AppUi()->AddToStackL(this, KStackPriority, ECoeStackFlagRefusesFocus | ECoeStackFlagSharable);
    iStacked = ETrue;
    }

CVnCatcher::~CVnCatcher()
    {
    delete iAsyncFlush;
    if (iStacked && iCoeEnv->AppUi())
        {
        iCoeEnv->AppUi()->RemoveFromStack(this);
        }
    }

void CVnCatcher::SetVietnamese(TBool aOn)
    {
    iVietnamese = aOn;
    iPending.iActive = EFalse;
    iPending.iSnap = EFalse;
    iEngine.Reset();
    }

// The editor that has the focus, if it takes a FEP's help; never a password field.
MCoeFepAwareTextEditor* CVnCatcher::Editor() const
    {
    CCoeAppUi* appUi = iCoeEnv->AppUi();
    if (!appUi)
        {
        return NULL;
        }
    const TCoeInputCapabilities caps(appUi->InputCapabilities());
    if (caps.SupportsSecretText())
        {
        return NULL;
        }
    return caps.FepAwareTextEditor();
    }

// Key events the catcher or the FEP sends itself go through untouched.
TBool CVnCatcher::IsSimulating() const
    {
    if (iBusy)
        {
        return ETrue;
        }
    const CCoeFep* fep = iCoeEnv->Fep();
    return fep && fep->IsSimulatingKeyEvent();
    }

TKeyResponse CVnCatcher::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
    {
    if (IsSimulating())
        {
        return EKeyWasNotConsumed;
        }
    MCoeFepAwareTextEditor* editor = Editor();
    if (iLogKeys)
        {
        LogKey(aKeyEvent, aType, editor);
        }
    switch (aType)
        {
        case EEventKeyDown:
            // AknFep has finished with the key before; it may type this one at once
            Flush();
            Snapshot(editor, aKeyEvent.iScanCode);
            return EKeyWasNotConsumed;
        case EEventKeyUp:
            // rewritten once AknFep has handled the release too: it may hold the letter
            // until then (a long press gives the Fn character)
            if (iPending.iActive && aKeyEvent.iScanCode == iPending.iScanCode)
                {
                iAsyncFlush->CallBack();
                }
            return EKeyWasNotConsumed;
        case EEventKey:
            break;
        default:
            return EKeyWasNotConsumed;
        }

    Flush();
    if ((aKeyEvent.iModifiers & EModifierCtrl)
        && (aKeyEvent.iScanCode == EStdKeySpace || aKeyEvent.iCode == ' '))
        {
        SetVietnamese(!iVietnamese);
        iObserver.CatcherModeChanged(iVietnamese);
        return EKeyWasConsumed;
        }
    if (!iVietnamese || !editor || (aKeyEvent.iModifiers & EModifierCtrl))
        {
        return EKeyWasNotConsumed;
        }
    if (aKeyEvent.iRepeats > 0)
        {
        // a long press: AknFep's (the Fn character), not Telex
        iPending.iActive = EFalse;
        iPending.iSnap = EFalse;
        iEngine.Reset();
        return EKeyWasNotConsumed;
        }
    const TUint code = aKeyEvent.iCode;
    if (IsAsciiLetter(code))
        {
        if (!iPending.iSnap || iPending.iScanCode != aKeyEvent.iScanCode || iPending.iEditor != editor)
            {
            Snapshot(editor, aKeyEvent.iScanCode); // no key down seen for it
            }
        if (iPending.iSnap)
            {
            iPending.iCode = code;
            iPending.iActive = ETrue;
            }
        }
    else if (EndsWord(code))
        {
        iPending.iSnap = EFalse;
        TRAPD(err, EndWordL(*editor));
        if (err != KErrNone)
            {
            Log(_L("end of word: error %d"), err);
            }
        }
    else
        {
        iPending.iSnap = EFalse;
        }
    return EKeyWasNotConsumed;
    }

void CVnCatcher::LogKey(const TKeyEvent& aKeyEvent, TEventCode aType, MCoeFepAwareTextEditor* aEditor)
    {
    TIgnoreOverflow overflow;
    TBuf<160> line;
    const TDesC* type = &KTypeKey;
    if (aType == EEventKeyDown)
        {
        type = &KTypeDown;
        }
    else if (aType == EEventKeyUp)
        {
        type = &KTypeUp;
        }
    line.AppendFormat(KKeyFmt, &overflow, type, aKeyEvent.iCode, aKeyEvent.iScanCode,
                      aKeyEvent.iModifiers, aKeyEvent.iRepeats);
    if (!aEditor)
        {
        line.Append(KNoEditor);
        }
    else
        {
        TCursorSelection sel;
        aEditor->GetCursorSelectionForFep(sel);
        line.AppendFormat(KEditorFmt, &overflow, sel.iCursorPos, sel.iAnchorPos,
                          aEditor->DocumentLengthForFep());
        MCoeFepAwareTextEditor_Extension1* ext = aEditor->Extension1();
        CAknEdwinState* state = ext ? static_cast<CAknEdwinState*>(ext->State(KNullUid)) : NULL;
        if (state)
            {
            line.AppendFormat(KStateFmt, &overflow, state->CurrentInputMode(), state->CurrentCase(),
                              state->Flags());
            }
        }
    iObserver.CatcherLog(line);
    }

// The word before aPos, into aWord (KVnMaxWord + 1 characters at most). Returns its
// length; KVnMaxWord + 1 means it is too long for the engine.
TInt CVnCatcher::ReadWordBefore(MCoeFepAwareTextEditor& aEditor, TInt aPos, TDes& aWord)
    {
    TBuf<KVnMaxWord + 1> before;
    const TInt from = Max(0, aPos - (KVnMaxWord + 1));
    aEditor.GetEditorContentForFep(before, from, aPos - from);
    TInt start = before.Length();
    while (start > 0 && TVnTelex::IsWordChar(before[start - 1]))
        {
        --start;
        }
    aWord.Copy(before.Mid(start).Left(aWord.MaxLength()));
    return before.Length() - start;
    }

// ETrue when the cursor at aPos is inside a word: Telex then leaves the key alone.
TBool CVnCatcher::WordCharAfter(MCoeFepAwareTextEditor& aEditor, TInt aPos)
    {
    if (aPos >= aEditor.DocumentLengthForFep())
        {
        return EFalse;
        }
    TBuf<1> after;
    aEditor.GetEditorContentForFep(after, aPos, 1);
    return after.Length() > 0 && TVnTelex::IsWordChar(after[0]);
    }

void CVnCatcher::Snapshot(MCoeFepAwareTextEditor* aEditor, TInt aScanCode)
    {
    iPending.iActive = EFalse;
    iPending.iSnap = EFalse;
    if (!iVietnamese || !aEditor)
        {
        return;
        }
    TRAPD(err, SnapshotL(*aEditor, aScanCode));
    if (err != KErrNone)
        {
        Log(_L("snapshot: error %d"), err);
        }
    }

void CVnCatcher::SnapshotL(MCoeFepAwareTextEditor& aEditor, TInt aScanCode)
    {
    TCursorSelection sel;
    aEditor.GetCursorSelectionForFep(sel);
    if (sel.Length() != 0)
        {
        return; // typing over a selection: plain typing
        }
    TBuf<KVnMaxWord + 1> word;
    if (ReadWordBefore(aEditor, sel.iCursorPos, word) > KVnMaxWord || WordCharAfter(aEditor, sel.iCursorPos))
        {
        return;
        }
    iPending.iEditor = &aEditor;
    iPending.iPos = sel.iCursorPos;
    iPending.iDocLength = aEditor.DocumentLengthForFep();
    iPending.iScanCode = aScanCode;
    iPending.iWord.Copy(word);
    iPending.iSnap = ETrue;
    }

TInt CVnCatcher::AsyncFlush(TAny* aSelf)
    {
    static_cast<CVnCatcher*>(aSelf)->Flush();
    return KErrNone;
    }

void CVnCatcher::Flush()
    {
    iAsyncFlush->Cancel();
    if (!iPending.iActive)
        {
        return;
        }
    TRAPD(err, FlushL());
    iPending.iActive = EFalse;
    iPending.iSnap = EFalse;
    if (err != KErrNone)
        {
        Log(_L("flush: error %d"), err);
        }
    }

// AknFep has typed the letter: if the editor holds exactly the word from before and the
// letter after it, the engine gets them.
void CVnCatcher::FlushL()
    {
    iPending.iActive = EFalse;
    MCoeFepAwareTextEditor* editor = Editor();
    if (!editor || editor != iPending.iEditor)
        {
        Log(_L("flush: another editor"));
        return;
        }
    TCursorSelection sel;
    editor->GetCursorSelectionForFep(sel);
    const TInt docLength = editor->DocumentLengthForFep();
    if (sel.Length() != 0 || sel.iCursorPos != iPending.iPos + 1 || docLength != iPending.iDocLength + 1)
        {
        Log(_L("flush: cur %d len %d, before %d %d"), sel.iCursorPos, docLength, iPending.iPos,
            iPending.iDocLength);
        iEngine.Reset();
        return;
        }
    const TInt wordLength = iPending.iWord.Length();
    const TInt start = iPending.iPos - wordLength;
    TBuf<KVnMaxWord + 1> now;
    editor->GetEditorContentForFep(now, start, wordLength + 1);
    if (now.Length() != wordLength + 1 || now.Left(wordLength) != iPending.iWord)
        {
        Log(_L("flush: the text changed"));
        iEngine.Reset();
        return;
        }
    const TText typed = now[wordLength];
    if (TVnTelex::ToLower(typed) != TVnTelex::ToLower(static_cast<TText>(iPending.iCode)))
        {
        Log(_L("flush: typed %04x for key %04x"), typed, iPending.iCode); // Fn, Chr
        iEngine.Reset();
        return;
        }
    TText out[KVnMaxOut];
    TInt outLength = 0;
    if (iEngine.Key(iPending.iWord.Ptr(), wordLength, typed, out, outLength))
        {
        ReplaceL(*editor, start, iPending.iPos + 1, TPtrC(out, outLength));
        }
    }

// A space, a digit, a punctuation mark or Enter is about to be typed.
void CVnCatcher::EndWordL(MCoeFepAwareTextEditor& aEditor)
    {
    TCursorSelection sel;
    aEditor.GetCursorSelectionForFep(sel);
    TBuf<KVnMaxWord + 1> word;
    if (sel.Length() != 0 || ReadWordBefore(aEditor, sel.iCursorPos, word) > KVnMaxWord
        || WordCharAfter(aEditor, sel.iCursorPos))
        {
        iEngine.Reset();
        return;
        }
    TText out[KVnMaxOut];
    TInt outLength = 0;
    if (iEngine.Break(word.Ptr(), word.Length(), &iMacros, out, outLength))
        {
        ReplaceL(aEditor, sel.iCursorPos - word.Length(), sel.iCursorPos, TPtrC(out, outLength));
        }
    }

// [aStart, aEnd) of the editor becomes aText, by the method chosen; the part both have in
// common at the start is left alone. Logs what the editor holds afterwards.
void CVnCatcher::ReplaceL(MCoeFepAwareTextEditor& aEditor, TInt aStart, TInt aEnd, const TDesC& aText)
    {
    TBuf<KVnMaxOut> old;
    aEditor.GetEditorContentForFep(old, aStart, aEnd - aStart);
    TInt common = 0;
    while (common < old.Length() && common < aText.Length() && old[common] == aText[common])
        {
        ++common;
        }
    if (common == aText.Length() && common > 0)
        {
        --common; // something to type, always
        }
    const TInt from = aStart + common;
    const TPtrC added(aText.Mid(common));
    iBusy = ETrue;
    TInt err = KErrNone;
    switch (iMethod)
        {
        case EMethodInline:
            TRAP(err, aEditor.SetCursorSelectionForFepL(TCursorSelection(aEnd, from));
                      InlineL(aEditor, from, added));
            break;
        case EMethodDeleteInline:
            TRAP(err, SimulateL(aEnd - from, KNullDesC); InlineL(aEditor, from, added));
            break;
        default:
            TRAP(err, SimulateL(aEnd - from, added));
            break;
        }
    iBusy = EFalse;

    TCursorSelection sel;
    aEditor.GetCursorSelectionForFep(sel);
    TBuf<KVnMaxOut> got;
    aEditor.GetEditorContentForFep(got, aStart, aText.Length());
    const TBool ok = err == KErrNone && got == aText && sel.Length() == 0
        && sel.iCursorPos == aStart + aText.Length();
    Log(_L("rewrite %d: %S -> %S %S (err %d, got %S, cur %d)"), iMethod, &old, &aText,
        ok ? &KOk : &KBad, err, &got, sel.iCursorPos);
    User::LeaveIfError(err);
    }

// aText typed at aPos as an inline edit, committed at once.
void CVnCatcher::InlineL(MCoeFepAwareTextEditor& aEditor, TInt aPos, const TDesC& aText)
    {
    iInlineEditor = &aEditor;
    iInlinePos = aPos;
    iInlineLength = aText.Length();
    TRAPD(err, aEditor.StartFepInlineEditL(aText, aText.Length(), ETrue, NULL, *this, *this));
    if (err == KErrNone)
        {
        TRAP(err, aEditor.CommitFepInlineEditL(*iCoeEnv));
        if (err != KErrNone)
            {
            aEditor.CancelFepInlineEdit();
            }
        }
    iInlineEditor = NULL;
    User::LeaveIfError(err);
    }

// Backspaces and characters through the FEP, which hands them to the editor as if typed.
void CVnCatcher::SimulateL(TInt aBackspaces, const TDesC& aText)
    {
    CCoeFep* fep = iCoeEnv->Fep();
    if (!fep)
        {
        User::Leave(KErrNotSupported);
        }
    RArray<TUint> chars;
    CleanupClosePushL(chars);
    for (TInt i = 0; i < aBackspaces; ++i)
        {
        chars.AppendL(EKeyBackspace);
        }
    for (TInt i = 0; i < aText.Length(); ++i)
        {
        chars.AppendL(aText[i]);
        }
    static_cast<CVnFepAccess*>(fep)->SimulateKeyEventsL(chars.Array());
    CleanupStack::PopAndDestroy(&chars);
    }

void CVnCatcher::GetFormatOfFepInlineText(TCharFormat& aFormat, TInt& aNumberOfCharactersWithSameFormat,
                                          TInt aPositionOfCharacter) const
    {
    if (iInlineEditor)
        {
        iInlineEditor->GetFormatForFep(aFormat, iInlinePos);
        }
    aNumberOfCharactersWithSameFormat = iInlineLength - aPositionOfCharacter;
    }

void CVnCatcher::HandlePointerEventInInlineTextL(TPointerEvent::TType /*aType*/, TUint /*aModifiers*/,
                                                 TInt /*aPositionInInlineText*/)
    {
    }

void CVnCatcher::Log(TRefByValue<const TDesC> aFmt, ...)
    {
    VA_LIST list;
    VA_START(list, aFmt);
    TIgnoreOverflow overflow;
    TBuf<256> line;
    line.AppendFormatList(aFmt, list, &overflow);
    VA_END(list);
    iObserver.CatcherLog(line);
    }
