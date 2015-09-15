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
 *     3. Signature (optional)
 */
#define TotalTextChunks 2


#define ValidatedDataFile "validated.data"

/*
 * cmdline options
 */
