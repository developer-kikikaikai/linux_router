#!/bin/bash
##COMMON SETTING
IPTABLES=/sbin/iptables
SYSCTL=/sbin/sysctl
IFCONFIG=/sbin/ifconfig
IP=/sbin/ip
COUNTRY_BLACKLIST=/root/script/firewall_BLACKLIST.sh
#COUNTRY_WHITELIST=/root/script/firewall_WHITELIST.sh
WANIF=$1
LANIF=$2
SKIPLIST=$3
BROADCAST_WAN=`$IFCONFIG ${WANIF} | grep broadcast | awk -F" " '{print $6}'`
BROADCAST_LAN=`$IFCONFIG ${LANIF} | grep broadcast | awk -F" " '{print $6}'`

#reset firewall
$IPTABLES -F

#set base
$IPTABLES -P INPUT DROP
$IPTABLES -P OUTPUT ACCEPT
$IPTABLES -P FORWARD DROP

##############################
#ACCEPT CURRENT
#############################
## lo
$IPTABLES -A INPUT -i lo -j ACCEPT
$IPTABLES -A OUTPUT -o lo -j ACCEPT
$IPTABLES -A FORWARD -i lo -j ACCEPT
$IPTABLES -A FORWARD -o lo -j ACCEPT

## Connection
$IPTABLES -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
$IPTABLES -A OUTPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
$IPTABLES -A FORWARD -m state --state ESTABLISHED,RELATED -j ACCEPT

##############################
#REJECT
#############################

##############################
#IP Spoofing
$IPTABLES -A INPUT -i $WANIF -s 127.0.0.1/8 -j DROP
IPAREA=`$IP addr show ${LANIF} | grep "inet " | awk -F" " '{print $2}'`
LOCAL_IP=`echo $IPAREA | awk -F"/" '{print $1}' | awk -F"." '{print $1 "." $2 "."$3 ".0"}'`
LOCAL_AREA=`echo $IPAREA | awk -F"/" '{print $2}'`
$IPTABLES -A INPUT -i $WANIF -s ${LOCAL_IP}/${LOCAL_AREA} -j DROP

##############################
#ping attack (Large
PING_MAX=85
$IPTABLES -N PING_ATTACK > /dev/nul 2>&1
$IPTABLES -A PING_ATTACK -m length --length :${PING_MAX} -j ACCEPT
#if you want to save log
#$IPTABLES -A PING_ATTACK -j LOG --log-prefix "[IPTABLES PINGATTACK] : " --log-level=debug
$IPTABLES -A PING_ATTACK -j DROP
#ping attack (length
$IPTABLES -A PING_ATTACK -p icmp --icmp-type echo-request -m length --length :${PING_MAX} -m limit --limit 1/s --limit-burst 4 -j ACCEPT
#Add PING_ATTACK
$IPTABLES -A INPUT -p icmp --icmp-type echo-request -j PING_ATTACK

#Smurf attack
$IPTABLES -A INPUT -d 255.255.255.255 -j DROP
$IPTABLES -A INPUT -d 224.0.0.1 -j DROP
$IPTABLES -A INPUT -d ${BROADCAST_WAN} -j DROP
$IPTABLES -A INPUT -d ${BROADCAST_LAN} -j DROP

#Smurf forward
$SYSCTL -w net.ipv4.icmp_echo_ignore_broadcasts=1 > /dev/null

#SYN cookies
$SYSCTL -w net.ipv4.tcp_syncookies=1 > /dev/null

#reject Auth/IDENT
$IPTABLES -A INPUT -p tcp --dport 113 -j REJECT --reject-with tcp-reset

#rp_filte
sysctl -w net.ipv4.conf.${WANIF}.rp_filter=1 > /dev/null

#ICMP redirect 
sysctl -w net.ipv4.conf.${WANIF}.accept_redirects=0 > /dev/null

#Soruce route
sysctl -w net.ipv4.conf.${WANIF}.accept_source_route=0 > /dev/null

#Disable tcp timestamp
sysctl -w net.ipv4.tcp_timestamps=1 > /dev/null

#Create Blacklist
${COUNTRY_BLACKLIST}
$IPTABLES -A INPUT -j BLACKLIST

##############################
#ACCEPT
#############################
##local ssh
LOGIN=`cat /etc/ssh/sshd_config | grep '^#\?Port ' | tail -n 1 | sed -e 's/^[^0-9]*\([0-9]\+\).*$/\1/'`
$IPTABLES -A INPUT -p tcp -i ${LANIF} --dport $LOGIN -j ACCEPT

#Create Whitelist
#${COUNTRY_WHITELIST}

accept_port_tcp() {
	for port in $1
	do
		#if you want to accept only whitelist, please change ACCEPT to WHITELIST
		$IPTABLES -A INPUT -p tcp --dport $port -j ACCEPT
		$IPTABLES -A FORWARD -p tcp --dport $port -j ACCEPT
	done
}

accept_port_udp() {
	for port in $1
	do
		$IPTABLES -A INPUT -p udp --dport $port -j ACCEPT
		$IPTABLES -A FORWARD -p udp --dport $port -j ACCEPT
	done
}

PORT_UDP=
PORT_TCP=
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
$IPTABLES -t nat -F
$IPTABLES -t nat -A POSTROUTING -o $WANIF -j MASQUERADE
$SYSCTL -w net.ipv4.ip_forward=1 > /dev/null
