#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <d2mce.h>

#include "../../src/d2mce.h"

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;

#define BITMAPFILEHEADERSIZE    14
#define BITMAPINFOHEADERSIZE    40
#define NODES 2

typedef struct tagBMP_header {
        WORD         bfType;
        DWORD        bfSize;
        WORD         bfReserved1;
        WORD         bfReserved2;
        DWORD        bfOffBits;
} BMP_header;

typedef struct tagBMP_info {
        DWORD        biSize;
        long         biWidth;
        long         biHeight;
        WORD         biPlanes;
        WORD         biBitCount;

        DWORD        biCompression;
        DWORD        biSizeImage;
        long         biXPelsPerMeter;
        long         biYPelsPerMeter;
        DWORD        biClrUsed;
        DWORD        biClrImportant;
} BMP_info;

BMP_header	bg_header;
BMP_info	bg_info;
BMP_header	now_header;
BMP_info	now_info;

void print_header_and_info_value( void )
{
	printf( "%X\n", bg_header.bfType );
	printf( "%X\n", bg_header.bfSize );
	printf( "%X\n", bg_header.bfReserved1 );
	printf( "%X\n", bg_header.bfReserved2 );
	printf( "%X\n", bg_header.bfOffBits );

	printf( "%X\n", bg_info.biSize );
	printf( "Width = %d\n", bg_info.biWidth );
	printf( "Height = %d\n", bg_info.biHeight );
	printf( "%X\n", bg_info.biPlanes );
	printf( "%X\n", bg_info.biBitCount );
	printf( "%X\n", bg_info.biCompression );
	printf( "SizeImage = %d\n", bg_info.biSizeImage );
	printf( "%X\n", bg_info.biXPelsPerMeter );
	printf( "%X\n", bg_info.biYPelsPerMeter );
	printf( "%X\n", bg_info.biClrUsed );
	printf( "%X\n", bg_info.biClrImportant );
}

