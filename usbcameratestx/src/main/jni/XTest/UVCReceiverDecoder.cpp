

#include <jni.h>
#include <android/native_window_jni.h>


#include <libusb.h>
#include <libuvc.h>
#include <stdio.h>
#include <cstring>

#include "../NDKHelper/MDebug.hpp"
#include "../NDKHelper/NDKArrayHelper.hpp"

#include "MyTime.hpp"
#include "MJPEGDecodeAndroid.hpp"

class UVCReceiverDecoder{
private:
    ANativeWindow* aNativeWindow=nullptr;
    // Need a static function that calls class instance for the c-style uvc lib
    static void callbackProcessFrame(uvc_frame_t* frame, void* self){
        ((UVCReceiverDecoder *) self)->processFrame(frame);
    }
    uvc_context_t *ctx;
    uvc_device_t *dev;
    uvc_device_handle_t *devh;
    boolean isStreaming;
public:
    // Investigate: Even tough the documentation warns about dropping frames if processing takes too long
    // I cannot experience dropped frames - ?
    // Using less threads (no extra thread for decoding) reduces throughput but also latency
    void processFrame(uvc_frame_t* frame_mjpeg){
        CLOGD("Got uvc_frame_t %d",frame_mjpeg->sequence);
        ANativeWindow_Buffer buffer;
        if(ANativeWindow_lock(aNativeWindow, &buffer, NULL)==0){
            //decode_mjpeg_into_ANativeWindowBuffer2(frame_mjpeg,buffer);
            const auto before=GetTicksNanos();
            MJPEGDecodeAndroid::DecodeMJPEGtoANativeWindowBuffer(frame_mjpeg,buffer);
            const auto after=GetTicksNanos();
            const auto deltaUS=after-before;
            CLOGD("Time decoding ms %d",(int)((deltaUS / 1000) / 1000));
            ANativeWindow_unlockAndPost(aNativeWindow);
        }else{
            CLOGD("Cannot lock window");
        }
    }
    // Connect via android java first (workaround ?!)
    void startReceiving(jint vid, jint pid, jint fd,
                        jint busnum,jint devAddr,
                        jstring usbfs_str,ANativeWindow* window){
        this->aNativeWindow=window;
        uvc_stream_ctrl_t ctrl;
        uvc_error_t res;
        /* Initialize a UVC service context. Libuvc will set up its own libusb
         * context. Replace NULL with a libusb_context pointer to run libuvc
         * from an existing libusb context. */
        const char* usbfs="/dev/bus/usb";
        res = uvc_init2(&ctx,NULL,usbfs);
        if (res < 0) {
            CLOGD("Error uvc_init %d",res);
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
                CLOGD("Error uvc_open"); /* unable to open device */
            } else {
                CLOGD("Device opened");

                //X MJPEG only
                res = uvc_get_stream_ctrl_format_size(
                        devh, &ctrl,
                        UVC_FRAME_FORMAT_MJPEG,
                        640, 480, 30
                );
                /* Print out the result */
                uvc_print_stream_ctrl(&ctrl, stderr);
                if (res < 0) {
                    uvc_perror(res, "get_mode"); /* device doesn't provide a matching stream */
                } else {
                    res = uvc_start_streaming(devh, &ctrl, this->callbackProcessFrame, this, 0);
                    if (res < 0) {
                        CLOGD("Error start_streaming %d",res); /* unable to start stream */
                    } else {
                        CLOGD("Streaming...");
                        //uvc_set_ae_mode(devh, 1); /* e.g., turn on auto exposure */
                        isStreaming=true;
                        return;
                        sleep(10); /* stream for 10 seconds */
                        /* End the stream. Blocks until last callback is serviced */
                        uvc_stop_streaming(devh);
                        puts("Done streaming.");
                    }
                }
                /* Release our handle on the device */
                uvc_close(devh);
            }
            /* Release the device descriptor */
            uvc_unref_device(dev);
        }
        /* Close the UVC context. This closes and cleans up any existing device handles,
         * and it closes the libusb context if one was not provided. */
        uvc_exit(ctx);
    }

    void stopReceiving(){
        if(isStreaming){
            uvc_stop_streaming(devh);
            uvc_close(devh);
            uvc_unref_device(dev);
            uvc_exit(ctx);
        }
        if(aNativeWindow!= nullptr){
            ANativeWindow_release(aNativeWindow);
            aNativeWindow= nullptr;
        }
    }
};

// ------------------------------------- Native Bindings -------------------------------------
#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_consti10_test_UVCReceiverDecoder_##method_name
extern "C" {

inline jlong jptr(UVCReceiverDecoder *p) {
    return reinterpret_cast<intptr_t>(p);
}
inline UVCReceiverDecoder *native(jlong ptr) {
    return reinterpret_cast<UVCReceiverDecoder*>(ptr);
}

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jclass jclass1) {
    return jptr(new UVCReceiverDecoder());
}
JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jclass jclass1, jlong p) {
    delete native(p);
}

JNI_METHOD(void, nativeStartReceiving)
(JNIEnv *env, jclass jclass1,jlong nativeInstance,
 jint vid, jint pid, jint fd,
 jint busnum,jint devAddr,
 jstring usbfs_str,jobject surface
) {
    ANativeWindow* window=ANativeWindow_fromSurface(env,surface);
    native(nativeInstance)->startReceiving(vid,pid,fd,busnum,devAddr,usbfs_str,window);
}
JNI_METHOD(void, nativeStopReceiving)
(JNIEnv *env, jclass jclass1, jlong p) {
   native(p)->stopReceiving();
}
}
