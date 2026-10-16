// VnTelex.cpp - the Telex engine (VnTelex.h). The rules it follows are in CLAUDE.md under
// "Telex rules"; symbian/test/VnTelexTest.cpp has one case for each of them.
//
// Every table here is plain data with no pointers in it, so that the engine can go into a
// DLL (the FEP plugin) without writable static data.
#include "VnTelex.h"

// ---- letters ---------------------------------------------------------------------------

enum TMark { EMarkNone = 0, EMarkHat, EMarkBreve, EMarkHorn, EMarkStroke };
enum TTone { EToneNone = 0, EToneSac, EToneHuyen, EToneHoi, EToneNga, EToneNang };

// One letter of a word: the base letter a to z, the mark on it (â ă ơ đ), the tone on it
// and its case.
struct TLetter
    {
    TUint8 iBase;
    TUint8 iMark;
    TUint8 iTone;
    TUint8 iUpper;
    };

struct TWord
    {
    TLetter iL[KVnMaxWord + 2];
    TInt iLen;
    };

// The twelve vowels, each with its six tones in lower and upper case, from Unicode's own
// data (generated, see CLAUDE.md "Telex rules"): a ă â e ê i o ô ơ u ư y.
const TInt KVowels = 12;
static const TUint8 KVowelBase[KVowels] = { 'a', 'a', 'a', 'e', 'e', 'i', 'o', 'o', 'o', 'u', 'u', 'y' };
static const TUint8 KVowelMark[KVowels] =
    { EMarkNone, EMarkBreve, EMarkHat, EMarkNone, EMarkHat, EMarkNone,
      EMarkNone, EMarkHat, EMarkHorn, EMarkNone, EMarkHorn, EMarkNone };
static const TUint16 KVowelText[KVowels][2][6] =
    {
    {{0x0061, 0x00E1, 0x00E0, 0x1EA3, 0x00E3, 0x1EA1}, {0x0041, 0x00C1, 0x00C0, 0x1EA2, 0x00C3, 0x1EA0}}, // a
    {{0x0103, 0x1EAF, 0x1EB1, 0x1EB3, 0x1EB5, 0x1EB7}, {0x0102, 0x1EAE, 0x1EB0, 0x1EB2, 0x1EB4, 0x1EB6}}, // ă
    {{0x00E2, 0x1EA5, 0x1EA7, 0x1EA9, 0x1EAB, 0x1EAD}, {0x00C2, 0x1EA4, 0x1EA6, 0x1EA8, 0x1EAA, 0x1EAC}}, // â
    {{0x0065, 0x00E9, 0x00E8, 0x1EBB, 0x1EBD, 0x1EB9}, {0x0045, 0x00C9, 0x00C8, 0x1EBA, 0x1EBC, 0x1EB8}}, // e
    {{0x00EA, 0x1EBF, 0x1EC1, 0x1EC3, 0x1EC5, 0x1EC7}, {0x00CA, 0x1EBE, 0x1EC0, 0x1EC2, 0x1EC4, 0x1EC6}}, // ê
    {{0x0069, 0x00ED, 0x00EC, 0x1EC9, 0x0129, 0x1ECB}, {0x0049, 0x00CD, 0x00CC, 0x1EC8, 0x0128, 0x1ECA}}, // i
    {{0x006F, 0x00F3, 0x00F2, 0x1ECF, 0x00F5, 0x1ECD}, {0x004F, 0x00D3, 0x00D2, 0x1ECE, 0x00D5, 0x1ECC}}, // o
    {{0x00F4, 0x1ED1, 0x1ED3, 0x1ED5, 0x1ED7, 0x1ED9}, {0x00D4, 0x1ED0, 0x1ED2, 0x1ED4, 0x1ED6, 0x1ED8}}, // ô
    {{0x01A1, 0x1EDB, 0x1EDD, 0x1EDF, 0x1EE1, 0x1EE3}, {0x01A0, 0x1EDA, 0x1EDC, 0x1EDE, 0x1EE0, 0x1EE2}}, // ơ
    {{0x0075, 0x00FA, 0x00F9, 0x1EE7, 0x0169, 0x1EE5}, {0x0055, 0x00DA, 0x00D9, 0x1EE6, 0x0168, 0x1EE4}}, // u
    {{0x01B0, 0x1EE9, 0x1EEB, 0x1EED, 0x1EEF, 0x1EF1}, {0x01AF, 0x1EE8, 0x1EEA, 0x1EEC, 0x1EEE, 0x1EF0}}, // ư
    {{0x0079, 0x00FD, 0x1EF3, 0x1EF7, 0x1EF9, 0x1EF5}, {0x0059, 0x00DD, 0x1EF2, 0x1EF6, 0x1EF8, 0x1EF4}}  // y
    };
const TUint16 KSmallDStroke = 0x0111; // đ
const TUint16 KCapitalDStroke = 0x0110; // Đ

static TInt VowelIndex(TUint8 aBase, TUint8 aMark)
    {
    for (TInt v = 0; v < KVowels; ++v)
        {
        if (KVowelBase[v] == aBase && KVowelMark[v] == aMark)
            {
            return v;
            }
        }
    return -1;
    }

