#include "makepng.h"

/*
 * validate
 *  - this is an optional routine, mainly used for
 *  debugging,
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
int
validate(const char *filein, const char *filepng, size_t padding)
{
     /*
      * Read original file (bytes)
      */
      FILE *fp;

      if (!(fp=fopen(filein,"r")))
            return ENOENT;

      fseek(fp, 0, SEEK_END);
      size_t size = ftell(fp);
      fseek(fp,0, SEEK_SET);

      unsigned char *inbuffer = NULL;
      if(!(inbuffer = malloc(size * sizeof(unsigned char))))
            return ENOMEM;

      if(fread(inbuffer, sizeof(unsigned char), size, fp)!=size)
            return EIO;
      fclose(fp);

     /*
      * Read PNG file we just generated
      *     extract bytes
      *     remove padding bytes
      */
      Dprintf("****Validating****\n");

      unsigned char sig[8];
      if (!(fp=fopen(filepng,"r")))
            return ENOENT;

      fread(sig, 1, 8, fp);
      if (!png_check_sig(sig, 8)) {
      		Dprintf("PNG signature...");
            Dprintf("validation failed\n");
            return 1;   /* bad signature */
      }

      png_structp png_ptr = png_create_read_struct
            (PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);
      if (!png_ptr)
            return -1;

      png_infop info_ptr = png_create_info_struct(png_ptr);
      if (!info_ptr) {
            png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
            return -1;
      }

      png_init_io(png_ptr, fp);
      png_set_sig_bytes(png_ptr, 8);
      png_read_info(png_ptr, info_ptr);

      size_t height, width;
      width = png_get_image_width(png_ptr, info_ptr);
      height = png_get_image_height(png_ptr, info_ptr);
      Dprintf("Image dimentions: %d x %d\n", height, width);


	  /* text chunks and padding bytes */
      int text_chunks = 0;
      png_textp valid_text[TotalTextChunks];
      unsigned num = png_get_text(png_ptr, info_ptr, valid_text, &text_chunks);

      if(!num || text_chunks!=TotalTextChunks){
      	    Dprintf("Text chunk count does not match expect count..");
            Dprintf("validation failed\n");
            return -1;
      }

      size_t padding_bytes = 0;
      for(int i=0; i<text_chunks; i++) {
          if(!strcmp(valid_text[i]->key,"Padding")) {
              padding_bytes = atol(valid_text[i]->text);
              break;
          }
      }
      if(padding_bytes!= padding) {
      	  Dprintf("Padding byte count does not match expect count..");
          Dprintf("validation failed\n");
      	  return -1;

      }

      Dprintf("Padding: %u(bytes)\n", padding_bytes);

      png_byte color_type = png_get_color_type(png_ptr, info_ptr);
      if (color_type != PNG_COLOR_TYPE_RGB) {
            Dprintf("ColorType is not PNG_COLOR_TYPE_RGB..");
            Dprintf("validation failed\n");
      }

      png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

      int number_of_passes = png_set_interlace_handling(png_ptr);
      png_read_update_info(png_ptr, info_ptr);

      size_t total_bytes = height * width * BytesPerPixel * (bit_depth/BitDepth);
      if(total_bytes != size + padding_bytes) {
            Dprintf("Total payload bytes do not match. PNG: %u, Input: %u\n", total_bytes, size + padding_bytes);
          return -1;
      }

      Dprintf("Total: %d = %d + %d\n", total_bytes, size, padding_bytes);

      /* 
       * resize the input buffer and add padding bytes
       * so we can count padding bytes as well
       */
      if(!(inbuffer = realloc(inbuffer,total_bytes)))
            return ENOMEM;

      memset(inbuffer+size, PAD, padding_bytes);

      png_bytep *row_pointers;
      row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);

      for (int y=0; y<height; y++)
              row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

      png_read_image(png_ptr, row_pointers);
      if(fp!=NULL)
            fclose(fp);

     /*
      * Compare the buffers and write to a new file
      */
      if (!(fp=fopen(ValidatedDataFile,"w")))
            return ENOENT;

      unsigned char *outbuffer=NULL;
      if(!(outbuffer = malloc(size * sizeof(unsigned char))))
            return ENOMEM;

      DDprintf("PNG\tINPUT\n");

      size_t x,y,c,r;

      for (y=0; y<height; y++) {

              png_byte* row = row_pointers[y];

              c = width * BytesPerPixel * y;

              for (x=0; x<width; x++) {

              	  	  r = BytesPerPixel * x;

                      png_byte* ptr = &(row[x*BytesPerPixel]);

                      DDprintf("%c%c%c\t", ptr[0],
                      		               ptr[1],
                      		               ptr[2]);
                      DDprintf("%c%c%c\n", inbuffer[c + r + 0],
                                           inbuffer[c + r + 1],
                                           inbuffer[c + r + 2]);

                      if(ptr[0] != inbuffer[c + r + 0] ||
                         ptr[1] != inbuffer[c + r + 1] ||
                         ptr[2] != inbuffer[c + r + 2]) {
                            Dprintf("Payload mismatch..");
            				Dprintf("validation failed\n");
                            return -1;
                      }

                      /*
                       * Copy the PNG data into another buffer
                       * which we will later copy to a file.
                       */
                      outbuffer[c + r + 0] = ptr[0];
                      outbuffer[c + r + 1] = ptr[1];
                      outbuffer[c + r + 2] = ptr[2];
                }
      }
      DDprintf("\n");

      free(inbuffer);
      inbuffer=NULL;

      /*
       * Write to a file
       * Dont write padding bytes
       */
      fwrite(outbuffer, sizeof(unsigned char), size, fp);

      if(fp!=NULL)
            fclose(fp);

      Dprintf("Validation data written to: %s\n", ValidatedDataFile);

      for (int y=0; y<height; y++)
              free(row_pointers[y]);

      free(row_pointers);
      row_pointers=NULL;

      Dprintf("Validation successful!\n");

      return 0;
}

/*
 * Increase the dimension to make a closest larger or equal
 * perfect square number.
 * This could add a lot of padding but fix it in later version.
 */
void
make_squared(unsigned long pixels, size_t *h, size_t *w)
{
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

int
main(int argc, const char **argv)
{
   int result = 0;

   /*
    * Arg handling
    */

   if (argc == 3)
   {
      /*
       * read the file into a buffer
       */
      FILE *fp;

      if (!(fp=fopen(argv[1],"r")))
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

      if (!(fp=fopen(argv[2],"w")))
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
      if(!validate(argv[1], argv[2], padding_bytes));

      return result;
   }

   else
      /* Wrong number of arguments */
      fprintf(stderr, "pngtopng: usage: pngtopng input-file output-file\n");


   return result;
}
