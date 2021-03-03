# Software for U4:
*	Read external inputs I1..I4
*	Write external outputs O1..O4
*	O1 is direct copy of IN1 which is output 1 of R320
*	O2 is direct copy of IN2 which is output 2 of R320
*	O3 and O4 are driven from serial commands from R320
*	Multiplexed communications:
*	Implement DLE # escape multiplexing for channels (0 for U4 internal, 1 for Bluetooth, 2 for RS232).  2 characters DLE (hex 10) character followed by single digit ‘0’,’1’,’2’ etc to switch comms channels.  Example:

* Example 1:
```
    R320 => U4:  <DLE>2Hello World<DLE>00W1:0001; 
    ( transmit ‘Hello World’ out RS232 and turn on output 1)
    U4=> R320: <DLE>09W1:0;      
    (ack write output 1)
```
* Example 2:
```
    Bluetooth => U4: 202210:; (Weight request)
    U4=> R320:<DLE>2202210:; (Weight request sent to R320)
    R320=> U4:<DLE>2202210:5242; (Weigh)
    U4=>Bluetooth: 202210:5242 (weight sent to Bluetooth)
```

*	Handle DC2,DC4 ring processing locally for RS232 port.  Strip DC2, and DC4 characters from RS232 port locally, deliver remaining characters to R320 and send DC4 at the end.  This will enable ring processing for instrument. 
*	Add ring processing to the main serial interface with R320.  This won’t be used directly on X350 but will form protocol for multi-accessory bus on R350 later.  In the case of multiple serial accessories the virtual serial port numbers are extended so <DLE>n can be identified immediately as a physical resource on the target accessory.


#	Accessory Protocol:

*	DLE PORT ADDR CMD REG : DATA ;
* DLE: 	Character starts the protocol message (0x10 in ascii table)
* PORT: Destination port (port which message ends up in. '0' is for the internal communication)
*	ADDR:  Single hex digit 0..7 with 0 for broadcast allows for 7 accessory boards to be addressed.  Most significant bit used to signify slave response in header if set.  
*	CMD:  Single character 'R' for read 16 bits of data, 'W' for write data
*	REG: Single hex character 0..F allows for 15 registers

