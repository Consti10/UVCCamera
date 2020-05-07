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
    private final UVCPlayer mUVCPlayer=new UVCPlayer();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        surfaceView=findViewById(R.id.xSurfaceView);
        surfaceView.getHolder().setFixedSize(640,480);
        surfaceView.getHolder().setFormat(PixelFormat.RGBA_8888);
        //surfaceView.getHolder().setFormat(ImageFormat.YUV_420_888);
        surfaceView.getHolder().addCallback(mUVCPlayer);
    }

    private void start(){
        final UsbManager usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);

        final PendingIntent permissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(UVCPlayer.ACTION_USB_PERMISSION), 0);

        final HashMap<String, UsbDevice> deviceList =usbManager.getDeviceList();

        Log.d(TAG,"There are "+deviceList.size()+" devices connected");
        UVCHelper.filterFOrUVC(deviceList);

        final Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
        while(deviceIterator.hasNext()){
            final UsbDevice device = deviceIterator.next();
            Log.d(TAG,"USB Device"+device.getDeviceName());
            usbManager.requestPermission(device, permissionIntent);
        }
    }

    @Override
    protected void onResume(){
        super.onResume();
        //register the broadcast receiver
        registerReceiver(mUVCPlayer,UVCPlayer.getIntentFilter());
        start();
    }

    @Override
    protected void onPause(){
        super.onPause();
        unregisterReceiver(mUVCPlayer);
        mUVCPlayer.cancel();
    }

}
