#!/bin/bash

sudo killall python
sudo killall infilter.o
sudo killall outfilter.o
sudo killall ecom_svr.o
sudo killall logging.o
echo "emulator stopped."

sudo iptables -F
sudo iptables -t mangle -F
echo "iptables rules flushed."
sudo iptables -L -v


