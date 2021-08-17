package com.arturo.aguilar.lagunas.iot.device.app.ui.myDevices;

import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.fragment.app.Fragment;

import com.arturo.aguilar.lagunas.iot.device.app.DeviceSwitchActivity;
import com.arturo.aguilar.lagunas.iot.device.app.MainActivity;
import com.arturo.aguilar.lagunas.iot.device.app.R;
import com.arturo.aguilar.lagunas.iot.device.app.databinding.FragmentMyDevicesBinding;
import com.arturo.aguilar.lagunas.iot.device.app.utils.CustomListAdapter;
import com.arturo.aguilar.lagunas.iot.device.app.utils.DataBase;
import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.google.android.material.snackbar.Snackbar;

import java.util.ArrayList;

public class MyDevicesFragment extends Fragment {

    private FragmentMyDevicesBinding binding;

    private ListView lvDevices;

    private CustomListAdapter customDeviceList;
    private ArrayList<String> titles = new ArrayList<String>();
    private ArrayList<String> descriptions = new ArrayList<String>();
    private ArrayList<Integer> icons = new ArrayList<Integer>();
    private Cursor cursor;

    private String TAG = "DEBUG MyDevices";

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {

        binding = FragmentMyDevicesBinding.inflate(inflater, container, false);
        View root = binding.getRoot();

        FloatingActionButton fab = root.findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                AlertDialog.Builder alert = new AlertDialog.Builder(getActivity());

                LayoutInflater inflater = getActivity().getLayoutInflater();
                View view1 = inflater.inflate(R.layout.alert_add_layout, null);
                alert.setView(view1);
                final EditText uuidText = view1.findViewById(R.id.uuid);
                final EditText typeText = view1.findViewById(R.id.type);

                typeText.setFocusable(false);
                typeText.setFocusableInTouchMode(false);
                typeText.setTextIsSelectable(false);

                alert.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        Log.d(TAG, "OK");
                        String uuid = uuidText.getText().toString();
                        String type = typeText.getHint().toString();

                        if (!uuid.isEmpty() && uuid.startsWith("iot-skill")){
                            MainActivity.db.execCommand(
                                    String.format("insert into devices (uuid, name, description, type) values ('%s', 'Some IoT Device', 'Device description', '%s');", uuid, type)
                            );
                            updateList();
                            Toast.makeText(getContext(), uuid, Toast.LENGTH_SHORT).show();
                        }else{
                            Toast.makeText(getContext(), R.string.toast_error, Toast.LENGTH_SHORT).show();
                        }
                    }
                });

                alert.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        Log.d(TAG, "CANCEL");
                    }
                });

                alert.show();
            }
        });

        lvDevices = root.findViewById(R.id.lv_devices);
        customDeviceList = new CustomListAdapter(icons, titles, descriptions, getLayoutInflater(), R.layout.list_devices);
        lvDevices.setAdapter(customDeviceList);
        updateList();

        lvDevices.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                Toast.makeText(getActivity(), titles.get(i), Toast.LENGTH_SHORT).show();
                Intent intent = new Intent(getActivity(), DeviceSwitchActivity.class);
                intent.putExtra("uuid", titles.get(i));
                startActivity(intent);
            }
        });

        return root;
    }

    @Override
    public void onDestroyView() {
        icons.clear();
        titles.clear();
        descriptions.clear();
        customDeviceList.notifyDataSetChanged();
        super.onDestroyView();
        binding = null;
    }

    private void updateList(){
        titles.clear();
        descriptions.clear();
        icons.clear();

        cursor = MainActivity.db.query("select * from devices");
        cursor.moveToFirst();

        for (int i = 0; i < cursor.getCount(); i++){
            String uuid = cursor.getString(1);
            String type = cursor.getString(4);

            Log.d(TAG, String.format("UUID: %s", uuid));
            Log.d(TAG, String.format("TYPE: %s", type));

            titles.add(uuid);
            descriptions.add(type);

            switch (type) {
                case "SWITCH":
                    icons.add(R.drawable.ic_switch_icon);
                    break;
                default:
                    icons.add(R.drawable.ic_android);
                    break;
            }

            cursor.moveToNext();
        }

        customDeviceList.notifyDataSetChanged();
    }
}