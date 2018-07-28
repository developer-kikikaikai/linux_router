#!/bin/bash
COUNTRY=/root/script/countryip.sh
IPTABLES=/sbin/iptables

#Create Blacklist
$IPTABLES -N BLACKLIST > /dev/nul 2>&1
$IPTABLES -F BLACKLIST
BLACKLIST=`$COUNTRY -r`
for address in ${BLACKLIST}
do
	$IPTABLES -A BLACKLIST -s $address -j DROP
done
