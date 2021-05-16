package com.tjdickerson.sacweather.task;

import java.util.Timer;
import java.util.concurrent.*;

public class RadarExecutorService
{
    private static RadarExecutorService mInstance = null;

    private final ScheduledExecutorService mExecutor;
    ScheduledFuture<?> mScheduledFuture;

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
        if (mScheduledFuture != null) mScheduledFuture.cancel(true);

        // run now, then schedule after a delay
        mExecutor.execute(runnable);

        // calculate delay here
        int delay = 1;

        mScheduledFuture = mExecutor.scheduleAtFixedRate(runnable, delay, 5, TimeUnit.MINUTES);
    }
}
