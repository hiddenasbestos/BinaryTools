
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

typedef void ( *fnTool )( int argc, char** argv );

struct Tool
{
	const char* pName;
	fnTool pFunction;
	const char* pDescription;
	const char* pHelpArgs;
	const char* pHelpDesc;
};

// ... add to this list as new tools are created.
extern void Help( int argc, char** argv );
extern void Join( int argc, char** argv );
extern void Pad( int argc, char** argv );
extern void ZXTap( int argc, char** argv );

// ... register the tools
static Tool gTools[] =
{
	{ "help", Help, "Show extended help for a specific tool. e.g. BinaryTools help pad", "tool-name", "Show help for a specific tool." },

	//-----------------

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
#ifdef _DEBUG
	Print80ColRuler();
#endif // _DEBUG

	int iTool = findTool( pName );

	if ( iTool < 0 )
	{
		printf( "ERROR: Unknown tool \"%s\". Cannot display help.\n\n", pName );
		printUsage();
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

static void Help( int argc, char** argv )
{
	if ( argc <= 2 )
	{
		printHello();
		printUsage();
		return;
	}

	char* pName = argv[ 2 ];
	PrintHelp( pName );
}

//==============================================================================

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main( int argc, char** argv )
{
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
			printf( "ERROR: Unknown tool \"%s\".\n\n", pName );

			printHello();
			printUsage();
		}
		else
		{
			// Alias the tool
			const Tool& tool = gTools[ iTool ];

			// Call it!
			tool.pFunction( argc, argv );
		}
	}

#ifdef _DEBUG
	printf( "\nFinished. Press Enter... " );
	getchar();
#endif

	// Done
	return 0;
}

//==============================================================================

