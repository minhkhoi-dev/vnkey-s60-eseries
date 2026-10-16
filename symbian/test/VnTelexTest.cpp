// VnTelexTest.cpp - the Telex engine's tests, with the host's compiler (no Symbian):
//   bash symbian/test/run.sh
// Each case types keys into a pretend editor the way VnKey does on the phone and compares
// the text that results.
#include <stdio.h>
#include <string.h>
#include "VnTelex.h"

static int gCases = 0;
static int gFailures = 0;

// UTF-8 to UTF-16, the Basic Multilingual Plane only (all of Vietnamese).
static int Utf8To16(const char* aIn, TText* aOut, int aMax)
    {
    int n = 0;
    const unsigned char* p = reinterpret_cast<const unsigned char*>(aIn);
    while (*p && n < aMax)
        {
        unsigned int c = *p++;
        if (c >= 0xE0)
            {
            c = ((c & 0x0F) << 12) | ((p[0] & 0x3F) << 6) | (p[1] & 0x3F);
            p += 2;
            }
        else if (c >= 0xC0)
            {
            c = ((c & 0x1F) << 6) | (p[0] & 0x3F);
            p += 1;
            }
        aOut[n++] = static_cast<TText>(c);
        }
    return n;
    }

static void Utf16To8(const TText* aIn, int aLen, char* aOut)
    {
    char* o = aOut;
    for (int i = 0; i < aLen; ++i)
        {
        unsigned int c = aIn[i];
        if (c < 0x80)
            {
            *o++ = static_cast<char>(c);
            }
        else if (c < 0x800)
            {
            *o++ = static_cast<char>(0xC0 | (c >> 6));
            *o++ = static_cast<char>(0x80 | (c & 0x3F));
            }
        else
            {
            *o++ = static_cast<char>(0xE0 | (c >> 12));
            *o++ = static_cast<char>(0x80 | ((c >> 6) & 0x3F));
            *o++ = static_cast<char>(0x80 | (c & 0x3F));
            }
        }
    *o = 0;
    }

static TBool IsAsciiLetter(char aChar)
    {
    return (aChar >= 'a' && aChar <= 'z') || (aChar >= 'A' && aChar <= 'Z');
    }

// An editor with the cursor at the end of its text. As VnKey does: a letter goes to Key()
// with the word before the cursor, any other key to Break() first; '<' is a backspace,
// which the engine is not told about.
class TPretendEditor
    {
public:
    TPretendEditor(TVnTelex& aEngine, const TVnMacros* aMacros)
        : iEngine(aEngine), iMacros(aMacros), iLen(0)
        {
        }

    void Type(const char* aKeys)
        {
        for (const char* p = aKeys; *p; ++p)
            {
            const char c = *p;
            if (c == '<')
                {
                if (iLen > 0)
                    {
                    --iLen;
                    }
                continue;
                }
            int start = iLen;
            while (start > 0 && TVnTelex::IsWordChar(iText[start - 1]))
                {
                --start;
                }
            const int wordLen = iLen - start;
            TText out[KVnMaxOut];
            TInt outLen = 0;
            if (IsAsciiLetter(c))
                {
                if (wordLen <= KVnMaxWord && iEngine.Key(iText + start, wordLen, c, out, outLen))
                    {
                    Replace(start, out, outLen);
                    }
                else
                    {
                    iText[iLen++] = c;
                    }
                }
            else
                {
                if (wordLen <= KVnMaxWord && iEngine.Break(iText + start, wordLen, iMacros, out, outLen))
                    {
                    Replace(start, out, outLen);
                    }
                else if (wordLen > KVnMaxWord)
                    {
                    iEngine.Reset();
                    }
                iText[iLen++] = c;
                }
            }
        }

    void Text(char* aOut) const
        {
        Utf16To8(iText, iLen, aOut);
        }

private:
    void Replace(int aStart, const TText* aText, int aLen)
        {
        for (int i = 0; i < aLen; ++i)
            {
            iText[aStart + i] = aText[i];
            }
        iLen = aStart + aLen;
        }

private:
    TVnTelex& iEngine;
    const TVnMacros* iMacros;
    TText iText[1024];
    int iLen;
    };

enum TStyle { EDefault, EModern, ENoSpell, ENoWAlone };