static TBool Decompose(TText aChar, TLetter& aLetter)
    {
    aLetter.iMark = EMarkNone;
    aLetter.iTone = EToneNone;
    aLetter.iUpper = 0;
    if (aChar >= 'a' && aChar <= 'z')
        {
        aLetter.iBase = static_cast<TUint8>(aChar);
        return ETrue;
        }
    if (aChar >= 'A' && aChar <= 'Z')
        {
        aLetter.iBase = static_cast<TUint8>(aChar - 'A' + 'a');
        aLetter.iUpper = 1;
        return ETrue;
        }
    if (aChar == KSmallDStroke || aChar == KCapitalDStroke)
        {
        aLetter.iBase = 'd';
        aLetter.iMark = EMarkStroke;
        aLetter.iUpper = aChar == KCapitalDStroke ? 1 : 0;
        return ETrue;
        }
    if (aChar < 0xC0)
        {
        return EFalse;
        }
    for (TInt v = 0; v < KVowels; ++v)
        {
        for (TInt u = 0; u < 2; ++u)
            {
            for (TInt t = 0; t < 6; ++t)
                {
                if (KVowelText[v][u][t] == aChar)
                    {
                    aLetter.iBase = KVowelBase[v];
                    aLetter.iMark = KVowelMark[v];
                    aLetter.iTone = static_cast<TUint8>(t);
                    aLetter.iUpper = static_cast<TUint8>(u);
                    return ETrue;
                    }
                }
            }
        }
    return EFalse;
    }

static TText Compose(const TLetter& aLetter)
    {
    if (aLetter.iBase == 'd' && aLetter.iMark == EMarkStroke)
        {
        return aLetter.iUpper ? KCapitalDStroke : KSmallDStroke;
        }
    const TInt v = VowelIndex(aLetter.iBase, aLetter.iMark);
    if (v >= 0)
        {
        return KVowelText[v][aLetter.iUpper ? 1 : 0][aLetter.iTone];
        }
    return static_cast<TText>(aLetter.iUpper ? aLetter.iBase - 'a' + 'A' : aLetter.iBase);
    }

static TBool IsVowel(const TLetter& aLetter)
    {
    switch (aLetter.iBase)
        {
        case 'a': case 'e': case 'i': case 'o': case 'u': case 'y':
            return ETrue;
        default:
            return EFalse;
        }
    }

static TBool SameText(const TText* aA, const TText* aB, TInt aLen)
    {
    for (TInt i = 0; i < aLen; ++i)
        {
        if (aA[i] != aB[i])
            {
            return EFalse;
            }
        }
    return ETrue;
    }

static TBool ToWord(const TText* aText, TInt aLen, TWord& aWord)
    {
    if (aLen < 0 || aLen > KVnMaxWord)
        {
        return EFalse;
        }
    for (TInt i = 0; i < aLen; ++i)
        {
        if (!Decompose(aText[i], aWord.iL[i]))
            {
            return EFalse;
            }
        }
    aWord.iLen = aLen;
    return ETrue;
    }

static void FromWord(const TWord& aWord, TText* aOut, TInt& aOutLen)
    {
    for (TInt i = 0; i < aWord.iLen; ++i)
        {
        aOut[i] = Compose(aWord.iL[i]);
        }
    aOutLen = aWord.iLen;
    }

// ---- the parts of a syllable -------------------------------------------------------------

// A word is initial consonants, vowels, final consonants: [0, iVStart), [iVStart, iVEnd)
// and [iVEnd, length). iOk is EFalse when a vowel comes after the final consonants. The u
// of qu and the i of gi before another vowel belong to the initial.
struct TParts
    {
    TInt iVStart;
    TInt iVEnd;
    TBool iOk;
    };

static void Parse(const TWord& aWord, TParts& aParts)
    {
    const TInt len = aWord.iLen;
    TInt i = 0;
    while (i < len && !IsVowel(aWord.iL[i]))
        {
        ++i;
        }
    if (i == 1 && len > 1 && aWord.iL[0].iBase == 'q' && aWord.iL[1].iBase == 'u'
        && aWord.iL[1].iMark == EMarkNone)
        {
        i = 2;
        }
    else if (i == 1 && len > 2 && aWord.iL[0].iBase == 'g' && aWord.iL[1].iBase == 'i'
             && IsVowel(aWord.iL[2]))
        {
        i = 2;
        }
    aParts.iVStart = i;
    while (i < len && IsVowel(aWord.iL[i]))
        {
        ++i;
        }
    aParts.iVEnd = i;
    while (i < len && !IsVowel(aWord.iL[i]))
        {
        ++i;
        }
    aParts.iOk = i == len;
    }

// Consonant letters [aFrom, aTo) as ASCII with đ as '#', into aBuf (4 characters at
// most). -1 when they do not fit or one of them carries a mark that no consonant has.
static TInt ConsonantCode(const TWord& aWord, TInt aFrom, TInt aTo, char* aBuf)
    {
    if (aTo - aFrom > 4)
        {
        return -1;
        }
    TInt n = 0;
    for (TInt i = aFrom; i < aTo; ++i)
        {
        const TLetter& l = aWord.iL[i];
        if (l.iBase == 'd' && l.iMark == EMarkStroke)
            {
            aBuf[n++] = '#';
            }
        else if (l.iMark == EMarkNone)
            {
            aBuf[n++] = static_cast<char>(l.iBase);
            }
        else
            {
            return -1;
            }
        }
    aBuf[n] = 0;
    return n;
    }

