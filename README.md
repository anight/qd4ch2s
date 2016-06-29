# qd4ch2s

```
 reset- 1 ++v++ 28
 rxd    2 +   + 27
 txd    3 +   + 26
        4 +   + 25
        5 +   + 24
        6 +   + 23
 vcc    7 +   + 22
 gnd    8 +   + 21
 p4b    9 +   + 20
 p4a   10 +   + 19   p3b
       11 +   + 18   p3a
       12 +   + 17   p2b
       13 +   + 16   p2a
 p1b   14 +++++ 15   p1b

```

1. vcc can be 5v or 3.3v so must be all other signals
2. "a" and "b" signals can be swapped, the counter will change a polarity in this case
3. rxd and txd are serial uart 1000000bps, 8N1
4. the following commands recognised by mcu:
   # set prescaler <N>
   # get prescaler
   # set auto_print <N>
   # get auto_print
   # <empty line> - print current status of ports
5. rxd line is optional if auto_print is set to non zero (e.g. 1000) and prescaler is set to actual value in in_uart.py and --notx flag is given

