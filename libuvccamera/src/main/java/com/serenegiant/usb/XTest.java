package com.serenegiant.usb;

public class XTest {

    private static boolean isLoaded;
    static {
        if (!isLoaded) {
            System.loadLibrary("jpeg-turbo1500");
            System.loadLibrary("usb1.0");
            System.loadLibrary("uvc");
            System.loadLibrary("XTest");
            isLoaded = true;
        }
    }

    public static native void nativeHello(String s);

}
