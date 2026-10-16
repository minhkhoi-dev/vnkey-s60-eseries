# Sourced by publish.sh and docker-build.sh: lan_ip prints this machine's address on the
# LAN, that of the interface with the default route, or nothing.
lan_ip() {
    if [ "$(uname)" = Darwin ]; then
        local iface
        iface="$(route -n get default 2>/dev/null | awk '/interface:/ { print $2 }')"
        [ -n "$iface" ] && ipconfig getifaddr "$iface" 2>/dev/null
    else
        ip -4 route get 1.1.1.1 2>/dev/null | sed -n 's/.* src \([0-9.]*\).*/\1/p' | head -n 1
    fi
}