static TBool CodeIs(const char* aCode, const char* aText)
    {
    while (*aCode && *aCode == *aText)
        {
        ++aCode;
        ++aText;
        }
    return *aCode == 0 && *aText == 0;
    }

// aCode is aText or, with aPrefix, the start of it.
static TBool CodeMatches(const char* aCode, TInt aLen, const char* aText, TBool aPrefix)
    {
    TInt i = 0;
    for (; i < aLen; ++i)
        {
        if (aText[i] == 0 || aText[i] != aCode[i])
            {
            return EFalse;
            }
        }
    return aPrefix || aText[i] == 0;
    }

static const char KInitials[][4] =
    {
    "b", "c", "ch", "d", "#", "g", "gh", "gi", "h", "k", "kh", "l", "m", "n", "ng", "ngh",
    "nh", "p", "ph", "qu", "r", "s", "t", "th", "tr", "v", "x"
    };

static TBool InitialOk(const char* aCode, TInt aLen, TBool aPrefix)
    {
    if (aLen == 0)
        {
        return ETrue;
        }
    for (TUint i = 0; i < sizeof(KInitials) / sizeof(KInitials[0]); ++i)
        {
        if (CodeMatches(aCode, aLen, KInitials[i], aPrefix))
            {
            return ETrue;
            }
        }
    return EFalse;
    }

// The final consonants, one bit each in TRhyme::iFinals.
static const char KFinals[8][3] = { "c", "ch", "m", "n", "ng", "nh", "p", "t" };
enum
    {
    KFc = 0x01, KFch = 0x02, KFm = 0x04, KFn = 0x08, KFng = 0x10, KFnh = 0x20, KFp = 0x40, KFt = 0x80,
    KFAll = 0xFF,
    KFMost = KFc | KFm | KFn | KFng | KFp | KFt
    };

static TBool FinalOk(const char* aCode, TInt aLen, TUint aMask, TBool aPrefix)
    {
    for (TInt i = 0; i < 8; ++i)
        {
        if ((aMask & (1u << i)) && CodeMatches(aCode, aLen, KFinals[i], aPrefix))
            {
            return ETrue;
            }
        }
    return EFalse;
    }

// The vowel groups of Vietnamese syllables, each vowel a letter with its mark after it:
// ( for ă, ^ for â ê ô, + for ơ ư. iOpen: the group can end a syllable; iFinals: the final
// consonants it takes.
struct TRhyme
    {
    char iVowels[8];
    TUint8 iOpen;
    TUint8 iFinals;
    };

static const TRhyme KRhymes[] =
    {
    { "a", 1, KFAll },           { "a(", 0, KFMost },          { "a^", 0, KFMost },
    { "e", 1, KFMost },          { "e^", 1, KFch | KFm | KFn | KFng | KFnh | KFp | KFt },
    { "i", 1, KFch | KFm | KFn | KFnh | KFp | KFt },
    { "o", 1, KFMost },          { "o^", 1, KFMost },          { "o+", 1, KFm | KFn | KFp | KFt },
    { "u", 1, KFMost },          { "u+", 1, KFc | KFm | KFn | KFng | KFt },
    { "y", 1, 0 },
    { "ai", 1, 0 },  { "ao", 1, 0 },  { "au", 1, 0 },  { "ay", 1, 0 },  { "a^u", 1, 0 }, { "a^y", 1, 0 },
    { "eo", 1, 0 },  { "e^u", 1, 0 }, { "ia", 1, 0 },  { "ie^", 0, KFMost },          { "iu", 1, 0 },
    { "oa", 1, KFAll },          { "oa(", 0, KFc | KFm | KFn | KFng | KFt },
    { "oe", 1, KFn | KFt },      { "oi", 1, 0 },  { "o^i", 1, 0 }, { "o+i", 1, 0 },
    { "oo", 0, KFc | KFng },     { "ua", 1, 0 },  { "ua^", 0, KFn | KFng | KFt },
    { "ue^", 1, KFch | KFn | KFnh | KFt },        { "ui", 1, 0 },
    { "uo^", 0, KFc | KFm | KFn | KFng | KFt },   { "uo+", 1, 0 },
    { "uy", 1, KFch | KFn | KFnh | KFp | KFt },
    { "u+a", 1, 0 }, { "u+i", 1, 0 }, { "u+o+", 0, KFMost },  { "u+u", 1, 0 },
    { "ye^", 0, KFm | KFn | KFng | KFt },
    { "ie^u", 1, 0 }, { "ye^u", 1, 0 }, { "oai", 1, 0 }, { "oay", 1, 0 }, { "oeo", 1, 0 },
    { "ua^y", 1, 0 }, { "uo^i", 1, 0 }, { "u+o+i", 1, 0 }, { "u+o+u", 1, 0 }, { "uya", 1, 0 },
    { "uye^", 0, KFn | KFt },    { "uyu", 1, 0 }
    };

