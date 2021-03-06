#include "makepng.h"

/*
 * encode
 */
int
encode(char *inputfile, char *outputfile, int flagval, char* meta)
{
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

   Dprintf("Image size: %d x %d pixels(sRGB), TotalBytes= %d = Original: %d + Padding: %d.\nPadding: %3.1f%%\n", height, width, height*width*BytesPerPixel, size, padding_bytes, (1.0*padding_bytes/size)*100);

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
   png_text text[i_total];

   /* #0. padding*/
   char pad_str[20];
   snprintf(pad_str, 20, "%d", padding_bytes);
   text[i_padding].compression    = PNG_TEXT_COMPRESSION_NONE;
   text[i_padding].key            = PADDING_KEY;
   text[i_padding].text           = pad_str;
   /* #1. metadata */
   text[i_metadata].compression  = PNG_TEXT_COMPRESSION_NONE;
   text[i_metadata].key          = META_KEY;
   text[i_metadata].text         = meta? meta : "No Metadata";

   png_set_text(png_ptr, info_ptr, text, i_total);

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
   if (flagval)
       if(!validate(inputfile, outputfile, padding_bytes));
           return 1;

   return result;
}

int
decode(char *fpng, char *fout, int print_metadata)
{
    FILE *fp;

    unsigned char sig[8];
    if (!(fp=fopen(fpng,"r")))
          return ENOENT;

    unsigned char sign[8];
    fread(sign, 1, 8, fp);

    png_check_sig(sig, 8);

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

    /* text chunks and padding bytes */
    size_t padding_bytes = 0;
    char *metadata = NULL;

    int text_chunks = 0;
    png_textp valid_text;

    unsigned num = png_get_text(png_ptr, info_ptr, &valid_text, &text_chunks);

    if(!num || text_chunks != i_total)
        fprintf(stderr, "Text chunk count mismatch, old encoding format?\n");

    if(!strcmp(valid_text[i_padding].key, PADDING_KEY)) {
        padding_bytes = atol(valid_text[i_padding].text);
        Dprintf("Padding#: %d\n", padding_bytes);
    } else {
        fprintf(stderr, "Padding is missing\n");
        return -1;
    }

    if(!strcmp(valid_text[i_metadata].key, META_KEY)) {
        metadata = (char *) malloc(valid_text[i_metadata].text_length);
        if (!metadata){
            fprintf(stderr, "metadata allocation failed\n");
            return -1;
        }
        metadata = strncpy(metadata, valid_text[i_metadata].text, valid_text[i_metadata].text_length);
        Dprintf("Metadata: \"%s\"\n", metadata);
    } else {
        fprintf(stderr, "Missing Metadeta\n");
        return -1;
    }

    if(print_metadata)
        printf("%s\n", metadata);
    free(metadata);

    size_t height, width;
    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);

    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    png_read_update_info(png_ptr, info_ptr);

    size_t total_bytes = height * width * BytesPerPixel * (bit_depth/BitDepth);
    size_t size = total_bytes - padding_bytes;

    png_bytep *row_pointers;
    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);

    for (int y=0; y<height; y++)
        row_pointers[y] =
            (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

    png_read_image(png_ptr, row_pointers);

    if(fp!=NULL)
          fclose(fp);


    if (!(fp=fopen(fout,"w")))
          return ENOENT;

    unsigned char *outbuffer=NULL;
    if(!(outbuffer = malloc(total_bytes * sizeof(unsigned char))))
          return ENOMEM;

    size_t x,y,c,r;

    for (y=0; y<height; y++) {

          png_byte* row = row_pointers[y];

          c = width * BytesPerPixel * y;

          for (x=0; x<width; x++) {

                r = BytesPerPixel * x;

                png_byte* ptr = &(row[x*BytesPerPixel]);

                outbuffer[c + r + 0] = ptr[0];
                outbuffer[c + r + 1] = ptr[1];
                outbuffer[c + r + 2] = ptr[2];
          }
    }

    fwrite(outbuffer, sizeof(unsigned char), size, fp);

    Dprintf("Extracting data...Done.\n");
    if(fp!=NULL)
          fclose(fp);

    for (int y=0; y<height; y++)
            free(row_pointers[y]);

    free(row_pointers);
    row_pointers=NULL;

    free(outbuffer);
    outbuffer = NULL;

    return 0;
}
