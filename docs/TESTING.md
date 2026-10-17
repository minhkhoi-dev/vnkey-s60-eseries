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

## Telex in all apps (roadmap 2, app 0.1.x)

The package puts VnKey and its plugin (`vnkeyfep.dll`) on the phone's memory, C:,
whatever the installer would choose. Rules while the plugin is the phone's FEP:

- Before installing a new VnKey: VnKey > Options > Telex in all apps, to off. With it on,
  every application has the plugin loaded and the installer may refuse to replace it.
- Never uninstall VnKey while it is on: the phone would be left with no FEP at all.
- The safe way to turn it off is to restart the phone: the setting does not survive a
  reboot, so the phone comes back with its own input (AknFep). The menu's off restarts the
  phone on the E72 (a process crashes mid-switch), so it now asks first and should be used
  only if a reboot is not handy; prefer the reboot.
- With it on, the [V]/[E] mark shows in every application that has a status bar
  (non-fullscreen); it tracks Vietnamese/English there too.
- Kill switch, should applications misbehave and VnKey not open: in File manager, on the
  memory card, open the folder `VnKey` and make a new folder in it named `off`
  (Options > Organise > New folder), then restart. Applications then get no Telex from
  the plugin, only the phone's own input. Delete the folder to undo it.

Steps:

1. Install 0.1.x as before. Open VnKey: the title is `VnKey [V] m1`, the menu has
   Telex in all apps (off).
2. VnKey > Options > Telex in all apps. A note says it is on and the title ends in
   `all`. Note anything else that happens (an application closing, the phone restarting).
   If the phone misbehaves, restart it: that turns the plugin off.
3. Without closing VnKey, open Messaging > New message > Text message: type the table of
   the probe's step 2 below, each line then a space. Ctrl+Space there, a word, Ctrl+Space
   again. A long press and Fn + a letter after a word, as in step 5 below.
4. OttPilot: type the same in a chat.
5. Back in VnKey: Options > Send log. Then either Options > Telex in all apps to off, or
   restart the phone, unless Telex is to stay on.

Trigger key: VnKey writes `triggers.txt` next to `macros.txt` the first time, with
`ctrl+space`. To change or add a trigger (Ctrl+Space is also the phone's own predictive
toggle), edit `triggers.txt` on the phone — one key per line, modifiers and a key joined by
`+` (`ctrl` `shift` `fn` `alt`; keys `space` `enter` `tab` `bksp` or a scan code like
`0x5f`). Turn on Options > Log every key to read a key's scan code from the log. Then
Options > Reload macros/triggers (in VnKey; other apps pick it up when they next load the
plugin, e.g. after a reboot and turning Telex on again).

The log has the plugin's lines with the application's program name in front
(`<name>: plugin 0.1.1: sid <its SID>`, `<name>: catcher added`, then the key and
`rewrite` lines as in the probe); VnKey's own lines have no name. Keys typed into password fields are never logged.

## The probe (roadmap 1, app 0.0.x)

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
