
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

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

//==============================================================================

//
// checksum function by Dandaman955. 
// "Feel free to do what you want with the code, the source is on GitHub."
// https://www.smspower.org/forums/16629-MasterSystemChecksumFixer#107180
//
static uint16_t checksum( unsigned char* pBuffer, uint16_t CC_Last, uint16_t ChecksumRange, int i )
{
	unsigned char cs1 = ( CC_Last >> 8 ) & 0xFF;
	unsigned char cs2 = CC_Last & 0xFF;
	unsigned char cs3 = 0;  // Artificial carry flag for ADC emulation.
	unsigned char e = 0;
	unsigned char ov1 = 0;
	unsigned char ov2 = 0;

	do
	{
		e = cs2;				// LD A, E
		ov1 = e;				// Set first overflow check flag.
		e += pBuffer[ i ];		// ADD A, (HL)
		ov2 = e;				// Set the second overflow check flag.
		if ( ov1 > ov2 )		// Is the last value larger than the first (indicating overflow)?
		{
			cs3 = 1;			// If it is, set the carry for the adc instruction.
		}
		cs2 = e;				// LD E, A
		e = cs1;				// LD A, D
		e += cs3;				// ADC A, $00
		cs3 = 0;				// Reset carry flag.
		cs1 = e;				// LD, D, A
		i++;					// INC HL
		ChecksumRange--;		// DEC BC
	}
	while ( ChecksumRange );

	CC_Last = ( cs1 << 8 ) & 0xFF00;
	CC_Last |= cs2;
	
	return CC_Last;
}

// Size options, for user friendly display.
static const char* ROMHeaderStr[ 16 ] =
{
	"256KB", "512KB", "1MB", "???", "???", "???", "???", "???", "???", "???", "8KB", "16KB", "32KB", "48KB", "64KB", "128KB"
};

//==============================================================================

