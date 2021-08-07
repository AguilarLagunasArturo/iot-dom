package com.arturo.aguilar.lagunas.iot.device.app;

import androidx.appcompat.app.AppCompatActivity;

import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.Menu;
import android.webkit.ValueCallback;
import android.webkit.WebView;

public class AccessPointHandlerActivity extends AppCompatActivity {

    private final String TAG = "DEBUG APHandlerActivity";
    private final int refreshRate = 1000;
    private String uuid;
    private String type;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_access_point_handler);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        // DEBUG: ADD LOADING SYMBOL
        Log.d(TAG, "Started");
        WebView webView = (WebView) findViewById(R.id.wv_handler);
        String url =  null;
        if (getIntent().getData() != null){
            Uri data = getIntent().getData();
            String scheme = data.getScheme();
            String fullPath = data.getEncodedSchemeSpecificPart();
            url = String.format("%s:%s", scheme, fullPath);

            webView.getSettings().setJavaScriptEnabled(true);
            webView.loadUrl(url);
        }

        // Refresh data
        Handler handler = new Handler();
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "Refresh data");
                // DEBUG: CHECK IF CONNECTION CHANGED -> REDIRECT
                webView.evaluateJavascript("(function(){return document.getElementsByName('uuid')[0].value})();",
                        new ValueCallback<String>() {
                            @Override
                            public void onReceiveValue(String value) {
                                Log.d(TAG, value);
                                if (value != null){
                                    uuid = value;
                                }
                            }
                        });
                webView.evaluateJavascript("(function(){return document.getElementsByName('type')[0].value})();",
                        new ValueCallback<String>() {
                            @Override
                            public void onReceiveValue(String value) {
                                Log.d(TAG, value);
                                if (value != null){
                                    type = value;
                                }
                            }
                        });
                handler.postDelayed(this, refreshRate);
            }
        };
        handler.postDelayed(runnable, refreshRate);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.main, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onSupportNavigateUp() {
        onBackPressed();
        return super.onSupportNavigateUp();
    }
}