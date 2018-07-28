#!/bin/bash
COUNTRY=/root/script/countryip.sh
IPTABLES=/sbin/iptables

#Create Whitelist
$IPTABLES -N WHITELIST > /dev/nul 2>&1
$IPTABLES -F WHITELIST
WHITE_LIST=`$COUNTRY -a`
for address in $WHITE_LIST
do
	$IPTABLES -A WHITELIST -s $address -j ACCEPT
done