//------------------------------------------------------------------------------
// SMSChk
//------------------------------------------------------------------------------
int SMSChk( int argc, char** argv )
{
	if ( argc < 3 )
	{
		PrintHelp( "smschk" );
		return 1;
	}

	uint16_t TMRValues[ 3 ] = { 0x1FF0, 0x3FF0, 0x7FF0 };
	uint8_t ChecksumRanges[ 9 ] = { 0x1F, 0x3F, 0x7F, 0xBF, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F };
	uint8_t ROMPages[ 4 ] = { 0x02, 0x06, 0x0E, 0x1E };
	uint8_t TMR[ 10 ] = "TMR SEGA";
	TMR[ 8 ] = 0xFF;
	TMR[ 9 ] = 0xFF;
	
	unsigned char* buffer;
	int i;
	int j;
	short TMRStart;
	short TMRAutoGen = 0;
	bool bHeaderDetected = false;

	int err;
	FILE* fp_in;
	FILE* fp_out;

	const char* pRomFile = argv[ 2 ];

	// ... input file
	err = fopen_s( &fp_in, pRomFile, "rb" );
	if ( err != 0 || fp_in == nullptr )
	{
		PrintError( "Cannot open ROM file \"%s\".", pRomFile );
		return 1;
	}

	Info( "Loading ROM: \"%s\" ", pRomFile );

	fseek( fp_in, 0, SEEK_END );
	int fsize = ftell( fp_in );
	rewind( fp_in );

	printf( "(%d bytes)\n", fsize );

	buffer = (unsigned char*)malloc( fsize );
	if ( buffer == NULL )
	{
		PrintError( "Couldn't allocate memory for the file." );
		return 1;
	}

	fread( buffer, sizeof( char ), fsize, fp_in );
	fclose( fp_in );

	Info( "Looking for \"TMR SEGA\" header ... " );

	// Detect the TMR_SEGA header at locations 0x1FF0, 0x3FF0 or 0x7FF0.
	for ( j = 0; j < 3; j++ )
	{
		TMRStart = TMRValues[ j ];

		// Small ROM?
		if ( TMRStart > fsize )
			break;

		TMRAutoGen = TMRStart;

		bHeaderDetected = true; // assume so

		for ( i = 0; i < 8; i++ )
		{
			if ( buffer[ TMRStart + i ] != TMR[ i ] )
			{
				bHeaderDetected = false;
				break;
			}
		}

		if ( bHeaderDetected )
		{
			printf( "found at 0x%02X\n", TMRStart );
			break;
		}
	}

	if ( bHeaderDetected == false )
	{
		if ( TMRAutoGen == 0 )
		{
			printf( "not found\n" );
			PrintError( "Couldn't create header." );
			free( buffer );
			return 1;
		}
		else
		{
			printf( "adding at 0x%02X\n", TMRAutoGen );

			TMRStart = TMRAutoGen;

			for ( i = 0; i < 10; i++ )
			{
				buffer[ TMRStart + i ] = TMR[ i ];
			}
		}
	}

	// Calculate ROM size
	unsigned char ROMHeader;
	int fsize8KB = fsize / 8192;

	ROMHeader = buffer[ TMRStart + 0x0F ] & 0x0F;

	if ( fsize8KB == 1 )
		ROMHeader = 0xA; // 8KB
	else if ( fsize8KB == 2 )
		ROMHeader = 0xB; // 16KB
	else if ( fsize8KB == 4 )
		ROMHeader = 0xC; // 32KB
	else if ( fsize8KB == 6 )
		ROMHeader = 0xD; // 48KB
	else if ( fsize8KB == 8 )
		ROMHeader = 0xE; // 64KB
	else if ( fsize8KB == 16 )
		ROMHeader = 0xF; // 128KB
	else if ( fsize8KB == 32 )
		ROMHeader = 0x0; // 256KB
	else if ( fsize8KB == 64 )
		ROMHeader = 0x1; // 512KB
	else if ( fsize8KB == 128 )
		ROMHeader = 0x2; // 1MB

	buffer[ TMRStart + 0x0F ] = ( buffer[ TMRStart + 0x0F ] & 0xF0 ) | ROMHeader;

	//  printf( "Size code = 0x%X\n", ROMHeader );

	ROMHeader = ( buffer[ TMRStart + 0x0F ] - 0x0A ) & 0x0F;
	uint16_t ChecksumRange = ( ( ChecksumRanges[ ROMHeader ] << 8 ) & 0xFF00 ) | 0xF0;

	//  printf( "Scan (0 - %d)... ", ChecksumRange );

	uint16_t ComputedChecksum = 0;
	ComputedChecksum = checksum( buffer, ComputedChecksum, ChecksumRange, 0 );
	int ROMPage;

	if ( ROMHeader > 3 )
	{
		ROMPage = ROMPages[ ROMHeader - 4 ] - 1;
		int i = 0x8000;

		for ( ; ; )
		{
			ComputedChecksum = checksum( buffer, ComputedChecksum, 0x4000, i );
			if ( ROMPage == 0 )
			{
				break;
			}
			ROMPage--;
			i += 0x4000;
		}
	}

	// Open the file for writing.

	ROMHeader = buffer[ TMRStart + 0x0F ] & 0x0F;
	Info( "Checksum = 0x%04X; Size Code = 0x%X (%s)\n", ComputedChecksum, ROMHeader, ROMHeaderStr[ ROMHeader ] );

	unsigned char Region;
	Region = buffer[ TMRStart + 0x0F ] >> 4;
	if ( Region != 3 /*SMS Japan*/ && Region != 4 /*SMS Export*/ )
	{
		// Only the export SMS BIOS actually checks this, so we use that code.
		Info( "Changing region to \"SMS Export\"\n" );
		buffer[ TMRStart + 0xF ] = ROMHeader | 0x40;
	}

	// ... output file
	err = fopen_s( &fp_out, pRomFile, "wb" );
	if ( err != 0 || fp_in == nullptr )
	{
		PrintError( "Cannot open output file \"%s\".", pRomFile );
		free( buffer );
		return 1;
	}

	Info( "Writing \"%s\" ... ", pRomFile );

	// Updating the new checksum.

	buffer[ TMRStart + 0xA ] = ComputedChecksum & 0xFF;
	buffer[ TMRStart + 0xB ] = ( ComputedChecksum >> 8 ) & 0xFF;
	fwrite( buffer, 1, fsize, fp_out );

	// Program-closing goodies.

	free( buffer );
	fclose( fp_out );

	printf( "OK\n" );

	return 0;
}

//==============================================================================
