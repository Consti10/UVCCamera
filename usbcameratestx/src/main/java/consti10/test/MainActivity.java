package consti10.test;

import androidx.appcompat.app.AppCompatActivity;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.PixelFormat;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.util.HashMap;
import java.util.Iterator;


public class MainActivity extends AppCompatActivity {
    private static final String TAG="MainActivityX";
    private SurfaceView surfaceView;
    private UVCPlayer mUVCPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mUVCPlayer=new UVCPlayer(this);
        surfaceView=findViewById(R.id.xSurfaceView);
        surfaceView.getHolder().setFixedSize(640,480);
        surfaceView.getHolder().addCallback(mUVCPlayer);
    }


    @Override
    protected void onResume(){
        super.onResume();
    }

    @Override
    protected void onPause(){
        super.onPause();
    }

}
