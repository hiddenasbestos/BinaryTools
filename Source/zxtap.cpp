
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
// ZXTap
//------------------------------------------------------------------------------
int ZXTap( int argc, char** argv )
{
	if ( argc != 6 )
	{
		PrintHelp( "zxtap" );
		return 1;
	}

	// ... code name
	const char* pCodeName = argv[ 3 ];
	size_t iCodeNameLength = strlen( pCodeName );
	if ( iCodeNameLength > 10 )
	{
		PrintError( "Code name \"%s\" is too long (%d). Must be 10 characters or less.", pCodeName, (int)iCodeNameLength );
		return 1;
	}

	// ... origin address
	int iOrigin = ParseValue( argv[ 4 ], 65535 );
	if ( iOrigin < 0 )
	{
		PrintError( "Invalid origin address \"%s\".", argv[ 4 ] );
		return 1;
	}

	int err;
	FILE* fp_in;
	FILE* fp_out;

	// ... input file
	err = fopen_s( &fp_in, argv[ 2 ], "rb" );
	if ( err != 0 || fp_in == nullptr )
	{
		PrintError( "Cannot open input file \"%s\".", argv[ 2 ] );
		return 1;
	}

	// ... measure the source size.
	fseek( fp_in, 0, SEEK_END );
	int iSourceSize = ftell( fp_in );
	fseek( fp_in, 0, SEEK_SET );

	// ... output file
	err = fopen_s( &fp_out, argv[ 5 ], "wb" );
	if ( err != 0 || fp_in == nullptr )
	{
		PrintError( "Cannot open output file \"%s\".", argv[ 5 ] );
		return 1;
	}


	//
	// -- Write Header

	Info( "Creating CODE block \"%s\", %d bytes at 0x%04X\n", pCodeName, iSourceSize, iOrigin );
	Info( "Writing \"%s\" ... ", argv[ 5 ] );

	uint8_t checkbit;

	/* ZX Spectrum TAP header */
	uint8_t zxtap_header[ 21 ];
	zxtap_header[ 0 ] = 19;
	zxtap_header[ 1 ] = 0;
	zxtap_header[ 2 ] = 0;
	zxtap_header[ 3 ] = 3;

	/* NEXT 10 BYTES ARE FILENAME */
	for ( int count = 0; count < 10; count++ )
	{
		char c = pCodeName[ count ];

		if ( c == 0 )
		{
			for ( /**/; count < 10; ++count )
			{
				zxtap_header[ 4 + count ] = 32;
			}

			break;
		}
		else
		{
			zxtap_header[ 4 + count ] = ( c < 32 ) ? 32 : c;
		}
	}

	/* CODE DATA */
	zxtap_header[ 14 ] = iSourceSize & 0xff;
	zxtap_header[ 15 ] = ( iSourceSize >> 8 ) & 0xff;
	zxtap_header[ 16 ] = iOrigin & 0xff;
	zxtap_header[ 17 ] = ( iOrigin >> 8 ) & 0xff;
	zxtap_header[ 18 ] = 0;
	zxtap_header[ 19 ] = 0x80;

	/* CHECK BIT */
	checkbit = 0;
	for ( int count = 2; count < 20; count++ )
	{
		checkbit = ( checkbit ^ zxtap_header[ count ] );
	}

	zxtap_header[ 20 ] = checkbit;

	// ... write TAP file header block.
	for ( int count = 0; count < 21; ++count )
	{
		fputc( zxtap_header[ count ], fp_out );
	}

	/* DATA HEADER */
	int iDataSize = iSourceSize + 2; // data length + 2 checksums
	zxtap_header[ 0 ] = iDataSize & 0xff;
	zxtap_header[ 1 ] = ( iDataSize >> 8 ) & 0xff;
	zxtap_header[ 2 ] = 255; // checksum 1
	for ( int count = 0; count < 3; ++count )
	{
		fputc( zxtap_header[ count ], fp_out );
	}

	// Copy Actual Data

	checkbit = 255;

	int input;
	
	while ( feof( fp_in ) == 0 )
	{
		input = fgetc( fp_in );

		if ( input == EOF )
			break;

		fputc( input, fp_out );
		checkbit = ( checkbit ^ ( input & 0xFF ) );
	}

	fputc( checkbit, fp_out ); // checksum 2

	printf( "OK\n" );

	// tidy up.
	fclose( fp_in );
	fclose( fp_out );

	return 0;
}

//==============================================================================

