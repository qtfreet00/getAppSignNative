package com.qtfreet00.getsignnative;

import android.content.pm.PackageManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import com.msarlab.library.sign.SignHelper;

import java.util.Arrays;

public class MainActivity extends AppCompatActivity {


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        TextView tv = (TextView) findViewById(R.id.sample_text);
        boolean a = getSignCharString().equals(SignHelper.toCharString());
        boolean b = Arrays.equals(getSignByteArray(), SignHelper.toByteArray());
        Log.e("qtfreet00", Arrays.toString(getSignByteArray()));
        Log.e("qtfreet00", Arrays.toString(SignHelper.toByteArray()));
        boolean c = getSignHashCode() == SignHelper.toHashCode();
        Log.e("qtfreet00", getSignHashCode() + "");
        Log.e("qtfreet00", SignHelper.toHashCode() + "");
        boolean d = getSignCharString().equals(SignHelper.toCharString());
        tv.setText("签名:  " + a + "  " + b + "  " + c + "  " + d);
    }

    private String getSignCharString() {
        try {
            return getPackageManager().getPackageInfo(getPackageName(), PackageManager.GET_SIGNATURES).signatures[0].toCharsString();
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        return "";
    }


    private byte[] getSignByteArray() {
        try {
            return getPackageManager().getPackageInfo(getPackageName(), PackageManager.GET_SIGNATURES).signatures[0].toByteArray();
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        return null;
    }

    private int getSignHashCode() {
        try {
            return getPackageManager().getPackageInfo(getPackageName(), PackageManager.GET_SIGNATURES).signatures[0].hashCode();
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        return 0;
    }

}
