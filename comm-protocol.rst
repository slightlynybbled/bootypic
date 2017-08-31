========================
Communication Protocol
========================

This document describes the communications protocol for the bootypic repository

There are two primary layers that this document is concerned with, framing and command.
The framing layer is primarily concerned with delimiting the beginning of a packet along
with ensuring data integrity of the packet using a fletcher16 checksum.  The command 
layer is concerned with interpreting the payload of the frame so that the software layer 
may respond accordingly.

------------------------
Framing Layer
------------------------

All frames have a Start of Frame (SOF) byte at the beginning and an End of Frame (EOF) 
byte at the end.  The bytes between the SOF and EOF comprise the data being transmitted.  
Any byte that corresponds to the SOF, EOF, or Escape (ESC) characters will be escaped 
and XORed so that they do not interfere with the overall transmission process.  The last
two bytes of the data bytes will be the fletcher16 checksum of the payload.

****************
Escaping
****************

Any byte between the SOF and EOF that corresponds to SOF, EOF, or ESC will be replaced 
by two bytes, the first will be the ESC byte and the second will be the data byte XORed 
with the XOR value.

****************
Structure
****************

The structure, without escaping, is as follows::

    [SOF] [dat0] [data1] [data2] [...] [dataX] [F16(0:7)] [F16(15:8)] [EOF]

Note that if F16 bytes correspond to special characters, they will be properly escaped.

If, for instance, data4 corresponded to the SOF byte, the stream would be modified as follows::

    [data2] [data3] [ESC] [ESC_XOR ^ data4] [data5]

In this way, it is possible to give up a small amount of transmission efficiency in order to
be able to transmit the entire range of data.

****************
Behavior
****************

If a F16 value is correct, then the packet is forwarded up to the next software layer.  If it
is incorrect, then the packet is discarded.

****************
Special Characters
****************

The special characters are the SOF, EOF, ESC, and ESC_XOR.  These values are ONLY to be used
as part of the framing protocol.  All payload bytes that correspond to these values are to
be escaped::

    [SOF] 0xf7
    [EOF] 0x7f
    [ESC] 0xf6
    [ESC_XOR] 0x20

------------------------
Command Layer
------------------------

The command layer may have multiple versions, which will be saved to the microcontroller at 
compile time.

****************
Command Structure
****************

A typical command packet, stripped of framing information::

    [reserve0] [reserve1] [CMD] [payload0] [payload1] [...] [payloadX]

The first two bytes are reserved for future use.  They may contain any type of data the 
user prefers.  For the remainder of this document, these reserved bytes will be ignored.

CMD refers to a 1-byte command.  The command will determine how the remainder of the payload 
is interpreted.  In some cases, there may be no additional bytes after the command, such as 
in the CMD_START_APP command.

****************
Behavior
****************

The PC will typically be the master and the microcontroller will simply respond to the commands.
Many commands have no required response, such as CMD_ERASE_PAGE.  Others that require a response 
will simply embed the same command into the structure of the response.

When strings are being passed, they will be passed as ASCII bytes and transmitted with the null
string terminator ``\0``.  These are represented in the commands as a string with quotes.::

    representation: "my str\0"
    transmitted: [0x6d] [0x79] [0x20] [0x73] [0x74] [0x72] [0x00]

****************
Command Set
****************

############
Read Platform
############

Character: 0x00
Command Sets: 0.1

The ``CMD_READ_PLATFORM`` command instructs the microcontroller to return a string containing
the platform, which usually corresponds to a microcontroller part number::

    master:   [CMD_READ_PLATFORM]
    response: [CMD_READ_PLATFORM] "dspic33ep32mc204\0"

############
Read Version
############

Character: 0x01
Command Sets: 0.1

The ``CMD_READ_VERSION`` command instructs the microcontroller to return a string containing
the instruction set that it supports::

    master:   [CMD_READ_VERSION]
    response: [CMD_READ_VERSION] "0.1\0"

############
Read Row Length
############

Character: 0x02
Command Sets: 0.1

The ``CMD_READ_ROW_LENGTH`` command instructs the microcontroller to return the smallest row length 
that can be programmed at one time::

    master:   [CMD_READ_ROW_LENGTH]
    response: [CMD_READ_ROW_LENGTH] [length(7:0)] [length(15:8)]

############
Read Page Length
############

Character: 0x03
Command Sets: 0.1

The ``CMD_READ_PAGE_LENGTH`` command instructs the microcontroller to return the page erasure size 
in instructions::

    master:   [CMD_READ_PAGE_LENGTH]
    response: [CMD_READ_PAGE_LENGTH] [length(7:0)] [length(15:8)]

############
Read Max Program Memory Length
############

