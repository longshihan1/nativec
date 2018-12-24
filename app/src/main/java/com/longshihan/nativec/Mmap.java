package com.longshihan.nativec;

/**
 * Created by LONGHE001.
 *
 * @time 2018/12/24 0024
 * @des MMAP实现
 * @function
 */

public class Mmap {

    static {
        System.loadLibrary("longshihan");
    }

    /**
     * 初始化
     * @param dir
     * @param fileName
     * @return 指针
     */
    public native long nativeInit(String dir,String fileName);

    /**
     * 写入数据
     * @param logWriterObject
     * @param msgContent
     * @return
     */
    public native long nativeWrite(long logWriterObject, String msgContent);

    /**
     * 关闭
     * @param logWriterObject
     */
    private native void nativeCloseAndRenew(long logWriterObject);
}
