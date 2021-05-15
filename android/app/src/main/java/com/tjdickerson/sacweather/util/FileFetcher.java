package com.tjdickerson.sacweather.util;

import android.os.AsyncTask;

import java.io.*;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.util.concurrent.Callable;
import java.util.concurrent.FutureTask;

public class FileFetcher implements Runnable
{
    private final String mFilesDir;
    private final String mTargetUrl;

    public FetchResponse mDelegate = null;

    public FileFetcher(String filesDir, String targetUrl, FetchResponse delegate)
    {
        mFilesDir = filesDir;
        mTargetUrl = targetUrl;
        mDelegate = delegate;
    }

    @Override
    public void run()
    {
        String result = downloadStart();
        mDelegate.downloadFinished(result);
    }

    private String downloadStart()
    {
        try
        {
            String srcUrl = mTargetUrl;
            URL url = new URL(srcUrl);
            URLConnection connection = url.openConnection();
            connection.connect();

            int byteLength = connection.getContentLength();

            String targetFilename = "latest";
            String targetFilepath = mFilesDir + "/" + targetFilename;

            int BUFFER_SIZE = 8192;
            InputStream is = new BufferedInputStream(url.openStream(), BUFFER_SIZE);
            OutputStream os = new FileOutputStream(targetFilepath);

            byte raw[] = new byte[1024];
            int read;
            long totalRead = 0;

            while((read = is.read(raw)) != -1)
            {
                totalRead += read;

                //
                // write out to progress bar or something
                //

                os.write(raw, 0, read);
            }

            os.flush();
            os.close();
            is.close();
        }
        catch (MalformedURLException e)
        {
            e.printStackTrace();
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }

        return "";
    }

}