int main( int argc, char *argv[] )
{
	int i, j, n, count, limit;
	int rgb_size;
	BYTE R, G, B;
	char bg_filename[10];
	char now_filename[10];
	int imagesize, width, height;
	unsigned long result[100] = {0};
	unsigned long *result_ptr = result;
	double time;
	BYTE *bg_buffer;
	BYTE *now_buffer;
	BYTE *bg_ptr;
	BYTE *now_ptr;

	BYTE *bg_rgb;
	BYTE *now_rgb;
	BYTE *bg_rgb_ptr;
	BYTE *now_rgb_ptr;

	FILE *fp_bg_in, *fp_now_in;
	BYTE temp;

	limit = atoi( argv[1] );

	/* set filename */
	strcpy( bg_filename, "bg.bmp" );
	strcpy( now_filename, "1.bmp" );

	fp_bg_in = fopen( bg_filename, "rb" );
	if ( fp_bg_in == NULL ) {
		perror( "open backgroud bmp file error" );
		exit ( EXIT_FAILURE );
	}


	/* Read Background Bitmap File Header and Information */
	n = fread( &bg_header, 1, BITMAPFILEHEADERSIZE, fp_bg_in );
	if ( n != BITMAPFILEHEADERSIZE ) {
		perror( "read header from bmp file error" );
		exit( EXIT_FAILURE );
	}

	n = fread( &bg_info, 1, BITMAPINFOHEADERSIZE, fp_bg_in );
	if ( n != BITMAPINFOHEADERSIZE ) {
		perror( "read information from bmp file error" );
		exit( EXIT_FAILURE );
	}


	width = bg_info.biWidth;
	height = bg_info.biHeight;
	imagesize = width * height;

	/* allocate image buffer */
	bg_buffer = (BYTE *) malloc ( sizeof(BYTE) * imagesize );
	if ( bg_buffer == NULL ) {
		perror( "allocate memory of bg_buffer error" );
		exit ( EXIT_FAILURE );
	}
	bg_ptr = bg_buffer;

	now_buffer = (BYTE *) malloc ( sizeof(BYTE) * imagesize );
	if ( now_buffer == NULL ) {
		perror( "allocate memory of now_buffer error" );
		exit ( EXIT_FAILURE );
	}
	now_ptr = now_buffer;

	/* print value */
	print_header_and_info_value();
	printf( "---------------------------------------------\n" );

#if 1
	d2mce_mutex_t lock;
	d2mce_barrier_t b1;
	int task_id;
	BYTE *share_data;

	/* Initiation system */
	d2mce_init();

	/* Join and get task id */
	task_id = d2mce_join( "motion", "eps",0 );

	/* Initiation lock */
	d2mce_mutex_init( &lock, "lock" );
	d2mce_barrier_init(&b1, "b1");
	/* Initiation shared data (allocation) */
	share_data = d2mce_malloc("share_data", sizeof(unsigned long)* limit);
	unsigned long *share_data_ptr = share_data;
	for ( i = 0; i < limit; i++ ) {
		*share_data_ptr++ = 0;
	}
	share_data_ptr = share_data;

	d2mce_barrier( &b1, NODES );
	/* Load Matrix A and B */
	d2mce_load( share_data );
	d2mce_barrier(&b1, NODES);
	time = -d2mce_time();
#endif

	for ( count = 0; count < limit; count++ ) {
		/* reset background bmp file pointer */
		n = fseek( fp_bg_in, 0, SEEK_SET );
		if ( n != 0 ) {
			perror( "fseek error" );
			exit( EXIT_FAILURE );
		}

		/* change to next file */
		sprintf( now_filename, "%d.bmp", count+1 );
		printf( "next filename = %s\n", now_filename );

		/* open now bmp file and initial the now buffer */
		fp_now_in = fopen( now_filename, "rb" );
		if ( fp_now_in == NULL ) {
			perror( "open now bmp file error" );
			exit ( EXIT_FAILURE );
		}
		/* set to the position of RGB elements */
		n = fseek( fp_now_in, 54, SEEK_SET );
		if ( n != 0 ) {
			perror( "fseek error" );
			exit( EXIT_FAILURE );
		}

		rgb_size = imagesize * 3;
		bg_rgb = (BYTE *) malloc ( sizeof(BYTE) * imagesize * 3 );
		if ( bg_rgb == NULL ) {
			perror( "memory allocate error" );
			exit ( EXIT_FAILURE );
		}
		now_rgb = (BYTE *) malloc ( sizeof(BYTE) * imagesize * 3 );
		if ( now_rgb == NULL ) {
			perror( "memory allocate error" );
			exit ( EXIT_FAILURE );
		}
		bg_rgb_ptr = bg_rgb;
		now_rgb_ptr = now_rgb;

		/* read Background's RGB elements from bmp file */
		n = 0;
		do {
			n += fread( bg_rgb, 3, rgb_size, fp_bg_in );
		} while ( n == rgb_size );

		n = 0;
		do {
			n += fread( now_rgb, 3, rgb_size, fp_now_in );
		} while ( n == rgb_size );

		switch ( task_id ) {
		case 0:
			for ( i = 0; i < width; i++ ) {
				for ( j = 0; j < height/4; j++ ) {
					/*
					 * here will transform from RGB to YUV
					 * but we just need Y value, so we can only calculate the Y value
					 */
					R = *bg_rgb_ptr++;
					G = *bg_rgb_ptr++;
					B = *bg_rgb_ptr++;
					*bg_ptr = (R*19595 + G*38469 + B*7472 ) >> 16;

					R = *now_rgb_ptr++;
					G = *now_rgb_ptr++;
					B = *now_rgb_ptr++;
					*now_ptr = ( R*19595 + G*38469 + B*7472 ) >> 16;

					temp = abs( *now_ptr - *bg_ptr );
					*result_ptr += ( temp * temp );

					bg_ptr++;
					now_ptr++;
				}
			}
			break;

		case 1:
			for ( i = 0; i < width; i++ ) {
				for ( j += height/4; j < height/4; j++ ) {
					/*
					 * here will transform from RGB to YUV
					 * but we just need Y value, so we can only calculate the Y value
					 */
					bg_rgb_ptr += ( width * height/4 );
					R = *bg_rgb_ptr++;
					G = *bg_rgb_ptr++;
					B = *bg_rgb_ptr++;
					*bg_ptr = (R*19595 + G*38469 + B*7472 ) >> 16;

					R = *now_rgb_ptr++;
					G = *now_rgb_ptr++;
					B = *now_rgb_ptr++;
					*now_ptr = ( R*19595 + G*38469 + B*7472 ) >> 16;

					temp = abs( *now_ptr - *bg_ptr );
					*result_ptr += ( temp * temp );

					bg_ptr++;
					now_ptr++;
				}
			}
			break;

		case 2:
			for ( i = 0; i < width; i++ ) {
				for ( j += height/2; j < height/4; j++ ) {
					/*
					 * here will transform from RGB to YUV
					 * but we just need Y value, so we can only calculate the Y value
					 */
					bg_rgb_ptr += ( width * height/2 );
					R = *bg_rgb_ptr++;
					G = *bg_rgb_ptr++;
					B = *bg_rgb_ptr++;
					*bg_ptr = (R*19595 + G*38469 + B*7472 ) >> 16;

					R = *now_rgb_ptr++;
					G = *now_rgb_ptr++;
					B = *now_rgb_ptr++;
					*now_ptr = ( R*19595 + G*38469 + B*7472 ) >> 16;

					temp = abs( *now_ptr - *bg_ptr );
					*result_ptr += ( temp * temp );

					bg_ptr++;
					now_ptr++;
				}
			}
			break;

		case 3:
			for ( i = 0; i < width; i++ ) {
				for ( j += height/4*3; j < height/4; j++ ) {
					/*
					 * here will transform from RGB to YUV
					 * but we just need Y value, so we can only calculate the Y value
					 */
					bg_rgb_ptr += ( width * height/4*3 );
					R = *bg_rgb_ptr++;
					G = *bg_rgb_ptr++;
					B = *bg_rgb_ptr++;
					*bg_ptr = (R*19595 + G*38469 + B*7472 ) >> 16;

					R = *now_rgb_ptr++;
					G = *now_rgb_ptr++;
					B = *now_rgb_ptr++;
					*now_ptr = ( R*19595 + G*38469 + B*7472 ) >> 16;

					temp = abs( *now_ptr - *bg_ptr );
					*result_ptr += ( temp * temp );

					bg_ptr++;
					now_ptr++;
				}
			}
		}

		d2mce_mutex_rw( &lock, 1, share_data, "rw" );
		switch ( task_id ) {
		case 0:
			*share_data_ptr = *result_ptr;
			break;

		case 1:
			*share_data_ptr = *result_ptr;
			break;

		case 2:
			*share_data_ptr = *result_ptr;
			break;

		case 3:
			*share_data_ptr = *result_ptr;
		}
		d2mce_mutex_unlock( &lock );

		if ( task_id == 0 ) {
			*share_data_ptr /= imagesize;
			*share_data_ptr = sqrt( *share_data_ptr );
			share_data_ptr++;
		}
		result_ptr++;

		/* wait for other nodes which have finished them work */
		d2mce_barrier( &b1, NODES );

		/* reset the pointer position */
		bg_ptr = bg_buffer;
		now_ptr = now_buffer;

		fclose( fp_now_in );
	}
	time += d2mce_time();

	j = 1;
	d2mce_mutex_rw( &lock, 1, share_data, "r" );
	switch ( task_id ) {
	case 0:
		share_data_ptr = share_data;
		for( i = 0; i < 100; i++ ) {
			printf( "result%d = %d\n", j++, *share_data++ );
		}
		break;

	case 1:
		share_data_ptr = share_data;
		for( i = 0; i < 100; i++ ) {
			printf( "result%d = %d\n", j++, *share_data++ );
		}
		break;

	case 2:
		share_data_ptr = share_data;
		for( i = 0; i < 100; i++ ) {
			printf( "result%d = %d\n", j++, *share_data++ );
		}
		break;

	case 3:
		share_data_ptr = share_data;
		for( i = 0; i < 100; i++ ) {
			printf( "result%d = %d\n", j++, *share_data++ );
		}
	}
	d2mce_mutex_unlock( &lock );

	/* wait for other nodes which have finished them work */
	d2mce_barrier( &b1, NODES );
	printf("time %f\n", time);
	print_overhead();
	d2mce_finalize();
}

