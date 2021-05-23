package com.tjdickerson.sacweather.task;

import java.io.*;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;

public class FileFetcher implements Runnable
{
    private final String mFilesDir;
    private final String mTargetUrl;

    public FetchResponse mDelegate = null;

    private String lastError;

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
        //@todo
        // This needs to handle slow/lack of connection in a better way.
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
        catch (IOException e)
        {
            return e.getMessage();
        }

        return "Download OK";
    }

}
