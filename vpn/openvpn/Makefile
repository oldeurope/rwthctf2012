.PHONY: default openvpn all

default:
	@echo "TARGETS: server*.conf openvpn learn all"

all: openvpn learn

openvpn: server1.conf server2.conf server3.conf server4.conf server5.conf server6.conf server7.conf server8.conf

learn: learn-server1.sh learn-server2.sh learn-server3.sh learn-server4.sh learn-server5.sh learn-server6.sh learn-server7.sh learn-server8.sh

server%.conf: templates/server.conf.template templates/generate
	templates/generate $< $* > $@

learn-server%.sh: templates/learn-server.sh.template templates/generate
	templates/generate $< $* > $@
	chmod +x $@
