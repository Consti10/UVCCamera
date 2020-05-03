package consti10.usbcameratestx;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import com.serenegiant.usb.XTest;

public class MainActivity extends AppCompatActivity {
    //XTest xTest;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        XTest.nativeHello("BLA");
    }
}
