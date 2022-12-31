## Assembly instructions
### Notes
It's easiest to start with the smalles (lowest profile) components and work the way up to the largest ones.

* *Note 1* Mind the polarity for the large capacitor C1!
* *Note 2* Mind the polarity for the Atmega chip U2!
* *Note 3* Mind the polarity for the LEDS! The flat side (-) is marked on the PCB. It's the connection with the square solder pad. It matches the LED's long legs.
* *Note 4* The legs of the 5V regulator (U1) must be bent straight in order to solder it flat to the PCB (see images). The VO (voltage out) pin must be connected to the upper solder pad, where U1 is marked. VI (voltage in) points toward the middle of the PCB.
* *Note 5* Remove pin 3 of the connector J1 as it is the key pin.
* *Note 6* Be careful not to short any LED pins, they are quite close to each other
* *Note 7* Switch 1 of the DIP switch SW3 points down to where SW3 is labelled

### Resistor values
The resistor values are
* R1: 10k
* R2, R3, R4, R5: 1k
* R6, R7: 220

### Assembly images
![Assembly 1](https://github.com/bitfieldlabs/afm_saucer/blob/master/doc/assembly_1.jpg)
![Assembly 1](https://github.com/bitfieldlabs/afm_saucer/blob/master/doc/assembly_2.jpg)
![Assembly 1](https://github.com/bitfieldlabs/afm_saucer/blob/master/doc/assembly_3.jpg)
![Assembly 1](https://github.com/bitfieldlabs/afm_saucer/blob/master/doc/assembly_4.jpg)
![Assembly 1](https://github.com/bitfieldlabs/afm_saucer/blob/master/doc/assembly_5.jpg)
![Assembly 1](https://github.com/bitfieldlabs/afm_saucer/blob/master/doc/assembly_6.jpg)
![Assembly 1](https://github.com/bitfieldlabs/afm_saucer/blob/master/doc/assembly_7.jpg)
![Assembly 1](https://github.com/bitfieldlabs/afm_saucer/blob/master/doc/assembly_8.jpg)

## Programming the Atmega328p

### Fuses
The Atmega's fuses must be set to following values:
 * hfuse: 0xDA
 * efuse: 0xFD
 * lfuse: 0xFF

avrdude command:
```
avrdude -v -V -patmega328p -cavrisp -b19200 -P/dev/ttyUSB0 -D -F -Ulfuse:w:0xff:m -Uhfuse:w:0xda:m -Uefuse:w:0xfd:m
```

## Configuration
The different modes can be chosen using the four DIP switches.
![DIP Switch](https://github.com/bitfieldlabs/afm_saucer/blob/master/doc/dip_switch.jpg)

. Switch Off / O Switch On

| Mode | Dip Switch 4321 | Description |
|-----:|----------------:|-------------|
|     0| ....| Random selection of mode 1-15 at each startup |
|     1| ...O| Teal background pulse. Red, orange and pink LEDs. |
|     2| ..O.| Rainbow background. Red, orange and light blue LEDs. |
|     3| ..OO| Yellow blinking background. Red, orange and light blue LEDs. |
|     4| .O..| Green breathing background. Red, orange and green LEDs. |
|     5| .O.O| No background, red LEDs. |
|     6| .OO.| Green breathing background. Red and green LEDs. |
|     7| .OOO| Red and green pulsing background. Red and blue LEDs. |
|     8| O...| Yellow and green breathing background. Red and blue LEDs. |
|     9| O..O| Alternating red and green background. Green and red LEDs. |
|    10| O.O.| No background. Rainbow LEDs. |
|    11| O.OO| No background. Red, green and ranbow LEDs. |
|    12| OO..| No background. Red, green and blue LEDs. |
|    13| OO.O| No background. Blue LEDs. |
|    14| OOO.| No background. Green LEDs. |
|    15| OOOO| Legacy mode. No fancy colors, just red LEDs. |

