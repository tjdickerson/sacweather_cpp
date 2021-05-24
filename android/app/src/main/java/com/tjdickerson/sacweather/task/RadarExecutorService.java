package com.tjdickerson.sacweather.task;

import android.app.Activity;
import com.tjdickerson.sacweather.data.RadarView;

import java.util.concurrent.*;

public class RadarExecutorService
{
    private static RadarExecutorService mInstance = null;

    private final ScheduledExecutorService mExecutor;
    ScheduledFuture<?> mNewScanFuture;

    private RadarView mRadarView;

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

    public void start(Activity activity, RadarView radarView, FetchResponse onComplete)
    {
        if (mNewScanFuture != null) mNewScanFuture.cancel(true);

        String filename = "latest";
        mExecutor.execute(new FileFetcher(activity, filename, radarView.getUrl(), onComplete));

         // calculate delay here
        int delay = 1;

        mNewScanFuture = mExecutor.scheduleAtFixedRate(
                new FileFetcher(activity, filename, radarView.getUrl(), onComplete),
                delay, 5, TimeUnit.MINUTES);

    }
}
