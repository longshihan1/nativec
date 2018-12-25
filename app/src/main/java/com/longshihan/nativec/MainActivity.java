package com.longshihan.nativec;

import android.app.ActivityManager;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("longshihan");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());
        TestNative testNative=new TestNative();
        tv.append(testNative.getJNI());
        Mmap mmap=new Mmap();
        long nativeLogWriter=  mmap.nativeInit(getTempDir(this).getAbsolutePath(),"test");
        mmap.nativeWrite(nativeLogWriter,"\n买一台，玩一年，流量不花一分钱\n");
        mmap.nativeWrite(nativeLogWriter,"小米的play被吐槽了。。。\n");

    }
    public static File getTempDir(Context context) {
        if (context == null) {
            return null;
        }

        File dir = getSDDirFile(context);
        if (dir == null) {
            dir = getCacheDirFile(context);
        }
        return dir;
    }

    private static File getCacheDirFile(Context context) {
        if (context == null) {
            return null;
        }
        File cacheFile = null;
        try {
            cacheFile = context.getCacheDir();
        } catch (Exception ex) {
            ex.printStackTrace();
            cacheFile = context.getExternalCacheDir();
        }

        if (cacheFile != null) {
            cacheFile = new File(cacheFile, getDirName(context));
            if (!cacheFile.exists() || !cacheFile.isDirectory()) {
                cacheFile.mkdirs();
            }
        }
        return cacheFile;
    }

    public static String getDirName(Context context) {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("LOGGGGGGGGG");
        stringBuilder.append(File.separator);
        stringBuilder.append(getCurProcessName(context));
        return stringBuilder.toString();
    }

    private static File getSDDirFile(Context context) {
        if (context == null || !Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState())) {
            return null;
        }
        File sdFile = Environment.getExternalStorageDirectory();
        if (sdFile != null) {
            sdFile = new File(sdFile, getDirName(context));
            if (!sdFile.exists() || !sdFile.isDirectory()) {
                sdFile.mkdirs();
            }
        }
        return sdFile;
    }

    /**
     * get the process name of the current process considering mutiprocess
     *
     * @param context
     * @return String
     */
    public static final String getCurProcessName(Context context) {
        if (context != null) {
            int pid = android.os.Process.myPid();
            ActivityManager mActivityManager = (ActivityManager) context
                    .getSystemService(Context.ACTIVITY_SERVICE);
            for (ActivityManager.RunningAppProcessInfo appProcess : mActivityManager
                    .getRunningAppProcesses()) {
                if (appProcess.pid == pid) {
                    return appProcess.processName;
                }
            }
        }
        return "UNKNOWN";
    }


    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}
