========================
Communication Protol
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
Version 0.1 Command Set
****************

############
Read Platform
############

Character: 0x00

The ``CMD_READ_PLATFORM`` command instructs the microcontroller to return a string containing
the platform, which usually corresponds to a microcontroller part number::

    master:   [CMD_READ_PLATFORM]
    response: [CMD_READ_PLATFORM] "dspic33ep32mc204\0"

############
Read Version
############

Character: 0x01

The ``CMD_READ_VERSION`` command instructs the microcontroller to return a string containing
the instruction set that it supports::

    master:   [CMD_READ_VERSION]
    response: [CMD_READ_VERSION] "0.1\0"

############
Read Row Length
############

Character: 0x02

The ``CMD_READ_ROW_LENGTH`` command instructs the microcontroller to return the smallest row length 
that can be programmed at one time::

    master:   [CMD_READ_ROW_LENGTH]
    response: [CMD_READ_ROW_LENGTH] [length(7:0)] [length(15:8)]

############
Read Page Length
############

Character: 0x02

The ``CMD_READ_PAGE_LENGTH`` command instructs the microcontroller to return the page erasure size 
in instructions::

    master:   [CMD_READ_PAGE_LENGTH]
    response: [CMD_READ_PAGE_LENGTH] [length(7:0)] [length(15:8)]
