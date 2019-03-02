# deviation-bootloader
Replacement Bootloader for Deviation supported transmitters.
*NOTE: Unless you are a developer, you should NOT install this bootloader yet!*
*Installing this bootloader requires a custom Deviation build to enable, which is not provided by current nightlies*

This bootloader currently provides the following functionality for supported transmitters:
 * LCD display when in bootloader mode
 * support for SPI read/write
 * non-encrypted DFU files

### Currently supported transmitters
 * Jumper T8SG-V1
 * Jumper T8SG-V2
 * Walkera Devo-7e
 * Walkera Devo-7e w/ 256k mod
 
###Compiling
*It is important to properly build for the intended tarnsmitter*
 * T8SGV1: `make TXVER=T8SGV1`
 * T8SGV2: `make TXVER=T8SGV2`
 * T8SGV2+: `make TXVER=T8SGV2+`
 * Devo7e: `make TXVER=DEVO7E`
 * Devo7e w/256k: `make TXVER=DEVO7E-256`

### Installing
If you currently have the default Walkera/Jumper Bootloader installed:
 * Install `installer-enc-<target>.dfu` using Deviation-Uploader

If you currently have an older version of Deviation Bootloader:
  * Install `installer-<target>.dfu` via:
  `dfu-util -a0 -D installer-<target>.dfu`

### Developer testing
If you would like to preserve your existing bootloader, we currently support a two-stage bootload option.
Set the 'TWO_STAGE' variable=1 in the makefile.  Only the unified t8sg build will work with the two-stage
bootloader!  You cannot load any other builds when using this mode of operation

### Installing a deviation build when using deviation-bootloader
Encryption must be removed prior to installing a new firmware when using the Deviation Bootloader.
This can be done by rerunniing `utils/dfu.py` with `-c 0`.  For example.  To install the stock t8sg-v2+ image
after installing deviation-bootloader:
```
make t8sg_v2_plus
./../utils/dfu.py --name "t8sg_v2_plus-v5.0.0-xxxxxxx Firmware" -c 0 -b 0x08003000:t8sg_v2_plus.bin t8sg_v2_plus.dfu
```
