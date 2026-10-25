#!/usr/bin/env bash
# Builds vnkey.sisx in the SDK's Docker image (GnuPoc + GCCE + S60 3rd FP2 SDK, public on
# Docker Hub). The sources are copied into the container and built there, and the package,
# the signing keys and the bumped version are copied back, only on success, so that a
# failed build does not use up a version number. Works on a Mac (Docker Desktop, OrbStack)
# and in WSL alike:  bash symbian/group/docker-build.sh
set -euo pipefail
IMAGE="${VNKEY_SDK_IMAGE:-minhkhoi4dev/s60v3-sdk-docker:1.1.0}"

repo="$(cd "$(dirname "$0")/../.." && pwd)"

# The app's first address for Options > Send log: this machine's web stub
# (publish.sh); the container's own address would be of no use. A public build
# (VNKEY_PUBLIC=1) has no Send log.
if [ "${VNKEY_PUBLIC:-0}" = 1 ]; then
    VNKEY_LOG_SERVER=""
elif [ -z "${VNKEY_LOG_SERVER:-}" ]; then
    # shellcheck source=lan.sh
    . "$repo/symbian/group/lan.sh"
    ip="$(lan_ip || true)"
    if [ -n "$ip" ]; then
        VNKEY_LOG_SERVER="$ip:${VNKEY_WEB_PORT:-8060}"
    fi
fi
env_args=(-e "VNKEY_KEEP_VERSION=${VNKEY_KEEP_VERSION:-0}" -e "VNKEY_PUBLIC=${VNKEY_PUBLIC:-0}")
if [ -n "${VNKEY_LOG_SERVER:-}" ]; then
    env_args+=(-e "VNKEY_LOG_SERVER=$VNKEY_LOG_SERVER")
fi

docker run --rm "${env_args[@]}" -v "$repo":/src "$IMAGE" bash -c '
    set -euo pipefail
    mkdir /build
    cp -r /src/symbian /build/symbian
    bash /build/symbian/group/build.sh
    cd /build/symbian
    cp sis/vnkey.sisx sis/dev.key sis/dev.cer /src/symbian/sis/
    cp VERSION /src/symbian/VERSION
    cp inc/VnKeyVersion.h inc/VnKeyBuild.hrh /src/symbian/inc/
'
if [ "${VNKEY_PUBLIC:-0}" = 1 ]; then
    built_for="public, no Send log"
else
    built_for="log server ${VNKEY_LOG_SERVER:-default}"
fi
echo "Built $repo/symbian/sis/vnkey.sisx (version $(tr -d '\r\n' < "$repo/symbian/VERSION"), $built_for)"
