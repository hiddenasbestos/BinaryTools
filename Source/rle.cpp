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
#include <vector>

#include "utils.h"

enum Endian
{
	LITTLE_ENDIAN,
	BIG_ENDIAN
};

// Simple RLE encoder.
template< typename T >
struct SimpleRleEncoder
{
	FILE* fp_out;
	int _reps;
	bool _bCtrlIsByte;
	Endian _endian;
	int _iMaxCount;

	std::vector< T > _rawbuf;

	SimpleRleEncoder( FILE* fp, bool bCtrlIsByte, Endian endian ) :

		_reps( 0 ),
		_bCtrlIsByte( bCtrlIsByte ),
		_endian( endian ),
		fp_out( fp )
	{
		//
		if ( _bCtrlIsByte )
		{
			// we only have 7
			_iMaxCount = 127;
		}
		else
		{
			// fill all available lower bits.
			_iMaxCount = ( 1 << ( ( sizeof( T ) * 8 ) - 1 ) ) - 1;
		}

		BeginPlane();
	}

	void BeginPlane()
	{
		_reps = 0;
		_rawbuf.clear();
	}

	void WriteBigEndian( T val )
	{
		uint8_t* p = (uint8_t*)&val;

		for ( int i = sizeof( T ) - 1; i >= 0; --i )
		{
			fputc( p[i], fp_out );
		}
	}

	void Add( T data )
	{
		if ( _rawbuf.empty() )
		{
			_rawbuf.push_back( data );
		}
		else
		{
			T lastData = _rawbuf[ _rawbuf.size() - 1 ];

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
				if ( _reps == _iMaxCount - 1 )
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
				if ( _rawbuf.size() == _iMaxCount )
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

			if ( _bCtrlIsByte )
			{
				uint8_t ctrl;
				ctrl = 0x80 | static_cast<uint8_t>( _reps + 1 ); // +1 to count initial ambiguous value
				fputc( ctrl, fp_out );
			}
			else
			{
				T ctrl;
				ctrl = ( 1 << ( ( sizeof( T ) * 8 ) - 1 ) ) | static_cast< uint8_t >( _reps + 1 ); // +1 to count initial ambiguous value

				if ( _endian == BIG_ENDIAN )
				{
					WriteBigEndian( ctrl );
				}
				else
				{
					fwrite( &ctrl, sizeof( T ), 1, fp_out );
				}
			}
			
			// ... data word
			T repData = _rawbuf[ 0 ];
			fwrite( &repData, sizeof( T ), 1, fp_out );
		}
		else if ( _rawbuf.empty() == false )
		{
			// Noisy data.

			if ( _bCtrlIsByte )
			{
				uint8_t ctrl;
				ctrl = static_cast<uint8_t>( _rawbuf.size() );
				fputc( ctrl, fp_out );
			}
			else
			{
				T ctrl;
				ctrl = static_cast<T>( _rawbuf.size() );
				
				if ( _endian == BIG_ENDIAN )
				{
					WriteBigEndian( ctrl );
				}
				else
				{
					fwrite( &ctrl, sizeof( T ), 1, fp_out );
				}
			}

			// ... data words
			for ( T ch : _rawbuf )
			{
				fwrite( &ch, sizeof( T ), 1, fp_out );
			}
		}

		_rawbuf.clear();
		_reps = 0;
	}

};

// Simple 8-bit RLE
static void SimpleRLE8( FILE* fp_out, int iPlanes, uint8_t* pInputData, int iInputSize )
{
	SimpleRleEncoder< uint8_t > enc8( fp_out, true, LITTLE_ENDIAN );

	for ( int iPlane = 0; iPlane < iPlanes; ++iPlane )
	{
		enc8.BeginPlane();

		// Read input, handle planes here - the encoder is unaware.
		for ( int iCursor = iPlane; iCursor < iInputSize; iCursor += iPlanes )
		{
			const uint8_t* pInput = reinterpret_cast<const uint8_t*>( pInputData + iCursor );
			enc8.Add( *pInput );
		}

		enc8.Flush();

		// end of plane.
		fputc( 0, fp_out );
	}
}

/*
// Simple 16-bit RLE Big-Endian (68000?)
static void SimpleRLE16BE( FILE* fp_out, int iPlanes, uint8_t* pInputData, int iInputSize )
{
	SimpleRleEncoder< uint16_t > enc16( fp_out, false, BIG_ENDIAN );

	for ( int iPlane = 0; iPlane < iPlanes; ++iPlane )
	{
		enc16.BeginPlane();

		// Read input, handle planes here - the encoder is unaware.
		for ( int iCursor = iPlane; iCursor < iInputSize; iCursor += iPlanes * 2 )
		{
			const uint16_t* pInput = reinterpret_cast<const uint16_t*>( pInputData + iCursor );
			enc16.Add( *pInput );
		}

		enc16.Flush();

		// end of plane.
		fputc( 0, fp_out );
	}
}
*/

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
	bool bOptAppend = false;
	int iPlanes = 1;
	int iWordSize = 1; // TODO: Other word sizes / algorithms

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
			else if ( _stricmp( pArg, "-append" ) == 0 )
			{
				bOptAppend = true;
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
	err = fopen_s( &fp_out, pOutputName, bOptAppend ? "ab" : "wb" );
	if ( err != 0 || fp_out == nullptr )
	{
		printf( "FAILED\n" );
		PrintError( "Cannot open output file \"%s\"", pOutputName );
		return 1;
	}
	
	if ( bOptAppend )
	{
		fseek( fp_out, 0, SEEK_END );
	}

	int fp_out_start = ftell( fp_out );

	fseek( fp_in, 0, SEEK_END );
	int iInputSize = ftell( fp_in );
	rewind( fp_in );

	// round up to word size, padding with zero
	int iAllocSize = iInputSize;
	while ( iAllocSize % iWordSize )
	{
		++iAllocSize;
	}

	uint8_t* pInputData;
	pInputData = (uint8_t*)malloc( iAllocSize );
	if ( pInputData == NULL )
	{
		printf( "FAILED\n" );
		PrintError( "Couldn't allocate memory for the file." );
		return 1;
	}

	// Read file data
	fread( pInputData, sizeof( char ), iInputSize, fp_in );
	fclose( fp_in );

	// Pad the padding space with zeros.
	uint8_t* pPad = pInputData + iInputSize;
	for ( int i = iInputSize; i < iAllocSize; ++i )
	{
		*pPad++ = 0;
	}

	// Encode
	switch ( iWordSize )
	{

	case 1:
		SimpleRLE8( fp_out, iPlanes, pInputData, iInputSize );
		break;

	/*case 2:
		SimpleRLE16BE( fp_out, iPlanes, pInputData, iInputSize );
		break;*/

	}; // switch ( iWordSize )

	// 
	int fp_out_len = ftell( fp_out ) - fp_out_start;

	printf( "OK (%d", iAllocSize );

	// indicate padding.
	if ( iAllocSize != iInputSize )
	{
		putchar( '*' );
	}

	printf( " -> %d bytes)\n", fp_out_len );

	// Tidy up
	free( pInputData );
	fclose( fp_out );

	return 0;
}

//==============================================================================

