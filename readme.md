
BinaryTools
-----------

A combined suite of utilities for manipulating binary data files.
It was developed for use on Windows but might compile on other systems.
Released under the MIT License.

**Usage**
```
BinaryTools tool [args ...]
```


The following tools are provided. Click on a tool name to jump to its specific documentation.

Tool  |Description
:---|:------------
[pad](#pad) | Pad a file to a given size.
[zxtap](#zxtap) | Convert machine code into a ZX Spectrum .TAP file.

---


##pad

**Usage**
```
 BinaryTools pad <file> size [fill]

  <file>   A binary file to pad. Caution: The file will be padded in-place.
           If the file doesn't exist, it will be created.

  size     The size to pad the file to. Supports the following suffixes: KB, MB
           or MBIT. If no suffix is specified, the size will be in bytes.
           Specify in hexadecimal using either 0x, & or $ prefix, or h suffix

  [fill]   Use this to specify a different byte value. Default is 0x00.
```

**Examples**

```> BinaryTools pad data.rom 32kb```

Extend a file 'data.rom' to 32KB with zeros.

```> BinaryTools pad new.bin 1mbit 0xFF```

Create a new file 'new.bin' (assuming, for the purposes of this example, that this file didn't exist already) of 1MBIT (128KB) in size, filling the space with the hex value FF (255).

**Notes**

* If the file already exceeds the padding size, only a minor error ("FAILED") will be reported.

* If the file specified doesn't exist, it will be created.

* Take care to make backups, or to use only on intermediate files, as the program will overwrite existing files without asking for confirmation.


---

##zxtap

**Usage**
```
BinaryTools zxtap <bin-file> name org-addr <tap-file>

  <bin-file>   A machine code file to process.

  name         The file name of the CODE block, up to 10 characters.

  org-addr     Base address in memory where the data will be loaded to. Specify
               in hexadecimal using either 0x, & or $ prefix or h suffix.

  <tap-file>   The output .TAP file containing a CODE block.
```

**Examples**

```> BinaryTools zxtap hello.bin HELLO 0x8000 hello.tap```

Create a .TAP file for a code block called "HELLO" which will load into memory starting at address 32768. 

**Notes**

* Make sure the origin address matches the `org` value used in your assembler, or absolute addresses will be invalid.

* Load this data from BASIC using the command `LOAD "" CODE`




