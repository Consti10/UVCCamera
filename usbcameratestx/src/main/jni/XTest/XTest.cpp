

#include <jni.h>
#include <android/native_window_jni.h>


#include <libusb.h>
#include <libuvc.h>
#include <stdio.h>
#include <cstring>

#include "../NDKHelper/MDebug.hpp"
#include "../NDKHelper/NDKArrayHelper.hpp"

#include <jpeglib.h>
#include <setjmp.h>
#include "HuffTables.hpp"
#include "myTime.h"

class XTEst{
public:

};

struct Handle{
    ANativeWindow* aNativeWindow;
};

static void debugANativeWindowBuffer(const ANativeWindow_Buffer& buffer){
    CLOGD("ANativeWindow_Buffer: W H Stride Format %d %d %d %d",buffer.width,buffer.height,buffer.stride,buffer.format);
}

// Since I only need to support android it is cleaner to write my own conversion function.
// inspired by the uvc_mjpeg_to_rgbx .. functions
// Supports the most common ANativeWindow_Buffer image formats
// No unnecessary memcpy's & correctly handle stride of ANativeWindow_Buffer
void x_decode_mjpeg_into_ANativeWindowBuffer2(uvc_frame_t* frame_mjpeg,const ANativeWindow_Buffer& nativeWindowBuffer){
    debugANativeWindowBuffer(nativeWindowBuffer);
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
        CLOGD("Lines read %d",lines_read);
        scanline_count+=lines_read;
    }
    //
    jpeg_finish_decompress(&dinfo);
    jpeg_destroy_decompress(&dinfo);
}


/* This callback function runs once per frame. Use it to perform any
 * quick processing you need, or have it put the frame into your application's
 * input queue. If this function takes too long, you'll start losing frames. */
// HM: Do we have enough time to decode mjpeg frame without dropping frames ?
void cb(uvc_frame_t *frame_mjpeg, void *ptr) {

    Handle* handle=(Handle*)ptr;
    CLOGD("Got uvc_frame_t %d",frame_mjpeg->sequence);
    ANativeWindow_Buffer buffer;
    if(ANativeWindow_lock(handle->aNativeWindow, &buffer, NULL)==0){
        //decode_mjpeg_into_ANativeWindowBuffer2(frame_mjpeg,buffer);
        const auto before=GetTicksNanos();
        x_decode_mjpeg_into_ANativeWindowBuffer2(frame_mjpeg,buffer);
        const auto after=GetTicksNanos();
        const auto deltaUS=after-before;
        CLOGD("Time decoding ms %d",(int)((deltaUS / 1000) / 1000));
        ANativeWindow_unlockAndPost(handle->aNativeWindow);
    }else{
        CLOGD("Cannot lock window");
    }
}

static int example(jint vid, jint pid, jint fd,
                   jint busnum,jint devAddr,
                   jstring usbfs_str,ANativeWindow* window) {

    uvc_context_t *ctx;
    uvc_device_t *dev;
    uvc_device_handle_t *devh;
    uvc_stream_ctrl_t ctrl;
    uvc_error_t res;
    /* Initialize a UVC service context. Libuvc will set up its own libusb
     * context. Replace NULL with a libusb_context pointer to run libuvc
     * from an existing libusb context. */
    const char* usbfs="/dev/bus/usb";
    res = uvc_init2(&ctx,NULL,usbfs);
    if (res < 0) {
        CLOGD("Error uvc_init %d",res);
        return res;
    }
    CLOGD("UVC initialized");
    /* Locates the first attached UVC device, stores in dev */
    //res = uvc_find_device(
    //        ctx, &dev,
    //        0, 0, NULL); /* filter devices: vendor_id, product_id, "serial_num" */
    //res = uvc_get_device_with_fd(ctx, &dev, pid, vid, NULL, fd, NULL, NULL);
    res = uvc_get_device_with_fd(ctx, &dev, vid, pid, NULL, fd, busnum, devAddr);
    if (res < 0) {
        uvc_perror(res, "uvc_find_device"); /* no devices found */
    } else {
        CLOGD("Device found");
        /* Try to open the device: requires exclusive access */
        res = uvc_open(dev, &devh);
        if (res < 0) {
            uvc_perror(res, "uvc_open"); /* unable to open device */
        } else {
            CLOGD("Device opened");
            /* Print out a message containing all the information that libuvc
             * knows about the device */
            const char* mLog;
            uvc_print_diag(devh,stderr);

            //X MJPEG only /* Try to negotiate a 640x480 30 fps YUYV stream profile */
            res = uvc_get_stream_ctrl_format_size(
                    devh, &ctrl, /* result stored in ctrl */
                    UVC_FRAME_FORMAT_MJPEG, /* YUV 422, aka YUV 4:2:2. try _COMPRESSED */
                    640, 480, 30 /* width, height, fps */
            );
            /* Print out the result */
            uvc_print_stream_ctrl(&ctrl, stderr);
            if (res < 0) {
                uvc_perror(res, "get_mode"); /* device doesn't provide a matching stream */
            } else {
                /* Start the video stream. The library will call user function cb:
                 *   cb(frame, (void*) 12345)
                 */
                Handle* handle=new Handle();
                handle->aNativeWindow=window;
                res = uvc_start_streaming(devh, &ctrl, cb, handle, 0);
                if (res < 0) {
                    uvc_perror(res, "start_streaming"); /* unable to start stream */
                } else {
                    puts("Streaming...");
                    uvc_set_ae_mode(devh, 1); /* e.g., turn on auto exposure */
                    //TODO start poling new frames ? Maybe ..:
                    sleep(10); /* stream for 10 seconds */
                    /* End the stream. Blocks until last callback is serviced */
                    uvc_stop_streaming(devh);
                    puts("Done streaming.");
                }
            }
            /* Release our handle on the device */
            uvc_close(devh);
            puts("Device closed");
        }
        /* Release the device descriptor */
        uvc_unref_device(dev);
    }
    /* Close the UVC context. This closes and cleans up any existing device handles,
     * and it closes the libusb context if one was not provided. */
    uvc_exit(ctx);
    puts("UVC exited");
    return 0;
}

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_consti10_test_XTest_##method_name
extern "C" {

JNI_METHOD(void, nativeHello)
(JNIEnv *env, jclass jclass1,
 jint vid, jint pid, jint fd,
 jint busnum,jint devAddr,
 jstring usbfs_str,jobject surface
        ) {
    ANativeWindow* window=ANativeWindow_fromSurface(env,surface);


    example(vid,pid,fd,busnum,devAddr,usbfs_str,window);
}

}
