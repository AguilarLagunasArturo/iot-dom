package com.arturo.aguilar.lagunas.iot.device.app;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.Menu;


public class DeviceSwitchActivity extends AppCompatActivity {

    private String TAG = "DEBUG DeviceActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_device_switch);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        setTitle( getIntent().getStringExtra("Device") );
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onSupportNavigateUp() {
        onBackPressed();
        return super.onSupportNavigateUp();
    }
}