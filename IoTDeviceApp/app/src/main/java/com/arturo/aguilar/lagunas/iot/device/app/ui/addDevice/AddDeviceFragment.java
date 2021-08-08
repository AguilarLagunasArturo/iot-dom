package com.arturo.aguilar.lagunas.iot.device.app.ui.addDevice;

import android.annotation.SuppressLint;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Toast;
import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;

import com.arturo.aguilar.lagunas.iot.device.app.MainActivity;
import com.arturo.aguilar.lagunas.iot.device.app.R;
import com.arturo.aguilar.lagunas.iot.device.app.databinding.FragmentAddDeviceBinding;
import com.arturo.aguilar.lagunas.iot.device.app.utils.CustomListAdapter;

import java.util.ArrayList;
import java.util.List;

public class AddDeviceFragment extends Fragment {

    private ListView lvWiFi;
    private Button buttonScan;

    private static List<ScanResult> wifiList;
    private static WifiManager wifiManager;
    private static WiFiReceiver wifiReceiver;

    private CustomListAdapter customWiFiList;
    private ArrayList<String> ssids = new ArrayList<String>();
    private ArrayList<String> bssids = new ArrayList<String>();

    private FragmentAddDeviceBinding binding;
    private final String TAG = "DEBUG ADD DEVICE";
    private final String AP_ID = "SS2021 - LOGGER";

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        Log.d(TAG, "INIT FRAGMENT ADD DEVICE");

        binding = FragmentAddDeviceBinding.inflate(inflater, container, false);
        View root = binding.getRoot();

        // WiFi Scan
        buttonScan = root.findViewById(R.id.button_scan);
        lvWiFi = root.findViewById(R.id.lv_wifi);

        customWiFiList = new CustomListAdapter(ssids, bssids, getLayoutInflater(), R.layout.list_wifi);
        lvWiFi.setAdapter(customWiFiList);

        wifiManager = (WifiManager) getActivity().getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        wifiReceiver = new WiFiReceiver();

        scanWiFi();

        buttonScan.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.d(TAG, "BUTTON CLICKED");
                scanWiFi();
            }
        });

        lvWiFi.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                if (wifiList.get(i).SSID.equals(AP_ID)){
                    Toast.makeText(getActivity(), String.format("%s : %s", R.string.toast_waiting_for_device, wifiList.get(i).SSID), Toast.LENGTH_LONG).show();
                    connectToWifi(wifiList.get(i).SSID);
                    MainActivity.navController.navigate(R.id.nav_my_devices);
                }else{
                    Toast.makeText(getActivity(), R.string.toast_not_a_device, Toast.LENGTH_SHORT).show();
                }
            }
        });

        // return View element
        return root;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        Log.d(TAG, "onDestroy");

        try {
            getActivity().unregisterReceiver(wifiReceiver);
        } catch (Exception e) {
            e.printStackTrace();
        }

        binding = null;
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
    }

    private void scanWiFi() {
        ssids.clear(); bssids.clear();
        customWiFiList.notifyDataSetChanged();
        Toast.makeText(getActivity(), R.string.act_scanning, Toast.LENGTH_SHORT).show();
        if (!wifiManager.isWifiEnabled()) {
            if (wifiManager.setWifiEnabled(true)) {
                Toast.makeText(getActivity(), R.string.wifi_enabled, Toast.LENGTH_SHORT).show();
                getActivity().registerReceiver(wifiReceiver, new IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION));
                wifiManager.startScan();
            } else {
                Toast.makeText(getActivity(), R.string.wifi_enable_error, Toast.LENGTH_SHORT).show();
            }
        } else if (wifiManager.isWifiEnabled()) {
            getActivity().registerReceiver(wifiReceiver, new IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION));
            wifiManager.startScan();
        }
    }

    private void connectToWifi(final String networkSSID) {
        if (!wifiManager.isWifiEnabled()) {
            wifiManager.setWifiEnabled(true);
        }
        WifiConfiguration conf = new WifiConfiguration();
        conf.SSID = String.format("\"%s\"", networkSSID); // conf.preSharedKey = "\""+ networkPass +"\"";
        conf.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);

        WifiManager wifiManager = (WifiManager) getActivity().getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        wifiManager.addNetwork(conf);

        @SuppressLint("MissingPermission") List<WifiConfiguration> wifiList = wifiManager.getConfiguredNetworks();
        for( WifiConfiguration i : wifiList ) {
            if(i.SSID != null && i.SSID.equals("\"" + networkSSID + "\"")) {
                Log.d(TAG, "ATTEMPT CONNECT START");
                wifiManager.disconnect();
                wifiManager.enableNetwork(i.networkId, true);
                wifiManager.reconnect();
                Log.d(TAG, "ATTEMPT CONNECT END");
                break;
            }
        }
    }

    class WiFiReceiver extends BroadcastReceiver {
        public void onReceive(Context c, Intent intent) {
            String action = intent.getAction();
            if (action.equals(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION)) {
                wifiList = wifiManager.getScanResults();
                if (wifiList.size() > 0) {
                    try {
                        ssids.clear(); bssids.clear();
                        for (ScanResult w : wifiList) {
                            Log.d(TAG, w.SSID);
                            ssids.add(w.SSID);
                            bssids.add(w.BSSID);
                            customWiFiList.notifyDataSetChanged();
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                } else {
                    Toast.makeText(getActivity(), R.string.wifi_not_found, Toast.LENGTH_SHORT).show();
                }
            }
        }
    }

}