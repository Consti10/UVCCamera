package consti10.test;

import android.view.Surface;

public class XTest {
    static{
        System.loadLibrary("jpeg-turbo");
        System.loadLibrary("usb1.0");
        System.loadLibrary("uvc");
        System.loadLibrary("XTest");
    }

    //public static native void nativeHello(long id_camera, int venderId, int productId, int fileDescriptor, int busNum, int devAddr, String usbfs);
    public static native void nativeHello(int venderId, int productId, int fileDescriptor, int busNum, int devAddr, String usbfs, Surface surface);
}
