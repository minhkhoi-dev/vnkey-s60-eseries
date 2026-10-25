<p align="center"><img src="symbian/gfx/vnkey.svg" width="88" height="88" alt="VnKey"></p>

<h1 align="center">VnKey</h1>

<p align="center">
Gõ tiếng Việt kiểu Telex trong mọi ứng dụng trên Nokia E72 (Symbian S60 3rd Edition FP2).<br>
<a href="https://minhkhoi-dev.github.io/vnkey-s60-eseries/">Trang giới thiệu</a> ·
<a href="https://github.com/minhkhoi-dev/vnkey-s60-eseries/releases/latest">Tải bản mới nhất</a> ·
<a href="README.en.md">English</a>
</p>

<p align="center"><a href="https://github.com/minhkhoi-dev/vnkey-s60-eseries/actions/workflows/build.yml"><img src="https://github.com/minhkhoi-dev/vnkey-s60-eseries/actions/workflows/build.yml/badge.svg" alt="Build"></a></p>

VnKey mang kiểu gõ Telex của Unikey lên Symbian. Chỉ cần gõ các phím chữ trên bàn phím
QWERTY, chữ Việt có dấu hiện ra ngay trong Tin nhắn, trình duyệt, ứng dụng chat và mọi ô
nhập chữ khác.

## Tính năng

- **Mọi ứng dụng.** Một plugin nhỏ được máy nạp vào từng ứng dụng, cạnh bộ gõ có sẵn của
  máy (AknFep). Shift, tự viết hoa đầu câu và phím Fn vẫn hoạt động như cũ.
- **Ctrl+Space** chuyển qua lại giữa tiếng Việt và tiếng Anh. Dấu [V] hoặc [E] nằm ở thanh
  dưới cạnh đồng hồ và tự ẩn khi chơi game hay xem video toàn màn hình. Có thể đặt phím
  khác trong `triggers.txt`.
- **Từ tiếng Anh vẫn là tiếng Anh.** Với kiểm tra chính tả, từ không thể là tiếng Việt được
  trả lại đúng các phím đã gõ: `tẽt` → `text`, `ưindow` → `window`.
- **Gõ tắt:** `ko` → không, `dc` → được, `vn` → Việt Nam. Sửa trong `macros.txt`.
- **Kiểu bỏ dấu:** kiểu cũ (hòa, thúy, khỏe) hoặc kiểu mới (hoà, thuý, khoẻ).
- **Tự chạy khi bật máy** và tự bật lại Telex. Màn hình cài đặt bằng tiếng Việt (mặc định)
  hoặc tiếng Anh. Mọi chữ là Unicode dựng sẵn (NFC).

## Gõ Telex

| Phím | Chữ | Phím | Dấu |
|------|-----|------|-----|
| `aa` | â | `s` | sắc: á |
| `aw` | ă | `f` | huyền: à |
| `ee` | ê | `r` | hỏi: ả |
| `oo` | ô | `x` | ngã: ã |
| `ow` | ơ | `j` | nặng: ạ |
| `uw`, `w` | ư | `z` | xóa dấu |
| `dd` | đ | | |

Dấu có thể gõ ở cuối từ (`toans` → toán, `vietej` → việt, `nguoiwf` → người), dấu tự dời
chỗ khi từ dài thêm (`hoafn` → hoàn), gõ hai lần để bỏ (`ass` → as).

## Cài đặt

Cần có:

- **Nokia E72** (S60 3rd Edition Feature Pack 2, Symbian OS 9.3). VnKey được viết và chạy
  thử trên máy này. Các máy FP2 có bàn phím QWERTY khác có thể chạy được nhưng chưa được thử.
- **Máy đã hack** (vá InstallServer và Open4All): plugin được nạp vào mọi ứng dụng, và file
  tự ký chỉ làm được điều đó trên máy đã vá.

Các bước:

