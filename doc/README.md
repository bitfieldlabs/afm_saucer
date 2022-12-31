## Assembly instructions
It's easiest to start with the smalles (lowest profile) components and work the way up to the largest ones.

* *Note 1* Mind the polarity for the large capacitor C1!
* *Note 2* Mind the polarity for the Atmega chip U2!
* *Note 3* Mind the polarity for the LEDS! The flat side (-) is marked on the PCB. It's the connection with the square solder pad. It matches the LED's long legs.
* *Note 4* The legs of the 5V regulator (U1) must be bent straight in order to solder it flat to the PCB (see images). The VO (voltage out) pin must be connected to the upper solder pad, where U1 is marked. VI (voltage in) points toward the middle of the PCB.
* *Note 5* Remove pin 3 of the connector J1 as it is the key pin.

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
