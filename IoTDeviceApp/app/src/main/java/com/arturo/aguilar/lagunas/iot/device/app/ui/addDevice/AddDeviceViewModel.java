package com.arturo.aguilar.lagunas.iot.device.app.ui.addDevice;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

public class AddDeviceViewModel extends ViewModel {

    private MutableLiveData<String> mText;

    public AddDeviceViewModel() {
        mText = new MutableLiveData<>();
        mText.setValue("Add devices");
    }

    public LiveData<String> getText() {
        return mText;
    }
}