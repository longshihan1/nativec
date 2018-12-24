package com.longshihan.nativec;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class TestNative {

    static {
        System.loadLibrary("longshihan");
    }



    public  String getJNI(){
        return stringFromJNI();
    }
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}
