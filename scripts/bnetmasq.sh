#!/bin/sh

#	Author: 	Marco Ziech (mmz@gmx.net)
#	Date:		Jan-24-2000
#
# Bnetd UDP masquerade for Linux 
# 
# NOTE:
#       This script is not needed if you have "loose UDP" turned
#       on.  It was a patch around the 2.2 series. It automates
#       all of this for any "well behaved" peer-to-peer games,
#       not just for Battle.net clients.
#
# Kernel:
# 	This script should work fine with 2.2 kernels but I don't
# 	know if it works with older kernels too. 
#
# Installation:
#	Copy this file into your local init.d directory.
#	The position of this directory may vary with diffrent
#	linux distributions. 
#	On Debian/GNU Linux it's /etc/init.d
#	On S.u.S.E. Linux it's /sbin/init.d		(I guess)
#	To start the script on system startup you have to
#	make a symlink to this file from a filename like 
#	"S99bnetmasq.sh" in the rc2.d directory. To stop it
#	on system shutdown do the same with "K01bnetmasq.sh"
#	in the rc0.d, rc1.d and rc6.d directories. These
#	directories are usually in the same as the init.d
#	directory.
#	If you have a dial-up connection and want to start the
#	script on dial-in you have to put a script which starts
#	or stops this script in /etc/ppp/ip-up.d and 
#	/etc/ppp/ip-down.d .
#
# Using on original Bnet:
#	This script should also work with the original Battle.net.
#
# Using on servers not running on your masquerading gateway:
#       Outside servers like the real Battle.net will not
#       know your correct address, only your internal IP:port.
#       to fix it, either bnproxy needs to be improved to
#       be used instead of ipmasq or this need to do some
#       content altering like with the ftp script.
#              --- Thanks to Ross Combs for these corrections.

# This is the destination port on the clients. It is usually 6112.
BNET_PORT=6112

# This is the list of the destination hosts with their ports on
# the firewall. The list should look like this:
# client1IP:GW1port client2IP:GW2port ... clientNIP:GWNport
# Remember that each port can only be allocated by one client.
# Some ports may already be reserved by some programs.
# I recomment using ports between 5000 and 7000. See 
# /etc/services on information about assigned ports.
REDIR_LIST="192.168.1.1:5000 192.168.1.2:5001"

# This is the external interface i.e. the interface which is
# connected to the Internet. On dial-up links this is usually
# ppp0 or ippp0 (for ISDN).
EXTERNAL_IF=eth0

# ------------------- END OF CONFIG SECTION -----------------

# This determines the ip of the external interface
EXTERNAL_IP=`LANGUAGE="C" LC_ALL="C" ifconfig $EXTERNAL_IF | grep 'inet addr:' | sed 's/.*inet addr:\([0-9.]*\).*/\1/g'`

case "$1" in
	start)
		echo -n "Starting bnet masquerading: "
		for i in $REDIR_LIST; do
			echo -n "$i "
			D_HOST="`echo $i | sed -e 's/:/ /g' | awk '{ print $1 }'`" 
			D_PORT="`echo $i | sed -e 's/:/ /g' | awk '{ print $2 }'`"
			ipmasqadm portfw -a -P udp -L $EXTERNAL_IP $D_PORT -R $D_HOST $BNET_PORT
# FIXME: according to: http://www.mail-archive.com/masq@tori.indyramp.com/msg01538.html
#> setting up a single machine would look like:
#> # ipmasqadm portfw -a -P tcp -L $1 6112 -R $2 6112
#> # ipmasqadm portfw -a -P tcp -L $3 6112 -R $2 6112
#> # ipmasqadm portfw -a -P udp -L $1 6112 -R $2 6112
#> # ipmasqadm portfw -a -P udp -L $3 6112 -R $2 6112
#> Replace $1 with the internal IP address of your gateway machine, $2
#> with the address (internal again, but it should be the only) of the
#> machine you're playing starcraft on and $3 with the external IP ad-
#> dress of the gateway machine.
# This implies that
# inside_gw --> client
# outside_gw --> inside_gw
# this script doesn't do the gw --> gw setup.... does it need to?!

		done
		echo "."
		;;
	stop)
		echo -n "Stopping bnet masquerading: "
		for i in $REDIR_LIST; do
			echo -n "$i "
			D_PORT="`echo $i | sed -e 's/:/ /g' | awk '{ print $2 }'`"
			ipmasqadm portfw -d -P udp -L $EXTERNAL_IP $D_PORT
		done
		echo "."
		;;

	*)
		echo "Usage: $0 {start|stop}"
		exit 1
		;;
esac
