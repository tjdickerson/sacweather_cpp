package com.tjdickerson.sacweather.util;

import android.app.Activity;
import android.content.Context;
import androidx.annotation.NonNull;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class CrashHelper implements Thread.UncaughtExceptionHandler
{
    private Activity mActivity;
    private Thread.UncaughtExceptionHandler mUeh;

    public CrashHelper(Activity activity)
    {
        mUeh = Thread.getDefaultUncaughtExceptionHandler();
        mActivity = activity;
    }

    @Override
    public void uncaughtException(@NonNull Thread t, @NonNull Throwable e)
    {
        String error = e.toString();
        try {
            FileOutputStream fos = mActivity.openFileOutput("sac_err.txt", Context.MODE_PRIVATE);
            fos.write(error.getBytes());
            fos.close();
        } catch (IOException exception)
        {
            //
        }

        mUeh.uncaughtException(t, e);
    }
}
