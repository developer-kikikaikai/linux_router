#!/bin/bash
COUNTRY=/root/script/countryip.sh
#パスを通す
PATH=/bin:/sbin:/usr/bin:/usr/sbin

#Create Whitelist
iptables -N WHITELIST > /dev/nul 2>&1
iptables -F WHITELIST
WHITE_LIST=`$COUNTRY -a`
for address in $WHITE_LIST
do
	iptables -A WHITELIST -s $address -j ACCEPT
done
