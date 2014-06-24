FullHC05m.ino Full demo program for the HC05 master library.

This module currently requires a 6 pin HC05 module with both KEY and STATE pins.
Many of these modules are EN and STATE which I do not support.

My test case used one master module and seven slave modules. All modules were
programmed with class 1F00.  All were programmed with the same PINs.

My program builds and uses a table of remote devices discovered during an inquiry.
Use +/- to move through the list, when on the desired entry use C to connect.
Once connected all data transfer occurs through the stream interface.  Connections
generate an automatic switch to data mode (1).

While connected you can use 0 to switch back to command mode and use D to disconnect
or use other commands.  Use 1 to go back to data mode.

If you reset this program while connected, a automatic disconnect occurs.  This is
because you can't determine who you are connected to during this reset, so I default
to command mode, no connect all of the time.

Commands:
0	Switch to command mode.
1	Switch to data mode.
2	Test case.
3	Manual load of device table.
4	Test case.
5	Set class to 1f00.
@	AT+RESET command sent.
A	AT command sent.
B	Set module baudrate to 38400.
C	Connect to selected device entry.
D	Disconnect
I	Start inquiry of devices (populate device table).
L	Show the device table.
P	Pair command (not connect!)
Q	Query Rnames of devices in the table.
R	Refresh the name of the current device.
S	Show current State
Z	Zero the device list.
+	Advance to entry in the list.
-	Previous entry in the list.
!	Configure the HC05 for master inquiry.
?	Dump all HC05 settings.

The minimal use of this library is to set the callback function and
call begin with the serial port the HC05 is connected to. I use 38400 as a default
user baud rate since it simplifies communications to the master.  These issues are
usually configured in the setup() function.  The HC05 instance must be declared
prior to setup().  You must specify the pins you use for the KEY (output) and 
STATE (input) functions.

The callback function is optional but really useful for most operations.  You only
need test for the callback cases you require.

If you execute the link function with an address the code will connect and switch to
data mode. While in data mode you use the hc05.available() and hc04.read() stream
functions just like you had opened a serial port.

This code was developed and tested on a Arduino Micro but should work as is with any
of the Arduino products.  The test case uses 60% of the code space of the Micro.

There is also the MinHC05M example which is a minimal use, change to your slaves
and go.  Best to configure and test your master with the FullHC05M example first.

Have Fun

David Farrell



