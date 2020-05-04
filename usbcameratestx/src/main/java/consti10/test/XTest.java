package consti10.test;

public class XTest {
    static{
        System.loadLibrary("jpeg-turbo1500");
        System.loadLibrary("usb1.0");
        System.loadLibrary("uvc");
        System.loadLibrary("XTest");
    }
    public static native void nativeHello(String s);
}
