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


enum eValueFormat
{
	DECIMAL,
	HEX_0X,
	HEX_DOLLAR,
	HEX_AMPERSAND,
	BIN_0B,
	BIN_PERCENT,
	OCTAL
};

enum eStatement
{
	BASIC_DATA,
	ASM_DOTBYTE,
	ASM_DB,
	ASM_DCB,
	ANSI_C
};

// ... values up to 255
static int valueLength( int val, eValueFormat mode )
{
	switch ( mode )
	{

	default:
	case DECIMAL:
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

	case HEX_0X:
		return 4; // 0x##

	case HEX_AMPERSAND:
	case HEX_DOLLAR:
		return 3; // $## or &##

	case BIN_0B:
		return 10; // 0x########

	case BIN_PERCENT:
		return 9; // %########

	case OCTAL:
		if ( val < 8 )
		{
			return 2; // 0#
		}
		else if ( val < 0100 /*64*/ )
		{
			return 3; // 0##
		}
		else
		{
			return 4; // 0###
		}

	};
}

static int write_byte_binary( uint8_t input, FILE* fp_out )
{
	for ( int i = 0; i < 8; ++i )
	{
		fputc( ( input & 0x80 ) ? '1' : '0', fp_out );
		input <<= 1;
	}

	return 8; // helps count output bytes
}

static int write_spaces( int count, FILE* fp_out )
{
	if ( count <= 0 )
		return 0;

	for ( int i = 0; i < count; ++i )
	{
		fputc( ' ', fp_out );
	}

	return count;
}

