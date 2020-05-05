package consti10.test;

import androidx.appcompat.app.AppCompatActivity;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;

import java.util.HashMap;
import java.util.Iterator;


public class MainActivity extends AppCompatActivity {
    private static final String TAG="MainActivityX";
    private static final String ACTION_USB_PERMISSION =
            "com.android.example.USB_PERMISSION";
    private final BroadcastReceiver usbReceiver = new BroadcastReceiver() {

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
                            XTest.nativeHello(device.getVendorId(),device.getProductId(),connection.getFileDescriptor(),busnum,devnum,device.getDeviceName());
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
    }

    @Override
    protected void onResume(){
        super.onResume();
        final UsbManager usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);

        PendingIntent permissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION), 0);
        IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
        registerReceiver(usbReceiver, filter);

        final HashMap<String, UsbDevice> deviceList =usbManager.getDeviceList();
        final Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
        while(deviceIterator.hasNext()){
            final UsbDevice device = deviceIterator.next();
            Log.d(TAG,"USB Device"+device.getDeviceName());
            usbManager.requestPermission(device, permissionIntent);
        }
    }

}
