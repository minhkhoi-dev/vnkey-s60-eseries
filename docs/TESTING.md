# Testing VnKey on the E72

The phone writes what it does to `vnkey.log` in `E:\VnKey\` on the memory card
(`C:\Data\VnKey\` without a card), one line per event with the phone's time. The log is
appended to, run after run; each run starts with `VnKey <version> start`.

To get it to the Mac, with `bash symbian/group/publish.sh` running there (the web stub):
VnKey's Options > Send log (OK on the address, which the build filled in), or the stub's
page in the phone's browser > Send a log > pick `E:\VnKey\vnkey.log` > Send. Either way
it lands in `logs/` of this repository (git-ignored) as `vnkey-<date>-<time>.log`. Then
tell Claude which steps were done and what was seen on the screen. Options > Clear log
empties it on the phone after it was sent.

## Tests for the lite web page (site/m/)

The introduction site has a light page for the phone's browser. Before it is on GitHub
Pages it can be served from the Mac on the Wi-Fi; Opera Mini cannot reach the Mac's
address (it loads pages through Opera's servers), so its test waits for GitHub Pages.

### Test 1: The phone's browser gets the lite page

Pre-condition:
- On the Mac: `python3 -m http.server 8061 --directory site` in the repository.

Steps:
1. Web (the phone's browser): open `http://192.168.1.16:8061/`.
2. Scroll to the end: English.
3. Tiếng Việt, then Bản đầy đủ.

Expected:
- Step 1: the page that opens is the light one: a red bar with the icon and VnKey, a
  yellow box `Tải VnKey (.sisx)` on the first screen, then Cài đặt, Gõ Telex, Tính
  năng, Cần biết. Every Vietnamese letter shows (no boxes), arrows are `->`.
- Step 2: the same page in English.
- Step 3: the full page opens (it may be slow and look plain); no crash.

### Test 2: Opera Mini and the download (after GitHub Pages is on)

Steps:
1. Opera Mini: open the site's address. Then the phone's browser: the same address.
2. In the phone's browser: Tải VnKey.

Expected:
- Step 1: both get the light page.
- Step 2: the link names the version (`Tải VnKey 0.1.11 (.sisx)`) and the phone offers
  to install it. A certificate question may come first (the TLS patch asks about a
  certificate it cannot check).

## Tests for app 0.1.11

New: an app icon, and VnKey's screens in Vietnamese (the default) or English, set in the
row Ngôn ngữ (Language). The log stays in English.

### Test 1: Update, and the new icon

Pre-condition:
- `bash symbian/group/publish.sh` running on the Mac.
- VnKey > Telex in all apps: Off, answer No, then restart the phone (the rules of test 5
  below).

Steps:
1. Browser: open `http://192.168.1.16:8060/` > Install the newest version (0.1.11).
2. Open the phone's menu where VnKey is.

Expected:
- Step 1: the install finishes with no error.
- Step 2: VnKey's icon is a red rounded square with a white key on it, a red V on the key
  and a small gold tone mark above the V. If the old icon is still there, restart the
  phone (the menu keeps icons in a cache) and look again.

### Test 2: Vietnamese by default

Pre-condition:
- Test 1 done.

Steps:
1. Open VnKey.
2. Tùy chọn (left softkey): look at the menu, then close it.
3. Telex mọi ứng dụng: Bật.

Expected:
- Step 1: the rows are in Vietnamese (Gõ tiếng Việt, Telex mọi ứng dụng, Chạy khi bật
  máy, Ngôn ngữ (Language) with Tiếng Việt, Kiểu bỏ dấu, ...), with their values as
  before the update. Softkeys `Tùy chọn` and `Thoát`. Every Vietnamese letter shows
  (no boxes or wrong letters).
- Step 2: the menu is in Vietnamese (Thay đổi, Gõ thử, Nạp lại gõ tắt, phím chuyển, Gửi
  nhật ký, ...). While it is open the softkeys may be in the phone's language (they are
  the phone's).
- Step 3: a note `Telex mọi ứng dụng: bật.`; the title ends in `tất cả`.

### Test 3: English, and back

Pre-condition:
- Test 2 done.

Steps:
1. Ngôn ngữ (Language) > English.
2. Options: look at the menu. Options > Typing test, then Back.
3. Options > Exit. Open VnKey again.
4. Language (Ngôn ngữ) > Tiếng Việt.

Expected:
- Step 1: at once, every row is in English, the softkeys are `Options` and `Exit`, and
  the Language row is still the selected one.
- Step 2: the menu is in English; in the typing test the softkeys are `Options` and
  `Back`.
- Step 3: VnKey opens in English.
- Step 4: at once, everything is in Vietnamese again.

### Test 4: Notes, and Telex still works

Pre-condition:
- Test 3 done, VnKey in Vietnamese.

