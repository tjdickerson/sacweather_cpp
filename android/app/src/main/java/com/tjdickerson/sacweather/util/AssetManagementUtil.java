package com.tjdickerson.sacweather.util;

import android.content.Context;

import java.io.*;

public class AssetManagementUtil
{
    public static void validateAssets(Context ctx) throws IOException
    {
        String storageDir = ctx.getFilesDir().getPath() + "/shapes";
        File targetFolder = new File(storageDir);

        boolean directoryExists = targetFolder.exists();
        if (!directoryExists)
            directoryExists = targetFolder.mkdir();

        if (directoryExists)
        {
            for(String filename : ctx.getAssets().list("shapes"))
            {
                File out = new File(storageDir + "/" + filename);
                if (out.exists()) continue;

                InputStream in = ctx.getAssets().open("shapes/" + filename);
                OutputStream os = new FileOutputStream(out);

                byte[] buf = new byte[1024];
                int read;
                while ((read = in.read(buf)) != -1)
                {
                    os.write(buf, 0, read);
                }

                in.close();
                os.close();
            }
        }
    }
}
