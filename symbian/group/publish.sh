#!/usr/bin/env bash
# Serves the package that docker-build.sh made to the phone's browser from this machine,
# on the local network only: copies it into symbian/web/ (git-ignored) as
# vnkey-<version>.sisx with a plain page that links it, and runs the web stub
# (webstub.py) on that folder over plain HTTP, port VNKEY_WEB_PORT (default 8060). The
# phone, on the same Wi-Fi, opens the address this prints. Nothing is uploaded anywhere.
# The stub also takes the phone's log (VnKey's Options > Send log, or its /upload page)
# into logs/ of the repository.
# A file name has the version in it, so the browser never installs an old one from its
# cache; packages served before stay in the folder. Ctrl+C stops the stub. When the port
# is in use already (the stub of an earlier run), only the files are renewed.
set -euo pipefail

repo="$(cd "$(dirname "$0")/../.." && pwd)"
port="${VNKEY_WEB_PORT:-8060}"
version="$(tr -d '\r\n' < "$repo/symbian/VERSION")"
sis="$repo/symbian/sis/vnkey.sisx"
if [ ! -f "$sis" ]; then
    echo "publish.sh: $sis is missing; run docker-build.sh first" >&2
    exit 1
fi

web="$repo/symbian/web"
mkdir -p "$web"
name="vnkey-$version.sisx"
cp "$sis" "$web/$name"
kb=$(( ($(wc -c < "$web/$name") + 1023) / 1024 ))
when="$(date '+%Y-%m-%d %H:%M')"

# HTML 4 with no script and no style sheet: the S60 3rd browser shows it as it is.
cat > "$web/index.html" <<EOF
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta name="viewport" content="width=device-width">
<title>VnKey $version</title>
</head>
<body>
<h3>VnKey $version</h3>
<p>Built $when.</p>
<p><a href="$name">Install</a><br>
$name, $kb KB</p>
<p>App manager: Software installation = All.</p>
<p><a href="/upload">Send a log</a> (or Options &gt; Send log in VnKey)</p>
</body>
</html>
EOF

# shellcheck source=lan.sh
. "$repo/symbian/group/lan.sh"
ip="$(lan_ip || true)"

url="http://${ip:-<LAN address of this Mac>}:$port/"

# A stub from an earlier run still serving the folder sees the new files at once.
if command -v lsof >/dev/null 2>&1 && lsof -nP -iTCP:"$port" -sTCP:LISTEN >/dev/null 2>&1; then
    echo "VnKey $version: port $port is already served (the stub from before); the page now offers $name"
    echo "On the phone: $url"
    exit 0
fi
echo "VnKey $version: on the phone open $url"
exec python3 -I "$repo/symbian/group/webstub.py" "$web" "$port" "$repo/logs"
