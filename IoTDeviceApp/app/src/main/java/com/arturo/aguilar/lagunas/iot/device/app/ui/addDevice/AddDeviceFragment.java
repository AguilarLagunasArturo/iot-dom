package com.arturo.aguilar.lagunas.iot.device.app.ui.addDevice;

import android.Manifest;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.fragment.app.Fragment;

import com.arturo.aguilar.lagunas.iot.device.app.R;
import com.arturo.aguilar.lagunas.iot.device.app.databinding.FragmentAddDeviceBinding;
import com.arturo.aguilar.lagunas.iot.device.app.utils.CustomListAdapter;

import java.util.ArrayList;
import java.util.List;

public class AddDeviceFragment extends Fragment {

    ListView lvWiFi;
    Button buttonScan;
    private static List<ScanResult> wifiList;
    private static WifiManager wifiManager;
    private static WiFiReceiver wifiReceiver;

    private ArrayList<String> wifiItemList = new ArrayList<String>();
    private ArrayAdapter<String> adapter;
    private CustomListAdapter wifiListAdapter;

    private FragmentAddDeviceBinding binding;
    private String TAG = "ADD DEVICE";

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        Log.println(Log.DEBUG, TAG, "INIT FRAGMENT ADD DEVICE");

        binding = FragmentAddDeviceBinding.inflate(inflater, container, false);
        View root = binding.getRoot();

        // WiFi Scan
        buttonScan = root.findViewById(R.id.button_scan);
        lvWiFi = root.findViewById(R.id.lv);

        // adapter = new ArrayAdapter<String>(getActivity(), android.R.layout.simple_list_item_1, wifiItemList);
        // lvWiFi.setAdapter(adapter);

        wifiManager = (WifiManager) getActivity().getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        wifiReceiver = new WiFiReceiver();

        scanWiFi();

        buttonScan.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.println(Log.DEBUG, TAG, "BUTTON CLICKED");
                scanWiFi();
            }
        });

        lvWiFi.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                Toast.makeText(getActivity(), wifiList.get(i).SSID, Toast.LENGTH_SHORT).show();
                connectToWifi(wifiList.get(i).SSID);
            }
        });

        // return View element
        return root;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        Log.println(Log.DEBUG, TAG, "onDestroy ");

        try {
            getActivity().unregisterReceiver(wifiReceiver);
        } catch (Exception e) {
            e.printStackTrace();
        }

        binding = null;
    }

    private void scanWiFi() {
        // wifiItemList.clear();
        // adapter.notifyDataSetChanged();
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
        conf.SSID = String.format("\"%s\"", networkSSID);
        // conf.preSharedKey = "\""+ networkPass +"\"";
        conf.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);

        WifiManager wifiManager = (WifiManager) getActivity().getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        wifiManager.addNetwork(conf);

        if (ActivityCompat.checkSelfPermission(getActivity(), Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            // TODO: Consider calling
            //    ActivityCompat#requestPermissions
            // here to request the missing permissions, and then overriding
            //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
            //                                          int[] grantResults)
            // to handle the case where the user grants the permission. See the documentation
            // for ActivityCompat#requestPermissions for more details.
            Log.println(Log.DEBUG, TAG, "Missing ACCESS_FINE_LOCATION");
            return;
        }
        List<WifiConfiguration> wifiList = wifiManager.getConfiguredNetworks();
        for( WifiConfiguration i : wifiList ) {
            if(i.SSID != null && i.SSID.equals("\"" + networkSSID + "\"")) {
                wifiManager.disconnect();
                wifiManager.enableNetwork(i.networkId, true);
                wifiManager.reconnect();
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
                    int totalWifiDevices = wifiList.size();
                    try {
                        String[] ssid = new String[totalWifiDevices];
                        String[] bssid = new String[totalWifiDevices];
                        for (int i = 0; i < totalWifiDevices; i++){
                            Log.println(Log.DEBUG, TAG, wifiList.get(i).SSID);
                            ssid[i] = wifiList.get(i).SSID;
                            bssid[i] = wifiList.get(i).BSSID;
                            // wifiItemList.add(wifiList.get(i).SSID + "\n" + wifiList.get(i).BSSID + "\n" + wifiList.get(i).capabilities);
                            // adapter.notifyDataSetChanged();
                        }
                        CustomListAdapter customWiFiList = new CustomListAdapter(ssid, bssid, getLayoutInflater(), R.layout.list_wifi);
                        lvWiFi.setAdapter(customWiFiList);

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