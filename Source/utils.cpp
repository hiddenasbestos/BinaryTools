
/*

Copyright (c) 2021-2022 David Walters

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
#include <cstdlib>
#include <errno.h>
#include <cstring>
#include <cstdint>
#include <stdarg.h>

#include "utils.h"

// from BinaryTools.cpp
extern const char* gpActiveToolName;


//------------------------------------------------------------------------------
// DetectHexPrefix
//------------------------------------------------------------------------------
int DetectHexPrefix( const char* pStr )
{
	// Single character prefixes.
	if ( pStr[ 0 ] == '$' || pStr[ 0 ] == '&' )
		return 1;

	// 0x prefix.
	if ( pStr[ 0 ] == '0' && ( pStr[ 1 ] == 'x' || pStr[ 1 ] == 'X' ) )
		return 2;

	// Nope.
	return 0;
}

//------------------------------------------------------------------------------
// PrintRuler
//------------------------------------------------------------------------------
void PrintRuler( int columns )
{
	for ( int i = 0; ( i + 10 ) <= columns; i += 10 )
	{
		printf( "-------%02d!", i + 10 );
	}

	putchar( '\n' );
}

//------------------------------------------------------------------------------
// DebugCmdArgs
//------------------------------------------------------------------------------
void DebugCmdArgs( int argc, char** argv )
{
	printf( "argc = %d\n", argc );

	for ( int i = 0; i < argc; ++i )
	{
		printf( "arg[ %d ] = %s\n", i, argv[ i ] );
	}
}

//------------------------------------------------------------------------------
// FindArg
//------------------------------------------------------------------------------
int FindArg( const char* pArg, int argc, char** argv )
{
	// skip arg[0] as it's the command name.
	for ( int i = 1; i < argc; ++i )
	{
		if ( _stricmp( pArg, argv[ i ] ) == 0 )
		{
			return i;
		}
	}

	// not found.
	return -1;
}

//------------------------------------------------------------------------------
// ParseWithSizeSuffix
//------------------------------------------------------------------------------
int64_t ParseSizeWithSuffix( const char* pStr )
{
	if ( pStr == nullptr )
		return -1;

	int iHexOffset;
	iHexOffset = DetectHexPrefix( pStr );

	int64_t iSize;
	char* pNumberEnd;

	errno = 0;

	if ( iHexOffset )
	{
		// Convert from hex.
		iSize = strtol( pStr + iHexOffset, &pNumberEnd, 16 );
	}
	else
	{
		// Convert from decimal
		iSize = strtol( pStr, &pNumberEnd, 10 );
	}

	if ( errno == ERANGE || pStr == pNumberEnd || iSize < 0 )
	{
		return -1;
	}

	// Suffix?
	if ( _stricmp( pNumberEnd, "KB" ) == 0 )
	{
		iSize *= 1024;
	}
	else if ( _stricmp( pNumberEnd, "MB" ) == 0 )
	{
		iSize *= 1048576;
	}
	else if ( _stricmp( pNumberEnd, "MBit" ) == 0 )
	{
		iSize *= 131072;
	}
	else if ( _stricmp( pNumberEnd, "h" ) == 0 )
	{
		if ( iHexOffset )
		{
			// Don't allow 0x123h !
			return -1;
		}
		else
		{
			// Convert from hex.
			errno = 0;
			iSize = strtol( pStr + iHexOffset, &pNumberEnd, 16 );

			if ( errno == ERANGE || pStr == pNumberEnd )
			{
				return -1;
			}
		}
	}
	else if ( *pNumberEnd != 0 )
	{
		return -2; // something, but unknown
	}

	return iSize;
}

//------------------------------------------------------------------------------
// ParseValue
//------------------------------------------------------------------------------
int ParseValue( const char* pStr, int iLimit )
{
	if ( pStr == nullptr )
		return -1;

	int iHexOffset;
	iHexOffset = DetectHexPrefix( pStr );

	int64_t iSize;
	char* pNumberEnd;

	errno = 0;

	if ( iHexOffset )
	{
		// Convert from hex.
		iSize = strtol( pStr + iHexOffset, &pNumberEnd, 16 );
	}
	else
	{
		// Convert from decimal
		iSize = strtol( pStr, &pNumberEnd, 10 );
	}

	if ( errno == ERANGE || pStr == pNumberEnd || iSize < 0 )
	{
		return -1;
	}

	// Suffix?
	if ( _stricmp( pNumberEnd, "h" ) == 0 )
	{
		if ( iHexOffset )
		{
			// Don't allow 0x123h !
			return -1;
		}
		else
		{
			// Convert from hex.
			errno = 0;
			iSize = strtol( pStr + iHexOffset, &pNumberEnd, 16 );

			if ( errno == ERANGE || pStr == pNumberEnd )
			{
				return -1;
			}
		}
	}
	else if ( *pNumberEnd != 0 )
	{
		return -2; // something, but unknown
	}

	// too big?
	if ( iSize > static_cast<int64_t>( iLimit ) )
	{
		return -1;
	}

	return static_cast<int>( iSize );
}

//------------------------------------------------------------------------------
// TestParsingSizes
//------------------------------------------------------------------------------
void TestParsingSizes()
{
	PrintRuler( 80 );
	printf( "Test function to develop/debug ParseWithSizeSuffix function.\n" );

	for ( ; ; )
	{
		char buf[ 1024 ];
		printf( "\n> " );
		char* pStr = gets_s( buf, sizeof( buf ) );

		int64_t val;
		val = ParseSizeWithSuffix( pStr );

		printf( "val = %lld\n", val );
	}
}

//------------------------------------------------------------------------------
// PrintError
//------------------------------------------------------------------------------
void PrintError( const char* pName, ... )
{
	char buffer[ 2048 ];

	int count;

	va_list	vl;
	va_start( vl, pName );
	count = vsprintf_s( buffer, sizeof( buffer ), pName, vl );
	va_end( vl );

	if ( gpActiveToolName )
	{
		printf( "%s:", gpActiveToolName );
	}
	else
	{
		printf( "BinaryTools:" );
	}

	printf( " ERROR: %s\n", buffer );
}

//------------------------------------------------------------------------------
// Info
//------------------------------------------------------------------------------
void Info( const char* pName, ... )
{
	char buffer[ 2048 ];

	int count;

	va_list	vl;
	va_start( vl, pName );
	count = vsprintf_s( buffer, sizeof( buffer ), pName, vl );
	va_end( vl );

	if ( gpActiveToolName )
	{
		printf( "%s:", gpActiveToolName );
	}
	else
	{
		printf( "BinaryTools: " );
	}

	printf( " %s", buffer );
}
