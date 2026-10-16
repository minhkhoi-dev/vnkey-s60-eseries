#!/usr/bin/env bash
# Builds vnkey.sisx in the SDK's Docker image (GnuPoc + GCCE + S60 3rd FP2 SDK, public on
# Docker Hub). The sources are copied into the container and built there, and the package,
# the signing keys and the bumped version are copied back, only on success, so that a
# failed build does not use up a version number. Works on a Mac (Docker Desktop, OrbStack)
# and in WSL alike:  bash symbian/group/docker-build.sh
set -euo pipefail
IMAGE="${VNKEY_SDK_IMAGE:-minhkhoi4dev/s60v3-sdk-docker:1.1.0}"

repo="$(cd "$(dirname "$0")/../.." && pwd)"

docker run --rm -v "$repo":/src "$IMAGE" bash -c '
    set -euo pipefail
    mkdir /build
    cp -r /src/symbian /build/symbian
    bash /build/symbian/group/build.sh
    cd /build/symbian
    cp sis/vnkey.sisx sis/dev.key sis/dev.cer /src/symbian/sis/
    cp VERSION /src/symbian/VERSION
    cp inc/VnKeyVersion.h /src/symbian/inc/VnKeyVersion.h
'
echo "Built $repo/symbian/sis/vnkey.sisx (version $(tr -d '\r\n' < "$repo/symbian/VERSION"))"
