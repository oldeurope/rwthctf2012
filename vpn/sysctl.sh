#!/bin/sh

### contrack tuning magic

# table size (default: 65536)
sysctl -w net.netfilter.nf_conntrack_max=4194304> /dev/null
sysctl -w net.ipv4.netfilter.ip_conntrack_max=4194304 > /dev/null
sysctl -w net.netfilter.nf_conntrack_expect_max=8192 > /dev/null

# hash-table size (1/8 von table size)
echo 65536 > /sys/module/nf_conntrack/parameters/hashsize

# tcp timeout (default: 432000 = 5 days!)
sysctl -w net.netfilter.nf_conntrack_tcp_timeout_established=1800 > /dev/null
sysctl -w net.ipv4.netfilter.ip_conntrack_tcp_timeout_established=1800 > /dev/null

# increase garbage collection levels
sysctl -w net.ipv4.neigh.default.gc_thresh1=1024 > /dev/null
sysctl -w net.ipv4.neigh.default.gc_thresh2=2048 > /dev/null
sysctl -w net.ipv4.neigh.default.gc_thresh3=4096 > /dev/null

# enable ip_forward
sysctl -w net.ipv4.ip_forward=1

# disable ipv6
sysctl -w net.ipv6.conf.all.disable_ipv6=1

# Do not accept ICMP redirects (prevent MITM attacks)
net.ipv4.conf.all.accept_redirects = 0
net.ipv6.conf.all.accept_redirects = 0
