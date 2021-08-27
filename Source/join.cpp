
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
// Join
//------------------------------------------------------------------------------
int Join( int argc, char** argv )
{
	if ( argc < 4 )
	{
		PrintHelp( "join" );
		return 1;
	}

	const char* pOutputName = argv[ argc - 1 ];
	FILE* fp_out;
	int err;


	// ... output file
	err = fopen_s( &fp_out, pOutputName, "wb" );
	if ( err != 0 || fp_out == nullptr )
	{
		PrintError( "Cannot open output file \"%s\"", pOutputName );
		return 1;
	}

	Info( "Joining %d files. Writing \"%s\" ... ", argc - 3, pOutputName );

	// ... all inputs
	for ( int arg = 2; arg < argc - 1; ++arg )
	{
		const char* pInputName = argv[ arg ];
		FILE* fp_in;

		err = fopen_s( &fp_in, pInputName, "rb" );
		if ( err != 0 || fp_in == nullptr )
		{
			printf( "FAILED\n" );
			PrintError( "Cannot open input file \"%s\"", pInputName );
			fclose( fp_out );
			return 1;
		}
		else
		{
			// ... copy data
			while ( feof( fp_in ) == 0 )
			{
				int input = fgetc( fp_in );

				if ( input == EOF )
					break;

				fputc( input, fp_out );
			}

			fclose( fp_in );
		}
	}

	// Tidy up
	printf( "OK\n" );
	fclose( fp_out );

	return 0;
}

//==============================================================================