// Does the vowel group [aFrom, aTo) of aWord match aRhyme? aExactLength: as many vowels as
// the rhyme has (else the start of it); aExactMarks: the same marks (else a vowel with no
// mark also matches one with a mark, which can still be typed).
static TBool RhymeMatches(const TWord& aWord, TInt aFrom, TInt aTo, const TRhyme& aRhyme,
                          TBool aExactLength, TBool aExactMarks)
    {
    const TInt n = aTo - aFrom;
    TInt k = 0;
    const char* p = aRhyme.iVowels;
    while (*p)
        {
        const TUint8 base = static_cast<TUint8>(*p++);
        TUint8 mark = EMarkNone;
        if (*p == '(')
            {
            mark = EMarkBreve;
            ++p;
            }
        else if (*p == '^')
            {
            mark = EMarkHat;
            ++p;
            }
        else if (*p == '+')
            {
            mark = EMarkHorn;
            ++p;
            }
        if (k < n)
            {
            const TLetter& l = aWord.iL[aFrom + k];
            if (l.iBase != base)
                {
                return EFalse;
                }
            if (l.iMark != mark && (aExactMarks || l.iMark != EMarkNone))
                {
                return EFalse;
                }
            }
        ++k;
        }
    if (k < n)
        {
        return EFalse;
        }
    return !aExactLength || k == n;
    }

static TInt CurrentTone(const TWord& aWord)
    {
    TInt tone = EToneNone;
    for (TInt i = 0; i < aWord.iLen; ++i)
        {
        if (aWord.iL[i].iTone != EToneNone)
            {
            tone = aWord.iL[i].iTone;
            }
        }
    return tone;
    }

static void ClearTone(TWord& aWord)
    {
    for (TInt i = 0; i < aWord.iLen; ++i)
        {
        aWord.iL[i].iTone = EToneNone;
        }
    }

static TBool HasVowel(const TWord& aWord)
    {
    for (TInt i = 0; i < aWord.iLen; ++i)
        {
        if (IsVowel(aWord.iL[i]))
            {
            return ETrue;
            }
        }
    return EFalse;
    }

// Can aWord be Vietnamese? aComplete: the word ends here; otherwise it is being typed and
// may still get letters and marks. Without aSpell only the vowel group is checked, against
// the table of groups, as Unikey does with its spell check off.
static TBool Valid(const TWord& aWord, TBool aComplete, TBool aSpell)
    {
    TParts p;
    Parse(aWord, p);
    if (!p.iOk)
        {
        return EFalse;
        }
    const TInt vowels = p.iVEnd - p.iVStart;
    char ini[5];
    const TInt iniLen = ConsonantCode(aWord, 0, p.iVStart, ini);
    if (vowels == 0)
        {
        if (!aSpell)
            {
            return ETrue;
            }
        return !aComplete && iniLen >= 0 && InitialOk(ini, iniLen, ETrue);
        }
    if (aSpell && (iniLen < 0 || !InitialOk(ini, iniLen, EFalse)))
        {
        return EFalse;
        }
    char fin[5];
    const TInt finLen = ConsonantCode(aWord, p.iVEnd, aWord.iLen, fin);
    if (finLen < 0)
        {
        return EFalse;
        }
    const TBool hasFinal = finLen > 0;
    TBool found = EFalse;
    for (TUint r = 0; r < sizeof(KRhymes) / sizeof(KRhymes[0]) && !found; ++r)
        {
        const TRhyme& rhyme = KRhymes[r];
        if (!RhymeMatches(aWord, p.iVStart, p.iVEnd, rhyme, hasFinal || aComplete, aComplete))
            {
            continue;
            }
        if (aSpell)
            {
            if (hasFinal)
                {
                if (!FinalOk(fin, finLen, rhyme.iFinals, !aComplete))
                    {
                    continue;
                    }
                }
            else if (aComplete && !rhyme.iOpen && !(iniLen == 0 && vowels == 1))
                {
                continue; // ă and â on their own are letters people do type
                }
            }
        found = ETrue;
        }
    if (!found)
        {
        return EFalse;
        }
    if (aSpell)
        {
        const TUint8 first = aWord.iL[p.iVStart].iBase;
        const TBool front = first == 'e' || first == 'i' || first == 'y';
        if (CodeIs(ini, "c") && front)
            {
            return EFalse; // k before e, i, y
            }
        if (CodeIs(ini, "k") && !front)
            {
            return EFalse;
            }
        if ((CodeIs(ini, "gh") || CodeIs(ini, "ngh")) && first != 'e' && first != 'i')
            {
            return EFalse;
            }
        if (CodeIs(ini, "g") && first == 'e')
            {
            return EFalse; // gh before e
            }
        if (CodeIs(ini, "ng") && (first == 'e' || first == 'i'))
            {
            return EFalse; // ngh before e, i
            }
        if (CodeIs(ini, "qu") && first == 'u')
            {
            return EFalse;
            }
        // c, ch, p, t end a syllable with sắc or nặng only (or none while typing)
        const TInt tone = CurrentTone(aWord);
        if (hasFinal && (fin[0] == 'c' || fin[0] == 'p' || fin[0] == 't')
            && (tone == EToneHuyen || tone == EToneHoi || tone == EToneNga))
            {
            return EFalse;
            }
        }
    return ETrue;
    }

