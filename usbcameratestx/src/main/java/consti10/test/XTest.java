package consti10.test;

public class XTest {
    static{
        System.loadLibrary("XTest");
    }
    public static native void nativeHello(String s);
}