static int write_tabs( int count, FILE* fp_out )
{
	for ( int i = 0; i < count; ++i )
	{
		fputc( '\t', fp_out );
	}

	return count;
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
		OPT_TABS,
		OPT_SPACES,
	};

	eOption specialNextArg = NONE;

	// defaults.
	eStatement statement = BASIC_DATA;
	int iTabs = 1;
	int iSpaces = 0;
	bool bOptCompact = false;
	eValueFormat valueFormat = DECIMAL;
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

			case OPT_TABS:

				{
					int iValue;
					char* pEnd = nullptr;
					iValue = strtol( pArg, &pEnd, 10 );

					if ( *pEnd != 0 )
					{
						// error.
						PrintError( "Invalid -tabs parameter \"%s\".", pArg );
						return 1;
					}
					else
					{
						iTabs = iValue;
						iSpaces = 0;
					}
				}

				break;

			case OPT_SPACES:

				{
					int iValue;
					char* pEnd = nullptr;
					iValue = strtol( pArg, &pEnd, 10 );

					if ( *pEnd != 0 )
					{
						// error.
						PrintError( "Invalid -spc parameter \"%s\".", pArg );
						return 1;
					}
					else
					{
						iSpaces = iValue;
						iTabs = 0;
					}
				}

				break;

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
			else if ( _stricmp( pArg, "-basic" ) == 0 )
			{
				statement = BASIC_DATA;
			}
			else if ( _stricmp( pArg, "-c" ) == 0 )
			{
				statement = ANSI_C;
			}
			else if ( _stricmp( pArg, "-db" ) == 0 )
			{
				statement = ASM_DB;
			}
			else if ( _stricmp( pArg, "-dcb" ) == 0 )
			{
				statement = ASM_DCB;
			}
			else if ( _stricmp( pArg, "-dotbyte" ) == 0 )
			{
				statement = ASM_DOTBYTE;
			}
			else if ( _stricmp( pArg, "-tab" ) == 0 )
			{
				specialNextArg = OPT_TABS;
			}
			else if ( _stricmp( pArg, "-spc" ) == 0 )
			{
				specialNextArg = OPT_SPACES;
			}
			else if ( _stricmp( pArg, "-line" ) == 0 )
			{
				specialNextArg = OPT_LINE_NUMBER;
			}
			else if ( _stricmp( pArg, "-cols" ) == 0 )
			{
				specialNextArg = OPT_COLUMNS;
			}
			else if ( _stricmp( pArg, "-dec" ) == 0 )
			{
				valueFormat = DECIMAL;
			}
			else if ( _stricmp( pArg, "-hex" ) == 0 )
			{
				valueFormat = HEX_0X;
			}
			else if ( _stricmp( pArg, "-bux" ) == 0 )
			{
				valueFormat = HEX_DOLLAR;
			}
			else if ( _stricmp( pArg, "-amp" ) == 0 )
			{
				valueFormat = HEX_AMPERSAND;
			}
			else if ( _stricmp( pArg, "-bin" ) == 0 )
			{
				valueFormat = BIN_0B;
			}
			else if ( _stricmp( pArg, "-pct" ) == 0 )
			{
				valueFormat = BIN_PERCENT;
			}
			else if ( _stricmp( pArg, "-oct" ) == 0 )
			{
				valueFormat = OCTAL;
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

	Info( "Writing " );

	switch ( statement )
	{
	default:
	case BASIC_DATA:
		printf( "DATA" );
		break;

	case ASM_DOTBYTE:
		printf( ".BYTE" );
		break;

	case ASM_DB:
		printf( "DB" );
		break;

	case ASM_DCB:
		printf( "DC.B" );
		break;

	case ANSI_C:
		printf( "C/C++" );
		break;

	}

	if ( iLine >= 0 )
	{
		printf( " from line %d", iLine );
	}

	printf( " to \"%s\" ... ", pOutputName );

	int iLineLength = 0;

	while ( feof( fp_in ) == 0 )
	{
		int input = fgetc( fp_in );

		if ( input == EOF )
			break;

		// existing line in progress?
		if ( iLineLength > 0 )
		{
			int iUnitLength;

			// measure next piece of data and the previous delimiter
			iUnitLength = valueLength( input, valueFormat );
			iUnitLength += bOptCompact ? 1 : 2;
			// .. and the possible trailing delimiter for EOL
			if ( statement == ANSI_C )
			{
				++iUnitLength;
			}

			// room for delimiter and another piece of data?
			if ( ( iLineWidth < 0 ) || ( iLineLength + iUnitLength < iLineWidth ) )
			{
				count = fprintf( fp_out, bOptCompact ? "," : ", " );
				iLineLength += count;
			}
			else
			{
				// end of line.
				count = fprintf( fp_out, ( statement == ANSI_C ) ? ",\n" : "\n" );
				iLineLength += count;

				// done.
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

			// line number?
			if ( iLine >= 0 )
			{
				count += fprintf( fp_out, "%d ", iLine );
			}

			// spacing.
			if ( ( statement != BASIC_DATA ) && iTabs > 0 && iLine < 0 )
			{
				count += write_tabs( iTabs, fp_out );
			}
			else if ( iSpaces > 0 )
			{
				count += write_spaces( ( iLine >= 0 ) ? iSpaces - 1 : iSpaces, fp_out );
			}
				
			// statement type
			switch ( statement )
			{
			default:
			case BASIC_DATA:
				count += fprintf( fp_out, "DATA " );
				break;

			case ASM_DOTBYTE:
				count += fprintf( fp_out, ".BYTE " );
				break;

			case ASM_DB:
				count += fprintf( fp_out, "db " );
				break;

			case ASM_DCB:
				count += fprintf( fp_out, "dc.b " );
				break;

			case ANSI_C:
				// nothing
				break;

			}

			iLineLength += count;
		}

		switch ( valueFormat )
		{

		case DECIMAL:
			count = fprintf( fp_out, "%d", input );
			break;

		case HEX_0X:
			count = fprintf( fp_out, "0x%X", input );
			break;

		case HEX_DOLLAR:
			count = fprintf( fp_out, "$%X", input );
			break;

		case HEX_AMPERSAND:
			count = fprintf( fp_out, "&%X", input );
			break;

		case BIN_0B:
			count = fprintf( fp_out, "0b" );
			count += write_byte_binary( input, fp_out );
			break;

		case BIN_PERCENT:
			count = fprintf( fp_out, "%%" );
			count += write_byte_binary( input, fp_out );
			break;

		case OCTAL:
			count = fprintf( fp_out, "0%o", input );
			break;

		}

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

