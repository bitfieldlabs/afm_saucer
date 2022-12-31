## Assembly instructions
It's easiest to start with the smalles (lowest profile) components and work the way up to the largest ones.

* *Note 1* Mind the polarity for the large capacitor C1!
* *Note 2* Mind the polarity for The Atmega chip U1!


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
