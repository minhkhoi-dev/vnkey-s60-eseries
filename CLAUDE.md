# VnKey — Vietnamese Telex input for the Nokia E72 (S60 3rd Edition FP2)

Telex typing like Unikey on Windows, for Khoi's Nokia E72: S60 3rd Edition Feature
Pack 2, Symbian OS 9.3, a QWERTY keyboard, a 320x240 screen. The phone is hacked
(InstallServer and Open4All patches), so a self-signed package may carry any
capability. Khoi runs every test on the phone. Files cannot be copied off it by cable or Bluetooth,
so the app writes a log to the memory card and sends it over the Wi-Fi to the web stub
on the Mac, which saves it in `logs/` (`docs/TESTING.md`). The
model for building S60v3 apps is Khoi's OttPilot (`/Users/khoi/Projects/ottpilot`,
its `symbian/CLAUDE.md`); this project copies its build setup and its Symbian rules.

Where it stands (2026-10-16): roadmap 1, the probe, is done; next is roadmap 2, the
system-wide plugin. On the E72 (logs of 0.0.1 to 0.0.4 in `logs/`): rewrite method 1
rewrote every word right (68 of 68), among them the restore (tẽt → text, ưindow →
window) and the shortcuts; the phone's capital at a sentence's start is kept; Ctrl+Space
switches; the indicator's place 1, in the bottom bar next to the phone's own icons and
clock, is the one Khoi wants. The letters on the phone keypad's keys broke words in 0.0.1
(see "Design"), fixed in 0.0.2. The phone has one FEP plugin, AknFEP `0x101FD65A` in ROM;
VnKey loaded it and had it make a second FEP object, which it deleted again, and went on
working (FEP checks 2 and 3, 0.0.4). Options > Send log works. App 0.0.5 drew the
indicator like the phone's icons (Khoi: "the same as memory card style"); on the phone
its letter touched the frame's top, so 0.0.6 draws it pixel by pixel with room around.

2026-10-17: roadmap 2 begun (app 0.1.x). On the E72 (0.1.1), "Telex in all apps" made
VnKey's plugin the system FEP and eight running processes loaded it and added the catcher
with no error (Popnotifierappserver, EikSrvs, screensaver, essysapp, fsmailserver, Xplore
and more) — the plugin loads system-wide. Not yet confirmed: typing in Messaging/OttPilot.
0.1.2/0.1.3 on the E72: "Telex in all apps" loaded the plugin into Messaging (Mce,
UniEditor sid 102072d8), OttPilot and more, and one word was corrected in the SMS editor
(`UniEditor: rewrite 0: Tiee -> Tiic ok`) — Telex reached real apps. But the inline-edit
rewrite (method 1) fights AknFep's own inline edit in live editors: it worked once then the
word came out wrong (`rewrite 0: ... BAD, got <one char>`) and the host application crashed
(UniEditor and VnKey both). It had worked 136/136 in VnKey's own editor in roadmap 1, so the
collision shows only over the plugin-loaded AknFep. 0.1.3 makes the rewrite method part of
the shared state so it can be switched live from VnKey's menu and tried in real apps. On the
E72 (0.1.3) method 3 (backspaces + retype through the FEP) typed Telex in Messaging, and the
crash was pinned to AknFep's predictive entry: the underlined word in "Abc" mode is an open
inline edit the rewrite collided with; with prediction off it worked. So 0.1.4 turns
predictive entry off in the focused editor while Vietnamese is on (`EAknEditorFlagNoT9` +
`ReportAknEdStateEventL`), and method 3 is the default (it is not restored to English
prediction until the editor is reopened). 0.1.5: the trigger key is configurable in
`triggers.txt` next to `macros.txt` (lines like `ctrl+space`, `fn+space`, `shift+space`, or
a raw `0x5f`; modifiers ctrl/shift/fn/alt + a key, several lines = several triggers), read
by VnKey and the plugin; Ctrl+Space is the default and the fallback when the file is empty.
It was made configurable because Ctrl+Space is also the phone's own predictive-text toggle.
Options > Reload macros/triggers re-reads both without a restart (VnKey's own catcher; other
apps re-read when their plugin next loads). Two faults were found and fixed in 0.1.2: the old
"FEP check 4" (and a pre-flight before the switch) loaded a second AknFep in VnKey's own
process while the boot one was still there, which crashed AknFep — both removed; the switch
is InstallFepL alone. The FEP
setting does not survive a reboot on this phone (`fep: system setting ... error -6`;
InstallFepL takes effect live but is not stored), so a restart is the safe way to turn it
off, and VnKey now tracks the on/off state itself (FepUid() stayed AknFep's in VnKey's own
process after the switch). The in-app off, which swaps the FEP in every process at once,
restarts the phone (0.1.2 and 0.1.3 logs: no plugin teardown line is written before the
reset, so a process crashes mid-switch; the reliable off is a reboot, which reverts to
AknFep). 0.1.6 warns before the in-app off (`r_vnkey_off_query`) and points to the reboot;
the live-off crash itself is unsolved and costly to retest (each try resets the phone).
0.1.6 also shows the [V]/[E] mark in every application that has a status pane (roadmap 3):
the plugin's catcher owns a `CVnIndicator` when `CEikStatusPaneBase::Current()` is there (a
normal, non-fullscreen application; not a server or a game), deleted with the catcher by the
app UI stack while the window group is still alive. The build's baked log server is the Mac's
LAN address from `group/lan.sh` (192.168.1.16 on 2026-10-17).