// Where the tone goes in the vowel group: on the vowel with a mark (the last one of ươ),
// on the second of two vowels before a final consonant, on the middle one of three, and
// otherwise on the first of two, except oa, oe, uy in the modern style. -1 for no vowel.
static TInt TonePos(const TWord& aWord, const TParts& aParts, TBool aModern)
    {
    const TInt vs = aParts.iVStart;
    const TInt n = aParts.iVEnd - vs;
    if (n <= 0)
        {
        return -1;
        }
    if (n == 1)
        {
        return vs;
        }
    TInt marked = -1;
    for (TInt i = vs; i < aParts.iVEnd; ++i)
        {
        if (aWord.iL[i].iMark != EMarkNone)
            {
            marked = i;
            }
        }
    if (marked >= 0)
        {
        return marked;
        }
    if (n >= 3 || aParts.iVEnd < aWord.iLen)
        {
        return vs + 1;
        }
    const TUint8 a = aWord.iL[vs].iBase;
    const TUint8 b = aWord.iL[vs + 1].iBase;
    if ((a == 'o' && (b == 'a' || b == 'e')) || (a == 'u' && b == 'y'))
        {
        return aModern ? vs + 1 : vs;
        }
    return vs;
    }

// Moves the tone to where it belongs (TonePos), as the word grows.
static void PlaceTone(TWord& aWord, TBool aModern)
    {
    TInt tone = EToneNone;
    TInt at = -1;
    for (TInt i = 0; i < aWord.iLen; ++i)
        {
        if (aWord.iL[i].iTone != EToneNone)
            {
            tone = aWord.iL[i].iTone;
            at = i;
            aWord.iL[i].iTone = EToneNone;
            }
        }
    if (tone == EToneNone)
        {
        return;
        }
    TParts p;
    Parse(aWord, p);
    TInt pos = p.iOk ? TonePos(aWord, p, aModern) : -1;
    if (pos < 0)
        {
        pos = at;
        }
    aWord.iL[pos].iTone = static_cast<TUint8>(tone);
    }

// ưo and uơ followed by another letter become ươ (tưong -> tương, ngưoi -> ngươi).
static void Normalize(TWord& aWord)
    {
    TParts p;
    Parse(aWord, p);
    if (!p.iOk)
        {
        return;
        }
    for (TInt i = p.iVStart; i + 1 < p.iVEnd; ++i)
        {
        TLetter& u = aWord.iL[i];
        TLetter& o = aWord.iL[i + 1];
        if (u.iBase != 'u' || o.iBase != 'o' || i + 2 >= aWord.iLen)
            {
            continue;
            }
        if ((u.iMark == EMarkHorn && o.iMark == EMarkNone) || (u.iMark == EMarkNone && o.iMark == EMarkHorn))
            {
            u.iMark = EMarkHorn;
            o.iMark = EMarkHorn;
            }
        }
    }

// ---- one key -----------------------------------------------------------------------------

enum TKind
    {
    ELiteral,       // the key is typed as it is
    ETransform,     // the key changed the word
    EUndo,          // the key took its own change back and is typed as it is (ass -> as)
    EBareW          // a lone w typed ư
    };

static void AppendLiteral(TWord& aWord, const TLetter& aKey, const TVnOptions& aOptions)
    {
    aWord.iL[aWord.iLen++] = aKey;
    if (Valid(aWord, EFalse, aOptions.iSpellCheck))
        {
        Normalize(aWord);
        PlaceTone(aWord, aOptions.iModernTone);
        }
    }

// With the spell check on, a word that cannot be Vietnamese takes every key as it is.
static TBool Closed(const TWord& aWord, const TVnOptions& aOptions)
    {
    return aOptions.iSpellCheck && !Valid(aWord, EFalse, ETrue);
    }

static TInt ToneOfKey(TUint8 aKey)
    {
    switch (aKey)
        {
        case 's': return EToneSac;
        case 'f': return EToneHuyen;
        case 'r': return EToneHoi;
        case 'x': return EToneNga;
        case 'j': return EToneNang;
        default:  return EToneNone;
        }
    }

static TKind ApplyTone(TWord& aWord, const TLetter& aKey, const TVnOptions& aOptions)
    {
    const TInt tone = ToneOfKey(aKey.iBase);
    TParts p;
    Parse(aWord, p);
    if (!p.iOk || p.iVEnd == p.iVStart || Closed(aWord, aOptions))
        {
        AppendLiteral(aWord, aKey, aOptions);
        return ELiteral;
        }
    if (CurrentTone(aWord) == tone)
        {
        ClearTone(aWord);
        AppendLiteral(aWord, aKey, aOptions);
        return EUndo;
        }
    TWord toned = aWord;
    ClearTone(toned);
    toned.iL[TonePos(toned, p, aOptions.iModernTone)].iTone = static_cast<TUint8>(tone);
    if (!Valid(toned, EFalse, aOptions.iSpellCheck))
        {
        AppendLiteral(aWord, aKey, aOptions);
        return ELiteral;
        }
    aWord = toned;
    return ETransform;
    }

