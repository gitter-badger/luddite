#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <png.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "image.h"

#define PNG_HEADER_SIZE (8)

void errorMsg( const char *msg, ...);

// This is kind of a mess because it was converted from earlier 
// C code that was really a mess.
FpImage::FpImage()
{
    w = 0;
    h = 0;
    buf = NULL;    
}

FpImage::FpImage( int width, int height, unsigned long color )
{
    w = width;
    h = height;    
    buf = (unsigned char *)malloc( w*h*4 );
    clear( color );    
}

FpImage::FpImage( const char *filename )
{
    _loadPng( filename );
}

FpImage::~FpImage()
{
    free(buf);
}


FpImage::FpImage( const FpImage &other )
{
    buf = (unsigned char *)malloc(other.w*other.h*3 );
    w = other.w;
    h = other.h;    
    paste( other, 0, 0 );
}

FpImage &FpImage::operator=( const FpImage &other ) 
{
    buf = (unsigned char *)malloc(other.w*other.h*4 );
    w = other.w;
    h = other.h;    
    paste( other, 0, 0 );
    
    return *this;    
}


void FpImage::clear( unsigned long color )
{
    unsigned char a,r,g,b;    
    unsigned char *p = buf;
    
    a = (unsigned char)((color >> 24) & 0xff);
    r = (unsigned char)((color >> 16) & 0xff);
    g = (unsigned char)((color >> 8) & 0xff);
    b = (unsigned char)(color & 0xff);
    
    for (int i=0; i < w*h; i++) {
		*(p++) = r;
		*(p++) = g;
		*(p++) = b;
        *(p++) = a;
    } 
}

void FpImage::setPixel( int x, int y, unsigned long color )
{
    unsigned char *p = buf + (x + (y* w))*4;
    
    
    *(p+0) = (unsigned char)((color >> 16) & 0xff);
    *(p+1) = (unsigned char)((color >> 8) & 0xff);
    *(p+2) = (unsigned char)((color >> 0) & 0xff);
    *(p+3) = (unsigned char)((color >> 24) & 0xff);
    
}


// swizzle color channel from BGR to RGB or back
void FpImage::swapBR()
{
    unsigned char r,g,b,a;    
    unsigned char *p = buf;       
    
    for (int i=0; i < w*h; i++) {

		b = *(p+0);
		g = *(p+1);
		r = *(p+2);
        a = *(p+3);
        
		*(p++) = r;
		*(p++) = g;
		*(p++) = b;
        *(p++) = a;

    }
}

void FpImage::paste( const FpImage &other, int pos_x, int pos_y )
{
    unsigned char *dest_p, *src_p;
    int x, y;
    
    for (int i =0; i < other.w; i++) {
        for (int j=0; j < other.h; j++) {
            x = pos_x + i;
            y = pos_y + j;
            if ((x >= 0) && (x <= w) &&
                (y >= 0) && (y <= h) ) {
                dest_p = buf + (x + (y*w))*4;
                src_p  = other.buf + (i + (j*other.w))*4;
                
                *(dest_p++) = *(src_p++);
                *(dest_p++) = *(src_p++);
                *(dest_p++) = *(src_p++);
                *(dest_p++) = *(src_p++);
            }
        }	
    }
}

void FpImage::pasteFTBitmap( FT_Bitmap *glyph_bmp, int x, int y,
                            unsigned long color, int mono )
{
    unsigned char *dest_p, *src_p;
    int ii, jj;
    float t;
    
	
	//    printf("x %d y %d rows %d cols %d\n",
	//	   x, y, glyph_bmp->rows, glyph_bmp->width );
    
	
    for (int j = 0; j < glyph_bmp->rows; j++) {
		for (int i=0; i < glyph_bmp->width; i++) {
			
			ii = x + i;
			jj = y + j;
			
			if ((ii>=0) && (ii<w)&&(jj>=0)&&(jj<h)) {
				dest_p = buf + (ii + (jj * w))*4;
				
				if (mono) {
					src_p = glyph_bmp->buffer +  (i/8) + (j*glyph_bmp->pitch);
					if (*src_p & (0x1 << (7-(i%8)))) {
						t = 1.0;			
					} else {
						t = 0.0;			
					}		    
				}
				else {
					src_p = glyph_bmp->buffer + i + (j*glyph_bmp->pitch);
					t = (float)(*src_p) / 255.0f;		
				}

                
				dest_p[0] = (char)( (t * ((color >> 16) & 0xff)) +  
                                   ((1.0-t) * dest_p[0]) );
				dest_p[1] = (char)( (t * ((color >> 8 ) & 0xff)) +  
                                   ((1.0-t) * dest_p[1]) );
				dest_p[2] = (char)( (t * (color & 0xff)) +  
                                   ((1.0-t) * dest_p[2]) );

                dest_p[3] = ( (t*0xff) + ((1.0-t) * dest_p[3]) );

			}
			
		}	
    }
}


