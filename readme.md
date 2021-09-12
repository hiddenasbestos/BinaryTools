
BinaryTools
-----------

A combined suite of utilities for manipulating binary data files.
It was developed for use on Windows but might compile on other systems.
Released under the MIT License.

The following tools are provided. Click on a tool name to jump to its specific documentation.

Tool  |Description
:---|:------------
[data](#data) | Convert a binary file into data statements.
[join](#join) | Join multiple files into a separate output.
[pad](#pad) | Pad a file to a given size.
[rle](#rle) | Compress a file using run-length encoding.
[smschk](#smschk) | Sign a Master System ROM with a valid checksum.
[zxtap](#zxtap) | Convert machine code into a ZX Spectrum .TAP file.


---

## data

Convert a binary file into data statements.

**Usage**
```
 BinaryTools data <file> <output> [-basic|-c|-db|-dcb|-dotbyte]
              [-line start[,step]] [-tab n|-spc n] [-cols width] [-compact]
              [-amp|-bin|-bux|-dec|-hex|-oct|-pct]

  <file>      An input file to read.

  <output>    Text output file for the statements.

  -basic      Write BASIC 'DATA' statements (default).
  -c          Write C/C++ initializer list.
  -db         Write assembly 'db' statements.
  -dcb        Write assembly 'dc.b' statements.
  -dotbyte    Write assembly '.BYTE' statements.

  -line L,S   Specify the starting line number and optionally a custom step.
              Default is no line numbers.
		
  -tab N      How many tab characters to prefix an assembly line. Default 1.
              Tabs are ignored in BASIC DATA mode or if line numbers are used.

  -spc N      How many space characters to prefix a line. Default 0.
              Spaces are ignored if tabs are used.

  -cols       Specify the maximum line length.
              Default is 40 columns, minimum is 20.

  -compact    Don't include a space after each comma delimiter between values.

  -amp        Write values in hexadecimal with '&' prefix.
  -bin        Write values in binary with '0b' prefix.
  -bux        Write values in hexadecimal with '$' prefix.
  -dec        Write values in decimal (default).
  -hex        Write values in hexadecimal with '0x' prefix.
  -oct        Write values in octal with '0' prefix.
  -pct        Write values in binary with '%' prefix.
```

**Examples**

```> BinaryTools data zxhello.bin zxhello.bas -line 100,5 -cols 30```

Converts the binary program `zxhello.bin` into BASIC DATA statements. Code will start at line 100 with an increment of 5 for each additional line. Each line will be no longer than 30 characters (including the line number and DATA statement).

```> BinaryTools data hello.txt hello.src -bux -cols 40 -dotbyte -spc 2 -line 10```

Convert a binary file into byte directives suitable for use by the Atari 8-bit Assembler Editor.

**Notes**

* Each byte of input data is stored as a separate value (in decimal, hexadecimal, binary or octal notation) with a comma delimiter.

* A simple BASIC 'loader' program could be written to `READ` this data and `POKE` it into memory.

---

## join

Join multiple files into a separate output.

**Usage**
```
 BinaryTools join <file> [<file> ...] <output>

  <file>    An input file to read. Multiple files can be specified.

  <output>  The output. Contains all input files in the order given.
            Caution: The output will be overwritten without confirmation.
```

**Examples**

```> BinaryTools join alpha.bin beta.bin gamma.bin omega.bin```

Copies files 'alpha.bin', 'beta.bin' and 'gamma.bin' into output 'omega.bin' in that order.

**Notes**

* Specify only one input file to perform a copy.

* Take care to make backups, or to use only on intermediate files, as the program will overwrite the output without asking for confirmation.


---

## pad

Pad a file to a given size.

**Usage**
```
 BinaryTools pad <file> size [fill]

  <file>   A binary file to pad. Caution: The file will be padded in-place.
           If the file doesn't exist, it will be created.

  size     The size to pad the file to. Supports the following suffixes: KB, MB
           or MBIT. If no suffix is specified, the size will be in bytes.
           Specify in hexadecimal using either 0x, & or $ prefix, or h suffix

  [fill]   Optional. Use this to specify a different byte value. Default is 0x00.
```

**Examples**

```> BinaryTools pad data.rom 32kb```

Extend a file 'data.rom' to 32KB with zeros.

```> BinaryTools pad new.bin 1mbit 0xFF```

Create a new file 'new.bin' (assuming, for the purposes of this example, that this file didn't exist already) with one mega power (128KB), filling the whole space with the hex value FF (255).

**Notes**

* If the file already exceeds the padding size, only a minor error ("FAILED") will be reported.

* If the file specified doesn't exist, it will be created.

* Take care to make backups, or to use only on intermediate files, as the program will overwrite existing files without asking for confirmation.

---

## rle

Compress a file using run-length encoding.

**Usage**
```
BinaryTools rle <file> <output> [-planes N]

  <file>      The input file.

  <output>    The RLE encoded/compressed output.

  -planes N   Specify the number of interleaved planes in the input.
              Default is 1 plane.
```

**Examples**

```> BinaryTools rle tilemap.bin tilemap.rle```

Compress an image file. Each byte of the input is checked against the previous byte and runs of equal bytes are compressed.

```> BinaryTools rle image.bin image.rle -planes 4```

Compress an image file, de-interleaving the file into 4 separate planes. Each plane starts at byte offset 0, 1, 2, 3 respectively and reading of each plane skips ahead by 4 bytes at a time to acquire the next byte of input.

**Output Format**

* The output data is a sequence of 'RLE blocks' with no additional header or footer data.

* An RLE block begins with a control byte with the following meanings:
  * If the high bit of the control byte is set, the low 7 bits are a *run length*. The next input byte is the specific value to repeat.
  * If the high bit is clear, the low 7 bits are a count of uncompressed data. This data immediately follows the control byte and should be copied to the output directly. 
  * If the control byte is `00`, this indicates the end of compressed data for this plane.

* Multiple planes are written sequentially to the output and each is separately terminated with a `00` control byte.

* The maximum run length (or raw data count) is 127. Longer runs are split into multiple RLE blocks.


**Notes**

* RLE isn't guaranteed to produce a smaller output for all inputs. The algorithm is most effective for inputs with large amounts of repetition such as images or tile maps.

* An example decompression routine written in Z80 assembly language can be found in the [Extras](https://github.com/hiddenasbestos/BinaryTools/tree/master/Extras) folder in the git repository.

---

## smschk

Sign a Master System ROM with a valid checksum.

**Usage**
```
BinaryTools smschk <rom-file>

  <rom-file>   A ROM file to sign with a valid checksum. Caution: The file will
               be modified in-place.
```

**Examples**

```> BinaryTools smschk game.sms```

Update the given Master System ROM file with a valid checksum.

**Notes**

* For homebrew software, typically you will want to use the 'pad' tool on the file first to grow it to a standard size such as 32KB, 128KB, 256KB or 512KB. Otherwise the checksum can't be written to the correct location in the file.

---

## zxtap

Convert machine code into a ZX Spectrum .TAP file.

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




