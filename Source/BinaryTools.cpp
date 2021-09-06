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

#include <cstdio>
#include <cstring>

#include "utils.h"


//------------------------------------------------------------------------------
// Tool Declarations
//------------------------------------------------------------------------------

typedef int ( *fnTool )( int argc, char** argv );

struct Tool
{
	const char* pName;
	fnTool pFunction;
	const char* pDescription;
	const char* pHelpArgs;
	const char* pHelpDesc;
};

// ... add to this list as new tools are created.
extern int Help( int argc, char** argv );
extern int Data( int argc, char** argv );
extern int Join( int argc, char** argv );
extern int Pad( int argc, char** argv );
extern int RLE( int argc, char** argv );
extern int SMSChk( int argc, char** argv );
extern int ZXTap( int argc, char** argv );

// ... register the tools
static Tool gTools[] =
{
	{ "help", Help, "Show help for a specific tool. e.g. BinaryTools help pad", "tool-name", "Show help for a specific tool." },

	//-----------------

	{
		"data", Data, "Convert a binary file into BASIC DATA statements.", "<file> <output> [-line start[,step]] [-cols width] [-compact]",
		"  <file>      An input file to read.\n\n"
		"  <output>    Text output for the DATA statements.\n\n"
		"  -line L,S   Specify the starting line number and step.\n"
	    "              Default is line 1000 with step 10.\n\n"
		"  -cols W     Specify the maximum line length.\n"
		"              Default is 40 columns, minimum is 20.\n\n"
		"  -compact    Don't include a space after each comma.\n"
	},

	{
		"join", Join, "Join multiple files into a separate output.", "<file> [<file> ...] <output>",
		"  <file>    An input file to read. Multiple files can be specified.\n\n"
		"  <output>  The output. Contains all input files in the order given.\n"
		"            Caution: The output will be overwritten without confirmation.\n"
	},

	{
		"pad", Pad, "Pad a file to a given size.", "<file> size [fill]",
		"  <file>   A binary file to pad. Caution: The file will be padded in-place.\n"
		"           If the file doesn't exist, it will be created.\n\n"
		"  size     The size to pad the file to. Supports the following suffixes: KB, MB\n"
		"           or MBIT. If no suffix is specified, the size will be in bytes.\n"
		"           Specify in hexadecimal using either 0x, & or $ prefix or h suffix.\n\n"
		"  [fill]   Use this to specify a different byte value. Default is 0x00.\n"
	},

	{
		"rle", RLE, "Compress a file using run-length encoding.", "<file> <output> [-planes N]",
		"  <file>      The input file.\n\n"
		"  <output>    The RLE encoded/compressed output.\n\n"
		"  -planes N   Specify the number of interleaved planes in the input.\n"
		"              Default is 1 plane.\n"
	},

	{
		"smschk", SMSChk, "Sign a Master System ROM with a valid checksum.", "<rom-file>",
		"  <rom-file>   A ROM file to sign with a valid checksum. Caution: The file will\n"
		"               be modified in-place.\n"
	},

	{
		"zxtap", ZXTap, "Convert machine code into a ZX Spectrum .TAP file.", "<bin-file> name org-addr <tap-file>",
		"  <bin-file>   A machine code file to process.\n\n"
		"  name         The file name of the CODE block, up to 10 characters.\n\n"
		"  org-addr     Base address in memory where the data will be loaded to. Specify\n"
		"               in hexadecimal using either 0x, & or $ prefix or h suffix.\n\n"
		"  <tap-file>   The output .TAP file containing a CODE block.\n"
	}
};

// ... how many tools?
static int gToolsCount = sizeof( gTools ) / sizeof( Tool );


//------------------------------------------------------------------------------
// Global Data
//------------------------------------------------------------------------------

const char* gpActiveToolName = nullptr;


//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

static int findTool( const char* pName )
{
	for ( int i = 0; i < gToolsCount; ++i )
	{
		// Alias the tool
		const Tool& tool = gTools[ i ];

		// Match?
		if ( _strcmpi( pName, tool.pName ) == 0 )
		{
			return i;
		}
	}

	// Not found.
	return -1;
}

static void printHello()
{
	printf( "\n------------------------------------------------------------------\n"
			" BinaryTools Utility Collection\n"
			" Copyright (c) 2021, by David Walters. See LICENSE for details.\n"
			"------------------------------------------------------------------\n\n" );
}

static void printUsage()
{
	// Usage
	printf( "USAGE: BinaryTools tool [args ...]\n\n" );

	// Files
	printf( "Specify the tool to use followed by its arguments.\n\n" );

	for ( int i = 0; i < gToolsCount; ++i )
	{
		// Alias the tool
		const Tool& tool = gTools[ i ];

		// List it.
		printf( "    %-12s : %s\n", tool.pName, tool.pDescription );

		// formatting
		if ( i == 0 )
		{
			putchar( '\n' );
		}
	}
}

void PrintHelp( const char* pName )
{
	int iTool = findTool( pName );

	if ( iTool < 0 )
	{
		PrintError( "Unknown tool \"%s\". Cannot display help.", pName );
		return;
	}
	else
	{
		// Alias the tool
		const Tool& tool = gTools[ iTool ];

		// Usage
		printf( "\n%s\n\nUSAGE: BinaryTools %s %s\n\n%s\n", tool.pDescription, pName, tool.pHelpArgs, tool.pHelpDesc );
	}
}

static int Help( int argc, char** argv )
{
	if ( argc <= 2 )
	{
		printHello();
		printUsage();
		return 0;
	}

	char* pName = argv[ 2 ];
	PrintHelp( pName );

	return 0;
}

//==============================================================================

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main( int argc, char** argv )
{
	int iReturnCode = 0;

#ifdef _DEBUG
	PrintRuler( 80 );
#endif // _DEBUG

	if ( argc < 2 )
	{
		printHello();
		printUsage();
	}
	else
	{
		char* pName = argv[ 1 ];
		int iTool = findTool( pName );

		if ( iTool < 0 )
		{
			PrintError( "Unknown tool \"%s\".", pName );

			printHello();
			printUsage();
		}
		else
		{
			// Alias the tool
			const Tool& tool = gTools[ iTool ];

			// Store the name
			gpActiveToolName = tool.pName;

			// Call it!
			iReturnCode = tool.pFunction( argc, argv );
		}
	}

#ifdef _DEBUG
	printf( "\nFinished. Press Enter... " );
	getchar();
#endif

	// Done
	return iReturnCode;
}

//==============================================================================

