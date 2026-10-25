<p align="center"><img src="symbian/gfx/vnkey.svg" width="88" height="88" alt="VnKey"></p>

<h1 align="center">VnKey</h1>

<p align="center">
Vietnamese Telex typing in every app on the Nokia E72 (Symbian S60 3rd Edition FP2).<br>
<a href="https://minhkhoi-dev.github.io/vnkey-s60-eseries/en/">Website</a> ·
<a href="https://github.com/minhkhoi-dev/vnkey-s60-eseries/releases/latest">Latest release</a> ·
<a href="README.md">Tiếng Việt</a>
</p>

<p align="center"><a href="https://github.com/minhkhoi-dev/vnkey-s60-eseries/actions/workflows/build.yml"><img src="https://github.com/minhkhoi-dev/vnkey-s60-eseries/actions/workflows/build.yml/badge.svg" alt="Build"></a></p>

VnKey brings Telex, the typing method of Unikey, to Symbian. Press plain letters on the
QWERTY keyboard and Vietnamese appears in Messaging, the browser, chat apps and every other
text field.

## Features

- **Every app.** The phone loads a small plugin into each application, next to its own input
  method (AknFep). Shift, automatic capitals and the Fn keys work as always.
- **Ctrl+Space** switches between Vietnamese and English. A [V] or [E] mark sits in the
  bottom bar by the clock and hides in full-screen games and video. Other keys can be set in
  `triggers.txt`.
- **English words stay English.** With spell check on, a word that cannot be Vietnamese gets
  its keys back: `tẽt` → `text`, `ưindow` → `window`.
- **Shortcuts (gõ tắt):** `ko` → không, `dc` → được, `vn` → Việt Nam. Edit them in
  `macros.txt`.
- **Tone placement:** traditional (hòa, thúy, khỏe) or modern (hoà, thuý, khoẻ).
- **Starts with the phone** and turns Telex back on by itself. Settings screens in Vietnamese
  (the default) or English. Every letter is precomposed Unicode (NFC).

## Telex

| Keys | Letter | Key | Tone |
|------|--------|-----|------|
| `aa` | â | `s` | sắc: á |
| `aw` | ă | `f` | huyền: à |
| `ee` | ê | `r` | hỏi: ả |
| `oo` | ô | `x` | ngã: ã |
| `ow` | ơ | `j` | nặng: ạ |
| `uw`, `w` | ư | `z` | no tone |
| `dd` | đ | | |

Marks and tones can come at the end of the word (`toans` → toán, `vietej` → việt,
`nguoiwf` → người), the tone moves as the word grows (`hoafn` → hoàn), and a key pressed
twice takes its change back (`ass` → as).

## Install

You need:

- **A Nokia E72** (S60 3rd Edition Feature Pack 2, Symbian OS 9.3). VnKey is built for it and
  tested on it. Other FP2 phones with a QWERTY keyboard may work but have not been tried.
- **A hacked phone** (InstallServer and Open4All patches): the plugin loads into every app,
  and a self-signed package can only do that on a patched phone.

Steps:

1. Download `vnkey-x.y.z.sisx` from [Releases](https://github.com/minhkhoi-dev/vnkey-s60-eseries/releases/latest).
   On the phone, open the [lite page](https://minhkhoi-dev.github.io/vnkey-s60-eseries/m/en.html)
   in the phone's browser or Opera Mini.
2. Install it. Everything goes to the phone memory (C:), so VnKey keeps working with the
   memory card out.
3. Open VnKey and set **Telex mọi ứng dụng** (Telex in all apps) to **Bật** (On).
4. In Messaging, type `Vieejt` and a space: you get **Việt**.

GitHub takes TLS 1.2 only, and the stock S60 browser has TLS 1.0. To download on the phone
itself, install the [TLS patch](https://nnproject.cc/tls/), or download on a computer and
copy the file to the phone.

### Good to know

- **Turning it off:** set Telex in all apps to Off, answer No, and restart the phone.
  Switching it off on the spot can restart the phone, so the restart is the safe way.
- **Updating or removing:** turn it off as above and restart first.
- **Emergency off:** make a folder named `off` in `E:\VnKey\` (or `C:\Data\VnKey\`) and the
  plugin leaves every app alone.
- **Privacy:** the release keeps its log on the phone (`E:\VnKey\vnkey.log`) and sends nothing
  anywhere. Password fields are never logged.

## Building

The build runs in the public Docker image
[`minhkhoi4dev/s60v3-sdk-docker`](https://hub.docker.com/r/minhkhoi4dev/s60v3-sdk-docker)
(S60 3rd Edition FP2 SDK, GnuPoc, GCCE), on a Mac, Linux or Windows (WSL):

```sh
bash symbian/test/run.sh             # the Telex engine's tests, with the host's compiler
bash symbian/group/docker-build.sh   # -> symbian/sis/vnkey.sisx
bash symbian/group/publish.sh        # serves it to the phone over the Wi-Fi, from this machine
```

Every local build adds 1 to the build number in `symbian/VERSION`. Local builds also have
Options > Send log, which sends the log to `publish.sh` while testing on the phone.

GitHub Actions (`.github/workflows/build.yml`) runs the tests and the build for every push
that changes `symbian/`. That is a public build (`VNKEY_PUBLIC=1`): no Send log, the log
stays on the phone. To release a version, tag `v<VERSION>` (for example `v0.1.11`) and push
the tag; Actions makes a release with the `.sisx` and updates the website (`site/`,
`.github/workflows/pages.yml`).

## Source

| Folder | Contents |
|--------|----------|
| `symbian/src`, `symbian/inc` | The VnKey app, the `vnkeyfep.dll` plugin, the boot launcher, the Telex engine (`VnTelex.cpp`) |
| `symbian/data` | Resources and screen texts (`vnkey_vi.loc`, `vnkey_en.loc`) |
| `symbian/group` | Build files (`.mmp`), build and publish scripts |
| `symbian/test` | The Telex engine's tests, built with a computer's compiler |
| `site` | The website and the lite page for phones (`site/m/`) |
| `docs/TESTING.md` | Test steps on the phone |
| `CLAUDE.md` | Design, Telex rules, Symbian constraints and project status |

C++98 for GCCE 3.4.3: no STL, no exceptions. The engine (`TVnTelex`) is pure logic and
builds both for the phone and for a computer's tests.

---

A hobby project by [Khoi Nguyen](https://github.com/minhkhoi-dev). Not affiliated with Nokia,
HMD Global or the Unikey project.
