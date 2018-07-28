#!/bin/sh
#http://nami.jp/ipv4bycc/がテキストの更新をしてくれている限りはこちらを使用
#サポートがきれてしまったら、 https://github.com/falsandtru/iptables-firewallを参考にして本家ARIN、RIPE NCC、APNIC、LACNIC、AfriNICからとるようにするのがベター

#curl result check
is_success_download() {
	local RES=`file $1 | grep "gzip compressed data"`
	if [ "x${RES}" = "x" ]; then
		return 0
	else
		return 1
	fi
}

IP_LIST_NEW=/tmp/cidr.txt
IP_LIST=/root/etc/cidr.txt
ACCEPT_COUNTLY="JP\|US"

#2017年Q3ベース 中国, (アメリカ), 韓国, ロシア, イギリス, オランダ, 香港
#アメリカは外すと色々とアクセスできないので除外
REJECT_COUNTLY="CN\|KR\|RO\|UK\|NL\|HK"

update_list() {
	curl -g "http://nami.jp/ipv4bycc/cidr.txt.gz" -o ${IP_LIST_NEW}.gz
	is_success_download ${IP_LIST_NEW}.gz
	if [ $? -ne 1 ]; then
		echo "Failed to get list."
		exit 1
	fi

	#to unzip it, remove .gz file
	gunzip -f ${IP_LIST_NEW}.gz

	#update ip
	mv ${IP_LIST_NEW} ${IP_LIST}
	chmod 400 ${IP_LIST}
}

get_accept_countryip() {
	#echo "ACCEPT_COUNTLY"
	if [ -s ${IP_LIST} ]; then
		sed -n "s/^\(${ACCEPT_COUNTLY}\)\t//p" ${IP_LIST}
	fi
}

get_reject_countryip() {
	#echo "REJECT_COUNTLY"
	if [ -s ${IP_LIST} ]; then
		sed -n "s/^\(${REJECT_COUNTLY}\)\t//p" ${IP_LIST}
	fi
}

if [ "x$1" = "x" ]; then
	update_list
else
	case $1 in
		--accept | -a) get_accept_countryip;;
		--reject | -r) get_reject_countryip;;
		*) get_accept_countryip;;
	esac
fi
