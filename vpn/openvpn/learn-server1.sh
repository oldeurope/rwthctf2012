#!/bin/bash

log() {
    logger -t "learn-server1[$$]" "$*"
}

# constants
DEVICE=tun1

# parameters from openvpn
ACTION="$1"
NET="$2"
NAME="$3"

if [[ "$#" < 2 ]]; then
    echo "usage: $0 ACTION NET [NAME]" >&2
    exit 1
fi

if ! echo "${NET}" | grep -q "/"; then
    #log "no network (${NET}), exiting"
    exit 0
fi

case "${ACTION}" in
    add|update)
        log "${DEVICE}: add/update ${NET} ${NAME}"
        sudo ip route replace "${NET}" dev "${DEVICE}"
        ;;
    delete)
        log "${DEVICE}: delete ${NET}"
        #ip route replace unreachable "${NET}"
        ;;
    *)
        log "unknown action ${ACTION}!"
        exit 1
        ;;
esac
