package com.tjdickerson.sacweather.task;

import android.app.Activity;
import android.app.DownloadManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.Uri;
import android.os.Environment;
import android.widget.Toast;

import java.io.File;

import static android.content.Context.DOWNLOAD_SERVICE;

public class FileDownloader
{
    private final DownloadManager mDownloadManager;
    private final Activity mActivity;

    private FetchResponse mResultDelegate;
    private long mDownloadId;

    public FileDownloader(Activity activity)
    {
        mActivity = activity;

        mDownloadManager = (DownloadManager) mActivity.getSystemService(DOWNLOAD_SERVICE);

        BroadcastReceiver bOnComplete = new BroadcastReceiver()
        {
            public void onReceive(Context context, Intent intent)
            {
                mResultDelegate.downloadFinished("Ok...");
            }
        };

        mActivity.registerReceiver(
                bOnComplete, new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE));
    }

    public void requestDownload(String sourceUrl, String filename, FetchResponse resultDelegate)
    {
        mDownloadManager.remove(mDownloadId);
        mResultDelegate = resultDelegate;

        Toast.makeText(mActivity, "Fetching latest radar data.", Toast.LENGTH_LONG).show();

        Uri source = Uri.parse(sourceUrl);
        DownloadManager.Request downloadRequest;

        downloadRequest = new DownloadManager.Request(source);
        downloadRequest.setAllowedNetworkTypes(
                DownloadManager.Request.NETWORK_WIFI |
                DownloadManager.Request.NETWORK_MOBILE);
        downloadRequest.setAllowedOverRoaming(false);
        downloadRequest.setTitle("What is this");
        downloadRequest.setDescription("What is this for");

        mDownloadId = mDownloadManager.enqueue(downloadRequest);
    }

    public String getDownloadPath()
    {
        return mDownloadManager.getUriForDownloadedFile(mDownloadId).getPath();
    }

    public void queryStatus()
    {
        Cursor c = mDownloadManager.query(new DownloadManager.Query().setFilterById(mDownloadId));

        if (c == null)
        {
            Toast.makeText(mActivity, "Unable to determine radar file.", Toast.LENGTH_LONG).show();
        }

        else
        {
            if (c.moveToFirst())
                Toast.makeText(mActivity, getStatusMessage(c), Toast.LENGTH_LONG).show();
            else
                Toast.makeText(mActivity, "Hmm", Toast.LENGTH_LONG).show();
        }
    }

    private String getStatusMessage(Cursor c)
    {
        String message = "UNKNOWN";

        switch(c.getInt(c.getColumnIndex(DownloadManager.COLUMN_STATUS)))
        {
            case DownloadManager.STATUS_FAILED:
                message = "Failed to download latest radar data.";
                break;

            case DownloadManager.STATUS_PAUSED:
                message = "Download of latest radar data paused.";
                break;

            case DownloadManager.STATUS_PENDING:
                message ="Download of latest radar data in progress...";
                break;

            case DownloadManager.STATUS_SUCCESSFUL:
                message = "Fetched latest radar data.";
                break;

            default:
                message = "Something went wrong attempting to get the latest data.";
                break;
        }

        return message;
    }

}
