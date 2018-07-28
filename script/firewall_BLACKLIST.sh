#!/bin/bash
COUNTRY=/root/script/countryip.sh
#パスを通す
PATH=/bin:/sbin:/usr/bin:/usr/sbin

#Create Blacklist
iptables -N BLACKLIST > /dev/nul 2>&1
iptables -F BLACKLIST
BLACKLIST=`$COUNTRY -r`
for address in ${BLACKLIST}
do
	iptables -A BLACKLIST -s $address -j DROP
done
