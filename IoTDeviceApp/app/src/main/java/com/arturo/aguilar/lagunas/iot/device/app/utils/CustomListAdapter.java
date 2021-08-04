package com.arturo.aguilar.lagunas.iot.device.app.utils;

import com.arturo.aguilar.lagunas.iot.device.app.R;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class CustomListAdapter extends BaseAdapter{

    private static final String TAG = "CustomListAdapter";

    private int[] drawable_id;
    private String[] title;
    private String[] description;
    private LayoutInflater inflater;
    private int layout;
    private boolean image;

    // Custom adapter for arrays
    public CustomListAdapter(int[] i, String[] t, String[] d, LayoutInflater l, int xml){
        drawable_id = i;
        title = t;
        description = d;
        inflater = l;
        layout = xml;
        if ((i.length == t.length) && (t.length == d.length))
            Log.d(TAG, "Same sizes.");
        else
            Log.d(TAG, "Different sizes.");
        image = true;
    }
    public CustomListAdapter(String[] t, String[] d, LayoutInflater l, int xml){
        title = t;
        description = d;
        inflater = l;
        layout = xml;
        if (t.length == d.length)
            Log.d(TAG, "Same sizes.");
        else
            Log.d(TAG, "Different sizes.");
        image = false;
    }

    @Override
    public int getCount() {
        return title.length;
    }

    @Override
    public Object getItem(int position) {
        return null;
    }

    @Override
    public long getItemId(int position) {
        return 0;
    }

    @Override
    public View getView(int position, View view, ViewGroup viewGroup) {
        view = inflater.inflate(layout, null);

        if (image){
            ImageView list_image = (ImageView) view.findViewById(R.id.list_image);
            list_image.setImageResource(drawable_id[position]);
        }

        TextView text_title = (TextView) view.findViewById(R.id.text_title);
        TextView text_description = (TextView) view.findViewById(R.id.text_description);

        text_title.setText(title[position]);
        text_description.setText(description[position]);

        return view;
    }
}