void FpImage::thicken( unsigned long bgcolor ) 
{
    unsigned char *img2 = (unsigned char *)malloc( w*h*4 ),
	*img_p, *img2_p;
    int x, y;    
    unsigned char r, g, b, a;
    
    // FIXME: this could be simpler now that there's an
    // alpha channel
    unsigned char bg_r,bg_g,bg_b;    
    bg_r = (unsigned char)((bgcolor >> 16) & 0xff);
    bg_g = (unsigned char)((bgcolor >> 8) & 0xff);
    bg_b = (unsigned char)(bgcolor & 0xff);
    
    
    for (int j=0; j < h; j++) {
        for (int i=0; i < w; i++) {
            
            r = bg_r; g = bg_g; b = bg_b;	    
            for (int jj = -1; jj <= 1; jj++) {
                for (int ii=-1; ii <=1; ii++ ) {
                    if ((ii==0)||(jj==0)) {
                        
                        x = i + ii;
                        y = j + jj;
                        if (( x>=0) && (x<w)&&(y>=0)&&(y<h)) {
                            img_p = buf + ((y * w) + x)*4;

                            r = img_p[0];
                            g = img_p[1];
                            b = img_p[2];
                            a = img_p[3];

                            if ((r!=bg_r)||(g!=bg_g)||(b!=bg_b)) {
                                break;
                            }
                        }
                        
                    }
                }
                
                if ((r!=bg_r)||(g!=bg_g)||(b!=bg_b))break;
            }
            
            img2_p = img2 + ((j*w) + i)*4;

            img2_p[0] = r;
            img2_p[1] = g;
            img2_p[2] = b;
            img2_p[3] = a;
        }	
    }
    
    memcpy( buf, img2, w*h*4 );
    free(img2);
}


                         
void FpImage::autoCrop( unsigned long bgcolor )
{
    unsigned char *shrunk_img, *img_p, 
	*img = buf;
    int x1, y1, x2, y2; 
    
    assert( 0 ); // FIXME: update for RGBA .. but I don't think this
                 // is used anywhere.
    
    unsigned char bg_r,bg_g,bg_b;    
    bg_r = (unsigned char)((bgcolor >> 16) & 0xff);
    bg_g = (unsigned char)((bgcolor >> 8) & 0xff);
    bg_b = (unsigned char)(bgcolor & 0xff);
    
    // find xmin
    int bail=0;    
    x1 = 0;
    while ((!bail)&&(x1 < w)) {
        for (int j=0; j < h; j++) {
            img_p = img + (x1 + (j*w))*3;
            if ( (*img_p != bg_r) ||
                (*(img_p+1) != bg_g ) ||
                (*(img_p+2) != bg_b )) {
                bail = 1;		
                break;		
            }
            
        }	
        if (!bail) x1++;
    }
    if (!bail) return;
    
    // find xmax
    bail=0;    
    x2 = w-1;
    while ((!bail)&&(x2>=x1)) {
        for (int j=0; j < h; j++) {
            img_p = img + (x2 + (j*w))*3;
            if ( (*img_p != bg_r) ||
                (*(img_p+1) != bg_g ) ||
                (*(img_p+2) != bg_b )) {
                bail = 1;		
                break;		
            }
        }	
        if (!bail) x2--;
    }
    if (!bail) return;
    
    
    // find ymin
    bail=0;    
    y1 = 0;
    while ((!bail)&&(y1 < h)) {
        for (int j=0; j < w; j++) {
            img_p = img + (j + (y1*w))*3;
            if ( (*img_p != bg_r) ||
                (*(img_p+1) != bg_g ) ||
                (*(img_p+2) != bg_b )) {
                bail = 1;		
                break;		
            }
            
        }	
        if (!bail) y1++;
    }
    if (!bail) return;
    
    // find ymax
    bail=0;    
    y2 = w-1;
    while ((!bail)&&(y2>=y1)) {
        
        for (int j=0; j < w; j++) {
            img_p = img + (j + (y2*w))*3;
            if ( (*img_p != bg_r) ||
                (*(img_p+1) != bg_g ) ||
                (*(img_p+2) != bg_b )) {
                bail = 1;		
                break;		
            }
        }	
        if (!bail) y2--;
    }
    if (!bail) return;
    
    int new_w, new_h, x, y;
    unsigned char *dest_p, *src_p;    
    new_w = (x2-x1)+1;
    new_h = (y2-y1)+1;
    
    shrunk_img = (unsigned char *)malloc( new_w * new_h * 3 );
    for (int i =0; i < new_w; i++) {
        for (int j=0; j < new_h; j++) {
            x = x1 + i;
            y = y1 + j;
            if ((x >= 0) && (x <= new_w) &&
                (y >= 0) && (y <= new_h) ) {
                dest_p = shrunk_img + (i + (j*new_w))*3;
                src_p  = img + (x + (y*w))*3;
                
                *(dest_p++) = *(src_p++);
                *(dest_p++) = *(src_p++);
                *(dest_p++) = *(src_p++);
            }
        }	
    }
    // done
    free (buf);
    buf = shrunk_img;
    w = new_w;
    h = new_h;    
}


