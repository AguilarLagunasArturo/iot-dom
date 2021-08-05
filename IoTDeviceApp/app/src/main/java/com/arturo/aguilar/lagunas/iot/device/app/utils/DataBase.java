package com.arturo.aguilar.lagunas.iot.device.app.utils;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

public class DataBase {
    private static final String TAG = "DEBUG DB";

    private SQLiteDatabase db;
    private Cursor cursor;
    private Context application;
    private String dbName;

    public DataBase(Context context){
        application = context;
        dbName = "iot.db";
        cursor = null;
        db = null;
    }

    public void setupDB(){
        Log.d(TAG, "========Setting up " + dbName + "========");

        // Check if db exists
        boolean dbExists = false;
        if (application.databaseList().length > 0){
            if (application.databaseList()[0].equals(dbName))
                dbExists = true;
        }

        // If db exists
        if (dbExists){
            Log.d(TAG, "db exists");
            db = application.openOrCreateDatabase(dbName, application.MODE_PRIVATE, null);
        }
        else {
            Log.d(TAG, "Brand new db");
            db = application.openOrCreateDatabase(dbName, application.MODE_PRIVATE, null);

            // Setup [Devices settings]
            db.execSQL("create table if not exists devices (id integer primary key, uuid int, name text, description text, type text, state integer);");
            db.execSQL("insert into devices (id, uuid, name, description, type, state) values (1, 0, 'Some IoT Switch', 'Device description', 'Type', 0);");
            db.execSQL("insert into devices (id, uuid, name, description, type, state) values (2, 1, 'Some IoT Switch', 'Device description', 'Type', 0);");
        }
    }

    public void logTable(String target){
        // Query
        cursor = db.rawQuery("select * from " + target, null);
        cursor.moveToFirst();

        // Table info
        Log.d(TAG, "====|Table: " + target + "|Rows: " + cursor.getCount() + "|Columns " + cursor.getColumnCount() + "|====");

        // Print rows
        for (int i = 0; i < cursor.getCount(); i ++){
            String row = "";
            for (int j = 0; j < cursor.getColumnCount(); j ++){
                if (j == cursor.getColumnCount()-1)
                    row += cursor.getColumnName(j) + " : " + cursor.getString(j) + ".";
                else
                    row += cursor.getColumnName(j) + " : " + cursor.getString(j) + ", ";
            }
            Log.d(TAG, row);
            cursor.moveToNext();
        }
    }

    public void update(String table, String values, String condition){
        String command = "update " + table + " set " + values + " where " + condition + ";";
        Log.d(TAG, "Query: " + command);
        db.execSQL(command);
        logTable(table);
    }

    public Cursor query(String query){
        return db.rawQuery(query, null);
    }
}
