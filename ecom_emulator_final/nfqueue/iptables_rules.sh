#!/bin/bash
INTERFACE_INDUSTRIAL=eth0
INTERFACE_LOGGING=eth1

sudo iptables -t filter --flush
sudo iptables -t mangle --flush

#drop broadcasts so they dont get logged
#sudo iptables -A INPUT -d 255.255.255.255 -j DROP -i $INTERFACE_INDUSTRIAL

#logging rules (using a queue)
sudo iptables -t mangle -A INPUT  -m mark ! --mark 0x2/2 -j NFQUEUE --queue-num 3 -i $INTERFACE_INDUSTRIAL
#sudo iptables -t mangle -A OUTPUT -m mark ! --mark 0x2/2 -j NFQUEUE --queue-num 3 -o $INTERFACE_INDUSTRIAL

#mangle table rules for outgoing packets for web and hap traffic, queue 2 is out_filter.c (made into outfilter.o) (this only modifies IPID to always equal 0x0000)
sudo iptables -t mangle -A OUTPUT -p tcp --sport 80 -m mark ! --mark 0x1/1 -m tcp -j NFQUEUE --queue-num 2 -o $INTERFACE_INDUSTRIAL
sudo iptables -t mangle -A OUTPUT -p tcp --sport 80 -m mark --mark 0x1/1 -m tcp -j ACCEPT -o $INTERFACE_INDUSTRIAL
sudo iptables -t mangle -A OUTPUT -p udp --sport 28784 -m mark ! --mark 0x1/1 -m udp -j NFQUEUE --queue-num 2 -o $INTERFACE_INDUSTRIAL
sudo iptables -t mangle -A OUTPUT -p udp --sport 28784 -m mark --mark 0x1/1 -m udp -j ACCEPT -o $INTERFACE_INDUSTRIAL


#set all TTLs for outgoing packets to 255 to match PLC
sudo iptables -t mangle -A POSTROUTING -j TTL --ttl-set 255 -o $INTERFACE_INDUSTRIAL
#logging rule (using a queue) postrouting so we can capture ICMP messages also (important for nmap scans)
sudo iptables -t mangle -A POSTROUTING -m mark ! --mark 0x2/2 -j NFQUEUE --queue-num 3 -o $INTERFACE_INDUSTRIAL


#rules for logging interface:

#accept ssh connections
sudo iptables -A INPUT -p tcp --dport 22 -m tcp -j ACCEPT -i $INTERFACE_LOGGING

#filter table rules for incoming packets, queue 1 is in_filter_allports.c (made into infilterall.o)
#TCP packet rules:
sudo iptables -A INPUT -p tcp --dport 80 -m tcp -j NFQUEUE --queue-num 1 -i $INTERFACE_INDUSTRIAL
sudo iptables -A INPUT -p tcp --dport 1 -m tcp -j NFQUEUE --queue-num 1 -i $INTERFACE_INDUSTRIAL
sudo iptables -A INPUT -p tcp --sport 502 -m tcp -j ACCEPT
sudo iptables -A INPUT -p tcp --dport 502 -m tcp -j ACCEPT
sudo iptables -A INPUT -p tcp -j REJECT --reject-with tcp-reset -i $INTERFACE_INDUSTRIAL

#non TCP packet rules:
sudo iptables -A INPUT -p udp --dport 28784 -m udp -j ACCEPT -i $INTERFACE_INDUSTRIAL
sudo iptables -A INPUT -p icmp -m icmp -j NFQUEUE --queue-num 1 -i $INTERFACE_INDUSTRIAL
sudo iptables -A INPUT -p udp --dport 1 -m udp -j NFQUEUE --queue-num 1 -i $INTERFACE_INDUSTRIAL
sudo iptables -A INPUT -p udp -j REJECT --reject-with icmp-port-unreachable -i $INTERFACE_INDUSTRIAL




#sudo iptables -L -v
