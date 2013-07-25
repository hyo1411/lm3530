## `lm3530_bl_remap.ko` ##

A kernel module to reduce back-light brightness for LG Optimus LTE III (Optimus L7) 


In this phone it looks light screen too bright even the backlight brightness is set to zero. That is very uncomfortable at the night-time. There is some apps that can use to set backlight brightness to a "negative" value but seems that they just adjust the brightness in the frame buffer before sending to screen. It's not saving your battery life in this case.

Modifying kernel to do that could be easy but this phone locks bootloader so that this kernel module is made to this purpose. It will remap the value that send to lm3530 chip to control the screen brightness.

#Install


	#su
	#insmod lm3530_bl_remmap.ko

Or putting in init script that run at booting.

Or you can try this app http://forum.xda-developers.com/showthread.php?t=1228605