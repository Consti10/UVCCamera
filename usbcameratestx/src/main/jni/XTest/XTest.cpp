

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

// from UVCPreview
static void copyFrame(const uint8_t *src, uint8_t *dest, const int width, int height, const int stride_src, const int stride_dest) {
    const int h8 = height % 8;
    for (int i = 0; i < h8; i++) {
        memcpy(dest, src, width);
        dest += stride_dest; src += stride_src;
    }
    for (int i = 0; i < height; i += 8) {
        memcpy(dest, src, width);
        dest += stride_dest; src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest; src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest; src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest; src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest; src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest; src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest; src += stride_src;
        memcpy(dest, src, width);
        dest += stride_dest; src += stride_src;
    }
}
//uvc_frame_t frame_rgba=

/* This callback function runs once per frame. Use it to perform any
 * quick processing you need, or have it put the frame into your application's
 * input queue. If this function takes too long, you'll start losing frames. */
void cb(uvc_frame_t *frame_mjpeg, void *ptr) {
    Handle* handle=(Handle*)ptr;

    uvc_frame_t *rgba;
    uvc_error_t ret;

    CLOGD("Frame here ! %d",frame_mjpeg->sequence);

    /* We'll convert the image from YUV/JPEG to BGR, so allocate space */
    rgba = uvc_allocate_frame(frame_mjpeg->width * frame_mjpeg->height * 4);
    if (!rgba) {
       CLOGD("unable to allocate rgba frame!");
        return;
    }

    uvc_error_t result = uvc_mjpeg2rgbx(frame_mjpeg, rgba);
    CLOGD("MJPEG conversion %d", result);

    if(result==UVC_SUCCESS){
        ANativeWindow_Buffer buffer;
        if(ANativeWindow_lock(handle->aNativeWindow, &buffer, NULL)==0){
            CLOGD("W H Stride %d %d %d",buffer.width,buffer.height,buffer.stride);
            //memcpy(buffer.bits,rgba->data,rgba->data_bytes);

            const int PREVIEW_PIXEL_BYTES=4; //RGBA

            const uint8_t *src = (uint8_t *)rgba->data;
            const int src_w = rgba->width * PREVIEW_PIXEL_BYTES;
            const int src_step = rgba->width * PREVIEW_PIXEL_BYTES;
            // destination = Surface(ANativeWindow)
            uint8_t *dest = (uint8_t *)buffer.bits;
            const int dest_w = buffer.width * PREVIEW_PIXEL_BYTES;
            const int dest_step = buffer.stride * PREVIEW_PIXEL_BYTES;
            // use lower transfer bytes
            const int w = src_w < dest_w ? src_w : dest_w;
            // use lower height
            const int h = rgba->height < buffer.height ? rgba->height : buffer.height;
            // transfer from frame data to the Surface
            copyFrame(src, dest, w, h, src_step, dest_step);

            ANativeWindow_unlockAndPost(handle->aNativeWindow);
        }else{
            CLOGD("Cannot lock window");
        }
    }
    uvc_free_frame(rgba);
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
