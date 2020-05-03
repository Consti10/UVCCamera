package com.serenegiant.usb;

public class XTest {

    private static boolean isLoaded;
    static {
        if (!isLoaded) {
            System.loadLibrary("jpeg-turbo1500");
            System.loadLibrary("usb100");
            System.loadLibrary("uvc");
            System.loadLibrary("XTest");
            isLoaded = true;
        }
    }

}
