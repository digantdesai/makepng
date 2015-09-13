/*- pngtopng
 *
 * COPYRIGHT: Written by John Cunningham Bowler, 2011.
 * To the extent possible under law, the author has waived all copyright and
 * related or neighboring rights to this work.  This work is published from:
 * United States.
 *
 * Read a PNG and write it out in a fixed format, using the 'simplified API'
 * that was introduced in libpng-1.6.0.
 *
 * This sample code is just the code from the top of 'example.c' with some error
 * handling added.  See example.c for more comments.
 */
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define PNG_DEBUG 3
#include <png.h>

#if defined(PNG_SIMPLIFIED_READ_SUPPORTED) && \
    defined(PNG_SIMPLIFIED_WRITE_SUPPORTED)

#define BytesPerPixel 3 // sRGB
#define PAD 'X'

#ifdef DEBUG
#define Dprintf printf(fmt, ##__VA_ARGS__)
#else
#define Dprintf 
#endif



int total_text = 2;

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

      Dprintf("PNG signature...");
      fread(sig, 1, 8, fp);
      if (!png_check_sig(sig, 8)) {
            Dprintf("Failed\n");
            return 1;   /* bad signature */
      }
      Dprintf("valid.\n");

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


      Dprintf("My signature...");
      png_textp valid_text[total_text];

	  int text_chunks = 0;
      unsigned num = png_get_text(png_ptr, info_ptr, valid_text, &text_chunks);

      if(!num || num!=total_text){
            Dprintf("Failed\n");
            return -1;
      }
      Dprintf("valid.\n");

      size_t padding_bytes = 0;
	  for(int i=0; i<text_chunks; i++) {
      	if(!strcmp(valid_text[i]->key,"Padding")) {
      		padding_bytes = atol(valid_text[i]->text);
      	}
      }
      Dprintf("Padding: %u(bytes)\n", padding_bytes);

      png_byte color_type = png_get_color_type(png_ptr, info_ptr);
      if (color_type != PNG_COLOR_TYPE_RGB)
      	  Dprintf("ColorType is not PNG_COLOR_TYPE_RGB\n");

      png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

      int number_of_passes = png_set_interlace_handling(png_ptr);
      png_read_update_info(png_ptr, info_ptr);

      size_t total_bytes = height * width * BytesPerPixel * (bit_depth/8);
      if(total_bytes != size + padding_bytes) {
      	  Dprintf("Total payload bytes do not match. PNG: %u, Input: %u\n", total_bytes, size + padding_bytes);
          return -1;
      }
      
      // resize the input buffer and add padding bytes
      if(!(inbuffer = realloc(inbuffer,total_bytes)))
            return ENOMEM;

      memset(inbuffer+size, PAD, padding_bytes);
	
	  png_bytep * row_pointers;
	  row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	  for (int y=0; y<height; y++)
      		row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

      png_read_image(png_ptr, row_pointers);
      if(fp!=NULL)
      	  fclose(fp);

     /*
      * Compare the buffers and
	  * write to a new file (validation.data)
      */
	  if (!(fp=fopen("validation.data","w")))
            return ENOENT;

      unsigned char *outbuffer=NULL; 
      if(!(outbuffer = malloc(size * sizeof(unsigned char))))
      	  return ENOMEM;

      Dprintf("Total: %d = %d + %d\n", total_bytes, size, padding_bytes);
	
	  size_t size_check=0;

      for (int y=0; y<height; y++) {
              png_byte* row = row_pointers[y];
              for (int x=0; x<width; x++) {
                      png_byte* ptr = &(row[x*BytesPerPixel]);

					  Dprintf("%c%c%c\n", ptr[0],ptr[1],ptr[2]);
					  Dprintf("%c%c%c\n", inbuffer[width*BytesPerPixel*y + BytesPerPixel*x+ 0],
                                         inbuffer[width*BytesPerPixel*y + BytesPerPixel*x+ 1],
                                         inbuffer[width*BytesPerPixel*y + BytesPerPixel*x+ 2]);

					  if(ptr[0]!=inbuffer[width*BytesPerPixel*y + BytesPerPixel*x + 0] || 
					  		  ptr[1]!= inbuffer[width*BytesPerPixel*y + BytesPerPixel*x+ 1] ||
					  		  ptr[2]!= inbuffer[width*BytesPerPixel*y + BytesPerPixel*x + 2]) {
							Dprintf("Fatal error: Payload mismatch\n");
							return -1;
					  }

					  // Copy the PNG data into another buffer
					  outbuffer[width*BytesPerPixel*y + BytesPerPixel*x+ 0]=ptr[0];
					  outbuffer[width*BytesPerPixel*y + BytesPerPixel*x+ 1]=ptr[1];
					  outbuffer[width*BytesPerPixel*y + BytesPerPixel*x+ 2]=ptr[2];

					  
					  }
	
              }

      Dprintf("\n");
	  free(inbuffer); 
	  inbuffer=NULL;

   	  Dprintf("****Validation successful!****\n");

	  // Dont write padding bytes
	  fwrite(outbuffer, sizeof(unsigned char), size, fp);

	  if(fp!=NULL)
	  	  fclose(fp);

	  Dprintf("Validation data written to: validation.data\n");

	  for (int y=0; y<height; y++)
      		free(row_pointers[y]);
      free(row_pointers);
      row_pointers=NULL;

      return 0;
}

void
gethw(unsigned long pixels, size_t *h, size_t *w)
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
 * here,
 *         BytesPerPixel = 3 for sRGB
 *        size = given (in bytes)
 * might have to add padding, this function will return number of padding bytes
 *
 */
