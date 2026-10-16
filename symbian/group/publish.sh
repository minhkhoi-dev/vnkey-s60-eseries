#!/usr/bin/env bash
# Puts the package that docker-build.sh made on the web for the phone's browser:
# vnkey-<version>.sisx and a plain page that links it, in VNKEY_DL_DIR on the ssh host
# VNKEY_DL_SSH (symbian/build.local). That is meant to be the folder vnkey/ inside the one
# where OttPilot's publish.sh puts its packages: OttPilot's nginx serves it at
# http://<host>/s60/vnkey/, with the type application/vnd.symbian.install that makes the
# S60 browser hand a .sisx to the installer (OttPilot's server/deploy/README.md,
# "Downloads for the phone"). The script refuses a folder with OttPilot's packages in it,
# whose index.html it would replace. A file name has the version in it, so the browser
# never gets an old one from its cache; packages published before stay there.
set -euo pipefail

repo="$(cd "$(dirname "$0")/../.." && pwd)"
if [ -f "$repo/symbian/build.local" ]; then
    # shellcheck disable=SC1091
    . "$repo/symbian/build.local"
fi
: "${VNKEY_DL_SSH:?set VNKEY_DL_SSH in symbian/build.local}"
: "${VNKEY_DL_DIR:?set VNKEY_DL_DIR in symbian/build.local}"
if [[ $VNKEY_DL_DIR == *\'* ]]; then
    echo "publish.sh: VNKEY_DL_DIR must not contain a quote" >&2
    exit 1
fi

version="$(tr -d '\r\n' < "$repo/symbian/VERSION")"
sis="$repo/symbian/sis/vnkey.sisx"
if [ ! -f "$sis" ]; then
    echo "publish.sh: $sis is missing; run docker-build.sh first" >&2
    exit 1
fi

if ssh "$VNKEY_DL_SSH" "ls '$VNKEY_DL_DIR'/ottpilot-*.sisx" >/dev/null 2>&1; then
    echo "publish.sh: $VNKEY_DL_DIR has OttPilot's packages; use a folder of VnKey's own (e.g. .../vnkey)" >&2
    exit 1
fi

stage="$(mktemp -d)"
trap 'rm -rf "$stage"' EXIT
name="vnkey-$version.sisx"
cp "$sis" "$stage/$name"
kb=$(( ($(wc -c < "$stage/$name") + 1023) / 1024 ))
when="$(date '+%Y-%m-%d %H:%M')"

# HTML 4 with no script and no style sheet: the S60 3rd browser shows it as it is.
cat > "$stage/index.html" <<EOF
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
</body>
</html>
EOF

ssh "$VNKEY_DL_SSH" "mkdir -p '$VNKEY_DL_DIR'"
scp -q "$stage/$name" "$stage/index.html" "$VNKEY_DL_SSH:$VNKEY_DL_DIR/"
echo "Published VnKey $version to $VNKEY_DL_SSH:$VNKEY_DL_DIR"
echo "On the phone: http://<host>/s60/vnkey/"
