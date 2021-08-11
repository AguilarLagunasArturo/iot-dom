package com.arturo.aguilar.lagunas.iot.device.app;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.webkit.ValueCallback;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.ProgressBar;

import com.arturo.aguilar.lagunas.iot.device.app.utils.DataBase;

public class AccessPointHandlerActivity extends AppCompatActivity {

    private final String TAG = "DEBUG APHandlerActivity";
    private String AP_ID;

    private String uuid;
    private String type;
    private WifiManager wifiManager;
    private WiFiReceiver wifiReceiver;
    private WebView webView;
    private DataBase db;
    private ProgressBar progressBar;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_access_point_handler);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        AP_ID = getString(R.string.ap);
        db = new DataBase(this);
        db.setupDB();

        wifiReceiver = new  AccessPointHandlerActivity.WiFiReceiver();
        wifiManager = (WifiManager) this.getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        this.registerReceiver(wifiReceiver, new IntentFilter(WifiManager.NETWORK_STATE_CHANGED_ACTION));

        // DEBUG: ADD LOADING SYMBOL
        Log.d(TAG, "Started");
        webView = (WebView) findViewById(R.id.wv_handler);
        String url =  null;
        if (getIntent().getData() != null){
            Uri data = getIntent().getData();
            String scheme = data.getScheme();
            String fullPath = data.getEncodedSchemeSpecificPart();
            url = String.format("%s:%s", scheme, fullPath);

            webView.getSettings().setJavaScriptEnabled(true);
            webView.setWebViewClient(new CustomWebViewClient(progressBar));
            webView.loadUrl(url);
        }
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        try {
            this.unregisterReceiver(wifiReceiver);
        } catch (Exception e) {
            e.printStackTrace();
        }
        super.onDestroy();
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        switch (item.getItemId()){
            case R.id.action_settings:
                Intent open_browser = new Intent(Intent.ACTION_VIEW,
                        Uri.parse(getString(R.string.github_url)));
                startActivity(open_browser);
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
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

    public class CustomWebViewClient extends WebViewClient {
        // private ProgressBar progressBar;

        public CustomWebViewClient(ProgressBar progressBar) {
            // this.progressBar=progressBar;
            // progressBar.setVisibility(View.VISIBLE);
            Log.d(TAG, "Loading ...");
        }
        @Override
        public boolean shouldOverrideUrlLoading(WebView view, String url) {
            // TODO Auto-generated method stub
            view.loadUrl(url);
            return true;
        }

        @Override
        public void onPageFinished(WebView view, String url) {
            // TODO Auto-generated method stub
            super.onPageFinished(view, url);
            Log.d(TAG, "Done ...");
            // progressBar.setVisibility(View.GONE);
        }
    }

    private class WiFiReceiver extends BroadcastReceiver {
        public void onReceive(Context c, Intent intent) {
            String action = intent.getAction();
            if (action.equals(WifiManager.NETWORK_STATE_CHANGED_ACTION)) {
                String ssid = wifiManager.getConnectionInfo().getSSID();
                Log.d(TAG, "NETWORK: " + ssid);
                if (ssid.equals(String.format("\"%s\"", AP_ID))){
                    Log.d(TAG, "CHANGED RECONNECTED TO LOGGER");
                } else if (ssid.equals("<unknown ssid>")){
                    Log.d(TAG, "CHANGED LOST CONNECTION");
                    webView.evaluateJavascript("(function(){return document.getElementsByName('uuid')[0].value})();",
                            new ValueCallback<String>() {
                                @Override
                                public void onReceiveValue(String value) {
                                    value = value.replace("\"", "");
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
                                    value = value.replace("\"", "");
                                    Log.d(TAG, value);
                                    if (value != null){
                                        type = value;
                                    }
                                }
                            });
                } else{
                    Log.d(TAG, "CHANGED RECONNECTED TO MAIN");
                    db.execCommand(
                            String.format("insert into devices (uuid, name, description, type) values ('%s', 'Some IoT Device', 'Device description', '%s');", uuid, type)
                    );
                    try {
                        AccessPointHandlerActivity.this.unregisterReceiver(wifiReceiver);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    Intent intentActivity = new Intent(AccessPointHandlerActivity.this, MainActivity.class);
                    intentActivity.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                    startActivity(intentActivity);
                }
            }
        }
    }
}