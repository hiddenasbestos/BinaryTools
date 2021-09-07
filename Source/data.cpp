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


// ... up to 255
static int decimalLength( int val )
{
	if ( val < 10 )
	{
		return 1;
	}
	else if ( val < 100 )
	{
		return 2;
	}
	else
	{
		return 3;
	}
}

//------------------------------------------------------------------------------
// Data
//------------------------------------------------------------------------------
int Data( int argc, char** argv )
{
	const char* pInputName = nullptr;
	const char* pOutputName = nullptr;


	enum eOption
	{
		NONE,
		OPT_LINE_NUMBER,
		OPT_COLUMNS,
	};

	eOption specialNextArg = NONE;

	// defaults.
	bool bOptCompact = false;
	int iLineWidth = 40;
	int iLine = -1; // -1 = default - no line numbers
	int iStep = 10;

	// parse arguments (after the tool name)
	for ( int i = 2; i < argc; ++i )
	{
		const char* pArg = argv[ i ];

		if ( specialNextArg != NONE )
		{
			switch ( specialNextArg )
			{

			case OPT_COLUMNS:

				{
					int iValue;
					char* pEnd = nullptr;
					iValue = strtol( pArg, &pEnd, 10 );

					if ( iValue >= 20 )
					{
						iLineWidth = iValue;
					}
					else if ( *pEnd != 0 )
					{
						// error.
						PrintError( "Invalid -cols parameter \"%s\".", pArg );
						return 1;
					}
					else if ( iValue < 20 )
					{
						// error.
						PrintError( "Invalid -cols width %d. Must be 20 or more.", iValue );
						return 1;
					}
				}

				break;

			case OPT_LINE_NUMBER:

				{
					int iValue;
					char* pEnd = nullptr;
					iValue = strtol( pArg, &pEnd, 10 );

					if ( iValue >= 0 )
					{
						iLine = iValue;
					}
					else
					{
						// error.
						PrintError( "Invalid -line number \"%d\".", iValue );
						return 1;
					}

					if ( pEnd[0] == ',' && pEnd[1] != 0 )
					{
						iValue = strtol( pEnd + 1, &pEnd, 10 );

						if ( iValue > 0 && iValue <= 100 )
						{
							iStep = iValue;
						}
						else
						{
							// error.
							PrintError( "Invalid -line step \"%d\".", iValue );
							return 1;
						}
					}
					else if ( *pEnd != 0 )
					{
						// error.
						PrintError( "Invalid -line parameter \"%s\".", pArg );
						return 1;
					}
				}

				break;

			}

			specialNextArg = NONE;
		}
		else if ( *pArg == '-' )
		{
			if ( _stricmp( pArg, "-compact" ) == 0 )
			{
				bOptCompact = true;
			}
			else if ( _stricmp( pArg, "-line" ) == 0 )
			{
				specialNextArg = OPT_LINE_NUMBER;
			}
			else if ( _stricmp( pArg, "-cols" ) == 0 )
			{
				specialNextArg = OPT_COLUMNS;
			}
			else
			{
				// error.
				PrintHelp( "data" );
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
			PrintHelp( "data" );
			return 1;
		}
	}

	if ( pInputName == nullptr || pOutputName == nullptr )
	{
		PrintHelp( "data" );
		return 1;
	}


	int err, count;
	FILE* fp_in;
	FILE* fp_out;

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
		PrintError( "Cannot open output file \"%s\"", pOutputName );
		return 1;
	}

	if ( iLine >= 0 )
	{
		Info( "Writing DATA from line %d to \"%s\" ... ", iLine, pOutputName );
	}
	else
	{
		Info( "Writing DATA to \"%s\" ... ", pOutputName );
	}

	int iLineLength = 0;

	while ( feof( fp_in ) == 0 )
	{
		int input = fgetc( fp_in );

		if ( input == EOF )
			break;

		// existing line in progress?
		if ( iLineLength > 0 )
		{
			// measure next piece of data and the previous delimiter
			int iUnitLength = decimalLength( input );
			iUnitLength += bOptCompact ? 2 : 1;
			
			// room for delimiter and another piece of data?
			if ( iLineLength + iUnitLength < iLineWidth )
			{
				count = fprintf( fp_out, bOptCompact ? "," : ", " );
				iLineLength += count;
			}
			else
			{
				// end of line.
				fprintf( fp_out, "\n" );
				iLineLength = 0;

				if ( iLine >= 0 )
				{
					iLine += iStep;
				}
			}
		}
		
		// begin a new line?
		if ( iLineLength == 0 )
		{
			count = 0;

			if ( iLine >= 0 )
			{
				count += fprintf( fp_out, "%d ", iLine );
			}

			count += fprintf( fp_out, "DATA " );

			iLineLength += count;
		}

		count = fprintf( fp_out, "%d", input );
		iLineLength += count;
	}

	fprintf( fp_out, "\n" );

	// Tidy up
	printf( "OK\n" );
	fclose( fp_in );
	fclose( fp_out );

	return 0;
}

//==============================================================================