static void Check(const char* aKeys, const char* aExpected, TStyle aStyle = EDefault,
                  const TVnMacros* aMacros = 0)
    {
    ++gCases;
    TVnOptions options;
    if (aStyle == EModern)
        {
        options.iModernTone = ETrue;
        }
    else if (aStyle == ENoSpell)
        {
        options.iSpellCheck = EFalse;
        }
    else if (aStyle == ENoWAlone)
        {
        options.iWAlone = EFalse;
        }
    TVnTelex engine;
    engine.SetOptions(options);
    TPretendEditor editor(engine, aMacros);
    editor.Type(aKeys);
    char got[4096];
    editor.Text(got);
    if (strcmp(got, aExpected) != 0)
        {
        ++gFailures;
        printf("FAIL %-24s style %d: got \"%s\", want \"%s\"\n", aKeys, aStyle, got, aExpected);
        }
    }

static void AddMacro(TVnMacros& aMacros, const char* aKey, const char* aValue)
    {
    TText key[64];
    TText value[128];
    const int keyLen = Utf8To16(aKey, key, 64);
    const int valueLen = Utf8To16(aValue, value, 128);
    if (!aMacros.Add(key, keyLen, value, valueLen))
        {
        ++gFailures;
        printf("FAIL macro %s not added\n", aKey);
        }
    }

static void TestLetters()
    {
    // every letter the engine knows goes to upper case and back
    const char* all = "aáàảãạăắằẳẵặâấầẩẫậeéèẻẽẹêếềểễệiíìỉĩịoóòỏõọôốồổỗộơớờởỡợuúùủũụưứừửữựyýỳỷỹỵđ";
    TText text[256];
    const int n = Utf8To16(all, text, 256);
    for (int i = 0; i < n; ++i)
        {
        ++gCases;
        const TText up = TVnTelex::ToUpper(text[i]);
        if (up == text[i] || TVnTelex::ToLower(up) != text[i] || !TVnTelex::IsWordChar(up))
            {
            ++gFailures;
            printf("FAIL case of U+%04X\n", text[i]);
            }
        }
    ++gCases;
    if (TVnTelex::IsWordChar(' ') || TVnTelex::IsWordChar('1') || TVnTelex::IsWordChar(0xEB))
        {
        ++gFailures;
        printf("FAIL IsWordChar of a space, a digit or U+00EB\n");
        }
    }