Steps:
1. Tùy chọn > Gửi nhật ký.
2. OK.
3. Messaging > New message: type `Vieejt` and a space.

Expected:
- Step 1: the question `Máy nhận nhật ký (địa chỉ:cổng)` with softkeys `OK` and `Hủy`.
- Step 2: `Đang gửi nhật ký`, then `Đã gửi nhật ký: ...`.
- Step 3: the text becomes `Việt `.

The log has `settings: ... lang 1` while English is saved, and a line `language: en,
error 0` (or `vi`) at each switch; an error there other than 0 means the texts did not
change.

## Tests for app 0.1.9 / 0.1.10

VnKey opens on a settings list. The selection key toggles an On/Off row or opens a list
to pick from. The old editor is under Options > Typing test.

### Test 1: Telex comes back after a restart

Pre-condition:
- VnKey 0.1.9 or later installed.
- VnKey settings: Vietnamese typing On, Telex in all apps On, Start at boot On.

Steps:
1. Restart the phone.
2. Do not open VnKey. Wait 2 minutes after the idle screen appears.
3. Messaging > New message > Text message.
4. Type `Vieejt` and a space.

Expected:
- The text becomes `Việt `.
- The [V] mark is in the bottom bar.

### Test 2: No mark in full-screen apps

Pre-condition:
- Test 1 passed.

Steps:
1. Open SkyForce Reloaded and play a few seconds.
2. Open a Java game.
3. Gallery: play a video full screen.
4. Open Messaging.

Expected:
- Steps 1 to 3: no [V]/[E] mark on the screen.
- Step 4: the mark is in the bottom bar.

### Test 3: Start at boot off

Pre-condition:
- Test 1 passed.

Steps:
1. VnKey > Start at boot: Off. Options > Exit.
2. Restart the phone. Wait 2 minutes.
3. Hold the menu key (open applications).
4. Messaging > New message: type `Vieejt` and a space.
5. VnKey > Start at boot: On.

Expected:
- Step 3: VnKey is not in the list.
- Step 4: the text stays `Vieejt` (Telex is off).

### Test 4: Send the log

Pre-condition:
- `bash symbian/group/publish.sh` running on the Mac.

Steps:
1. VnKey > Options > Send log > OK.

Expected:
- A note `Log sent`.

### Test 5: Update to a new version

Pre-condition:
- Tests 1 to 4 done.

Steps:
1. VnKey > Telex in all apps: Off. Answer No.
2. Restart the phone.
3. Browser: open `http://192.168.1.16:8060/` > Install the newest version. If the
   installer asks to close VnKey, answer Yes.
4. Open VnKey. Telex in all apps: On.

Expected:
- Step 1: a note `Telex in all apps stays on until the phone restarts.`
- Step 3: the install finishes with no error.
- Step 4: the other settings are as before; the title ends in `all`.

Rules while Telex in all apps is on: never install or uninstall VnKey before steps 1
and 2 of test 5. If the phone ever restarts while VnKey turns Telex back on at boot, VnKey
leaves it off and shows a note; turn it on again by hand.

## Telex in all apps (roadmap 2, app 0.1.x)

The package puts VnKey and its plugin (`vnkeyfep.dll`) on the phone's memory, C:,
whatever the installer would choose. Rules while the plugin is the phone's FEP:

