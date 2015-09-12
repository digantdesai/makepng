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
void
gethw(unsigned long pixels, unsigned *h, unsigned *w)
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
        printf("Error: %s: invalid input\n", __func__);
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
      
      // printf("size: %u\n", size);

      unsigned char *inbuffer = NULL;
      if(!(inbuffer = malloc(size * sizeof(unsigned char))))
            return ENOMEM;

      // printf("inbuffer: %p\n", inbuffer);
     
      unsigned n = size/sizeof(size_t);
      if(!(fread(inbuffer, size, n, fp))==n)
            return EIO;

      /*
       * make a square image out of input data
       */
      size_t height, width;
      unsigned padding_bytes = make_box(&height, &width, size);

      size = size + padding_bytes;
      if(!(inbuffer = realloc(inbuffer,size * sizeof(unsigned char))))
            return ENOMEM;

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
       
      row_pointers = png_malloc (png_ptr, height * sizeof (png_bytep));
      for (y = 0; y < height; ++y) {
          png_bytep row = 
              png_malloc (png_ptr, sizeof (uint8_t) * width * pixel_size);
          row_pointers[y] = row;
          for (x = 0; x < width; ++x) {
              pixel_t * pixel = inbuffer + width *y + x;
              *row++ = pixel;
              *row++ = pixel+1;
              *row++ = pixel+2;
          }
      }
    

      printf("success\n");

      if (png_image_begin_read_from_file(&image, argv[1]))
      {
         png_bytep buffer;

         /* Change this to try different formats!  If you set a colormap format
          * then you must also supply a colormap below.
          */
         image.format = PNG_FORMAT_RGBA;

         buffer = malloc(PNG_IMAGE_SIZE(image));

         if (buffer != NULL)
         {
            if (png_image_finish_read(&image, NULL/*background*/, buffer,
               0/*row_stride*/, NULL/*colormap for PNG_FORMAT_FLAG_COLORMAP */))
            {
               if (png_image_write_to_file(&image, argv[2],
                  0/*convert_to_8bit*/, buffer, 0/*row_stride*/,
                  NULL/*colormap*/))
                  result = 0;

               else
                  fprintf(stderr, "pngtopng: write %s: %s\n", argv[2],
                      image.message);

               free(buffer);
            }

            else
            {
               fprintf(stderr, "pngtopng: read %s: %s\n", argv[1],
                   image.message);

               /* This is the only place where a 'free' is required; libpng does
                * the cleanup on error and success, but in this case we couldn't
                * complete the read because of running out of memory.
                */
               png_image_free(&image);
            }
         }

         else
            fprintf(stderr, "pngtopng: out of memory: %lu bytes\n",
               (unsigned long)PNG_IMAGE_SIZE(image));
      }

      else
         /* Failed to read the first argument: */
         fprintf(stderr, "pngtopng: %s: %s\n", argv[1], image.message);

       free(inbuffer);
       inbuffer = NULL;
   }

   else
      /* Wrong number of arguments */
      fprintf(stderr, "pngtopng: usage: pngtopng input-file output-file\n");


   return result;
}
#endif /* READ && WRITE */