0.1.9 (2026-10-17, built, not yet run on the E72), three things Khoi asked for: VnKey opens
on a settings list (`CVnSettingList`, a standard `CAknSettingItemList`) instead of the long
Options menu, with the typing test behind Options > Typing test; VnKey starts at boot
(`vnkeyboot.exe`, a startup list entry, as OttPilot's) and turns Telex in all apps back
on; and the [V]/[E] mark hides while an application is full screen (Khoi: it showed over
games and video; it is to be in the bar only). See "Design". On the E72 (0.1.9, log of
15:47): the launcher started VnKey at boot (`started at boot, in the background`); the
settings list worked (options, Telex in all apps on: plugin 0.1.9 in Phone, menu2, OttPilot
and the rest, no crash; the mark places); `indicator: shown` in the applications. The
restore at boot was not tried (Telex in all apps was off then), nor full-screen apps.
Found: the 0.1.7 live off at 15:36 restarted the phone in the middle of a save and left
`settings.txt` empty, which read as every option off. 0.1.10 saves through `settings.tmp`
and `RFs::Replace`, keeps the defaults when `flags` is missing, and no longer saves again
after a live off. Tests 1 to 4 of `docs/TESTING.md` passed on the E72 (0.1.9, log of
16:13): at boot VnKey started, restored Telex in all apps 0.1 s later and removed the
guard a minute after (`the phone kept going`), Telex typed in Messaging without opening
VnKey; the mark hid in a Java game (`j9midps60: indicator: hidden, status pane hidden`,
0.3 s after it showed); SkyForce Reloaded has no status pane, so it got no mark; no mark
over the video player; with Start at boot off nothing started at boot. Test 5 (the
update to 0.1.10) passed (log of 16:23): Off with No left the plugin on until the
restart, the boot start then left it off, the installer closed VnKey and 0.1.10 started
with the settings kept (`flags 319`), and Telex in all apps went on again.

0.1.11 (2026-10-17, passed on the E72: Khoi, "it's OK"; log of 18:04), two things Khoi
asked for: an app icon "with a modern look" (there was none: `number_of_icons = 0`), and
VnKey's screens in Vietnamese or English, Vietnamese by default. The icon is
`symbian/gfx/vnkey.svg`: a red rounded tile, a white key cap on it with a red V and a
gold sắc mark (Vietnamese typing), chosen from four drafts rendered at 88 to 22 px. The
language is the row Ngôn ngữ (Language) and `lang=` in `settings.txt`; it switches at
once (see "Design"). On the phone: the update kept the settings and started in
Vietnamese (`settings: ... lang 0`), Telex still typed in OttPilot, and the live switch
to English worked (`language: en, error 0`); the SVG icon in the MIF shows.

## Requirements

Decided by Khoi on 2026-10-16:

- Telex works in every application, as Unikey does (not only in VnKey's own editor).
- Only Khoi's hacked E72 has to run it; stock phones do not matter.
- The phone's font shows every Vietnamese letter: VnKey types precomposed Unicode (NFC).
- Tone placement can be chosen in the settings: traditional (hòa, thúy, khỏe) or modern
  (hoà, thuý, khoẻ).
- Ctrl+Space switches between Vietnamese and English; the trigger key is configurable and
  more than one may be set (`triggers.txt`), since Ctrl+Space is also the phone's own
  predictive-text toggle.
- A [V] / [E] mark in the status bar, like the memory card, clock and WLAN icons; only
  in the bar: hidden while an application is full screen (a game, a video; Khoi,
  2026-10-17).
- VnKey's own screen is a settings screen, not a long menu of toggles, and VnKey starts by
  itself at boot (Khoi, 2026-10-17).
- VnKey's screens in Vietnamese or English, Vietnamese by default, and an app icon with a
  modern look (Khoi, 2026-10-17). The log stays in English.
- Besides plain Telex: w alone types ư; a spell check that restores the keys of words
  that are not Vietnamese (class, email, window); typing shortcuts (gõ tắt, ko → không).
  VNI is not wanted.
- Done means: Messaging (SMS) and OttPilot's chat editor take Telex.
- The public release has no Send log: its log stays on the phone (Khoi, 2026-10-25).
  Builds on the Mac keep Send log for testing (see "Building", public builds).

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
   the editor holds exactly the old word plus one new character. A letter goes to the
   engine with the word, and the word is rewritten in place. Anything else (space,
   digit, punctuation, Enter) ends the word: shortcuts, the restore of non-Vietnamese
   words and thươ → thuở are applied to the word, with the character kept after it. A
   long press (repeats) is AknFep's: no rewrite.
4. What a key types is decided by what appears in the editor, never by the key event's
   code: on the E72 the letter keys that carry the phone keypad (R T Y F G H V B N, U
   `*`, J `#` and so on) send the keypad's character as the code, and AknFep types the
   letter. App 0.0.1 took those codes for word ends, so f, j, t, n, u and the rest
   never got Telex (2026-10-16: Hoaf, Viêjt, ngươif on the screen; `daaus`, where u ended
   the word at dâ and the restore made it daa). The log has them: those keys have the
   scan code and the code of the keypad's key, `'1'` to `'9'` (R T Y, F G H, V B N),
   `'0'`, `'*'` (U) and `'#'` (J, scan 7F); the other letters have their capital as the
   scan code and the small letter as the code. Ctrl+Space is scan 05, code 0.
5. Ctrl+Space is the one key it consumes.

Password editors are left alone. How a word is rewritten is the probe's question; there
are three methods (setting "Rewrite method"): 1 selects the old text and replaces it with
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
VnKey's own window group; the plugin will show it in each application. It looks like the
phone's own indicator icons: the letter V or E drawn pixel by pixel (5 x 7, so that no font
fills the frame: 0.0.5 drew it with the dense font, which touched the top, and Khoi asked
for room around it), 2 pixels inside a thin frame of 11 x 13, in the skin's colour for
universal indicator icons (`KAknsIIDQsnIconColors`, `EAknsCIQsnIconColorsCG2`), on the
bar's own background, drawn with the bottom bar's skin context
(`CEikButtonGroupContainer::Current()->ButtonGroup()->AsControl()`); black where that
cannot be had. Its window is `ECoeWinPriorityAlwaysAtFront`, so it would show over a
full-screen game or video: since 0.1.9 it shows only while the bar is on the screen
(`CVnIndicator::ShouldShow`). Avkon tells no other control when an application hides its
status pane or softkeys, so the mark looks again 50 ms, 0.75 s and 1.45 s after every key
up (the catcher), focus change (`MCoeFocusObserver`), return to the foreground
(`MCoeForegroundObserver`) and layout switch. It hides when there is no status pane, the
status pane is hidden, the pane of its place is not in the status pane's current layout,
the application's `ClientRect()` reaches it (status pane or softkeys gone), the softkeys'
bar under it is hidden, or the focused control's window lies over it and is in front of
that bar's window (siblings compared with `FullOrdinalPosition`; a game that draws over a
status pane it left visible). Each change is a log line (`indicator: shown`, `indicator:
hidden, <why> | mark ... | client ... | focused ...`). Not handled: a full-screen window
that is neither focused nor reflected in the status pane or `ClientRect()`.

