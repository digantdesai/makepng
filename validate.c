#include "makepng.h"

int
validate(const char *filein, const char *filepng, size_t padding) {
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
      Dprintf("**** Validating ****\n");

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
      if(!(outbuffer = malloc(total_bytes * sizeof(unsigned char))))
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


      Dprintf("**** Validation successful! ****\n");
      free(outbuffer);
      outbuffer=NULL;

      return 0;
}
