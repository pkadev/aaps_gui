#!/bin/sh 
sudo avrdude -c $1 -p m88 -P usb -U flash:w:main.hex -B10
#avrispv2
#dragon_isp
#jtag2isp
