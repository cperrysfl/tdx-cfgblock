# tdx-cfgblock

Toradex boards using U-Boot have a small region of non-volatile memory that
contains variables such has a product id, a serial number, a MAC address, etc.
This non-volatile memory can be in an EEPROM, on a disk or in a NAND partition
and is usually only accessed by U-Boot with the `cfgblock` command. This repo is
a userspace tool, named `tdx-cfgblock`, that allows reading and writing this
non-volatile storage.
