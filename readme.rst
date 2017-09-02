========================
Purpose
========================

To provide a relatively easy-to-use bootloader that is compatible with most PIC24 and DSPIC33
series processors.  This bootloader does NOT use interrupts, so your default compilation
steps should work with only *minor* changes to the linker script.

The communications protocol is described in the comm-protocol.rst document.

========================
Supported Devices
========================

This list will be updated as I test more devices (which will be slowly).  Each supported device 
will have a linker script for the bootloader (this project) and the application located in the 
``gld`` directory.  You must add the proper linker file to your ``linker files`` in your MPLAB
project in order to get the bootloader to operate correctly.  Modification of the stock linker 
files to be compatible with bootypic is described in the gld directory readme.

 - dsPIC33EP32MC204
 - dsPIC33EP64MC504

Contributions in this area are welcome!

========================
Environment
========================

The anticipated environment is MPLAB X with an XC16 compiler.  This will probably work in other 
environments, but I am only testing with Microchip-provided tools.  

In order to be most compatible with devices, the bootloader must fit within a small and well-defined
memory footprint.  As a result, optimizations must be turned up to -O1 or the application will have 
to be moved to a higher memory location on some devices with larger flash erase pages.


