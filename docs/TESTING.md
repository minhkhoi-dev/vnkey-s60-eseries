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
