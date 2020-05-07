package consti10.test;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

public class UVCPlayer extends BroadcastReceiver implements SurfaceHolder.Callback {
    private static final String TAG="UVCPlayer";
    private final UVCReceiverDecoder mUVCReceiverDecoder=new UVCReceiverDecoder();
    public static final String ACTION_USB_PERMISSION =
            "com.android.example.USB_PERMISSION";
    public static final String USB_DEVICE_ATTACHED="android.hardware.usb.action.USB_DEVICE_ATTACHED";
    public static final String USB_DEVICE_DETACHED="android.hardware.usb.action.USB_DEVICE_DETACHED";
    public static final IntentFilter getIntentFilter(){
        final IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
        filter.addAction(USB_DEVICE_ATTACHED);
        filter.addAction(USB_DEVICE_DETACHED);
        return filter;
    }

    public void cancel(){
        mUVCReceiverDecoder.stopReceiving();
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (action.contentEquals(ACTION_USB_PERMISSION)) {
            Log.d(TAG,"ACTION_USB_PERMISSION");
            final UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
            if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                if(device != null){
                    mUVCReceiverDecoder.startReceiving(context,device);
                }
            }
            else {
                Log.d(TAG, "permission denied for device " + device);
            }

        }else if(action.contentEquals(USB_DEVICE_ATTACHED)){
            Log.d(TAG,"USB_DEVICE_ATTACHED");
            final UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
            //.startReceiving(context,device,surfaceView.getHolder().getSurface());
        }else if(action.contentEquals(USB_DEVICE_DETACHED)){
            Log.d(TAG,"USB_DEVICE_DETACHED");
        }else{
            Log.d(TAG,"Unknwn broadcast");
        }
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
       mUVCReceiverDecoder.setSurface(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        mUVCReceiverDecoder.setSurface(null);
        mUVCReceiverDecoder.stopReceiving();
    }
}
