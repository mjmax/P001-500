@ERASE
0 REM @UNPAIR

0 REM The names the master will pair with
2 LF02
3 Ges3S_S
4 Bluestick_S450
5 RS420_

0 REM The address of the slave that the master is paired to
8 0

0 REM Lines 10 and 11 are scratchpad vars
0 REM don't use for anything permanent
10 0
11 0


@INIT 50
0 REM Initialise serial port
0 REM uartcfg has one byte bit field 1PPSBBB
0 REM Parity PP = 00 for none, 10 E, 01 O
0 REM Stop bits S = 0 for 1 stop, 1 for two
0 REM Baud BBBB = 0 1200, 1 2400, 2 4800, 3 9600, 4 19200
0 REM 5 38400, 8 115200, 12 1382400
0 REM 0x83 (131) = 9600 n 8 1
50 A = uartoff
51 A = uartcfg 131

0 REM blue LED is attached to PIO 20
0 REM J stores this
53 J = 20
0 REM D is state of the blue LED
54 D = 0

0 REM E is state of master
0 REM E = 0 unpaired
0 REM E = 1 paired, disconnected
0 REM E = 2 paired, connected
0 REM E = 3 paired, connected, linked
55 E = 0

56 $8 = ""

0 REM Debug Control
0 REM Z enables built in OS debugging (see documentation)
0 REM Y enables PRINTU calls
60 Z = 0
61 Y = 0

62 A = zerocnt
0 REM PIO 19 is red LED on SMD module
65 A = pioout 19
66 ALARM 1
67 IF Y = 0 THEN 69
68 PRINTU "Startup\r\n"
69 RETURN


@IDLE 80
80 IF Y = 0 THEN 82
81 PRINTU "Idle called\r\n"
82 RETURN


@ALARM 150
0 REM Flash blue LED at 1/2Hz if not paired, connected, linked
150 IF E = 3 THEN 155
151 IF D = 0 THEN 155
152 A = pioclr J
153 D = 0
154 GOTO 160
155 A = pioset J
156 D = 1

160 IF E = 1 THEN 184
161 IF E = 2 THEN 195
162 IF E = 3 THEN 195

0 REM E = 0, disconnected, every 10 seconds start another enquiry
170 A = readcnt
171 IF Y = 0 THEN 175
172 PRINTU "readcnt "
173 PRINTU A
174 PRINTU "\r\n"
175 IF A < 10 THEN 180
176 A = inquiry 10
177 A = zerocnt
178 IF Y = 0 THEN 180
179 PRINTU "Init enq\r\n"
180 GOTO 240

0 REM E = 1, a device has been discovered, let's try to connect
184 IF Y = 0 THEN 186
185 PRINTU "Conn mast\r\n"
186 A = readcnt
187 IF A > 30 THEN 200
189 A = master $8
190 GOTO 240

0 REM E = 2, we are connected, lets check we are still connected
195 A = status
196 IF A = 0 THEN 200
197 GOTO 240

0 REM connected, but now we were disconnected
200 IF Y = 0 THEN 202
201 PRINTU "Mst discon\r\n"
202 E = 0
203 A = unpair $8
204 $8 = ""
205 A = disconnect 1

240 ALARM 1
241 RETURN


@INQUIRY 250
0 REM when this interrupt gets called we have on $0 an string like this:
0 REM 00112233445566 NAME where the number is the bt address.
0 REM First extract the name, if it matches our name filter then connect
0 REM and cancel any further inquiry
0 REM If not, keep looking

250 IF Y = 0 THEN 254
251 PRINTU "Enq from: ";
252 PRINTU $0;
253 PRINTU "\r\n";

0 REM Are we already connected
254 IF E <> 0 THEN 280;

0 REM Does it match expected
255 $10 = $0;
256 $11 = $0[13];
257 $0 = $11;
258 A = strcmp $2;
259 IF A = 0 THEN 270;
260 A = strcmp $3;
261 IF A = 0 THEN 270;
262 A = strcmp $4;
263 IF A = 0 THEN 270;
264 A = strcmp $5;
265 IF A = 0 THEN 270;

266 IF Y = 0 THEN 268
267 PRINTU "No match\r\n"
268 RETURN

0 REM Yes, match, connect state (E=1), next alarm will connect
270 $8 = $10
271 IF Y = 0 THEN 273
272 PRINTU "Match, conn\r\n"
273 E = 1
274 A = cancel
275 RETURN 

280 IF Y = 0 THEN 282
281 PRINTU "Alrdy conn\r\n"
282 RETURN


@MASTER 300
300 IF Y = 0 THEN 304
301 PRINTU "Master of "
302 PRINTU $8
303 PRINTU "\r\n"
304 E = 2
305 GOSUB 700
319 RETURN


0 REM Function to connect based on pin 4 state (input M=state)
700 IF Y = 0 THEN 751
701 PRINTU "Master stuff\r\n"
751 IF E = 0 THEN 770
752 IF E = 1 THEN 770
753 IF E = 3 THEN 760

0 REM Master connected but not linked
754 IF Y = 0 THEN 756
755 PRINTU "Mst Con->Lnk\r\n"
756 A = link 2
757 E = 3
758 RETURN

0 REM Master already linked
760 IF Y = 0 THEN 762
761 PRINTU "Mst alrdy link\r\n"
762 RETURN

0 REM Master not connected
770 IF Y = 0 THEN 772
771 PRINTU "Mst not con\r\n"
772 RETURN


@PIN_CODE 900
900 IF Y = 0 THEN 902
901 PRINTU "Pin code\r\n";
902 $0 = $8[13];

0 REM LF02
903 A = strcmp $2;
904 IF A <> 0 THEN 907;
905 $0 = "1234";
906 GOTO 940;

0 REM GES
907 A = strcmp $3;
908 IF A <> 0 THEN 911;
909 $0 = "0";
910 GOTO 940;

0 REM Bluestick
911 A = strcmp $4;
912 IF A <> 0 THEN 915;
913 $0 = "0";
914 GOTO 940;

0 REM Allflex RS420
915 A = strcmp $5;
916 IF A <> 0 THEN 920;
917 $0 = "1234";
918 GOTO 940;

0 REM Default pin code
920 $0 = "0000";
921 GOTO 940;

940 IF Y = 0 THEN 944
941 PRINTU "Pin code: ";
942 PRINTU $0;
943 PRINTU "\r\n";
944 RETURN

