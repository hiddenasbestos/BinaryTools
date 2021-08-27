
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

#include "utils.h"

//------------------------------------------------------------------------------
// Pad
//------------------------------------------------------------------------------
int Pad( int argc, char** argv )
{
	if ( argc < 4 || argc > 5 )
	{
		PrintHelp( "pad" );
		return 1;
	}

	const char* pFile = argv[ 2 ];
	const char* pSize = argv[ 3 ];

	int iFillByte = 0x00; // Default
	if ( argc == 5 )
	{
		const char* pFill = argv[ 4 ];
		iFillByte = ParseValue( pFill, 255 );

		if ( iFillByte < 0 )
		{
			printf( "ERROR: Invalid fill byte value \"%s\"\n\n", pFill );
			return 1;
		}
	}

	// ... determine size.
	int64_t iNewSize = ParseSizeWithSuffix( pSize );
	if ( iNewSize < 0 )
	{
		printf( "ERROR: Invalid size \"%s\"\n\n", pSize );
		PrintHelp( "pad" );
		return 1;
	}

	int err;
	FILE* fp;
	bool bNewFile = false;

	// ... file exists?
	err = fopen_s( &fp, pFile, "r" );
	if ( err != 0 || fp == nullptr )
	{
		bNewFile = true;
	}
	else
	{
		fclose( fp );
	}

	// ... open file for appending
	err = fopen_s( &fp, pFile, "ab" );
	if ( err != 0 || fp == nullptr )
	{
		printf( "ERROR: Failed to open file \"%s\" for writing.\n\n", pFile );
		return 1;
	}

	// ... seek to the end and measure.
	fseek( fp, 0, SEEK_END );
	int64_t iOldSize = ftell( fp );

	// Say hello
	if ( bNewFile )
	{
		printf( "[BinaryTools] Creating \"%s\" with %lld bytes of 0x%02X ... ", pFile, iNewSize, iFillByte );
	}
	else
	{
		printf( "[BinaryTools] Padding \"%s\" to %lld bytes with 0x%02X ... ", pFile, iNewSize, iFillByte );
	}

	// Pad !
	while ( iOldSize < iNewSize )
	{
		int ret;
		ret = fputc( iFillByte, fp );
		if ( ret == EOF )
		{
			break;
		}

		++iOldSize;
	}

	if ( iOldSize == iNewSize )
	{
		printf( "DONE\n" );
	}
	else
	{
		printf( "FAILED\n" );
	}

	fclose( fp );

	return 0;
}

//==============================================================================

