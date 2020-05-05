

#include <jni.h>
#include <android/native_window_jni.h>


#include <libusb.h>
#include <libuvc.h>
#include <stdio.h>
#include <cstring>

#include "../NDKHelper/MDebug.hpp"


struct Handle{
    ANativeWindow* aNativeWindow;
};

// Input1: uvc_frame_t of type MJPEG (ONLY)
// Input2: ANativeWindow_Buffer of RGBA / RGBX ONLY !!
void decode_mjpeg_into_ANativeWindowBuffer(uvc_frame_t* frame_mjpeg,const ANativeWindow_Buffer& buffer){
    CLOGD("ANativeWindow_Buffer: W H Stride Format %d %d %d %d",buffer.width,buffer.height,buffer.stride,buffer.format);
    uvc_frame_t* rgba = uvc_allocate_frame(frame_mjpeg->width * frame_mjpeg->height * 4);
    uvc_error_t result = uvc_mjpeg2rgbx(frame_mjpeg, rgba);
    if(result!=UVC_SUCCESS){
        CLOGD("Error MJPEG conversion %d", result);
        uvc_free_frame(rgba);
        return;
    }
    CLOGD("bytes %d actual bytes %d",(int)rgba->data_bytes,(int)rgba->actual_bytes);
    memcpy(buffer.bits,rgba->data,rgba->data_bytes);
    uvc_free_frame(rgba);
}

// skip the one unnecessary memcpy
// more supported ANativeWindow_Buffer layouts
// Log error if unsupported ANativeWindow hardware layout
void decode_mjpeg_into_ANativeWindowBuffer2(uvc_frame_t* frame_mjpeg,const ANativeWindow_Buffer& buffer){
    CLOGD("ANativeWindow_Buffer: W H Stride Format %d %d %d %d",buffer.width,buffer.height,buffer.stride,buffer.format);
    typedef uvc_error_t (*convFunc_t)(uvc_frame_t *in, uvc_frame_t *out);

    unsigned int BYTES_PER_PIXEL;
    convFunc_t conversionFunction;
    if(buffer.format==AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM || buffer.format==AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM){
        BYTES_PER_PIXEL=4;
        conversionFunction=uvc_mjpeg2rgbx;
    }else if(buffer.format==AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM){
        BYTES_PER_PIXEL=3;
        conversionFunction=uvc_mjpeg2rgb;
    }else if(buffer.format==AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM){
        BYTES_PER_PIXEL=2;
        conversionFunction=uvc_mjpeg2rgb565;
    }else{
        CLOGD("Error Unsupported format");
        return;
    }
    uvc_frame_t frame_decoded;
    frame_decoded.data=buffer.bits;
    frame_decoded.data_bytes=frame_mjpeg->width * frame_mjpeg->height*BYTES_PER_PIXEL;
    frame_decoded.step=640*BYTES_PER_PIXEL;
    uvc_error_t result = conversionFunction(frame_mjpeg, &frame_decoded);
    if(result!=UVC_SUCCESS){
        CLOGD("Error MJPEG conversion %d", result);
        return;
    }
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
        decode_mjpeg_into_ANativeWindowBuffer2(frame_mjpeg,buffer);
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
