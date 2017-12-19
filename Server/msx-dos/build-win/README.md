This folder contains what's necessary for building the MSX-DOS OPC server on a Windows machine, together with an easy way to test it using NestorMSX. Requisites:

* [SDCC](http://sdcc.sourceforge.net) installed with Z80 support
  * See the ["About the console functions and the standard Z80 library" warning](http://www.konamiman.com/msx/msx-e.html#sdcc) at Konamiman's MSX page
* [hex2bin](http://gnuwin32.sourceforge.net/packages/hex2bin.htm) at any location accessible in PATH
* [ImDisk Tookit](https://sourceforge.net/projects/imdisk-toolkit) installed
* [NestorMSX](https://github.com/Konamiman/NestorMSX) "installed" at `\bin\NestorMSX` in the current drive
* [The TCP/IP UNAPI plugin for NestorMSX](https://github.com/Konamiman/TCP-IP-for-NestorMSX)

The `build.bat` script will build the `OPC.COM` file and then copy it into NestorMSX' `NextorAndMsxDos.dsk` disk image after mounting it. If you prefer to change/remove the copying to disk image part (or the location of NestorMSX), just change/remove the appropriate parts at `build.bat`, `mount.bat` and/or `unmount.bat`.