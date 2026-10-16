#ifndef VNCATCHER_H
#define VNCATCHER_H

#include <coecntrl.h>
#include <fepbase.h>
#include <fepitfr.h>
#include "VnTelex.h"

class MCoeFepAwareTextEditor;

// What the catcher tells its owner.
class MVnCatcherObserver
    {
public:
    virtual void CatcherLog(const TDesC& aLine) = 0;          // one line for the test log
    virtual void CatcherModeChanged(TBool aVietnamese) = 0;   // Ctrl+Space was pressed
    };

// Telex in the editors of the application it lives in (CLAUDE.md "Design"). It sits on the
// control stack just above the phone's own FEP (AknFep), so it sees every key first, and it
// never takes a key away from it: AknFep types as always, with its case, its Fn layer and
// the rest, and when the key is released (or the next key comes) the catcher looks at the
// one character that appeared after the word before the cursor. A letter goes to the Telex
// engine with the word, which is rewritten in place; anything else (space, digit,
// punctuation, Enter) ends the word: typing shortcuts and the restore of words that are not
// Vietnamese. Ctrl+Space switches Vietnamese on and off and is the one key it keeps.
class CVnCatcher : public CCoeControl, public MFepInlineTextFormatRetriever,
                   public MFepPointerEventHandlerDuringInlineEdit
    {
public:
    // How a word is rewritten in the editor (the probe tries each, docs/TESTING.md).
    enum TMethod
        {
        EMethodInline,          // select the old text and replace it with an inline edit
        EMethodDeleteInline,    // FEP backspaces, then the new text as an inline edit
        EMethodKeys,            // FEP backspaces and the new characters as FEP key events
        EMethodCount
        };

    static CVnCatcher* NewL(MVnCatcherObserver& aObserver);
    ~CVnCatcher();

    void SetVietnamese(TBool aOn);
    TBool Vietnamese() const { return iVietnamese; }
    void SetOptions(const TVnOptions& aOptions) { iEngine.SetOptions(aOptions); }
    const TVnOptions& Options() const { return iEngine.Options(); }
    TVnMacros& Macros() { return iMacros; }
    void SetMethod(TMethod aMethod) { iMethod = aMethod; }
    TMethod Method() const { return iMethod; }
    void SetLogKeys(TBool aOn) { iLogKeys = aOn; }
    TBool LogKeys() const { return iLogKeys; }

    // From CCoeControl
    TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);

private:
    CVnCatcher(MVnCatcherObserver& aObserver);
    void ConstructL();

    MCoeFepAwareTextEditor* Editor() const;
    TBool IsSimulating() const;
    void LogKey(const TKeyEvent& aKeyEvent, TEventCode aType, MCoeFepAwareTextEditor* aEditor);
    void Snapshot(MCoeFepAwareTextEditor* aEditor, TInt aScanCode);
    void SnapshotL(MCoeFepAwareTextEditor& aEditor, TInt aScanCode);
    static TInt ReadWordBefore(MCoeFepAwareTextEditor& aEditor, TInt aPos, TDes& aWord);
    static TBool WordCharAfter(MCoeFepAwareTextEditor& aEditor, TInt aPos);
    static TInt AsyncFlush(TAny* aSelf);
    void Flush();
    void FlushL();
    void ReplaceL(MCoeFepAwareTextEditor& aEditor, TInt aStart, TInt aEnd, const TDesC& aText);
    void InlineL(MCoeFepAwareTextEditor& aEditor, TInt aPos, const TDesC& aText);
    void SimulateL(TInt aBackspaces, const TDesC& aText);
    void Log(TRefByValue<const TDesC> aFmt, ...);

    // From MFepInlineTextFormatRetriever
    void GetFormatOfFepInlineText(TCharFormat& aFormat, TInt& aNumberOfCharactersWithSameFormat,
                                  TInt aPositionOfCharacter) const;
    // From MFepPointerEventHandlerDuringInlineEdit
    void HandlePointerEventInInlineTextL(TPointerEvent::TType aType, TUint aModifiers,
                                         TInt aPositionInInlineText);

private:
    // A letter AknFep is about to type, and what the editor looked like before it: taken
    // when the key goes down (iSnap), and active once the key turned out to be a letter.
    struct TPending
        {
        TBool iSnap;
        TBool iActive;
        MCoeFepAwareTextEditor* iEditor;    // not owned; only compared
        TInt iPos;                          // the cursor before the letter
        TInt iDocLength;
        TInt iScanCode;
        TBuf<KVnMaxWord> iWord;             // the word before the cursor
        };

    MVnCatcherObserver& iObserver;
    TBool iStacked;
    TBool iVietnamese;
    TBool iLogKeys;
    TBool iBusy;                    // rewriting a word: the catcher's own key events pass
    TMethod iMethod;
    TPending iPending;
    TVnTelex iEngine;
    TVnMacros iMacros;
    CAsyncCallBack* iAsyncFlush;    // after AknFep has handled the release of the key
    MCoeFepAwareTextEditor* iInlineEditor;  // during an inline edit, for its format
    TInt iInlinePos;
    TInt iInlineLength;
    };

#endif // VNCATCHER_H
