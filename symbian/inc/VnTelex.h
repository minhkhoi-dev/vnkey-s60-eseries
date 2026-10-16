// VnTelex.h - the Telex engine: turns the word before the cursor plus one typed key into
// the text that should replace them, as Unikey does on Windows. Pure logic with the basic
// Symbian types only (no descriptors, no heap, no leaves), so that it builds for the phone
// and with the host's compiler for its tests (symbian/test, CLAUDE.md "Testing the engine").
//
// The engine is stateless about the editor: the caller reads the word before the cursor
// from the editor at every key, so a moved cursor costs nothing. It remembers only the
// keys that made the current word, for the auto-restore at the end of a word and for the
// undo of a lone w; that memory is checked against the word the caller passes, and a word
// that is not the one the engine returned last starts a fresh memory.
#ifndef VNTELEX_H
#define VNTELEX_H

#include <e32def.h>
#include <e32const.h> // ETrue, EFalse

const TInt KVnMaxWord = 32;  // longest word the engine changes, in characters
const TInt KVnMaxRaw = 48;   // keys remembered for one word
const TInt KVnMaxOut = 64;   // size of every aOut buffer given to the engine

// The options of Unikey's dialog that VnKey has (CLAUDE.md "Requirements").
class TVnOptions
    {
public:
    TVnOptions();
public:
    TBool iModernTone;   // hoà, thuý, khoẻ instead of hòa, thúy, khỏe
    TBool iWAlone;       // w with no vowel to change types ư
    TBool iSpellCheck;   // marks only where they make a Vietnamese syllable
    TBool iRestore;      // a word that is not Vietnamese gets its keys back at its end (needs iSpellCheck)
    TBool iMacros;       // typing shortcuts at the end of a word
    };

// Typing shortcuts (gõ tắt): a key word and the text it becomes at the end of the word.
// About 10 KB: a member of a heap object, never on the stack.
class TVnMacros
    {
public:
    enum { KMaxEntries = 64, KMaxKey = 16, KMaxValue = KVnMaxOut - 1 };
    TVnMacros();
    void Reset();
    // EFalse when the table is full, a length is out of range or the key is there already.
    TBool Add(const TText* aKey, TInt aKeyLen, const TText* aValue, TInt aValueLen);
    TInt Count() const { return iCount; }
    // The value for aWord, compared without regard to case, or NULL.
    const TText* Find(const TText* aWord, TInt aLen, TInt& aValueLen) const;
private:
    TText iKey[KMaxEntries][KMaxKey];
    TText iValue[KMaxEntries][KMaxValue];
    TUint8 iKeyLen[KMaxEntries];
    TUint8 iValueLen[KMaxEntries];
    TInt iCount;
    };

class TVnTelex
    {
public:
    TVnTelex();
    void SetOptions(const TVnOptions& aOptions) { iOptions = aOptions; }
    const TVnOptions& Options() const { return iOptions; }

    // aWord (aLen characters) is the word before the cursor and aKey the letter typed after
    // it, as the editor shows them. aOut (KVnMaxOut) receives the text that replaces aWord
    // plus aKey. Returns ETrue when that text differs from aWord plus aKey.
    TBool Key(const TText* aWord, TInt aLen, TText aKey, TText* aOut, TInt& aOutLen);

    // A key that ends a word (space, punctuation, Enter) is about to be typed after aWord.
    // Returns ETrue with the text that replaces aWord in aOut (KVnMaxOut): a typing
    // shortcut, the keys of a word that is not Vietnamese, or thươ made thuở. The memory
    // of the word is cleared either way. aMacros may be NULL.
    TBool Break(const TText* aWord, TInt aLen, const TVnMacros* aMacros, TText* aOut, TInt& aOutLen);

    // Forgets the keys of the current word.
    void Reset();

    // ETrue for a letter the engine knows: a to z, đ and every Vietnamese vowel, in either case.
    static TBool IsWordChar(TText aChar);
    // The letter in the other case, or aChar unchanged when it is not a letter the engine knows.
    static TText ToUpper(TText aChar);
    static TText ToLower(TText aChar);

private:
    void Remember(const TText* aText, TInt aLen);

private:
    TVnOptions iOptions;
    TText iMem[KVnMaxOut];     // the word as the engine returned it last
    TInt iMemLen;
    TText iRaw[KVnMaxRaw];     // the keys typed for it
    TInt iRawLen;
    TBool iRawValid;           // iRaw holds every key of the word
    TInt iBareW;               // index in iMem of an ư typed by a lone w, or -1
    };

#endif // VNTELEX_H
