///////////////////////////////////////////////////////////////////////////////
//
// File:    ftProInterface2013JpegDecode.cpp
//
// Project: ftPro - fischertechnik Control Graphical Programming System
//
// Module:  JPEG decode for images received from the TXT
//
// Author:  Michael Sögtrop
//
///////////////////////////////////////////////////////////////////////////////
//
// Urheberrecht im Sinne des Urheberrechtsgesetzes bei
//
// Michael Sögtrop
// Germany
//
// Copyright (C) 2015
//
///////////////////////////////////////////////////////////////////////////////
//
// TO DO:
//
///////////////////////////////////////////////////////////////////////////////
//
// Implementation details for module ftProInterface2013TransferAreaCom
//
// The images received from the ft camera are slightly damaged.
// At the end there is no EOR marker, but a bunch of garbage bytes.
// In order to decode such images successfully, we stop decoding
// after sucessfull decoding of the first scan.
//
// see also:
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <setjmp.h>

extern "C" {
//#pragma warning(disable : 4996)
#include "jpeglib.h"
#include "jerror.h"
//#pragma warning(default : 4996)
};

/* Read JPEG image from a memory segment  (This already exists in jpeglib v8) */
static void init_source (j_decompress_ptr cinfo) {}

static /*wxjpeg_*/boolean fill_input_buffer (j_decompress_ptr cinfo)
{
    ERREXIT(cinfo, JERR_INPUT_EMPTY);
    return TRUE;
}

static void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
    struct jpeg_source_mgr* src = (struct jpeg_source_mgr*) cinfo->src;

    if (num_bytes > 0) {
        src->next_input_byte += (size_t) num_bytes;
        src->bytes_in_buffer -= (size_t) num_bytes;
    }
}

static void term_source (j_decompress_ptr cinfo) {}

static void jpeg_mem_src (j_decompress_ptr cinfo, const void* buffer, long nbytes)
{
    struct jpeg_source_mgr* src;

    if (cinfo->src == NULL) {   /* first time for this JPEG object? */
        cinfo->src = (struct jpeg_source_mgr *)
            (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
            sizeof(struct jpeg_source_mgr));
    }

    src = (struct jpeg_source_mgr*) cinfo->src;
    src->init_source = init_source;
    src->fill_input_buffer = fill_input_buffer;
    src->skip_input_data = skip_input_data;
    src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
    src->term_source = term_source;
    src->bytes_in_buffer = nbytes;
    src->next_input_byte = (JOCTET*)buffer;
}

/* error handler info structure */

struct ftProJpegDecErrDataT {
  struct jpeg_error_mgr pub;  /* "public" fields */
  jmp_buf setjmp_buffer;         /* for return to caller */
};

/* ftPro JPEG decoder error handler */

void ftProJpegDecErrHandler (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  ftProJpegDecErrDataT * err = (ftProJpegDecErrDataT *) cinfo->err;

  /* Return control to the setjmp point */
  longjmp(err->setjmp_buffer, 1);
}

/* Jpeg decoder, adopted from LIBJPEG example.c */

bool ftProJpegDec(const UINT8 *jpegdata, int jpegsize, UINT8 *yuvdata, int yuvsize, size_t *bytes_read)
{
    /* This struct contains the JPEG decompression parameters and pointers to
    * working space (which is allocated as needed by the JPEG library).
    */
    struct jpeg_decompress_struct cinfo;
    /* We use our private extension JPEG error handler.
    * Note that this struct must live as long as the main JPEG parameter
    * struct, to avoid dangling-pointer problems.
    */
    struct ftProJpegDecErrDataT jerr;

    /* Step 1: allocate and initialize JPEG decompression object */

    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = ftProJpegDecErrHandler;

    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer))
    {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object, close the input file, and return.
         */
        jpeg_destroy_decompress(&cinfo);
        return false;
    }

    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */

    jpeg_mem_src(&cinfo, jpegdata, jpegsize);

    /* Step 3: read file parameters with jpeg_read_header() */

    (void) jpeg_read_header(&cinfo, TRUE);
    /* We can ignore the return value from jpeg_read_header since
     *   (a) suspension is not possible with the stdio data source, and
     *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
     * See libjpeg.txt for more info.
     */

    /* Step 4: set parameters for decompression */
    cinfo.raw_data_out = 1;
    cinfo.out_color_space = JCS_YCbCr;
    cinfo.dct_method = JDCT_IFAST;
    cinfo.dither_mode = JDITHER_NONE;
    cinfo.do_fancy_upsampling = 0;
    cinfo.do_block_smoothing = 0;

    /* Step 5: Start decompressor */

    (void) jpeg_start_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* Step 6: read raw data and place into YUV buffer */
    JSAMPARRAY bufY = (*cinfo.mem->alloc_sarray)( (j_common_ptr)&cinfo, JPOOL_IMAGE, cinfo.image_width, DCTSIZE);
    JSAMPARRAY bufU = (*cinfo.mem->alloc_sarray)( (j_common_ptr)&cinfo, JPOOL_IMAGE, cinfo.image_width/2, DCTSIZE);
    JSAMPARRAY bufV = (*cinfo.mem->alloc_sarray)( (j_common_ptr)&cinfo, JPOOL_IMAGE, cinfo.image_width/2, DCTSIZE);
    JSAMPARRAY image[3] = { bufY, bufU, bufV };
    UINT8 *outpos = yuvdata;

    for( JDIMENSION iscan=0; iscan<cinfo.image_height; iscan+=DCTSIZE )
    {
        jpeg_read_raw_data(&cinfo, image, DCTSIZE );

        for( int line=0; line<DCTSIZE; line++ )
        {
            for( JDIMENSION x=0; x<cinfo.image_width; x+=2 )
            {
                *outpos++ = bufY[line][x];
                *outpos++ = bufU[line][x>>1];
                *outpos++ = bufY[line][x+1];
                *outpos++ = bufV[line][x>>1];
            }
        }
    }

    /* Step 7: Finish decompression */
    
    /* Don't do this. The MJPEG stream has some bogus stuff after the JPEG data end */
    // (void) jpeg_finish_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
    * with the stdio data source.
    */

    /* Save number of bytes read */
    if( bytes_read )
    {
        *bytes_read = cinfo.src->next_input_byte-jpegdata;
    }

    /* Step 8: Release JPEG decompression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(&cinfo);

    /* At this point you may want to check to see whether any corrupt-data
    * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
    */

    /* And we're done! */
    return true;
}