static TKind RemoveTone(TWord& aWord, const TLetter& aKey, const TVnOptions& aOptions)
    {
    TParts p;
    Parse(aWord, p);
    if (p.iOk && CurrentTone(aWord) != EToneNone && !Closed(aWord, aOptions))
        {
        ClearTone(aWord);
        return ETransform;
        }
    AppendLiteral(aWord, aKey, aOptions);
    return ELiteral;
    }

// aa â, ee ê, oo ô: the last vowel of the group with the key's letter gets the hat.
static TKind ApplyHat(TWord& aWord, const TLetter& aKey, const TVnOptions& aOptions)
    {
    TParts p;
    Parse(aWord, p);
    TInt at = -1;
    if (p.iOk && !Closed(aWord, aOptions))
        {
        for (TInt i = p.iVEnd - 1; i >= p.iVStart; --i)
            {
            if (aWord.iL[i].iBase == aKey.iBase)
                {
                at = i;
                break;
                }
            }
        }
    if (at < 0)
        {
        AppendLiteral(aWord, aKey, aOptions);
        return ELiteral;
        }
    if (aWord.iL[at].iMark == EMarkHat)
        {
        aWord.iL[at].iMark = EMarkNone;
        AppendLiteral(aWord, aKey, aOptions);
        return EUndo;
        }
    TWord hatted = aWord;
    hatted.iL[at].iMark = EMarkHat; // also ă -> â, ơ -> ô
    if (!Valid(hatted, EFalse, aOptions.iSpellCheck))
        {
        AppendLiteral(aWord, aKey, aOptions);
        return ELiteral;
        }
    PlaceTone(hatted, aOptions.iModernTone);
    aWord = hatted;
    return ETransform;
    }

// A lone w types ư when that can be a syllable (tw -> tư, giw -> giư).
static TBool TryBareW(TWord& aWord, const TLetter& aKey, const TVnOptions& aOptions, TInt& aBareW)
    {
    if (!aOptions.iWAlone || aWord.iLen >= KVnMaxWord + 1)
        {
        return EFalse;
        }
    TWord withU = aWord;
    TLetter& u = withU.iL[withU.iLen++];
    u.iBase = 'u';
    u.iMark = EMarkHorn;
    u.iTone = EToneNone;
    u.iUpper = aKey.iUpper;
    if (!Valid(withU, EFalse, aOptions.iSpellCheck))
        {
        return EFalse;
        }
    PlaceTone(withU, aOptions.iModernTone);
    aBareW = withU.iLen - 1;
    aWord = withU;
    return ETrue;
    }

// aw ă, ow ơ, uw ư, uow ươ.
static TKind ApplyW(TWord& aWord, const TLetter& aKey, const TVnOptions& aOptions, TInt& aBareW)
    {
    TParts p;
    Parse(aWord, p);
    if (!p.iOk || Closed(aWord, aOptions))
        {
        AppendLiteral(aWord, aKey, aOptions);
        return ELiteral;
        }
    const TInt vs = p.iVStart;
    const TInt ve = p.iVEnd;
    if (vs == ve)
        {
        if (TryBareW(aWord, aKey, aOptions, aBareW))
            {
            return EBareW;
            }
        AppendLiteral(aWord, aKey, aOptions);
        return ELiteral;
        }
    for (TInt i = vs; i + 1 < ve; ++i)
        {
        if (aWord.iL[i].iBase != 'u' || aWord.iL[i + 1].iBase != 'o')
            {
            continue;
            }
        if (aWord.iL[i].iMark == EMarkHorn && aWord.iL[i + 1].iMark == EMarkHorn)
            {
            aWord.iL[i].iMark = EMarkNone;
            aWord.iL[i + 1].iMark = EMarkNone;
            AppendLiteral(aWord, aKey, aOptions);
            return EUndo;
            }
        TWord horned = aWord;
        horned.iL[i].iMark = EMarkHorn;
        horned.iL[i + 1].iMark = EMarkHorn;
        if (Valid(horned, EFalse, aOptions.iSpellCheck))
            {
            PlaceTone(horned, aOptions.iModernTone);
            aWord = horned;
            return ETransform;
            }
        AppendLiteral(aWord, aKey, aOptions);
        return ELiteral;
        }
    for (TInt i = vs; i < ve; ++i)
        {
        const TLetter& l = aWord.iL[i];
        if ((l.iBase == 'a' && l.iMark == EMarkBreve)
            || ((l.iBase == 'o' || l.iBase == 'u') && l.iMark == EMarkHorn))
            {
            aWord.iL[i].iMark = EMarkNone;
            AppendLiteral(aWord, aKey, aOptions);
            return EUndo;
            }
        }
    for (TInt i = vs; i < ve; ++i)
        {
        const TUint8 base = aWord.iL[i].iBase;
        if (base != 'a' && base != 'o' && base != 'u')
            {
            continue;
            }
        TWord marked = aWord;
        marked.iL[i].iMark = base == 'a' ? EMarkBreve : EMarkHorn;
        Normalize(marked);
        if (Valid(marked, EFalse, aOptions.iSpellCheck))
            {
            PlaceTone(marked, aOptions.iModernTone);
            aWord = marked;
            return ETransform;
            }
        }
    if (TryBareW(aWord, aKey, aOptions, aBareW))
        {
        return EBareW;
        }
    AppendLiteral(aWord, aKey, aOptions);
    return ELiteral;
    }