unsigned
make_box(size_t *height, size_t *width, size_t size) {
    if (size < BytesPerPixel){
        Dprintf("Error: %s: invalid input\n", __func__);
        exit(EINVAL);
    }
    size_t new_size = size;
    *height = 0;
    *width     = 0;

    // Make the size multiple of BytesPerPixel
    switch(new_size%BytesPerPixel) {
    case(0):
        break;
    case(2):
        new_size += 1;
        break;
    case(1):
        new_size += 2;
        break;
    }
    unsigned pixels = new_size / BytesPerPixel;
    gethw(pixels, height, width);

    /* Padding bytes =
     * Bytes added to make original size a multiple of BytesPerPixel +
     * New pixel added to make a square image
     */
    Dprintf("Original Bytes : %d\n", size);
    Dprintf("New 3x Bytes   : %d\n", new_size);
    Dprintf("Original Pixels: %d, %d(bytes)\n",pixels, pixels*BytesPerPixel);
    Dprintf("New e2 Pixels  : %d, %d(bytes)\n",(*height * *width),((*height * *width) - pixels) * BytesPerPixel);
	Dprintf("dimensions	   : %d x %d\n", *height, *width);
    return ((new_size - size) + (((*height * *width) - pixels) * BytesPerPixel));
}

int main(int argc, const char **argv)
{
   int result = 1;

   if (argc == 3)
   {
      png_image image;

      /* Only the image structure version number needs to be set. */
      memset(&image, 0, sizeof image);
      image.version = PNG_IMAGE_VERSION;

      /*
       * read the file into a buffer
       */
      FILE *fp;

      if (!(fp=fopen(argv[1],"r")))
            return ENOENT;

      fseek(fp, 0, SEEK_END);
      size_t size = ftell(fp);
      fseek(fp,0, SEEK_SET);

      // Dprintf("size: %u\n", size);

      unsigned char *inbuffer = NULL;
      if(!(inbuffer = malloc(size * sizeof(unsigned char))))
            return ENOMEM;

      // Dprintf("inbuffer: %p\n", inbuffer);

      if(fread(inbuffer, sizeof(unsigned char), size, fp)!=size)
            return EIO;
     
      for(int i=0;i<size;i++)
      	  Dprintf("%c", inbuffer[i]);
      /*
       * make a square image out of input data
       */
      size_t height, width;
      unsigned padding_bytes = make_box(&height, &width, size);

      Dprintf("Image size: %d x %d pixels(sRGB), TotalBytes= %d = Original: %d + Padding: %d.\nPadding: %3.1f%%\n", height, width, height*width*BytesPerPixel, size, padding_bytes, (1.0*size/padding_bytes)*100);

      size = size + padding_bytes;
      if(!(inbuffer = realloc(inbuffer,size * sizeof(unsigned char))))
            return ENOMEM;

      memset(inbuffer+size-padding_bytes, PAD, padding_bytes);

      if (fp != NULL)
            fclose(fp);

      png_structp png_ptr = png_create_write_struct
          (PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);

      if (!png_ptr)
            return -1;

      png_infop info_ptr = png_create_info_struct(png_ptr);

      if (!info_ptr) {
         png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
         return -1;
      }

      /* Set image attributes. */
      int depth = 8;
      int pixel_size = BytesPerPixel;

      png_set_IHDR (png_ptr,
                  info_ptr,
                  width,
                  height,
                  depth,
                  PNG_COLOR_TYPE_RGB,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);

      png_bytepp row_pointers = NULL;

      /* Set the rows */
      size_t x, y;

      row_pointers = png_malloc (png_ptr, height * sizeof (png_bytep));
      for (y = 0; y < height; ++y) {
          png_bytep row = png_malloc (png_ptr, sizeof (unsigned char) * width * pixel_size);
          row_pointers[y] = row;
          for (x = 0; x < width; ++x) {
              *row++ = inbuffer[width * BytesPerPixel * y + BytesPerPixel * x + 0];
              *row++ = inbuffer[width * BytesPerPixel * y + BytesPerPixel * x + 1];
              *row++ = inbuffer[width * BytesPerPixel * y + BytesPerPixel * x + 2];
			  Dprintf("%c%c%c", inbuffer[width * BytesPerPixel * y + BytesPerPixel * x + 0],
				  inbuffer[width * BytesPerPixel * y + BytesPerPixel * x + 1],
				  inbuffer[width * BytesPerPixel * y + BytesPerPixel * x + 2]);
          }
      }
      Dprintf("\n");

      /* text */
      int num_text = 0;
      png_text text[total_text];

      /* #0. padding*/
      text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
      text[num_text].key = "Padding";
      char pad_str[20];
      snprintf(pad_str, 20, "%d", padding_bytes);
      text[num_text].text = pad_str;
      ++num_text;

      /* #1. signature */
      text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
      text[num_text].key = "Signature";
      text[num_text].text = "Navsari";
 
      png_set_text(png_ptr, info_ptr, text, total_text);

      if (!(fp=fopen(argv[2],"w")))
            return ENOENT;

      /* Write the image data to "fp". */
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

      /* Optional Validate */
      if(!validate(argv[1], argv[2], padding_bytes));

      return result;
   }

   else
      /* Wrong number of arguments */
      fprintf(stderr, "pngtopng: usage: pngtopng input-file output-file\n");


   return result;
}
#endif /* READ && WRITE */
