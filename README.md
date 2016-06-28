# qd4ch2s
* ~~prescaler write by i2c~~
* ~~stabililize i2c @ 100khz~~
* serial demo
* auto mode for serial, eeprom config setting

```
 vcc    1 ++v++ 28
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
4. ~~scl and sda are i2c, slave address is 0x22~~
