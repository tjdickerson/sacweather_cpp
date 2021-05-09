package com.tjdickerson.sacweather;

import android.content.Intent;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;
import com.tjdickerson.sacweather.util.AssetManagementUtil;

import java.io.IOException;

public class MainActivity extends AppCompatActivity
{

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
    }

    @Override
    protected void onStart()
    {
        super.onStart();


        try
        {
            AssetManagementUtil.validateAssets(getBaseContext());

            Intent intent = new Intent(this, SacwMapActivity.class);
            startActivity(intent);
        } catch (IOException e)
        {
            e.printStackTrace();
        }


    }
}