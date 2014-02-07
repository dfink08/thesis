#!/bin/bash


sudo ifconfig eth0 down
sudo ifconfig eth0 mtu 552			#config eth0 for proper Max Segment Size
sudo ifconfig eth0 hw ether 00:e0:62:60:46:25 	#set mac address for interface.  interface MAC will be loaded in by weberver.py into config file to be used throughout emulator
sudo ifconfig eth0 up


#change the icmp rate limit from 1000 to 0, prevents the 1 pkt/sec limit which makes udp scans take forever
sudo echo "0">> /proc/sys/net/ipv4/icmp_ratelimit

#turn off TCP selective acknowledgment, important for fingerprint (shows up in 3-way handshake)
sudo echo "0">> /proc/sys/net/ipv4/tcp_sack

#turn off TCP window scaling, important for fingerprint (shows up in 3-way handshake)
sudo echo "0">> /proc/sys/net/ipv4/tcp_window_scaling

#turn off TCP timestamps, important for fingerprint (shows up in 3-way handshake)
sudo echo "0">> /proc/sys/net/ipv4/tcp_timestamps
