#!/bin/sh

### contrack tuning magic

# table size (default: 65536)
/sbin/sysctl -w net.netfilter.nf_conntrack_max=524288 > /dev/null
/sbin/sysctl -w net.ipv4.netfilter.ip_conntrack_max=524288 > /dev/null

# hash-table size (1/8 von table size)
echo 65536 > /sys/module/nf_conntrack/parameters/hashsize

# tcp timeout (default: 432000 = 5 days!)
/sbin/sysctl -w net.netfilter.nf_conntrack_tcp_timeout_established=1800 > /dev/null
/sbin/sysctl -w net.ipv4.netfilter.ip_conntrack_tcp_timeout_established=1800 > /dev/null