- Before installing a new VnKey, the plugin must be off: with it on, every application
  has the plugin loaded and the installer may refuse to replace it. Since 0.1.9, VnKey
  starts at boot and turns it back on, so: Telex in all apps: Off, answer No ("off at the
  next restart"), restart the phone, then install. Turn it on again afterwards.
- Never uninstall VnKey while it is on: the phone would be left with no FEP at all. Turn
  it off the same way first (Off, No, restart).
- The safe way to turn it off is to restart the phone: the setting does not survive a
  reboot, so the phone comes back with its own input (AknFep). Setting the row to Off
  saves that first; answering Yes also switches it off at once, which restarts the
  phone on the E72 (a process crashes mid-switch); prefer No and a restart.
- If the phone ever restarts while VnKey turns Telex back on by itself (at boot), the
  next start leaves it off and says so; turn it on again by hand to retry.
- With it on, the [V]/[E] mark shows in every application that has a status bar
  (non-fullscreen); it tracks Vietnamese/English there too.
- Kill switch, should applications misbehave and VnKey not open: in File manager, on the
  memory card, open the folder `VnKey` and make a new folder in it named `off`
  (Options > Organise > New folder), then restart. Applications then get no Telex from
  the plugin, only the phone's own input. Delete the folder to undo it.

Steps:

1. Install 0.1.x as before. Open VnKey: the title is `VnKey [V] m3`, the settings have
   Telex in all apps (Off).
2. VnKey > Telex in all apps: On. A note says it is on and the title ends in
   `all`. Note anything else that happens (an application closing, the phone restarting).
   If the phone misbehaves, restart it: that turns the plugin off.
3. Without closing VnKey, open Messaging > New message > Text message: type the table of
   the probe's step 2 below, each line then a space. Ctrl+Space there, a word, Ctrl+Space
   again. A long press and Fn + a letter after a word, as in step 5 below.
4. OttPilot: type the same in a chat.
5. Back in VnKey: Options > Send log.

Trigger key: VnKey writes `triggers.txt` next to `macros.txt` the first time, with
`ctrl+space`. To change or add a trigger (Ctrl+Space is also the phone's own predictive
toggle), edit `triggers.txt` on the phone — one key per line, modifiers and a key joined by
`+` (`ctrl` `shift` `fn` `alt`; keys `space` `enter` `tab` `bksp` or a scan code like
`0x5f`). Turn on the setting Log every key to read a key's scan code from the log. Then
Options > Reload macros/triggers (in VnKey; other apps pick it up when they next load the
plugin, e.g. after a reboot and turning Telex on again).

The log has the plugin's lines with the application's program name in front
(`<name>: plugin 0.1.1: sid <its SID>`, `<name>: catcher added`, then the key and
`rewrite` lines as in the probe); VnKey's own lines have no name. Keys typed into password fields are never logged.

## The probe (roadmap 1, app 0.0.x)

Since 0.1.9 the probe's editor is VnKey > Options > Typing test, and the options below
(Rewrite method, Indicator place, Tone, ...) are rows of the settings list, not menu items.

Before the first run:

- Install the package (App manager: Software installation = All, as for OttPilot):
  run `bash symbian/group/publish.sh` on the Mac, open the address it prints
  (`http://<the Mac's LAN address>:8060/`) in the phone's browser on the same Wi-Fi and
  choose Install; the stub's terminal shows the download. Or copy
  `symbian/sis/vnkey.sisx` to the memory card over USB (Mass storage) or Bluetooth and
  open it on the phone.
- Predictive text off: in Messaging, write a new message, then Options > Input options
  > Prediction > Off (the setting is the phone's, so it holds in VnKey too).

1. Open VnKey (Applications, or wherever installed apps go). The title says
   `VnKey [V] m1`: Vietnamese on, rewrite method 1. A small blue box with V is somewhere
   in the status bar. The menu (Options) has everything below.
2. Type each line, then a space, and compare with what it should become:

   | Keys | Should become |
   |------|---------------|
   | `Tieesng Vieejt cos daaus` | Tiếng Việt có dấu |
   | `nguwowif ddepj` | người đẹp |
   | `Hoaf Bifnh` | Hòa Bình |
   | `dduowngf phoos` | đường phố |
   | `toans` (tone at the end) | toán |
   | `ass aaa ddd` (twice takes it back) | as aa dd |
   | `class text window` (English stays) | class text window |
   | `ko dc vn` (shortcuts from macros.txt) | không được Việt Nam |

   Type the first letter of a sentence without Shift too: the phone capitalises it, and
   VnKey must keep the capital (`Tieesng` at the start → `Tiếng`).
3. If what appears is wrong (letters doubled, a word left over, the cursor in the wrong
   place), note it, then Options > Rewrite method (it goes 1 → 2 → 3, shown as m1 m2 m3
   in the title), Options > Clear text, and type step 2 again. Do it for all three
   methods even when 1 works: the log compares them.
4. Ctrl+Space: the box turns grey with E and the title says `[E]`; `Vieejt` now stays as
   typed. Ctrl+Space again for V.
5. Keys the phone handles itself, typed after a word in Vietnamese mode:
   - Hold a letter key down (a long press) after `a`: the phone's own character for it
     must appear, with nothing else changed.
   - A digit with Fn (Fn + a letter key) after `a`.
   - Shift held with a letter, and Shift pressed once before a letter.
   - Chr: pick a character from the table after `toan`.
6. The box's place: Options > Indicator place goes through 1 to 4. Say which looks right
   next to the phone's own icons (a photo of the screen helps).
7. Options > Tone: modern, then `hoaf thuyr khoer` → hoà thuỷ khoẻ; back to traditional.
8. FEP checks, last, in this order (check 1 runs by itself at every start):
   - Options > FEP check 2: load plugin.
   - Options > FEP check 3: second FEP. This makes a second copy of the phone's FEP for
     a moment; VnKey may close or misbehave after it. Exit VnKey afterwards and start it
     again to check it still works.
9. Options > Send log, then Options > Exit.

What the log shows: `key`/`down`/`up` lines for every key event (code, scan code,
modifiers, cursor, length, the editor's input mode and case), `rewrite <method>: old ->
new ok|BAD` for every change VnKey made, `flush:` lines when it held back (the phone
typed something else, a long press), `indicator place` with the status bar's panes, and
`fep` lines for the FEP checks.
