# lobot_util

A simple utility program to control HiWonder LX-D15 servo

```
Usage: lobot_util COMMAND [-i id] [-w VAL1[,VAL2] [-d port] [-h] [-v]

COMMAND:
	id                            Read/Write(-w new_id) servo ID
	pos                           Read/Write(-w angle,time) servo position
	offset                        Read/Write(-w new_offset) servo angle offset
	limit                         Read/Write(-w angle_min,angle_max) servo angle limit
	load                          Enable([-w 1])/Disable(-w 0) servo load output

Options:
	-i|--id id                    Target servo ID to communicate with, default 254(broadcast)
	-d|--device port              Serial port for Lobot servo, default /dev/tty/USB0
	-w|--write VAL1[,VAL2]        Write VAL1 [and VAL2 if applicable] to command

	-v|--version                  Version information
	-h|--help                     This message

When write option is not present, value corresponding to command will be read
and returned, otherwise a write will be performed instead of a read operation

Examples:
lobot_util id
  read servo ID on default port /dev/ttyUSB0
lobot_util id -w 2
  set servo ID to 2 for all servos on default port /dev/ttyUSB0
lobot_util id -i 1 -w 2 -d /dev/ttyUSB1
  change servo ID from 1 to 2 on port /dev/ttyUSB1
lobot_util pos -i 1 -w 20 -d /dev/ttyUSB1
  move servo (ID==1) on port /dev/ttyUSB1 to position 20
lobot_util -i 1 load -w 0
  disable(unload) servo (ID==1) output load
```
