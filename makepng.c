#include "makepng.h"

/*
 * Increase the dimension to make a closest larger or equal
 * perfect square number.
 * This could add a lot of padding but fix it in later version.
 */
void
make_squared(unsigned long pixels, size_t *h, size_t *w) {
    unsigned long isqrt = (int)(sqrt(pixels));
    if(isqrt*isqrt < pixels)
        isqrt += 1;
    *h = *w = isqrt;
    return;
}

/*
 * size = rows * columns
 *      = (BytesPerPixel * height) * width
 * might have to add padding, this function will return number of padding bytes
 * and set height and width assuming pixed format sRGB.
 *
 */
unsigned
make_box(size_t *height, size_t *width, size_t size) {
    /*
     * Cant work with <1 pixel, something is wrong
     */
    if (size < BytesPerPixel){
        Dprintf("Error: %s: invalid input\n", __func__);
        exit(EINVAL);
    }

    *height = *width  = 0;

    /*
     * Make image size multiple of BytesPerPixel
     */
    size_t new_size = size + (BytesPerPixel - (size%BytesPerPixel));
    unsigned pixels = new_size / BytesPerPixel;

    make_squared(pixels, height, width);

    /*
     * Padding bytes = Bytes added to make original size a multiple of
     * BytesPerPixel + New pixel added to make a square image
     */
    Dprintf("Original Bytes : %d\n", size);
    Dprintf("New 3x Bytes   : %d\n", new_size);
    Dprintf("Original Pixels: %d, %d(bytes)\n",pixels, pixels*BytesPerPixel);
    Dprintf("New e2 Pixels  : %d, %d(bytes)\n",(*height * *width),((*height * *width) - pixels) * BytesPerPixel);
    Dprintf("Dimensions     : %d x %d\n", *height, *width);
    return ((new_size - size) + (((*height * *width) - pixels) * BytesPerPixel));
}

void
print_help() {
	printf("Usage: makepng -mio[vdx]\n"
"cmdline args\n"
"   	-m mode (string : encode, decode)\n"
"   	-i input file (string)\n"
"   	-o output file (string)\n"
"   	-v validation (flag)\n"
"   	-d debug (flag)\n"
"   	-x deep debug (flag)\n");
	return;
}


