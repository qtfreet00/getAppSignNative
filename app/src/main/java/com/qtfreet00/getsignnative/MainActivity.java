package com.qtfreet00.getsignnative;

import android.content.pm.PackageManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("sign_native");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        TextView tv = (TextView) findViewById(R.id.sample_text);
        String sign = getSign();
        if (getSignJava().equals(sign)) {
            tv.setText("签名一致");
        } else {
            tv.setText("签名不相同");
        }
        benchmark();
    }

    private void benchmark() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                int counts = 10000;
                long start = System.currentTimeMillis();
                for (int i = 0; i < counts; i++) {
                    getSign();
                }
                long end = System.currentTimeMillis();
                long cost = end - start;
                Log.e("qtfreet00", "cost time " + cost);
            }
        }).start();
    }

    public native String getSign();

    private String getSignJava() {
        try {
            return getPackageManager().getPackageInfo(getPackageName(), PackageManager.GET_SIGNATURES).signatures[0].toCharsString();
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        return "";
    }
}
