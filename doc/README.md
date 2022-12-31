## Assembly instructions


![Assembly 1](https://github.com/bitfieldlabs/afm_saucer/blob/master/doc/assembly_1.jpg)

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
