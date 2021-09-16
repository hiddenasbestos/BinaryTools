#
# MIT License
#
# Copyright (c) 2021 David Walters
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

#===============================================================================

# Simple program to generate the memory address of each line of screen
# memory on a ZX Spectrum, and write each to a binary file.
# Intended for use as a lookup table in conjunction with "BinaryTools data"

# ZX Spectrum video addresses use the following bit encoding:

# -----------------------------------------------------------------
# |              H                |              L                | 
# -----------------------------------------------------------------
# | 15| 14| 13| 12| 11| 10| 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
# -----------------------------------------------------------------
# | 0 | 1 | 0 | Y7| Y6| Y2| Y1| Y0| Y5| Y4| Y3| X4| X3| X2| X1| X0|
# -----------------------------------------------------------------

file = open( "zx-screen-addr.bin", "wb" )

c = 0

for y in range( 0, 192, 1 ):

	base = 0x4000; # address base
	
	y012 = y & 0b111;
	y345 = ( y & 0b111000 ) >> 3;
	y67 = ( y & 0b11000000 ) >> 6;

	a = base | ( y012 << 8 ) | ( y345 << 5 ) | ( y67 << 11 );

	print( "output #" + str(c) + ": y = " + str(y) + ", address = " + str(hex(a)) );
	
	b = a.to_bytes(2, byteorder='little')
	
	file.write(b);
	
	c = c + 1

file.close();



