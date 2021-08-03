package com.arturo.aguilar.lagunas.iot.device.app.ui.myDevices;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

public class MyDevicesViewModel extends ViewModel {

    private MutableLiveData<String> mText;

    public MyDevicesViewModel() {
        mText = new MutableLiveData<>();
        mText.setValue("Show my devices");
    }

    public LiveData<String> getText() {
        return mText;
    }
}