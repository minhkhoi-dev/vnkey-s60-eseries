# VnKey — Vietnamese Telex input for the Nokia E72 (S60 3rd Edition FP2)

Telex typing like Unikey on Windows, for Khoi's Nokia E72: S60 3rd Edition Feature
Pack 2, Symbian OS 9.3, a QWERTY keyboard, a 320x240 screen. The phone is hacked
(InstallServer and Open4All patches), so a self-signed package may carry any
capability. Khoi runs every test on the phone; text is hard to copy off it, so the app
writes a log to the memory card that Khoi copies over USB (`docs/TESTING.md`). The
model for building S60v3 apps is Khoi's OttPilot (`/Users/khoi/Projects/ottpilot`,
its `symbian/CLAUDE.md`); this project copies its build setup and its Symbian rules.

Where it stands (2026-10-16): app 0.0.1 is the probe of roadmap 1, built, not yet run
on the phone. The Telex engine passes its tests on the Mac.

## Requirements

Decided by Khoi on 2026-10-16:

- Telex works in every application, as Unikey does (not only in VnKey's own editor).
- Only Khoi's hacked E72 has to run it; stock phones do not matter.
- The phone's font shows every Vietnamese letter: VnKey types precomposed Unicode (NFC).
- Tone placement can be chosen in the settings: traditional (hòa, thúy, khỏe) or modern
  (hoà, thuý, khoẻ).
- Ctrl+Space switches between Vietnamese and English.
- A [V] / [E] mark in the status bar, like the memory card, clock and WLAN icons.
- Besides plain Telex: w alone types ư; a spell check that restores the keys of words
  that are not Vietnamese (class, email, window); typing shortcuts (gõ tắt, ko → không).
  VNI is not wanted.
- Done means: Messaging (SMS) and OttPilot's chat editor take Telex.

## Design

Two ways were weighed for typing in other applications:

- A background process that captures keys (`RWindowGroup::CaptureKey`) and sends
  backspaces and the Vietnamese letter to the focused application, as Unikey does with
  its keyboard hook. Rejected as the main way: it cannot see the editor, so it gets the
  case wrong where the phone's FEP capitalises by itself (the start of a sentence, a
  sticky Shift) and loses track at every cursor move.
- In each application's own process, next to the phone's FEP. Chosen. The phone's FEP
  is AknFep, an ECOM plugin (`CCoeFepPlugIn`, interface `0x1020233F`) that CONE loads
  into every GUI application. VnKey's plugin is to be installed as the system's FEP in
  its place, load AknFep itself and return AknFep's FEP object, so that everything
  AknFep does goes on unchanged, and add `CVnCatcher` to the application.

`CVnCatcher` (`symbian/src/VnCatcher.cpp`) is the part that does Telex in an
application. It is a control on the control stack at `ECoeStackPriorityFep + 1`, just
above AknFep, so it sees every key first, and it never takes a letter away from it:

1. At a key's down event it notes the focused editor's cursor, length and the word
   before the cursor (`MCoeFepAwareTextEditor`, from `CCoeAppUi::InputCapabilities()`).
