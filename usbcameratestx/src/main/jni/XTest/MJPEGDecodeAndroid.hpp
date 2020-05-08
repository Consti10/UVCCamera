//
// Created by geier on 07/05/2020.
//

#ifndef UVCCAMERA_MJPEGDECODEANDROID_HPP
#define UVCCAMERA_MJPEGDECODEANDROID_HPP

#include "HuffTables.hpp"
#include <jni.h>
#include <android/native_window_jni.h>


// Since I only need to support android it is cleaner to write my own conversion function.
// inspired by the uvc_mjpeg_to_rgbx .. functions
// Including this file adds dependency on Android and libjpeg-turbo

namespace MJPEGDecodeAndroid{
    // Helper that prints the current configuration of ANativeWindow_Buffer
    static void debugANativeWindowBuffer(const ANativeWindow_Buffer& buffer){
        CLOGD("ANativeWindow_Buffer: W H Stride Format %d %d %d %d",buffer.width,buffer.height,buffer.stride,buffer.format);
    }
    // Supports the most common ANativeWindow_Buffer image formats
    // No unnecessary memcpy's & correctly handle stride of ANativeWindow_Buffer
    // input uvc_frame_t frame has to be of type MJPEG
    static void DecodeMJPEGtoANativeWindowBuffer(uvc_frame_t* frame_mjpeg,const ANativeWindow_Buffer& nativeWindowBuffer){
        //debugANativeWindowBuffer(nativeWindowBuffer);
        if(nativeWindowBuffer.width!=frame_mjpeg->width || nativeWindowBuffer.height!=frame_mjpeg->height){
            CLOGD("Error window & frame : size / width does not match");
            return;
        }
        struct jpeg_decompress_struct dinfo;
        struct error_mgr jerr;
        dinfo.err = jpeg_std_error(&jerr.super);
        jerr.super.error_exit = _error_exit;
        jpeg_create_decompress(&dinfo);

        jpeg_mem_src(&dinfo, (const unsigned char*)frame_mjpeg->data, frame_mjpeg->actual_bytes);
        jpeg_read_header(&dinfo, TRUE);
        if (dinfo.dc_huff_tbl_ptrs[0] == NULL) {
            /* This frame is missing the Huffman tables: fill in the standard ones */
            insert_huff_tables(&dinfo);
        }
        unsigned int BYTES_PER_PIXEL;
        if(nativeWindowBuffer.format==AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM || nativeWindowBuffer.format==AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM){
            dinfo.out_color_space = JCS_EXT_RGBA;
            BYTES_PER_PIXEL=4;
        }else if(nativeWindowBuffer.format==AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM){
            dinfo.out_color_space = JCS_EXT_RGB;
            BYTES_PER_PIXEL=3;
        }else if(nativeWindowBuffer.format==AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM){
            dinfo.out_color_space = JCS_RGB565;
            BYTES_PER_PIXEL=2;
        }else{
            CLOGD("Unsupported image format");
            return;
        }
        dinfo.dct_method = JDCT_IFAST;
        jpeg_start_decompress(&dinfo);
        // libjpeg error ? - output_components is 3 ofr RGB_565 ?
        //CLOGD("dinfo.output_components %d | %d",dinfo.output_components,dinfo.out_color_components);

        const unsigned int scanline_len = ((unsigned int)nativeWindowBuffer.stride) * BYTES_PER_PIXEL;
        JSAMPARRAY jsamparray[dinfo.output_height];
        for(int i=0;i<dinfo.output_height;i++){
            JSAMPROW row = (JSAMPROW)(((unsigned char*)nativeWindowBuffer.bits) + (i*scanline_len));
            jsamparray[i]=(JSAMPARRAY)row;
        }
        unsigned int scanline_count = 0;
        while (dinfo.output_scanline < dinfo.output_height)
        {
            // JSAMPROW row = (JSAMPROW)(((unsigned char*)nativeWindowBuffer.bits) + (scanline_count * scanline_len));
            JSAMPROW row2= (JSAMPROW)jsamparray[scanline_count];
            auto lines_read=jpeg_read_scanlines(&dinfo,&row2, 8);
            // unfortunately reads only one line at a time CLOGD("Lines read %d",lines_read);
            scanline_count+=lines_read;
        }
        //
        jpeg_finish_decompress(&dinfo);
        jpeg_destroy_decompress(&dinfo);
    }
}
#endif //UVCCAMERA_MJPEGDECODEANDROID_HPP
