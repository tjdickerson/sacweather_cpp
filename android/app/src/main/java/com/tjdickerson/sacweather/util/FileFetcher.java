package com.tjdickerson.sacweather.util;

import android.os.AsyncTask;

import java.io.*;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;

public class FileFetcher extends AsyncTask<String ,String, String>
{
    private final String mFilesDir;

    public FetchResponse mDelegate = null;

    public FileFetcher(String filesDir, FetchResponse delegate)
    {
        super();
        mFilesDir = filesDir;
        mDelegate = delegate;
    }

    @Override
    protected void onPreExecute()
    {
        super.onPreExecute();
        // make some progress ui view show
    }

    @Override
    protected String doInBackground(String... strings)
    {
        try
        {
            String srcUrl = strings[0];
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

        return null;
    }


    @Override
    protected void onPostExecute(String result)
    {
        mDelegate.downloadFinished(result);
        super.onPostExecute(result);
    }
}
