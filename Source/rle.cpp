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
#include <cstdlib>

#include "utils.h"

#include <vector>


// 8-bit RLE encoder.
struct Rle8Encoder
{
	FILE* fp_out;
	int _reps;
	enum { MAX_COUNT = 127 };

	std::vector< uint8_t > _rawbuf;

	Rle8Encoder( FILE* fp ) :
		_reps( 0 ),
		fp_out( fp )
	{
		//
	}

	void Add( uint8_t data )
	{
		if ( _rawbuf.empty() )
		{
			_rawbuf.push_back( data );
		}
		else
		{
			uint8_t lastData = _rawbuf[ _rawbuf.size() - 1 ];

			if ( data == lastData )
			{
				// We had a noise sequence? Flush that first.
				if ( _reps == 0 && _rawbuf.size() >= 2 )
				{
					// Don't flush the final entry as it's really the first
					// of a new uniform repeated sequence.
					_rawbuf.pop_back();

					Flush();

					// Replace the new starting character
					_rawbuf.push_back( data );
				}

				// Count the repeated character.
				++_reps;

				// Overflow?
				if ( _reps == MAX_COUNT - 1 )
				{
					Flush();
				}
			}
			else
			{
				// We had a repeated sequence? Flush that first.
				if ( _reps )
				{
					Flush();
				}

				// Add a new character
				_rawbuf.push_back( data );

				// Overflow?
				if ( _rawbuf.size() == MAX_COUNT )
				{
					Flush();
				}
			}
		}
	}

	void Flush()
	{
		if ( _reps )
		{
			// Uniform data.

			uint8_t repData = _rawbuf[ 0 ];
			
			uint8_t ctrl;
			ctrl = 0x80 | static_cast< uint8_t >( _reps + 1 ); // +1 to count initial ambiguous value
			fputc( ctrl, fp_out );
			fputc( repData, fp_out );
		}
		else if ( _rawbuf.empty() == false )
		{
			// Noisy data.

			uint8_t ctrl;
			ctrl = static_cast< uint8_t >( _rawbuf.size() );
			fputc( ctrl, fp_out );

			for ( uint8_t ch : _rawbuf )
			{
				fputc( ch, fp_out );
			}
		}

		_rawbuf.clear();
		_reps = 0;
	}

};


//------------------------------------------------------------------------------
// RLE
//------------------------------------------------------------------------------
int RLE( int argc, char** argv )
{
	const char* pInputName = nullptr;
	const char* pOutputName = nullptr;


	enum eOption
	{
		NONE,
		OPT_PLANES,
	};

	eOption specialNextArg = NONE;

	// defaults.
	int iPlanes = 1;

	// parse arguments (after the tool name)
	for ( int i = 2; i < argc; ++i )
	{
		const char* pArg = argv[ i ];

		if ( specialNextArg != NONE )
		{
			switch ( specialNextArg )
			{

			case OPT_PLANES:

				{
					int iValue;
					char* pEnd = nullptr;
					iValue = strtol( pArg, &pEnd, 10 );

					if ( iValue > 0 )
					{
						iPlanes = iValue;
					}
					else if ( *pEnd != 0 )
					{
						// error.
						PrintError( "Invalid -planes parameter \"%s\".", pArg );
						return 1;
					}
					else
					{
						// error.
						PrintError( "Invalid -planes %d. Must be 1 or more.", iValue );
						return 1;
					}
				}

				break;

			}

			specialNextArg = NONE;
		}
		else if ( *pArg == '-' )
		{
			if ( _stricmp( pArg, "-planes" ) == 0 )
			{
				specialNextArg = OPT_PLANES;
			}
			else
			{
				// error.
				PrintHelp( "rle" );
				return 1;
			}
		}
		else if ( pInputName == nullptr )
		{
			pInputName = pArg;
		}
		else if ( pOutputName == nullptr )
		{
			pOutputName = pArg;
		}
		else
		{
			// error.
			PrintHelp( "rle" );
			return 1;
		}
	}

	if ( pInputName == nullptr || pOutputName == nullptr )
	{
		PrintHelp( "rle" );
		return 1;
	}


	int err;
	FILE* fp_in;
	FILE* fp_out;

	Info( "Encoding \"%s\"", pInputName );

	if ( iPlanes > 1 )
	{
		printf( " (%d planes)", iPlanes );
	}

	printf( " ... " );

	// ... open input.
	err = fopen_s( &fp_in, pInputName, "rb" );
	if ( err != 0 || fp_in == nullptr )
	{
		printf( "FAILED\n" );
		PrintError( "Cannot open input file \"%s\"", pInputName );
		return 1;
	}

	// ... output file
	err = fopen_s( &fp_out, pOutputName, "wb" );
	if ( err != 0 || fp_out == nullptr )
	{
		printf( "FAILED\n" );
		PrintError( "Cannot open output file \"%s\"", pOutputName );
		return 1;
	}

	fseek( fp_in, 0, SEEK_END );
	int iInputSize = ftell( fp_in );
	rewind( fp_in );

	unsigned char* pInputData;
	pInputData = (unsigned char*)malloc( iInputSize );
	if ( pInputData == NULL )
	{
		printf( "FAILED\n" );
		PrintError( "Couldn't allocate memory for the file." );
		return 1;
	}

	fread( pInputData, sizeof( char ), iInputSize, fp_in );
	fclose( fp_in );

	for ( int iPlane = 0; iPlane < iPlanes; ++iPlane )
	{
		// Create an encoder for this plane.
		Rle8Encoder enc( fp_out );

		// Read input, handle planes here - the encoder is unaware.
		for ( int iCursor = iPlane; iCursor < iInputSize; iCursor += iPlanes )
		{
			const uint8_t* pInput = pInputData + iCursor;

			enc.Add( *pInput );
		}

		enc.Flush();

		// end of plane.
		fputc( 0, fp_out );
	}
	
	// 
	int fp_out_len = ftell( fp_out );
	printf( "OK (%d -> %d bytes)\n", iInputSize, fp_out_len );
	
	// Tidy up
	free( pInputData );
	fclose( fp_out );

	return 0;
}

//==============================================================================