// dd đ, right after the d only.
static TKind ApplyD(TWord& aWord, const TLetter& aKey, const TVnOptions& aOptions)
    {
    if (aWord.iLen == 1 && aWord.iL[0].iBase == 'd')
        {
        if (aWord.iL[0].iMark == EMarkNone)
            {
            aWord.iL[0].iMark = EMarkStroke;
            return ETransform;
            }
        if (aWord.iL[0].iMark == EMarkStroke)
            {
            aWord.iL[0].iMark = EMarkNone;
            AppendLiteral(aWord, aKey, aOptions);
            return EUndo;
            }
        }
    AppendLiteral(aWord, aKey, aOptions);
    return ELiteral;
    }

// ---- TVnOptions, TVnMacros -------------------------------------------------------------

TVnOptions::TVnOptions()
    : iModernTone(EFalse), iWAlone(ETrue), iSpellCheck(ETrue), iRestore(ETrue), iMacros(ETrue)
    {
    }

TVnMacros::TVnMacros()
    : iCount(0)
    {
    }

void TVnMacros::Reset()
    {
    iCount = 0;
    }

static TBool SameNoCase(const TText* aA, TInt aALen, const TText* aB, TInt aBLen)
    {
    if (aALen != aBLen)
        {
        return EFalse;
        }
    for (TInt i = 0; i < aALen; ++i)
        {
        if (TVnTelex::ToLower(aA[i]) != TVnTelex::ToLower(aB[i]))
            {
            return EFalse;
            }
        }
    return ETrue;
    }

TBool TVnMacros::Add(const TText* aKey, TInt aKeyLen, const TText* aValue, TInt aValueLen)
    {
    if (iCount >= KMaxEntries || aKeyLen < 1 || aKeyLen > KMaxKey || aValueLen < 1
        || aValueLen > KMaxValue)
        {
        return EFalse;
        }
    TInt len = 0;
    if (Find(aKey, aKeyLen, len))
        {
        return EFalse;
        }
    for (TInt i = 0; i < aKeyLen; ++i)
        {
        iKey[iCount][i] = aKey[i];
        }
    for (TInt i = 0; i < aValueLen; ++i)
        {
        iValue[iCount][i] = aValue[i];
        }
    iKeyLen[iCount] = static_cast<TUint8>(aKeyLen);
    iValueLen[iCount] = static_cast<TUint8>(aValueLen);
    ++iCount;
    return ETrue;
    }

const TText* TVnMacros::Find(const TText* aWord, TInt aLen, TInt& aValueLen) const
    {
    for (TInt i = 0; i < iCount; ++i)
        {
        if (SameNoCase(iKey[i], iKeyLen[i], aWord, aLen))
            {
            aValueLen = iValueLen[i];
            return iValue[i];
            }
        }
    return 0;
    }

// ---- TVnTelex ------------------------------------------------------------------------------

TVnTelex::TVnTelex()
    {
    Reset();
    }

void TVnTelex::Reset()
    {
    iMemLen = 0;
    iRawLen = 0;
    iRawValid = ETrue;
    iBareW = -1;
    }

void TVnTelex::Remember(const TText* aText, TInt aLen)
    {
    for (TInt i = 0; i < aLen; ++i)
        {
        iMem[i] = aText[i];
        }
    iMemLen = aLen;
    }

TBool TVnTelex::IsWordChar(TText aChar)
    {
    TLetter l;
    return Decompose(aChar, l);
    }

TText TVnTelex::ToUpper(TText aChar)
    {
    TLetter l;
    if (!Decompose(aChar, l))
        {
        return aChar;
        }
    l.iUpper = 1;
    return Compose(l);
    }

TText TVnTelex::ToLower(TText aChar)
    {
    TLetter l;
    if (!Decompose(aChar, l))
        {
        return aChar;
        }
    l.iUpper = 0;
    return Compose(l);
    }

