
/*

Copyright (c) 2021 David Walters

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#pragma once

#include <cstdint>

//------------------------------------------------------------------------------
// Utility Functions
//------------------------------------------------------------------------------

// Detect a hex prefix and return a character offset after it. Zero if not found.
int DetectHexPrefix( const char* pStr );

// Helper to fit, for example, usage text within 80 columns
void Print80ColRuler();

// Print all of the command line arguments with thier index.
void DebugCmdArgs( int argc, char** argv );

// Parse a string, detecting a size suffix (KB, MBIT, etc.) and return a byte amount.
// Supports hexadecimal mode. Returns -1 on invalid number.
int64_t ParseSizeWithSuffix( const char* pStr );

// Parse a positive integer value, supports hexadecimal.
// Returns -1 on invalid number.
int ParseValue( const char* pStr, int iLimit );

// Testing for ParseSizeWithSuffix
void TestParsingSizes();

// Print help for a specific tool.
// NOTE: This function is implemented in BinaryTools.cpp
void PrintHelp( const char* pName );

//==============================================================================
