package com.arturo.aguilar.lagunas.iot.device.app.ui.myDevices;

import android.content.Intent;
import android.database.Cursor;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;

import com.arturo.aguilar.lagunas.iot.device.app.DeviceSwitchActivity;
import com.arturo.aguilar.lagunas.iot.device.app.MainActivity;
import com.arturo.aguilar.lagunas.iot.device.app.R;
import com.arturo.aguilar.lagunas.iot.device.app.databinding.FragmentMyDevicesBinding;
import com.arturo.aguilar.lagunas.iot.device.app.utils.CustomListAdapter;

import java.util.ArrayList;

public class MyDevicesFragment extends Fragment {

    private FragmentMyDevicesBinding binding;

    private ListView lvDevices;

    private CustomListAdapter customDeviceList;
    private ArrayList<String> titles = new ArrayList<String>();
    private ArrayList<String> descriptions = new ArrayList<String>();
    private Cursor cursor;

    private String TAG = "DEBUG MyDevices";

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {

        binding = FragmentMyDevicesBinding.inflate(inflater, container, false);

        View root = binding.getRoot();
        lvDevices = root.findViewById(R.id.lv_devices);

        cursor = MainActivity.db.query("select * from devices");
        cursor.moveToFirst();

        for (int i = 0; i < cursor.getCount(); i++){
            String uuid = cursor.getString(1);
            String name = cursor.getString(2);
            String description = cursor.getString(3);

            Log.d(TAG, String.format("UUID: %s", uuid));
            Log.d(TAG, String.format("NAME: %s", name));
            Log.d(TAG, String.format("DESC: %s", description));

            titles.add(String.format("%s : %s", uuid, name));
            descriptions.add(description);

            cursor.moveToNext();
        }

        customDeviceList = new CustomListAdapter(titles, descriptions, getLayoutInflater(), R.layout.list_devices);
        lvDevices.setAdapter(customDeviceList);

        lvDevices.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                Toast.makeText(getActivity(), titles.get(i), Toast.LENGTH_SHORT).show();
                Intent intent = new Intent(getActivity(), DeviceSwitchActivity.class);
                intent.putExtra("Device", titles.get(i));
                startActivity(intent);
            }
        });

        return root;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
    }
}