TBool TVnTelex::Key(const TText* aWord, TInt aLen, TText aKey, TText* aOut, TInt& aOutLen)
    {
    const TBool synced = aLen == iMemLen && SameText(aWord, iMem, aLen);
    if (!synced)
        {
        iRawLen = 0;
        iRawValid = aLen == 0;
        iBareW = -1;
        }
    TWord word;
    TLetter key;
    if (aLen + 1 > KVnMaxWord || !ToWord(aWord, aLen, word) || !Decompose(aKey, key)
        || key.iMark != EMarkNone || key.iTone != EToneNone)
        {
        Reset(); // a word the engine does not handle, or a key that is not a to z
        return EFalse;
        }

    const TBool bareWLast = synced && iBareW >= 0 && iBareW == aLen - 1;
    TInt bareW = -1;
    TKind kind = ELiteral;
    if (key.iBase == 'w' && bareWLast && word.iL[aLen - 1].iBase == 'u'
        && word.iL[aLen - 1].iMark == EMarkHorn)
        {
        word.iL[aLen - 1] = key; // ư from a lone w, then w again: w
        kind = EUndo;
        }
    else
        {
        switch (key.iBase)
            {
            case 's': case 'f': case 'r': case 'x': case 'j':
                kind = ApplyTone(word, key, iOptions);
                break;
            case 'z':
                kind = RemoveTone(word, key, iOptions);
                break;
            case 'a': case 'e': case 'o':
                kind = ApplyHat(word, key, iOptions);
                break;
            case 'w':
                kind = ApplyW(word, key, iOptions, bareW);
                break;
            case 'd':
                kind = ApplyD(word, key, iOptions);
                break;
            default:
                AppendLiteral(word, key, iOptions);
                break;
            }
        }

    FromWord(word, aOut, aOutLen);
    if (kind == EUndo && aOutLen <= KVnMaxRaw)
        {
        // what is shown now is what was meant: ass -> as is restored as as
        for (TInt i = 0; i < aOutLen; ++i)
            {
            iRaw[i] = aOut[i];
            }
        iRawLen = aOutLen;
        iRawValid = ETrue;
        }
    else if (iRawValid && iRawLen < KVnMaxRaw)
        {
        iRaw[iRawLen++] = aKey;
        }
    else
        {
        iRawValid = EFalse;
        }
    iBareW = bareW;
    Remember(aOut, aOutLen);

    if (aOutLen != aLen + 1)
        {
        return ETrue;
        }
    return !(SameText(aOut, aWord, aLen) && aOut[aLen] == aKey);
    }

// thươ and huươ at the end of a word are thuở and huơ: ươ needs a letter after it.
static TBool FixOpenUo(TWord& aWord)
    {
    TParts p;
    Parse(aWord, p);
    if (!p.iOk || p.iVStart == 0 || p.iVEnd != aWord.iLen || p.iVEnd - p.iVStart != 2)
        {
        return EFalse;
        }
    TLetter& u = aWord.iL[p.iVStart];
    const TLetter& o = aWord.iL[p.iVStart + 1];
    if (u.iBase == 'u' && u.iMark == EMarkHorn && o.iBase == 'o' && o.iMark == EMarkHorn)
        {
        u.iMark = EMarkNone;
        return ETrue;
        }
    return EFalse;
    }

static TBool IsUpper(TText aChar)
    {
    return TVnTelex::ToLower(aChar) != aChar;
    }

// A shortcut takes the case of what was typed: ko -> không, Ko -> Không, KO -> KHÔNG.
static void CopyInCase(const TText* aValue, TInt aValueLen, const TText* aWord, TInt aWordLen,
                       TText* aOut, TInt& aOutLen)
    {
    TBool allUpper = aWordLen > 1;
    for (TInt i = 0; i < aWordLen; ++i)
        {
        if (!IsUpper(aWord[i]))
            {
            allUpper = EFalse;
            }
        }
    const TBool firstUpper = aWordLen > 0 && IsUpper(aWord[0]);
    for (TInt i = 0; i < aValueLen; ++i)
        {
        TText c = aValue[i];
        if (allUpper || (i == 0 && firstUpper))
            {
            c = TVnTelex::ToUpper(c);
            }
        aOut[i] = c;
        }
    aOutLen = aValueLen;
    }

TBool TVnTelex::Break(const TText* aWord, TInt aLen, const TVnMacros* aMacros, TText* aOut,
                      TInt& aOutLen)
    {
    const TBool synced = aLen == iMemLen && SameText(aWord, iMem, aLen);
    const TBool rawKnown = synced && iRawValid && iRawLen > 0;
    TBool replaced = EFalse;
    TWord word;
    if (aLen > 0 && ToWord(aWord, aLen, word))
        {
        const TBool fixed = FixOpenUo(word);
        const TText* value = 0;
        TInt valueLen = 0;
        if (iOptions.iMacros && aMacros)
            {
            value = aMacros->Find(aWord, aLen, valueLen);
            if (!value && rawKnown)
                {
                value = aMacros->Find(iRaw, iRawLen, valueLen);
                }
            }
        if (value)
            {
            CopyInCase(value, valueLen, aWord, aLen, aOut, aOutLen);
            replaced = ETrue;
            }
        else if (iOptions.iRestore && iOptions.iSpellCheck && rawKnown && HasVowel(word)
                 && !(iRawLen == aLen && SameText(iRaw, aWord, aLen))
                 && !Valid(word, ETrue, ETrue))
            {
            for (TInt i = 0; i < iRawLen; ++i)
                {
                aOut[i] = iRaw[i];
                }
            aOutLen = iRawLen;
            replaced = ETrue;
            }
        else if (fixed)
            {
            FromWord(word, aOut, aOutLen);
            replaced = ETrue;
            }
        }
    Reset();
    return replaced;
    }
