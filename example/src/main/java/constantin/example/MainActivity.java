package constantin.example;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.SurfaceView;

import constantin.test.UVCPlayer;


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
