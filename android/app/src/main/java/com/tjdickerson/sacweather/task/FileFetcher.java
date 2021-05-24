package com.tjdickerson.sacweather.task;

import android.app.Activity;
import android.widget.Toast;

import java.io.*;
import java.net.URL;
import java.net.URLConnection;

public class FileFetcher implements Runnable
{
    private final String mFilesDir;
    private final String mFilename;
    private final String mTargetUrl;

    public FetchResponse mDelegate = null;

    private final Activity mActivity;

    private int mContentLength;
    private int mTotalRead;
    private URL mUrl;

    public FileFetcher(
            Activity activity,
            String filename,
            String targetUrl,
            FetchResponse delegate)
    {
        mActivity = activity;
        mFilesDir = mActivity.getFilesDir().getPath();
        mFilename = filename;
        mTargetUrl = targetUrl;
        mDelegate = delegate;
    }

    @Override
    public void run()
    {
        if (establishConnection())
        {
            String result = downloadStart();
            mDelegate.downloadFinished(result);
        }
    }

    public float getPercentComplete()
    {
        return mTotalRead / (float) mContentLength;
    }

    private boolean establishConnection()
    {
        boolean success = false;
        URLConnection connection;

        try
        {
            mUrl = new URL(mTargetUrl);
            connection = mUrl.openConnection();
            connection.connect();

            mContentLength = connection.getContentLength();
            success = true;
        } catch (IOException malformedURLException)
        {
            Toast.makeText(
                    mActivity,
                    "Failed to connect to radar service.", Toast.LENGTH_LONG
            ).show();
        }

        return success;
    }

    private String downloadStart()
    {
        String targetFilepath = mFilesDir + "/" + mFilename;

        try
        {
            int BUFFER_SIZE = 8192;
            InputStream is = new BufferedInputStream(mUrl.openStream(), BUFFER_SIZE);
            OutputStream os = new FileOutputStream(targetFilepath);

            byte raw[] = new byte[1024];
            int read;
            mTotalRead = 0;

            while ((read = is.read(raw)) != -1)
            {
                mTotalRead += read;
                os.write(raw, 0, read);
            }

            os.flush();
            os.close();
            is.close();
        }
        catch (IOException e)
        {
            return e.getMessage();
        }

        return "Download OK";
    }

}