The plugin (roadmap 2; `symbian/src/VnFepPlugIn.cpp`, `vnkeyfep.dll`, ECOM implementation
`0xE7A10003` of the FEP interface `0x1020233F`):

- CONE makes one in each GUI process when it is the system's FEP and keeps it as long as
  the FEP object it returned, which CONE deletes first. Its `NewFepL` loads AknFep's
  plugin (`0x101FD65A`; else the first other FEP plugin), keeps it (so that AknFep's DLL
  stays loaded while AknFep's FEP lives), and returns the FEP AknFep makes. Then it adds
  a `CVnCatcher`, unless the process is VnKey's own (which has its own catcher) or the
  kill switch is there (`VnFiles::KillSwitch`: a file or folder `off` in `E:\VnKey\` or
  `C:\Data\VnKey\`; File manager can make a folder on the phone).
- The catcher there goes on the app UI's control stack with
  `ECoeStackFlagOwnershipTransfered` (not shared with embedded app UIs, which could
  delete it twice): at an application's exit whichever of the app UI and the plugin CONE
  deletes first, the catcher goes with it once (`CatcherDestroyed` tells the plugin).
- The state and the options are shared through one Publish and Subscribe property in
  VnKey's category (its SID; `TVnShared`), which VnKey defines at its start, readable and
  writable by any process. The catcher reads it at every key (a fast kernel call) and
  writes it at Ctrl+Space. In the plugin a catcher does nothing while the property is
  not there: after the phone restarts, until VnKey has run, so that a plugin that broke
  applications could not break the phone's start.
- Every application writes to the one log (each line at the file's end, after the
  program's name); keys typed into a password field are never logged. Its buffers are
  members, not on the stack (8 KB stacks).
- VnKey's "Telex in all apps" calls `CCoeEnv::InstallFepL` with the plugin's UID (off
  installs AknFep's UID again); every running application then loads it. It does NOT
  pre-load the plugin by hand first: that made a second AknFep in VnKey's own process
  (the boot one is still loaded) and crashed AknFep. On this phone the setting is not
  stored (reading the FEP framework repository `0x10272618` gives `error -6`; the install
  is live only), so a reboot returns to AknFep and VnKey tracks the state itself
  (`iSystemFepOn`) rather than from `FepUid()`, which stays AknFep's in VnKey's own
  process. The live off swaps the FEP in every process at once and is the suspect for the
  phone restart of 2026-10-17. Since 0.1.9 the setting's row shows the saved preference
  (the title ends in `all` while the plugin is the FEP now); setting it to Off saves the
  preference first, then asks whether to switch now as well (`r_vnkey_off_query`; No
  leaves it on until the phone restarts, the safe off).
- The package puts everything on C:, so that with the memory card out the system's FEP
  and VnKey, which switches it back, are still there. While the plugin is the FEP, a new
  VnKey is installed only after switching it off, and VnKey is never uninstalled while it
  is on in the same session (a new application would find no FEP). The FEP setting is kept
  in RAM only on this phone (it is not stored, `error -6`), so a reboot always returns to
  AknFep: that is both the safe off and the safe state before uninstalling. For this reason
  VnKey does not run a risky live FEP swap at uninstall; it relies on the reboot. VnKey
  saves its own settings in `settings.txt` next to the log (`flags` = the shared encoding
  of the options, Vietnamese/English, the log key flag and the rewrite method; `place` the
  indicator place; `system` the Telex-in-all-apps preference; `boot` start at boot, 1 when
  absent; `lang` the language of the screens, `vi` or `en`, Vietnamese when absent;
  `logserver` the web stub).
  At start it loads them, publishes the shared state, and when `system` is on but the phone
  came back as AknFep it reinstalls the plugin once (`HandleChangeInLoadedFep`), so Telex in
  all apps survives a reboot as far as reopening VnKey. Same-boot it reads the live shared
  state instead, and detects the plugin already being the FEP from `FepUid()`. The
  automatic reinstall is skipped while the kill switch is there, and it is guarded: VnKey
  writes `restoring.txt` next to the log before it and deletes it a minute after (or at
  its exit); if the phone restarts with the file still there, the next start leaves Telex
  in all apps off, sets `system` to 0 and says so in a note, so that a plugin that
  restarted the phone cannot make it restart at every boot.
- Start at boot (0.1.9): the package installs a startup list entry
  (`data/vnkey_startup.rss` as `c:\private\101f875a\import\[E7A10001].rsc`) that names
  the launcher `vnkeyboot.exe` (`src/VnKeyBoot.cpp`, UID `0xE7A10004`, no capabilities),
  as OttPilot does (its `docs/DEVICE.md`: the E72 takes the entry from a self-signed
  package and runs it about 70 s after boot). The launcher reads `settings.txt` in VnKey's
  folder: with a line `boot=0` it does nothing; otherwise, unless VnKey runs already, it
  starts VnKey through the application server with `EApaCommandBackground` (ten tries 3 s
  apart). VnKey then sends itself behind the other applications
  (`ProcessCommandParametersL`, log line `started at boot, in the background`), restores
  Telex in all apps as at any start, and stays running. Telex in all apps does not need
  VnKey running: after `VnKey exit` the plugin went on in every application (0.1.7 log,
  `menu2` keys after the exit) and the shared state stays defined.
- VnKey's own screens (0.1.9): `CVnSettingList` is the main screen, one row per option
  (`TVnSettingId` in `VnKey.hrh`, rows in `vnkey.rss`, their texts in the `.loc`
  files); the selection key toggles an on/off row or opens the page of a row with more
  values, Options > Change opens any row's page. Options > Typing test swaps in the
  editor of roadmap 1 (`CVnKeyAppView`; softkeys Options/Back); only the screen in front
  is on the control stack.
- The language of VnKey's screens (0.1.11). Every text VnKey shows is a `qtn_vnkey_...`
  name in `data/vnkey_vi.loc` and `data/vnkey_en.loc` (UTF-8; `vnkey.rss` says
  `CHARACTER_SET UTF8`, and rcomp then stores ữ as U+1EEF, checked 2026-10-17; without it
  the bytes stay Latin-1). `vnkey.rss` is built twice: as `vnkey.rsc` with the Vietnamese
  file, and through `vnkey_en.rss` (`#define VNKEY_LANG_EN`, then the same `vnkey.rss`) as
  `vnkey_en.rsc`. Same NAME, same resources in the same order: same IDs (`vnkey.rsg`) and
  the same offset. The framework loads `vnkey.rsc` at the start (no `.rNN` files, so
  `NearestLanguageFile` keeps it); when `lang=en` is saved, `CVnKeyAppUi::UseLanguageL`
  swaps it before the screens are made (`DeleteResourceFile` of the application's offset,
  `AddResourceFileL` of the other file: the offset stays, so the application frees it at
  the exit as it would the first). A change of the Language row saves first and swaps
  after the list's `EditItemL` has returned (an async callback), then makes the screens
  again: the softkeys, the title, and the settings list from the resource, on the same
  row; the menu reads its texts each time it opens. The softkeys are VnKey's own CBA
  resources with Avkon's command IDs (Avkon's would be in the phone's language), and the
  setting pages and queries use them too. Still the phone's: the softkeys while the
  Options menu is open, and the access point list of Send log. The log, the macros and
  triggers samples stay in English.
- The app icon (0.1.11): `gfx/vnkey.svg`, 88 x 88, into `vnkey_aif.mif` by `build.sh`
  with GnuPoc's own `mifconv`, which puts the SVG text in as it is (no `svgtbinencode`, a
  Windows tool not in the image); the S60 SVG engine reads SVG text. Only what SVG Tiny
  1.1+ has: `rect` with `rx`, paths of lines, linear gradients in `userSpaceOnUse`; no
  text, no arcs, no group opacity. `LOCALISABLE_APP_INFO` names it (`number_of_icons =
  1`).

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
  Texts on VnKey's screens are not in the code at all: they go in both `.loc` files, in
  UTF-8 (see "Design", the language).
- The engine and anything that will go into the FEP plugin DLL must have no writable
  static data: tables are plain arrays of numbers or `char[N]`, never arrays of
  pointers. A `TVnMacros` (10 KB) never goes on the stack: applications that will load
  the plugin may have an 8 KB stack.
- UID3s are in the development range: the app `0xE7A10001`; `0xE7A10002` the plugin DLL,
  `0xE7A10003` its implementation, `0xE7A10004` the boot launcher.
- The E72's own behaviour, measured by OttPilot, is in OttPilot's `docs/DEVICE.md`.

## Building

```
bash symbian/test/run.sh                 # the engine's tests, with the Mac's clang++
bash symbian/group/docker-build.sh       # -> symbian/sis/vnkey.sisx
bash symbian/group/publish.sh            # serves it to the phone's browser from this Mac
```

`publish.sh` serves the package on the local network only, from this machine: it
copies it into `symbian/web/` (git-ignored) as `vnkey-<version>.sisx` with a plain page
that links it, prints `http://<LAN address>:8060/` and runs the web stub
`symbian/group/webstub.py` there (plain HTTP, a `.sisx` sent as
`application/vnd.symbian.install` so that the S60 browser installs it, no caching, no
directory listings, every request shown) until Ctrl+C. The E72 opens that address on
the same Wi-Fi. `VNKEY_WEB_PORT` changes the port. Nothing is uploaded anywhere: Khoi
chose that on 2026-10-16, over OttPilot's way of putting the packages on the VPS.

The stub also takes the phone's log into `logs/` (git-ignored) as
`vnkey-<date>-<time>.log`: VnKey's Options > Send log POSTs it to `/log`
(`CVnLogSender`, which joins the Wi-Fi that is up or asks for the access point, as
OttPilot does), and `/upload` is a form for the phone's browser, which can pick
`E:\VnKey\vnkey.log` (used on 2026-10-16). The build bakes the Mac's LAN address and
port into the app as the first address for Send log; the query lets it be changed.

The build runs in the SDK's Docker image `minhkhoi4dev/s60v3-sdk-docker:1.1.0`
(GnuPoc, GCCE, the S60 3rd FP2 SDK; public on Docker Hub). Every build adds 1 to the
build number in `symbian/VERSION` and generates `symbian/inc/VnKeyVersion.h`
(git-ignored); the version is in the log's first line and in the package. Commit
`symbian/VERSION` with the change it was built for. `abld` says nothing useful when a
file does not compile; `build.sh` stops when `vnkey.exe`, `vnkeyfep.dll`, either
resource file or the icon's `vnkey_aif.mif` was not made. The plugin's DLL must have no `.data` or `.bss` (GnuPoc's `elf2e32` does not
check it; `readelf -S` on the objects in the image does). The signing keys
`symbian/sis/dev.key` and `dev.cer` are made by the first build and never committed.

Public builds and GitHub (2026-10-25). `.github/workflows/build.yml` runs the engine's
tests and the same Docker build on GitHub Actions for every push to main that touches
`symbian/`, with two settings the Mac's builds do not have: `VNKEY_KEEP_VERSION=1` (the
commit's own version, no bump) and `VNKEY_PUBLIC=1`. A public build has no Send log: no
menu item, no Log server row (the last row, so no other row moves), no `CVnLogSender`, no
address baked in; `build.sh` writes `VNKEY_SEND_LOG 0` to the generated
`symbian/inc/VnKeyBuild.hrh` (git-ignored), which `vnkey.rss` and the code read. The
package is the run's artifact; a tag `v<VERSION>` also makes a GitHub release with it.
It is signed with the repository secrets `VNKEY_DEV_KEY` and `VNKEY_DEV_CER` (the
contents of `dev.key` and `dev.cer`) when they are set, otherwise with a throwaway key.

The introduction page is `site/` (no build step), published to GitHub Pages by
`.github/workflows/pages.yml` (Pages source: GitHub Actions). It is in Vietnamese by
default, `site/index.html`, with English as the other choice, `site/en/index.html`
(Khoi, 2026-10-25); a VI / EN switch in the header links the two. Both pages share
`style.css`, `app.js` (the latest release and the typing demo) and `vnkey.svg`, so a
change of text goes into both pages. Its download links point at the latest release.
The typing demo's frames (in `app.js`) and the Telex examples were made by the engine
itself (`TVnTelex` driven like the tests' pretend editor), so regenerate them the same
way if the demo text or the engine changes.

The lite page for the phone's own browsers (Khoi, 2026-10-25: the S60v3 browser and
Opera Mini) is `site/m/index.html` (Vietnamese) and `site/m/en.html`: XHTML Mobile
Profile 1.2, about 5 KB, no scripts, no web fonts, only `icon.png` (32 px, rendered
from the SVG) loaded, and only ASCII and letters in the text (the phone's font has
every Vietnamese letter but maybe not symbols such as →, so arrows are `->`). The full
pages send S60 and Opera Mini user agents there (a script at the top of `<head>`;
`?full` keeps the full page) and link it in the footer. GitHub Pages is HTTPS only
(http:// on github.io answers 301) and takes TLS 1.2 and later, refusing TLS 1.0
(checked 2026-10-25 from the SDK image's OpenSSL 1.0.1); a stock S60 browser has only
TLS 1.0, Khoi's E72 has the nnproject TLS patch (OttPilot's `docs/DEVICE.md`, TLS).
The release downloads on github.com refuse TLS 1.0 too, so the Pages workflow puts the
latest release's `.sisx` next to the lite page and points its link (`id="dl"`, the
`VERSION` comment) at it; Pages serves `.sisx` as `application/vnd.symbian.install`
(mime-db), as the web stub does. A release made by `build.yml` runs `pages.yml` again
(`gh workflow run`), since a release made with the job's token starts no workflow.

## Roadmap

1. The probe (app 0.0.x). Telex in VnKey's own editor with the phone's FEP in it, as
   in any application; every key event and every rewrite in the log; the three rewrite
   methods; Ctrl+Space; the indicator in four places; FEP checks: 1 the FEP plugins on
   the phone and the loaded one, 2 load that plugin, 3 make a second FEP from it.
   Done when: on the E72 Telex gives the right text with one of the methods (the log
   says `ok`), Ctrl+Space switches, one indicator place is chosen, and the three FEP
   checks are in the log. Done 2026-10-16 (app 0.0.4): method 1, place 1, all three
   checks passed.
2. The plugin: `vnkeyfep.dll`, an ECOM `CCoeFepPlugIn` with capabilities All -TCB, that
   loads AknFep (the UID from FEP check 1), returns its FEP and adds `CVnCatcher`;
   VnKey switches the system's FEP to it and back (`CCoeEnv::InstallFepL`,
   WriteDeviceData); the on/off state and the options shared between applications;
   a kill switch (a file on the memory card that makes the plugin add nothing), and
   VnKey's own process left alone, so that a broken plugin can be switched off.
   Done when: Messaging and OttPilot's chat take Telex. Begun 2026-10-17 (app 0.1.x).
3. The [V] / [E] mark in every application. Begun 2026-10-17 (0.1.6): shown in every
   application that has a status pane (non-fullscreen); the plugin's catcher owns it.
   On the E72 it showed over some full-screen applications (games, video); 0.1.9 hides it
   while the bar is not on the screen (see "Design"); passed on the E72 (a Java game,
   SkyForce Reloaded, the video player).
4. Settings that last: the options, the shortcuts edited on the phone, the state at
   start; uninstalling switches the FEP back first. Begun 2026-10-17 (0.1.7,
   `settings.txt`). On the E72 the options set before a reboot (modern tone) were there
   after it, and VnKey's start put Telex in all apps back (`settings: restoring Telex in
   all apps`; the plugin loaded with its catcher in OttPilot, menu2, EikSrvs and the rest,
   no crash), and Telex typed in the applications after it (Khoi: every test passed). Up
   to 0.1.8 VnKey had to be opened once after each boot for that; 0.1.9 starts it at boot
   (setting "Start at boot", on by default) and has the settings list; both passed on the
   E72 (2026-10-17). Uninstalling relies
   on the reboot (see "Design"). In the log, method 3 always
   says `BAD`: the check runs before the simulated keys reach the editor; the text is right.