void FpImage::writePng( const char *filename )
{
    FILE *fp = fopen( filename, "wb" );
    if (!fp)
    {
        printf( "Error writing cache file %s\n", filename );
        return;
    }
    
    png_structp png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING,
                                                  NULL, NULL, NULL );
    
    if (!png_ptr)
    {
        errorMsg( "error creating png write struct\n" );
    }   
    
    png_infop info_ptr = png_create_info_struct( png_ptr );
    if (!info_ptr)
    {
        png_destroy_write_struct( &png_ptr, (png_infopp)NULL );
        errorMsg( "error creating png info struct\n" );
    }
    
    // init IO and compression
    png_init_io( png_ptr, fp );
    png_set_compression_level( png_ptr, 6 /* Compression level, 0..9 */ );	
    
    // set content header
    png_set_IHDR( png_ptr, info_ptr, w, h,
                 8, PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT );
    
    // write header info
    png_write_info(png_ptr, info_ptr);	
    
    // write the thing
    png_byte *row_pointers[h];
    for (int i=0; i < h; i++)
    {
        // flip 
        //row_pointers[i] = &(data[(1023-i) * sz * 3]);
        
        row_pointers[i] = &(buf[i * w * 4]);
    }
    png_write_image( png_ptr, row_pointers );
    
    png_write_end( png_ptr, info_ptr );
    png_destroy_write_struct( &png_ptr, &info_ptr );
    
    fclose( fp );
    
    printf( "Wrote image %s\n", filename );
}

void FpImage::_errorImage()
{
    w = 1;
    h = 1;    
    buf = (unsigned char *)malloc( w*h*4 );
    clear( 0xFF0000 );    
   
}

void FpImage::_loadPng( const char *filename )
{
    // right now just load PNG
    FILE *fpPng = fopen( filename, "rb");
    
    if (!fpPng) 
    {
        printf("load_png: file not found %s\n", filename );
        _errorImage();
        return;
    }
    
    unsigned char PNG_header[ PNG_HEADER_SIZE ];
    fread (PNG_header, 1, PNG_HEADER_SIZE, fpPng );
    if (png_sig_cmp( PNG_header, 0, PNG_HEADER_SIZE ) != 0)
    {
        printf( "%s is not a PNG file\n", filename );
        _errorImage();
        return;
    }
    
    png_structp PNG_reader = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (PNG_reader == NULL)
    {
        printf("Can't start reading PNG file %s\n", filename );
        fclose(fpPng);
        _errorImage();
        return;
    
    }
    
    png_infop PNG_info = png_create_info_struct(PNG_reader);
    if (PNG_info == NULL)
    {
        printf("Can't get info for PNG file %s\n", filename);
        
        png_destroy_read_struct(&PNG_reader, NULL, NULL);
        fclose(fpPng);
        _errorImage();
        return;    
    }
    
    png_infop PNG_end_info = png_create_info_struct(PNG_reader);
    if (PNG_end_info == NULL)
    {
        printf("Can't get end info for PNG file %s\n", filename);
        
        png_destroy_read_struct(&PNG_reader, &PNG_info, NULL);
        fclose(fpPng);
        _errorImage();
        return;  
    }
    
    if (setjmp(png_jmpbuf(PNG_reader)))
    {
        printf("Can't load PNG file %s\n", filename);
        
        png_destroy_read_struct(&PNG_reader, &PNG_info, &PNG_end_info);
        fclose(fpPng);
        _errorImage();
        return;     
    }
    
    png_init_io(PNG_reader, fpPng);
    png_set_sig_bytes(PNG_reader, PNG_HEADER_SIZE);
    
    png_read_info(PNG_reader, PNG_info);
        
    png_uint_32 width, height;
    int bit_depth, color_type;
    
    png_get_IHDR(PNG_reader, PNG_info, &width, &height, &bit_depth, &color_type,NULL, NULL, NULL);
    
    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_palette_to_rgb(PNG_reader);
    }
    
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) 
    {
        png_set_expand_gray_1_2_4_to_8(PNG_reader);
    }
    
    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        png_set_gray_to_rgb(PNG_reader);
    }
    
    if (png_get_valid(PNG_reader, PNG_info, PNG_INFO_tRNS))
    {
        png_set_tRNS_to_alpha(PNG_reader);
    }
    else
    {
        png_set_filler(PNG_reader, 0xff, PNG_FILLER_AFTER);
    }
    
    
    if (bit_depth == 16)
    {
        png_set_strip_16(PNG_reader);
    }
    
    png_read_update_info(PNG_reader, PNG_info);
    
    png_byte* PNG_image_buffer = (png_byte*)malloc(4 * width * height);
    memset(PNG_image_buffer,0,4*width*height); // clear image buffer
    png_byte** PNG_rows = (png_byte**)malloc(height * sizeof(png_byte*));
    
    
    png_uint_32 rowBytes = width*4;
    
    // load the image
    unsigned int row;
    for (row = 0; row < height; ++row)
    {
        PNG_rows[row] = PNG_image_buffer + (row * rowBytes);
    }
    png_read_image(PNG_reader, PNG_rows);
    
    free(PNG_rows);
    
    png_destroy_read_struct(&PNG_reader, &PNG_info, &PNG_end_info);
    fclose(fpPng);
    
    // fill in the texture info
    w = width;
    h = height;
        
    // return the texture data
    buf = PNG_image_buffer;    
}


