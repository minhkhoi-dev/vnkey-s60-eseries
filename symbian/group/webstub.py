#!/usr/bin/env python3
"""The local web stub for the phone, started by publish.sh:

    python3 -I symbian/group/webstub.py <folder> <port> <logs folder>

Serves the files of <folder> (symbian/web/, made by publish.sh) over plain HTTP on every
address of this machine, so that the E72 on the same Wi-Fi can download the package. A
.sisx goes out as application/vnd.symbian.install, which makes the S60 browser hand it to
the installer, and nothing is cached. No directory listings.

It also takes the phone's test log, since the E72's log cannot be copied off it by cable or
Bluetooth: VnKey's Options > Send log POSTs it to /log as it is, and /upload is a form for
the phone's browser. Each one is saved in <logs folder> (the repository's logs/,
git-ignored) under a name of the stub's own, vnkey-<date>-<time>.log.

Each request is logged to the terminal. Ctrl+C stops it.
"""
import datetime
import functools
import http.server
import os
import sys

SYMBIAN_INSTALL = "application/vnd.symbian.install"
MAX_UPLOAD = 16 * 1024 * 1024

UPLOAD_FORM = b"""<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta name="viewport" content="width=device-width">
<title>Send a log</title>
</head>
<body>
<h3>Send a log</h3>
<form method="post" action="/upload" enctype="multipart/form-data">
<p>E:\\VnKey\\vnkey.log<br>
<input type="file" name="log"></p>
<p><input type="submit" value="Send"></p>
</form>
<p><a href="/">Back</a></p>
</body>
</html>
"""


class Handler(http.server.SimpleHTTPRequestHandler):
    extensions_map = dict(http.server.SimpleHTTPRequestHandler.extensions_map)
    extensions_map.update({
        ".sisx": SYMBIAN_INSTALL,
        ".sis": SYMBIAN_INSTALL,
        ".html": "text/html; charset=utf-8",
    })
    logs = "."

    def end_headers(self):
        self.send_header("Cache-Control", "no-cache")
        super().end_headers()

    def list_directory(self, path):
        self.send_error(404)
        return None

    def do_GET(self):
        if self.path == "/upload":
            self.reply(200, UPLOAD_FORM, "text/html; charset=utf-8")
            return
        super().do_GET()

    def do_POST(self):
        length = int(self.headers.get("Content-Length") or 0)
        if length <= 0 or length > MAX_UPLOAD:
            self.reply(400, b"no body, or too big\n")
            return
        body = self.rfile.read(length)
        if self.path == "/log":
            data = body
        elif self.path == "/upload":
            data = file_from_form(self.headers.get("Content-Type", ""), body)
            if data is None:
                self.reply(400, b"no file in the form\n")
                return
        else:
            self.reply(404, b"not here\n")
            return
        name = self.save(data)
        print(f"webstub: saved {name}, {len(data)} bytes", flush=True)
        if self.path == "/upload":
            page = f"<html><body><p>Saved {name}, {len(data)} bytes.</p><p><a href=\"/\">Back</a></p></body></html>"
            self.reply(200, page.encode(), "text/html; charset=utf-8")
        else:
            self.reply(200, f"saved {name} {len(data)}\n".encode())

    def save(self, data):
        os.makedirs(self.logs, exist_ok=True)
        stamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
        name = f"vnkey-{stamp}.log"
        n = 1
        while os.path.exists(os.path.join(self.logs, name)):
            n += 1
            name = f"vnkey-{stamp}-{n}.log"
        with open(os.path.join(self.logs, name), "wb") as f:
            f.write(data)
        return name

    def reply(self, code, body, ctype="text/plain; charset=utf-8"):
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)


def file_from_form(content_type, body):
    """The content of the form's file field "log" in a multipart/form-data body, or None."""
    boundary = None
    for param in content_type.split(";")[1:]:
        key, _, value = param.strip().partition("=")
        if key.lower() == "boundary":
            boundary = value.strip('"').encode()
    if not boundary:
        return None
    for part in body.split(b"--" + boundary):
        head, sep, content = part.partition(b"\r\n\r\n")
        if not sep or b'name="log"' not in head:
            continue
        if content.endswith(b"\r\n"):
            content = content[:-2]
        return content if content else None
    return None


def main():
    if len(sys.argv) != 4:
        sys.exit("usage: webstub.py <folder> <port> <logs folder>")
    folder, port, logs = sys.argv[1], int(sys.argv[2]), sys.argv[3]
    Handler.logs = logs
    handler = functools.partial(Handler, directory=folder)
    with http.server.ThreadingHTTPServer(("", port), handler) as server:
        print(f"webstub: serving {folder} on port {port}, logs to {logs}, Ctrl+C to stop", flush=True)
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            print("\nwebstub: stopped")


if __name__ == "__main__":
    main()