Character: 0x04
Command Sets: 0.1

The ``CMD_READ_PROG_LENGTH`` command instructs the microcontroller to return the program length, 
which is the maximum address that may be programmed to::

    master:   [CMD_READ_PROG_LENGTH]
    response: [CMD_READ_PROG_LENGTH] [length(7:0)] [length(15:8)] [length(23:16)] [length(31:24)]

############
Read Max Program Size
############

Character: 0x05
Command Sets: 0.1

The ``CMD_READ_MAX_PROG_SIZE`` command instructs the microcontroller to return the maximum programming
size that it will support in instructions::

    master:   [CMD_READ_MAX_PROG_SIZE]
    response: [CMD_READ_MAX_PROG_SIZE] [length(7:0)] [length(15:8)]

############
Read App Start Address
############

Character: 0x06
Command Sets: 0.1

The ``CMD_READ_APP_START_ADDRESS`` command instructs the microcontroller to return the starting address
of the application.  This will usually be 0x1000.  This will be utilized for checking application integrity
during the verification stage.

    master:   [CMD_READ_MAX_PROG_SIZE]
    response: [CMD_READ_MAX_PROG_SIZE] [address(7:0)] [address(15:8)]

############
Erase Page
############

Character: 0x10
Command Sets: 0.1

The ``CMD_ERASE_PAGE`` command instructs the microcontroller erase a page of flash memore starting 
at the provided address.::

    master:   [CMD_ERASE_PAGE] [address(7:0)] [address(15:8)] [address(23:16)] [address(31:24)]
    response: -

############
Read Address
############

Character: 0x20
Command Sets: 0.1

The ``CMD_READ_ADDRESS`` command instructs the microcontroller read a single value from flash memory 
and to return that value.

    master:   [CMD_READ_ADDRESS] [address(7:0)] [address(15:8)] [address(23:16)] [address(31:24)]
    response: [CMD_READ_ADDRESS] [address(7:0)] [address(15:8)] [address(23:16)] [address(31:24)]
                                 [value(7:0)] [value(15:8)] [value(23:16)] [value(31:24)]

############
Read Max
############

Character: 0x21
Command Sets: 0.1

The ``CMD_READ_MAX`` command instructs the microcontroller read the maximum number of values from 
flash memory and return them as an array of values.  This allows for much more efficient reading 
of memory::

    master:   [CMD_READ_ADDRESS] [address(7:0)] [address(15:8)] [address(23:16)] [address(31:24)]
    response: [CMD_READ_ADDRESS] [address(7:0)] [address(15:8)] [address(23:16)] [address(31:24)]
                                 [value0(7:0)] [value0(15:8)] [value0(23:16)] [value0(31:24)]
                                 [value1(7:0)] [value1(15:8)] [value1(23:16)] [value1(31:24)]
                                 [...]
                                 [valueX(7:0)] [valueX(15:8)] [valueX(23:16)] [valueX(31:24)]

############
Write Row
############

Character: 0x30
Command Sets: 0.1

The ``CMD_WRITE_ROW`` command instructs the microcontroller to write an entire row of data, as defined
by the microcontroller datasheet, starting at the address.  In many cases, a row consists of only 2 
instructions, so it may not be very efficient.::

    master:   [CMD_WRITE_ROW] [address(7:0)] [address(15:8)] [address(23:16)] [address(31:24)]
                                 [value0(7:0)] [value0(15:8)] [value0(23:16)] [value0(31:24)]
                                 [value1(7:0)] [value1(15:8)] [value1(23:16)] [value1(31:24)]
                                 [...]
                                 [valueX(7:0)] [valueX(15:8)] [valueX(23:16)] [valueX(31:24)]

    response: -

############
Write Max
############

Character: 0x31
Command Sets: 0.1

The ``CMD_WRITE_ROW`` command instructs the microcontroller to write an entire row of data, as defined
by the return value of ``READ_MAX_PROG_SIZE``, starting at the address.  This is usually a much more 
efficient method of writing.::

    master:   [CMD_WRITE_ROW] [address(7:0)] [address(15:8)] [address(23:16)] [address(31:24)]
                              [value0(7:0)] [value0(15:8)] [value0(23:16)] [value0(31:24)]
                              [value1(7:0)] [value1(15:8)] [value1(23:16)] [value1(31:24)]
                              [...]
                              [valueX(7:0)] [valueX(15:8)] [valueX(23:16)] [valueX(31:24)]

    response: -

############
Start Application
############

Character: 0x40
Command Sets: 0.1

The ``CMD_WRITE_ROW`` command instructs the microcontroller to start the application.  Note that the 
bootloader will no longer respond after the application is started.::

    master:   [CMD_START_APP]
    response: -
