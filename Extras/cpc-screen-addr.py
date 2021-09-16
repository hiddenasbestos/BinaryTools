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
# memory on an Amstrad CPC, and write each to a binary file.
# Intended for use as a lookup table in conjunction with "BinaryTools data"

# CPC screen addresses are generated with the following formula:
# This applies to all 3 video modes, which each have an 80 byte line.

# Address = 0xC000 + ((Line / 8) * 80) + ((Line % 8) * 2048)

file = open( "cpc-screen-addr.bin", "wb" )

c = 0

for y in range( 0, 200, 1 ):

	base = 0xC000; # address base

	a = base + ( int( y / 8 ) * 80 ) + ( int( y % 8 ) * 2048 )

	print( "output #" + str(c) + ": y = " + str(y) + ", address = " + str(hex(a)) );
	
	b = a.to_bytes(2, byteorder='little')
	
	file.write(b);
	
	c = c + 1

file.close();



