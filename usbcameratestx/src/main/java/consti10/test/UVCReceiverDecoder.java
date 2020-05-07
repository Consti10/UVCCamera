package consti10.test;

import android.content.Context;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;

// Handles receiving & decoding of UVC devices that supply MJPEG frames (like ROTG02)
public class UVCReceiverDecoder {
    private static final String TAG="UVCReceiverDecoder";
    static{
        System.loadLibrary("jpeg-turbo");
        System.loadLibrary("usb1.0");
        System.loadLibrary("uvc");
        System.loadLibrary("UVCReceiverDecoder");
    }
    private long nativeInstance;
    private boolean alreadyStreaming=false;

    public UVCReceiverDecoder(){
        nativeInstance=nativeConstruct();
    }

    public void startReceiving(final Context context,final UsbDevice device,final Surface surface){
        if(alreadyStreaming){
            Log.d(TAG,"startReceiving() already called");
            return;
        }

        final UsbManager usbManager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        final UsbDeviceConnection connection=usbManager.openDevice(device);

        final String name = device.getDeviceName();
        final String[] v = !TextUtils.isEmpty(name) ? name.split("/") : null;
        int busnum = 0;
        int devnum = 0;
        if (v != null) {
            busnum = Integer.parseInt(v[v.length-2]);
            devnum = Integer.parseInt(v[v.length-1]);
        }
        //
        nativeStartReceiving(nativeInstance,device.getVendorId(),device.getProductId(),connection.getFileDescriptor(),busnum,devnum,device.getDeviceName(),
                surface);
        alreadyStreaming=true;
        //nativeSetOutputSurface(surface);
    }

    public void stopReceiving(){
        nativeStopReceiving(nativeInstance);
        alreadyStreaming=false;
    }


    private static native long nativeConstruct();
    private static native void nativeDelete(long nativeInstance);
    private static native void nativeStartReceiving(long nativeInstance,int venderId, int productId, int fileDescriptor, int busNum, int devAddr, String usbfs,Surface surface);
    private static native void nativeStopReceiving(long nativeInstance);
}
