package com.arturo.aguilar.lagunas.iot.device.app;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.Menu;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.android.volley.AuthFailureError;
import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;

import org.json.JSONObject;

import java.util.HashMap;
import java.util.Map;


public class DeviceSwitchActivity extends AppCompatActivity {

    private final String TAG = "DEBUG DeviceActivity";

    private final String url = "https://km83gb3bwa.execute-api.us-east-1.amazonaws.com/default/iot-skill-api";
    private final int refreshRate = 4000;
    private RequestQueue queue;
    private StringRequest availableRequest;
    private StringRequest toggleRequest;
    private StringRequest getStateRequest;

    private ImageView ivButton;
    private TextView tvOutput;
    private String deviceUUID;

    private boolean buttonState;
    private boolean available;

    private Handler handler;
    private Runnable runnable;

    private int ic_unavailable = R.drawable.ic_power_waiting;
    private int ic_on = R.drawable.ic_power_on;
    private int ic_off = R.drawable.ic_power_off;
    private int ic_pressed = R.drawable.ic_power_base;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_device_switch);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        deviceUUID = getIntent().getStringExtra("uuid");
        setTitle( deviceUUID );

        ivButton = (ImageView) findViewById(R.id.iv_toggle);
        tvOutput = (TextView) findViewById(R.id.tv_output);

        // Setup states
        available = false;
        buttonState = false;

        /* API Connection */
        queue = Volley.newRequestQueue(this);
        // API: is-available action
        HashMap<String, String> params = new HashMap<String, String>();
        params.put("action", "is-available");
        params.put("device-id", deviceUUID);
        JSONObject bodyAvailable = new JSONObject(params);
        availableRequest = new StringRequest(Request.Method.POST, url,
                new Response.Listener<String>() {
                    @Override
                    public void onResponse(String response) {
                        Log.d(TAG, "Response is-available received");
                        Log.d(TAG, "-> " + response);
                        if (response.endsWith("False\"")){
                            // DEBUG: set state OFF
                            Log.d(TAG, "Handle set-device-state off and update labels");
                            available = false;
                            updateScreenUnknown();
                        }else{
                            Log.d(TAG, "Handle get-device-state and update labels");
                            buttonState = response.endsWith("N - True\"");
                            available = true;
                            updateScreen();
                        }
                    }
                },
                new Response.ErrorListener() {
                    @Override
                    public void onErrorResponse(VolleyError error) {
                        Log.d(TAG, "Response is-available error");
                        Toast.makeText(getApplication(), R.string.toast_error, Toast.LENGTH_SHORT).show();
                        updateScreenUnknown();
                        error.printStackTrace();
                }
            }) {
            @Override
            public byte[] getBody() throws AuthFailureError {
                return bodyAvailable.toString().getBytes();
            }
            @Override
            public Map<String, String> getHeaders() throws AuthFailureError {
                Map<String, String> headers = new HashMap<>();
                headers.put("Content-Type","application/json");
                return headers;
            }
        };
        // API: toggle-device-state action
        params.clear();
        params.put("action", "toggle-device-state");
        params.put("device-id", deviceUUID);
        JSONObject bodyToggle = new JSONObject(params);
        toggleRequest = new StringRequest(Request.Method.POST, url,
                new Response.Listener<String>() {
                    @Override
                    public void onResponse(String response) {
                        Log.d(TAG, "Response toggle received");
                        Log.d(TAG, "-> " + response);
                        buttonState = response.endsWith("N\"");
                        updateScreen();
                    }
                },
                new Response.ErrorListener() {
                    @Override
                    public void onErrorResponse(VolleyError error) {
                        Log.d(TAG, "Response toggle error");
                        Toast.makeText(getApplication(), R.string.toast_error, Toast.LENGTH_SHORT).show();
                        updateScreenUnknown();
                        error.printStackTrace();
                    }
                }) {
            @Override
            public byte[] getBody() throws AuthFailureError {
                return bodyToggle.toString().getBytes();
            }
            @Override
            public Map<String, String> getHeaders() throws AuthFailureError {
                Map<String, String> headers = new HashMap<>();
                headers.put("Content-Type","application/json");
                return headers;
            }
        };
        // Refresh states
        handler = new Handler();
        runnable = new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "Refresh screen");
                queue.add(availableRequest);
                handler.postDelayed(this, refreshRate);
            }
        };
        handler.postDelayed(runnable, refreshRate);
        ivButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                switch (motionEvent.getAction()){
                    case MotionEvent.ACTION_DOWN:
                        Log.d(TAG, "Button pressed");
                        ivButton.setImageResource(ic_pressed);
                        tvOutput.setText(R.string.state_sending);
                        return true;
                    case MotionEvent.ACTION_UP:
                        Log.d(TAG, "Button toggle");
                        if (available){
                            queue.add(toggleRequest);
                        } else {
                            Toast.makeText(getApplication(), R.string.toast_device_unavailable, Toast.LENGTH_SHORT).show();
                            updateScreenUnknown();
                        }
                        return true;
                    default:
                        return false;
                }
            }
        });
    }

    private void updateScreen(){
        if (buttonState){
            tvOutput.setText(R.string.state_on);
            ivButton.setImageResource(ic_on);
        } else{
            tvOutput.setText(R.string.state_off);
            ivButton.setImageResource(ic_off);
        }
    }

    private void updateScreenUnknown(){
        tvOutput.setText(R.string.state_unknown);
        ivButton.setImageResource(ic_unavailable);
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        handler.removeCallbacks(runnable);
        super.onDestroy();
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