2. AknFep types the letter, with its case, Fn layer and the rest.
3. After AknFep has handled the release of the key (an async callback queued at the up
   event, or the next key's down event, whichever comes first), the catcher checks that
   the editor holds exactly the old word plus one letter that matches the key, gives
   both to the engine and rewrites the word in place. A long press (repeats) is
   AknFep's: no rewrite.
4. A key that ends a word (space, digit, punctuation, Enter) gets the end-of-word
   changes first: shortcuts, the restore of non-Vietnamese words, thươ → thuở.
5. Ctrl+Space is the one key it consumes.

Password editors are left alone. How a word is rewritten is the probe's question; there
are three methods (menu "Rewrite method"): 1 selects the old text and replaces it with
an inline edit committed at once; 2 deletes with backspaces through the FEP, then types
the new text as an inline edit; 3 sends backspaces and the characters through the FEP
(`CCoeFep::SimulateKeyEventsL`, protected, reached through `CVnFepAccess`).

The engine (`symbian/src/VnTelex.cpp`, `TVnTelex`) is pure logic with the basic Symbian
types only, built for the phone and for the Mac's compiler. It is stateless about the
editor: the catcher passes the word before the cursor at every key, so a moved cursor
costs nothing and a tone can be added to a word typed earlier. It remembers only the
keys of the current word (for the restore and the undo of a lone w), and checks that
memory against the word it is given.

The [V] / [E] mark: an application cannot add an icon to the system's universal
indicator pane (`CAknIndicatorContainer::CreateIndicatorFromResourceL` works only in an
application's own status pane), so `CVnIndicator` is a small window of the application,
in front of its other windows, placed over the status pane. In the probe it is in
VnKey's own window group; the plugin will show it in each application.

## Telex rules

What `TVnTelex` does; `symbian/test/VnTelexTest.cpp` has a case for each.

- Marks: aa â, aw ă, ee ê, oo ô, ow ơ, uw ư; ươ from uow, uwow or a w at the end of the
  word (nguoiw); dd đ (right after the d only). Tones: s sắc, f huyền, r hỏi, x ngã,
  j nặng; z removes the tone.
- Marks and tones may come after the rest of the word (toans, vietej, nguoiwf), and
  the tone moves as the word grows (hòa + n → hoàn).
- A key pressed twice takes its change back and types itself: ass → as, aaa → aa,
  ddd → dd, ww after a lone w → w.
- w alone (option): ư where it can be a syllable (w → ư, tw → tư, giw → giư).
- Tone placement: on the vowel with a mark (ơ of ươ); with a final consonant on the
  second of two vowels; the middle of three; otherwise the first of two, except oa, oe,
  uy, which take it on the second in the modern style.
- Spell check (option): a mark is made only where the word can still be Vietnamese
  (initial consonant, vowel group from a table of 57, final consonant allowed for that
  group, c/k, g/gh, ng/ngh spelling, and c ch p t only with sắc or nặng). A word that
  cannot be Vietnamese takes every key as it is. Without it only the vowel group is
  checked, as in Unikey.
- Restore (option, needs the spell check): at the end of a word that is not Vietnamese
  and that VnKey changed, the keys as typed come back (text: tẽt → text). Not for a word
  with no vowel (đ, đt), nor after a backspace in the word (its keys are not known).
- ươ with nothing after it at the end of a word that has an initial is uơ (thuở, huơ).
- Shortcuts (option): at the end of a word that is a key in the table (no regard to
  case), the text replaces it, in the case typed: ko → không, Ko → Không, KO → KHÔNG.
  The probe reads them from `macros.txt` next to the log (UTF-8, `key text` per line,
  `#` comments) and writes a sample there when there is none.
- The letter table (12 vowels × 6 tones × 2 cases) was generated from Unicode's data
  with Python (`unicodedata.normalize("NFC", ...)`), not typed by hand.

## Hard constraints for Symbian code

As in OttPilot (`symbian/CLAUDE.md` there):

- Compiler is GCCE 3.4.3: C++98 only, no STL, no exceptions, no `auto`/`nullptr`.
  Descriptors, `RArray`, leave/trap, two-phase construction, active objects.
  Class prefixes `C T R M`, members `iFoo`, arguments `aFoo`, constants `KFoo`.
- No non-ASCII characters in string literals (the compiler's source character set is
  not to be trusted): write `\x01B0` in a `_LIT`, and UTF-8 bytes as `\xC6\xB0` in a
  `_LIT8`, splitting the literal where a hex digit follows. Comments may have them.
- The engine and anything that will go into the FEP plugin DLL must have no writable
  static data: tables are plain arrays of numbers or `char[N]`, never arrays of
  pointers. A `TVnMacros` (10 KB) never goes on the stack: applications that will load
  the plugin may have an 8 KB stack.
- UID3s are in the development range: the app `0xE7A10001`; `0xE7A10002` and on are
  kept for the plugin DLL and its implementation.
- The E72's own behaviour, measured by OttPilot, is in OttPilot's `docs/DEVICE.md`.

## Building

```
bash symbian/test/run.sh                 # the engine's tests, with the Mac's clang++
bash symbian/group/docker-build.sh       # -> symbian/sis/vnkey.sisx
bash symbian/group/publish.sh            # -> http://<host>/s60/vnkey/ for the phone's browser
```

`publish.sh` uploads `vnkey-<version>.sisx` and a plain page that links it over ssh,
to `VNKEY_DL_SSH` and `VNKEY_DL_DIR` from `symbian/build.local` (git-ignored, made from
OttPilot's `symbian/build.local`). That folder is `vnkey/` inside OttPilot's download
folder, which OttPilot's nginx serves at `http://<host>/s60/` with the type that makes
the S60 browser install a `.sisx` (OttPilot's `server/deploy/README.md`, "Downloads for
the phone"); the script refuses a folder that holds OttPilot's packages, whose page it
would replace. The host, its name and the folders stay out of every file in git, as in
OttPilot: write `<host>`. Publishing puts a package on the internet: run it when Khoi
asks for it.

The build runs in the SDK's Docker image `minhkhoi4dev/s60v3-sdk-docker:1.1.0`
(GnuPoc, GCCE, the S60 3rd FP2 SDK; public on Docker Hub). Every build adds 1 to the
build number in `symbian/VERSION` and generates `symbian/inc/VnKeyVersion.h`
(git-ignored); the version is in the log's first line and in the package. Commit
`symbian/VERSION` with the change it was built for. `abld` says nothing useful when a
file does not compile; `build.sh` stops when `vnkey.exe` was not made. The signing keys
`symbian/sis/dev.key` and `dev.cer` are made by the first build and never committed.

## Roadmap

1. The probe (app 0.0.x). Telex in VnKey's own editor with the phone's FEP in it, as
   in any application; every key event and every rewrite in the log; the three rewrite
   methods; Ctrl+Space; the indicator in four places; FEP checks: 1 the FEP plugins on
   the phone and the loaded one, 2 load that plugin, 3 make a second FEP from it.
   Done when: on the E72 Telex gives the right text with one of the methods (the log
   says `ok`), Ctrl+Space switches, one indicator place is chosen, and the three FEP
   checks are in the log.
2. The plugin: `vnkeyfep.dll`, an ECOM `CCoeFepPlugIn` with capabilities All -TCB, that
   loads AknFep (the UID from FEP check 1), returns its FEP and adds `CVnCatcher`;
   VnKey switches the system's FEP to it and back (`CCoeEnv::InstallFepL`,
   WriteDeviceData); the on/off state and the options shared between applications;
   a kill switch (a file on the memory card that makes the plugin add nothing), and
   VnKey's own process left alone, so that a broken plugin can be switched off.
   Done when: Messaging and OttPilot's chat take Telex.
3. The [V] / [E] mark in every application, while an editor has the focus.
4. Settings that last: the options, the shortcuts edited on the phone, the state at
   start; uninstalling switches the FEP back first.
