#include "makepng.h"

int
decode(char *fpng, char *fout) {
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
    int text_chunks = 0;
    png_textp valid_text[TotalTextChunks];
    unsigned num = png_get_text(png_ptr, info_ptr, valid_text, &text_chunks);

	size_t padding_bytes = 0;
    for(int i=0; i<text_chunks; i++) {
        if(!strcmp(valid_text[i]->key,"Padding")) {
            padding_bytes = atol(valid_text[i]->text);
            break;
        }
    }

    size_t height, width;
    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);


    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    png_read_update_info(png_ptr, info_ptr);

    size_t total_bytes = height * width * BytesPerPixel * (bit_depth/BitDepth);
    size_t size = total_bytes - padding_bytes;

    size_t x,y,c,r;
	png_bytep *row_pointers;
    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);

    for (int y=0; y<height; y++)
            row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

    png_read_image(png_ptr, row_pointers);
    if(fp!=NULL)
          fclose(fp);


    if (!(fp=fopen(fout,"w")))
          return ENOENT;

    unsigned char *outbuffer=NULL;
    if(!(outbuffer = malloc(size * sizeof(unsigned char))))
          return ENOMEM;


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
