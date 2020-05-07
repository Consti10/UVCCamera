package consti10.test;

import androidx.appcompat.app.AppCompatActivity;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.PixelFormat;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.util.HashMap;
import java.util.Iterator;


public class MainActivity extends AppCompatActivity {
    private static final String TAG="MainActivityX";
    private static final String ACTION_USB_PERMISSION =
            "com.android.example.USB_PERMISSION";
    private SurfaceView surfaceView;
    private boolean started=false;
    private final BroadcastReceiver usbPermissionReceiver = new BroadcastReceiver() {

        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                synchronized (this) {

                    UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);

                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        if(device != null){
                            //call method to set up device communication
                            final UsbManager usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
                            final UsbDeviceConnection connection=usbManager.openDevice(device);

                            //
                            final String name = device.getDeviceName();
                            final String[] v = !TextUtils.isEmpty(name) ? name.split("/") : null;
                            int busnum = 0;
                            int devnum = 0;
                            if (v != null) {
                                busnum = Integer.parseInt(v[v.length-2]);
                                devnum = Integer.parseInt(v[v.length-1]);
                            }
                            //
                            while (surfaceView.getHolder().getSurface()==null){
                                //wait !
                            }
                            XTest.nativeHello(device.getVendorId(),device.getProductId(),connection.getFileDescriptor(),busnum,devnum,device.getDeviceName(),
                                    surfaceView.getHolder().getSurface());
                        }
                    }
                    else {
                        Log.d(TAG, "permission denied for device " + device);
                    }
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        // register the broadcast receiver
        final IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
        registerReceiver(usbPermissionReceiver, filter);
        surfaceView=findViewById(R.id.xSurfaceView);
        surfaceView.getHolder().setFixedSize(640,480);
        surfaceView.getHolder().setFormat(PixelFormat.RGBA_8888);
        //surfaceView.getHolder().setFormat(ImageFormat.YUV_420_888);
        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                start();
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });
    }

    private void start(){
        if(started)return;
        started=true;
        final UsbManager usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);

        final PendingIntent permissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION), 0);

        final HashMap<String, UsbDevice> deviceList =usbManager.getDeviceList();

        Log.d(TAG,"There are "+deviceList.size()+" devices connected");
        filterFOrUVC(deviceList);

        final Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
        while(deviceIterator.hasNext()){
            final UsbDevice device = deviceIterator.next();
            Log.d(TAG,"USB Device"+device.getDeviceName());
            usbManager.requestPermission(device, permissionIntent);
        }
    }

    private static HashMap<String,UsbDevice> filterFOrUVC(final HashMap<String,UsbDevice> deviceList){
        final HashMap<String,UsbDevice> ret=new HashMap<>();
        final Iterator<String> keyIterator = deviceList.keySet().iterator();
        while(keyIterator.hasNext()){
            final String key=keyIterator.next();
            final UsbDevice device = deviceList.get(key);
            if(device.getDeviceClass()==255 && device.getDeviceSubclass()==2){
                Log.d(TAG,"Found okay device");
            }
            ret.put(key,device);
        }
        return ret;
    }

    @Override
    protected void onResume(){
        super.onResume();
    }

}
