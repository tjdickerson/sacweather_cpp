package com.tjdickerson.sacweather.task;

import java.util.Timer;
import java.util.concurrent.*;

public class RadarExecutorService
{
    private static RadarExecutorService mInstance = null;

    private final ScheduledExecutorService mExecutor;

    private RadarExecutorService()
    {
        mExecutor = Executors.newScheduledThreadPool(1);
    }

    public static RadarExecutorService getInstance()
    {
        if (mInstance == null)
        {
            mInstance = new RadarExecutorService();
        }

        return mInstance;
    }

    public void run(Runnable runnable)
    {
        mExecutor.scheduleAtFixedRate(runnable, 0, 5, TimeUnit.MINUTES);
    }
}
