#!/bin/bash
#usge: ./firewall.sh wanif lanif
##COMMON SETTING
if [ $# != 2 ]; then
	echo "Usage: $0 <wanif> <lanif>"
	exit 1
fi

COUNTRY_BLACKLIST=/root/script/firewall_BLACKLIST.sh
PATH=/bin:/sbin:/usr/bin:/usr/sbin

#COUNTRY_WHITELIST=/root/script/firewall_WHITELIST.sh
WANIF=$1
LANIF=$2
SKIPLIST=$3
BROADCAST_WAN=`ifconfig ${WANIF} | grep broadcast | awk -F" " '{print $6}'`
BROADCAST_LAN=`ifconfig ${LANIF} | grep broadcast | awk -F" " '{print $6}'`

#reset firewall
iptables -F INPUT
iptables -F OUTPUT
iptables -F FORWARD

#set base
iptables -P INPUT DROP
iptables -P OUTPUT ACCEPT
iptables -P FORWARD DROP

##############################
#ACCEPT CURRENT
#############################
## lo
iptables -A INPUT -i lo -j ACCEPT
iptables -A OUTPUT -o lo -j ACCEPT
iptables -A FORWARD -i lo -j ACCEPT
iptables -A FORWARD -o lo -j ACCEPT

## Connection
iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
iptables -A OUTPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
iptables -A FORWARD -m state --state ESTABLISHED,RELATED -j ACCEPT

##############################
#REJECT
#############################

##############################
#IP Spoofing
iptables -A INPUT -i $WANIF -s 127.0.0.1/8 -j DROP
IPAREA=`ip addr show ${LANIF} | grep "inet " | awk -F" " '{print $2}'`
LOCAL_IP=`echo $IPAREA | awk -F"/" '{print $1}' | awk -F"." '{print $1 "." $2 "."$3 ".0"}'`
LOCAL_AREA=`echo $IPAREA | awk -F"/" '{print $2}'`
iptables -A INPUT -i $WANIF -s ${LOCAL_IP}/${LOCAL_AREA} -j DROP

##############################
#ping attack (Large
PING_MAX=85
iptables -N PING_ATTACK > /dev/nul 2>&1
iptables -F PING_ATTACK
iptables -A PING_ATTACK -m length --length :${PING_MAX} -j ACCEPT
#if you want to save log
iptables -A PING_ATTACK -j LOG --log-prefix "ping_attack : "
iptables -A PING_ATTACK -j DROP
#ping attack (length
iptables -A PING_ATTACK -p icmp --icmp-type echo-request -m length --length :${PING_MAX} -m limit --limit 1/s --limit-burst 4 -j ACCEPT
#Add PING_ATTACK
iptables -A INPUT -p icmp --icmp-type echo-request -j PING_ATTACK

##############################
#Smurf attack
iptables -A INPUT -i ${WANIF} -d 255.255.255.255 -j DROP
iptables -A INPUT -i ${WANIF} -d 224.0.0.1 -j DROP
iptables -A INPUT -i ${WANIF} -d ${BROADCAST_WAN} -j DROP
iptables -A INPUT -i ${WANIF} -d ${BROADCAST_LAN} -j DROP

##############################
#Smurf forward
sysctl -w net.ipv4.icmp_echo_ignore_broadcasts=1 > /dev/null

##############################
#SYN Flood Attack
sysctl -w net.ipv4.tcp_syncookies=1 > /dev/null

##############################
#reject Auth/IDENT
iptables -A INPUT -p tcp --dport 113 -j REJECT --reject-with tcp-reset

##############################
#rp_filte
sysctl -w net.ipv4.conf.${WANIF}.rp_filter=1 > /dev/null

##############################
#ICMP redirect 
sysctl -w net.ipv4.conf.${WANIF}.accept_redirects=0 > /dev/null

##############################
#Soruce route
sysctl -w net.ipv4.conf.${WANIF}.accept_source_route=0 > /dev/null

##############################
#Disable tcp timestamp
sysctl -w net.ipv4.tcp_timestamps=1 > /dev/null

###########################################################
#Stealth Scan
iptables -N STEALTH_SCAN > /dev/nul 2>&1
iptables -F STEALTH_SCAN
iptables -A STEALTH_SCAN -j LOG --log-prefix "stealth_scan_attack: "
iptables -A STEALTH_SCAN -j DROP

#janp packet stealth scan to STEALTH_SCAN
iptables -A INPUT -p tcp --tcp-flags SYN,ACK SYN,ACK -m state --state NEW -j STEALTH_SCAN
iptables -A INPUT -p tcp --tcp-flags ALL NONE -j STEALTH_SCAN

iptables -A INPUT -p tcp --tcp-flags SYN,FIN SYN,FIN         -j STEALTH_SCAN
iptables -A INPUT -p tcp --tcp-flags SYN,RST SYN,RST         -j STEALTH_SCAN
iptables -A INPUT -p tcp --tcp-flags ALL SYN,RST,ACK,FIN,URG -j STEALTH_SCAN

iptables -A INPUT -p tcp --tcp-flags FIN,RST FIN,RST -j STEALTH_SCAN
iptables -A INPUT -p tcp --tcp-flags ACK,FIN FIN     -j STEALTH_SCAN
iptables -A INPUT -p tcp --tcp-flags ACK,PSH PSH     -j STEALTH_SCAN
iptables -A INPUT -p tcp --tcp-flags ACK,URG URG     -j STEALTH_SCAN

#Create Blacklist
#${COUNTRY_BLACKLIST}
iptables -A BLACKLIST -j LOG --log-prefix "blacklist_access: "
iptables -A INPUT -j BLACKLIST

##############################
#ACCEPT
#############################
##local ssh
LOGIN=`cat /etc/ssh/sshd_config | grep '^#\?Port ' | tail -n 1 | sed -e 's/^[^0-9]*\([0-9]\+\).*$/\1/'`
iptables -A INPUT -p tcp -i ${LANIF} --dport $LOGIN -j ACCEPT

#Create Whitelist
#${COUNTRY_WHITELIST}

accept_port_tcp() {
	for port in $1
	do
		#if you want to accept only whitelist, please change ACCEPT to WHITELIST
		iptables -A INPUT -p tcp --dport $port -j ACCEPT
		iptables -A FORWARD -p tcp --dport $port -j ACCEPT
	done
}

accept_port_udp() {
	for port in $1
	do
		iptables -A INPUT -p udp --dport $port -j ACCEPT
		iptables -A FORWARD -p udp --dport $port -j ACCEPT
	done
}

PORT_UDP=
PORT_TCP=

#DHCP
PORT_UDP+="67 68 "

#DNS
PORT_UDP+="53 "
PORT_TCP+="53 "

##http setting
#http
PORT_TCP+="80 "
#https
PORT_TCP+="443 "

##main setting
#smtp
PORT_TCP+="25 "
#smtps
PORT_TCP+="465 "
#pop3
PORT_TCP+="110 "
#pop3s
PORT_TCP+="995 "

accept_port_tcp "$PORT_TCP"
accept_port_udp "$PORT_UDP"

##############################
#Forwarding
#############################
iptables -t nat -F
iptables -t nat -A POSTROUTING -o $WANIF -j MASQUERADE
sysctl -w net.ipv4.ip_forward=1 > /dev/null