#if 0
// unit test for FpImage
bool FpImage::selfTest()
{
    int i,j;
    unsigned int k;
    
    // create and clear an image
    printf("FpImage::selfTest().....Create and clear an image\n");    
    unsigned char *p;    
    FpImage img( 100, 100, 0xa0a0a0 );
    p = img.buf;
    
    for (j=0; j < img.h; j++) {
        for (i=0; i < img.w; i++) {
            assert(0xa0==(*p)); p++;
            assert(0xa0==(*p)); p++;
            assert(0xa0==(*p)); p++;
        }	
    }
    
    // copy an image
    printf("FpImage::selfTest().....Copy constructor\n");
    FpImage img2 = FpImage( img );    
    p = img2.buf;
    
    for ( j=0; j < img2.h; j++) {
        for (int i=0; i < img2.w; i++) {
            assert(0xa0==(*p)); p++;
            assert(0xa0==(*p)); p++;
            assert(0xa0==(*p)); p++;
        }	
    }
    
    
    // thicken
    printf("FpImage::selfTest().....Thicken\n");
    FpImage img3( 20, 20, 0xff00ff );
    
    img3.buf[ (10 + (10*20))*3 + 0 ] = 0xA0; // seed pixel
    img3.buf[ (10 + (10*20))*3 + 1 ] = 0xA0;
    img3.buf[ (10 + (10*20))*3 + 2 ] = 0xA0;
    
    int expectedFgPixels[] = {
        1,5,13,25,41,61,85,113,145,181,219,
        255,287,315,339,359,375,387,395,399,400,
        400, 400 };    
    
    for (k=0; k < sizeof(expectedFgPixels)/sizeof(int); k++) {
        int fg_count = 0;
        int bg_count = 0;
        
        p = img3.buf;	
        for (int j=0; j < img3.h; j++) {
            for (int i=0; i < img3.w; i++) {
                if (*p==0xff) {
                    bg_count++;
                    assert(0xff==(*p)); p++;
                    assert(0x00==(*p)); p++;
                    assert(0xff==(*p)); p++;
                } else if (*p==0xa0) {
                    fg_count++;
                    assert(0xa0==(*p)); p++;
                    assert(0xa0==(*p)); p++;
                    assert(0xa0==(*p)); p++;
                } else {
                    // some other color
                    assert( 0 );		    
                }
            }
        }
        
        // make sure the count is what we expect
        assert( fg_count  + bg_count == 400 );
        assert( expectedFgPixels[k] == fg_count );	
        
        img3.thicken( 0xff00ff );	
    }
    
    
    // auto crop
    printf("FpImage::selfTest().....AutoCrop\n");
    FpImage img_src( 20, 20, 0xff00ff );
    FpImage img_cropped;
    
    img_src.buf[ (10 + (10*20))*3 + 0 ] = 0xA0; // seed pixel
    img_src.buf[ (10 + (10*20))*3 + 1 ] = 0xA0;
    img_src.buf[ (10 + (10*20))*3 + 2 ] = 0xA0;
    
    int expectedSize = 1;    
    for ( k=0; k < 13; k++) {
        img_cropped = FpImage( img_src );
        
        img_cropped.autoCrop( 0xff00ff );
        assert( img_cropped.w == expectedSize );
        assert( img_cropped.h == expectedSize );
        expectedSize += 2;
        if (expectedSize > 20) expectedSize=20;	
        
        img_src.thicken( 0xff00ff );
    }
    
    
    return true;    
}
#endif