int
main(int argc, char *argv[]) {
   opterr = 0;
   int _x_ = 0;
   char *mode = NULL;
   char *inputfile = NULL;
   char *outputfile = NULL;
   int flagValidation = 0;
   int flagDebug = 0;
   int flagDeepDebug = 0;
   while ((_x_ = getopt (argc, argv, "m:i:o:vdxh")) != -1)
   switch (_x_)
   {
   case 'm':
     mode = optarg;
     break;
   case 'i':
     inputfile = optarg;
     break;
   case 'o':
     outputfile = optarg;
     break;
   case 'v':
	 flagValidation = 1;
	 break;
   case 'd':
	 flagDebug = 1;
	 break;
   case 'x':
	 flagDeepDebug = 1;
	 break;
   case 'h':
   	 print_help();
   	 return 1;
   	 break;
   case '?':
     if (optopt == 'm' || optopt == 'i' || optopt == 'o') {
       printf("Option -%c requires an argument.\n", optopt);
     } else {
       printf("Unknown option `-%c'.\n", optopt);
     }
   	 print_help();
     return 1;
   default:
     abort ();
   }

   if ( mode == NULL || inputfile == NULL || outputfile == NULL) {
       printf("Invalid arguments\n");
   	   exit(EXIT_FAILURE);
   	}

   Dprintf("Mode (-m)       = %s\n"
   		  "Inputfile (-i)  = %s\n"
   		  "Outputfile (-o) = %s\n"
   		  "Validation (-v) = %s\n"
   		  "Debug (-d)      = %s\n"
   		  "Deep Debug (-x) = %s\n",
   		   mode,
   		   inputfile,
   		   outputfile,
   		   flagValidation?"On":"Off",
   		   flagDebug?"On":"Off",
   		   flagDeepDebug?"On":"Off");

   int result = 0;

   /*
    * read the file into a buffer
    */
   FILE *fp;

   if (!(fp=fopen(inputfile,"r")))
         return ENOENT;

   fseek(fp, 0, SEEK_END);
   size_t size = ftell(fp);
   fseek(fp,0, SEEK_SET);

   Dprintf("Input size: %u(bytes)\n", size);

   unsigned char *inbuffer = NULL;
   if(!(inbuffer = malloc(size * sizeof(unsigned char))))
         return ENOMEM;

   if(fread(inbuffer, sizeof(unsigned char), size, fp)!=size)
         return EIO;

   for(int i=0;i<size;i++)
         DDprintf("%c", inbuffer[i]);

   /*
    * make a square image out of input data
    */
   size_t height, width;
   unsigned padding_bytes = make_box(&height, &width, size);

   Dprintf("Image size: %d x %d pixels(sRGB), TotalBytes= %d = Original: %d + Padding: %d.\nPadding: %3.1f%%\n", height, width, height*width*BytesPerPixel, size, padding_bytes, (1.0*size/padding_bytes)*100);

   /*
    * Add Padding bytes
    */
   size_t new_size = size + padding_bytes;
   if(!(inbuffer = realloc(inbuffer,new_size *  sizeof (unsigned char))))
         return ENOMEM;

   /*
    * Reset Padding bytes
    */
   memset(inbuffer+size, PAD, padding_bytes);

   if (fp != NULL)
         fclose(fp);
   /* Done with input file */

   /*
    * Prepare to write input data to a PNG file
    */
   png_structp png_ptr = png_create_write_struct
       (PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);
   if (!png_ptr)
         return -1;

   png_infop info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr) {
      png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
      return -1;
   }

   /*
    * Set image attributes.
    */
   png_set_IHDR (png_ptr, info_ptr,
               width, height, BitDepth,
               PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

   png_bytepp row_pointers = NULL;

   /*
    * Set the rows
    */
   size_t x, y, c, r;

   row_pointers = png_malloc (png_ptr, height * sizeof (png_bytep));

   for (y = 0; y < height; ++y) {

       png_bytep row = png_malloc (png_ptr, sizeof (unsigned char) * width * BytesPerPixel);

       row_pointers[y] = row;

       c = width * BytesPerPixel * y;

       for (x = 0; x < width; ++x) {

       	  r = BytesPerPixel * x;

           *row++ = inbuffer[c + r + 0];
           *row++ = inbuffer[c + r + 1];
           *row++ = inbuffer[c + r + 2];

           DDprintf("%c%c%c",
           	  inbuffer[c + r + 0],
               inbuffer[c + r + 1],
               inbuffer[c + r + 2]);
       }
   }
   DDprintf("\n");

   /*
    * Text chunks
    */
   int num_text = 0;
   png_text text[TotalTextChunks];

   /* #1. padding*/
   text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
   text[num_text].key = "Padding";
   char pad_str[20];
   snprintf(pad_str, 20, "%d", padding_bytes);
   text[num_text].text = pad_str;
   ++num_text;

   /* #2. signature */
   text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
   text[num_text].key = "Signature";
   text[num_text].text = "Navsari";

   /* #3. original filename
    * 	TODO we can add later
    */

   png_set_text(png_ptr, info_ptr, text, TotalTextChunks);

   if (!(fp=fopen(outputfile,"w")))
         return ENOENT;

   /*
    * Write the image data to "fp".
    */
   png_init_io (png_ptr, fp);
   png_set_rows (png_ptr, info_ptr, row_pointers);
   png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
   png_write_end(png_ptr, NULL);

   Dprintf("Generating PNG...Done.\n");

   for (y = 0; y < height; y++) {
       png_free (png_ptr, row_pointers[y]);
   }
   png_free (png_ptr, row_pointers);

   fclose (fp);
   free(inbuffer);
   inbuffer = NULL;

   /*
    * Optional Validation
    */
   if (flagValidation)
   	if(!validate(inputfile, outputfile, padding_bytes));
   		return 1;

   return result;
}