1. Tải `vnkey-x.y.z.sisx` ở [Releases](https://github.com/minhkhoi-dev/vnkey-s60-eseries/releases/latest).
   Trên điện thoại, mở [bản nhẹ của trang](https://minhkhoi-dev.github.io/vnkey-s60-eseries/m/)
   bằng trình duyệt của máy hoặc Opera Mini.
2. Cài đặt. Mọi thứ vào bộ nhớ máy (C:), nên VnKey vẫn chạy khi rút thẻ nhớ.
3. Mở VnKey, chọn **Telex mọi ứng dụng: Bật**.
4. Vào Tin nhắn, gõ `Vieejt` và dấu cách: được **Việt**.

GitHub chỉ nhận TLS 1.2, còn trình duyệt gốc của S60 chỉ có TLS 1.0. Muốn tải trực tiếp trên
điện thoại thì cài [bản vá TLS](https://nnproject.cc/tls/), hoặc tải bằng máy tính rồi chép
sang điện thoại.

### Cần biết

- **Tắt:** chọn Telex mọi ứng dụng: Tắt, trả lời Không, rồi khởi động lại máy. Tắt ngay lập
  tức có thể làm máy tự khởi động lại, nên khởi động lại là cách an toàn.
- **Cập nhật hoặc gỡ cài đặt:** tắt như trên và khởi động lại máy trước.
- **Tắt khẩn cấp:** tạo thư mục `off` trong `E:\VnKey\` (hoặc `C:\Data\VnKey\`); plugin sẽ
  không đụng tới ứng dụng nào.
- **Riêng tư:** bản phát hành chỉ lưu nhật ký trên máy (`E:\VnKey\vnkey.log`) và không gửi gì
  đi đâu. Ô mật khẩu không bao giờ được ghi lại.

## Build từ mã nguồn

Việc build chạy trong Docker image công khai
[`minhkhoi4dev/s60v3-sdk-docker`](https://hub.docker.com/r/minhkhoi4dev/s60v3-sdk-docker)
(SDK S60 3rd Edition FP2, GnuPoc, GCCE), trên Mac, Linux hoặc Windows (WSL):

```sh
bash symbian/test/run.sh             # kiểm thử bộ gõ Telex, bằng trình biên dịch của máy
bash symbian/group/docker-build.sh   # -> symbian/sis/vnkey.sisx
bash symbian/group/publish.sh        # mở file cho điện thoại tải qua Wi-Fi, từ máy này
```

Mỗi lần build trên máy, số build trong `symbian/VERSION` tăng 1. Bản build trên máy có thêm
Tùy chọn > Gửi nhật ký để gửi nhật ký về `publish.sh` khi thử máy.

GitHub Actions (`.github/workflows/build.yml`) chạy kiểm thử và build mỗi lần push thay đổi
trong `symbian/`. Đó là bản
công khai (`VNKEY_PUBLIC=1`): không có Gửi nhật ký, nhật ký chỉ nằm trên máy. Phát hành một
phiên bản: gắn tag `v<VERSION>` (ví dụ `v0.1.11`) rồi push; Actions tạo release kèm file
`.sisx` và cập nhật trang giới thiệu (`site/`, `.github/workflows/pages.yml`).

## Mã nguồn

| Thư mục | Nội dung |
|---------|----------|
| `symbian/src`, `symbian/inc` | Ứng dụng VnKey, plugin `vnkeyfep.dll`, trình khởi động lúc bật máy, bộ gõ Telex (`VnTelex.cpp`) |
| `symbian/data` | Tài nguyên, chữ trên màn hình (`vnkey_vi.loc`, `vnkey_en.loc`) |
| `symbian/group` | File build (`.mmp`), script build và phát hành |
| `symbian/test` | Kiểm thử bộ gõ Telex, chạy bằng trình biên dịch của máy tính |
| `site` | Trang giới thiệu và bản nhẹ cho điện thoại (`site/m/`) |
| `docs/TESTING.md` | Các bước thử trên điện thoại |
| `CLAUDE.md` | Thiết kế, quy tắc Telex, ràng buộc của Symbian và tình trạng dự án |

Code C++98 cho GCCE 3.4.3: không STL, không exception. Bộ gõ (`TVnTelex`) là logic thuần,
build được cả cho điện thoại lẫn máy tính để kiểm thử.

---

Dự án cá nhân của [Khoi Nguyen](https://github.com/minhkhoi-dev). Không liên quan đến Nokia,
HMD Global hay dự án Unikey.
