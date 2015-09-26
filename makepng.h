/*
 * makepng
 * Copyright digantdesai@gmail.com
 */
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <png.h>

#define PNG_DEBUG 3

/*
 * File data will be stored as sRGB = RGB888
 * format. In png terms, colortype=2, bitdepth=8
 */
#define BytesPerPixel 3
#define BitDepth 8

/*
 * Padding byte, this hard-coded value is not used
 * anywhere as a sentinal.
 */
#define PAD 'X'

/*
 * Debug printf
 */
// #define DEBUG
// #define DDEBUG

/* Normal */
#ifdef DEBUG
#define Dprintf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define Dprintf
#endif

/* Heavy */
#ifdef DDEBUG
#define DDprintf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DDprintf
#endif

/*
 * Number of text chunks
 *     1. Number of padding bytes
 *     2. fix debug and stderr, return and exit codes
 *     3. Signature (optional)
 */
#define TotalTextChunks 2


#define ValidatedDataFile "validated.data"

/* APIs */
/* 
 * encode
 *
 */
int encode(char *fileInput, char *filepng, int validate);

/*
 * decode
 *
 */
int decode(char *filepng, char *fileOutput);

/*
 * validate
 *  - this is an optional routine, mainly used for
 *  debugging, and has a lot of code from decode.
 *  TODO: Overload decode to do this and delete this routine.
 *
 *     - Extract the bytes out of the PNG file and validate it byte by byte aginst
 *     the original fle.
 *     - this routine will write raw bytes in 'validation.data' file. This could be
 *     used to compare agaist the original input file.
 *
 *    Args:
 *     @filein: Original input file name
 *     @filepng: Original outout file name, assumed a valid PNG file
 *     @padding: Padding bytes to compare aginst
 *     return value: 0 - validated successfully, else validation failed.
 */

int validate(const char *filein, const char *filepng, size_t padding);
