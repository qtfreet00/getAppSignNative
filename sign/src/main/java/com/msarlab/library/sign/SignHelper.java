package com.msarlab.library.sign;

public class SignHelper {

    static {
        System.loadLibrary("msarlab_sign");
    }

    private static native String k0();

    private static native byte[] k1();

    private static native int k2();

    private static native String k3();

    public static String toCharString() {
        return k0();
    }

    public static byte[] toByteArray() {
        return k1();
    }

    public static int toHashCode() {
        return k2();
    }

    public static String getAppPath() {
        return k3();
    }

}