static void TestTelex()
    {
    // the marks
    Check("aa ", "â ");
    Check("aw ", "ă ");
    Check("ee ", "ê ");
    Check("oo ", "ô ");
    Check("ow ", "ơ ");
    Check("uw ", "ư ");
    Check("dd ", "đ ");
    Check("awn ", "ăn ");
    Check("aan ", "ân ");
    // the tones
    Check("as af ar ax aj ", "á à ả ã ạ ");
    Check("asz", "a");
    Check("cos ", "có ");
    Check("laf ", "là ");
    // whole words
    Check("Vieejt Nam ", "Việt Nam ");
    Check("Tieesng Vieejt ", "Tiếng Việt ");
    Check("tieengs", "tiếng");
    Check("khoong ", "không ");
    Check("ddi ", "đi ");
    Check("Ddi ", "Đi ");
    Check("DDI ", "ĐI ");
    Check("VIEETJ ", "VIỆT ");
    Check("dduowcj ", "được ");
    Check("chuwa ", "chưa ");
    Check("nawm ", "năm ");
    Check("ddaau ", "đâu ");
    Check("ddaay ", "đây ");
    Check("ddoongf ", "đồng ");
    Check("luaan ", "luân ");
    Check("tuaans ", "tuấn ");
    Check("huynhf ", "huỳnh ");
    Check("uyeen ", "uyên ");
    Check("ruouwj ", "rượu ");
    Check("muaw ", "mưa ");
    Check("quawn ", "quăn ");
    Check("quaw ", "quaw "); // quă is no word
    Check("xoawn ", "xoăn ");
    Check("quas ", "quá ");
    Check("quyeets ", "quyết ");
    Check("gias ", "giá ");
    Check("gif ", "gì ");
    Check("giuwax ", "giữa ");
    Check("giw ", "giư ");
    Check("Dduwowngf ", "Đường ");
    // ươ, typed in its three ways, and the w at the end of the word
    Check("nguwowif ", "người ");
    Check("nguoiwf ", "người ");
    Check("nguwoif ", "người ");
    Check("dduowngf ", "đường ");
    Check("duongwf ", "dường ");
    Check("thuowr ", "thuở ");
    Check("huow ", "huơ ");
    // marks and tones typed after the end of the word (free marking)
    Check("toans ", "toán ");
    Check("toasn ", "toán ");
    Check("vietej ", "việt ");
    Check("tienge ", "tiêng ");
    Check("hoafn ", "hoàn ");
    // more everyday words
    Check("ddaauf ", "đầu ");
    Check("Thuwowng ", "Thương ");
    Check("tooi ", "tôi ");
    Check("nhieeuf ", "nhiều ");
    Check("khuyeets ", "khuyết ");
    Check("nghieeng ", "nghiêng ");
    Check("cuwus ", "cứu ");
    Check("ngoaif ", "ngoài ");
    Check("giuwowngf ", "giường ");
    Check("gieengs ", "giếng ");
    Check("quoocs ", "quốc ");
    Check("Hoaf Bifnh ", "Hòa Bình ");
    Check("xooong ", "xoong ");
    Check("TW ", "TƯ ");
    Check("uow ", "uow ");
    Check("is ", "í ");
    // the place of the tone, traditional and modern
    Check("hoaf ", "hòa ");
    Check("hoaf ", "hoà ", EModern);
    Check("thuyr ", "thủy ");
    Check("thuyr ", "thuỷ ", EModern);
    Check("khoer ", "khỏe ");
    Check("khoer ", "khoẻ ", EModern);
    Check("hoangf ", "hoàng ");
    Check("hoangf ", "hoàng ", EModern);
    Check("thuys ", "thúy ");
    // a key twice takes its mark back
    Check("ass ", "as ");
    Check("aaa ", "aa ");
    Check("ooo ", "oo ");
    Check("aww ", "aw ");
    Check("ddd ", "dd ");
    Check("asss ", "ass ");
    // w alone
    Check("w ", "ư ");
    Check("tw ", "tư ");
    Check("tww ", "tw ");
    Check("Tw ", "Tư ");
    Check("w ", "w ", ENoWAlone);
    // English words come back at their end (spell check and restore)
    Check("class ", "class ");
    Check("text ", "text ");
    Check("window ", "window ");
    Check("law ", "law ");
    Check("wow ", "wow ");
    Check("we ", "we ");
    Check("banana ", "banana ");
    Check("email ", "email ");
    Check("facebook ", "facebook ");
    Check("Nokia ", "Nokia ");
    Check("Unikey.", "Unikey.");
    Check("text", "tẽt"); // no end yet, so no restore
    // without the spell check every vowel group takes a mark
    Check("clas ", "clá ", ENoSpell);
    Check("class ", "clas ", ENoSpell);
    Check("text ", "tẽt ", ENoSpell);
    // a backspace: the engine finds its way from what is in the editor
    Check("toans<n ", "toán ");
    Check("toa<as ", "tóa ");
    Check("vieet<<eejt ", "việt ");
    // the cursor after a word typed earlier: the tone still goes on
    Check("toan s", "toan s");
    }

static void TestMacros()
    {
    static TVnMacros macros; // too big for comfort on the stack
    macros.Reset();
    AddMacro(macros, "ko", "không");
    AddMacro(macros, "vn", "Việt Nam");
    AddMacro(macros, "dc", "được");
    Check("ko ", "không ", EDefault, &macros);
    Check("Ko ", "Không ", EDefault, &macros);
    Check("KO ", "KHÔNG ", EDefault, &macros);
    Check("vn.", "Việt Nam.", EDefault, &macros);
    Check("dc ", "được ", EDefault, &macros);
    // docs/TESTING.md, the probe's step 2, line by line
    Check("Tieesng Vieejt cos daaus ", "Tiếng Việt có dấu ", EDefault, &macros);
    Check("nguwowif ddepj ", "người đẹp ", EDefault, &macros);
    Check("Hoaf Bifnh ", "Hòa Bình ", EDefault, &macros);
    Check("dduowngf phoos ", "đường phố ", EDefault, &macros);
    Check("toans ", "toán ", EDefault, &macros);
    Check("ass aaa ddd ", "as aa dd ", EDefault, &macros);
    Check("class text window ", "class text window ", EDefault, &macros);
    Check("ko dc vn ", "không được Việt Nam ", EDefault, &macros);
    Check("hoaf thuyr khoer ", "hoà thuỷ khoẻ ", EModern, &macros);
    Check("kos ", "kos ", EDefault, &macros); // k before o is no Vietnamese
    Check("ko ", "ko ");
    ++gCases;
    TText key[2] = { 'K', 'O' };
    TText value[1] = { 'x' };
    if (macros.Add(key, 2, value, 1))
        {
        ++gFailures;
        printf("FAIL a second ko was added\n");
        }
    }

int main()
    {
    TestLetters();
    TestTelex();
    TestMacros();
    printf("%d cases, %d failures\n", gCases, gFailures);
    return gFailures == 0 ? 0 : 1;
    }
