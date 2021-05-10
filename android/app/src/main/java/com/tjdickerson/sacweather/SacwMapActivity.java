package com.tjdickerson.sacweather;


import android.os.Bundle;
import android.view.MotionEvent;
import androidx.appcompat.app.AppCompatActivity;
import com.tjdickerson.sacweather.util.FileFetcher;
import com.tjdickerson.sacweather.view.SacwMapView;

public class SacwMapActivity extends AppCompatActivity
{
    private SacwMapView mSacwMapView;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_sacwmap);
        mSacwMapView = findViewById(R.id.surfaceView);

        String nexradFile =
                "https://tgftp.nws.noaa.gov/SL.us008001/DF.of/DC.radar/DS.p94r0/SI.kmxx/sn.last";

        // init radar stuff
        new FileFetcher(
                getFilesDir().getPath(),
                result -> startShowingRadar()).execute(nexradFile);


        // get site data
        //List<WsrInfo> wsrList = getWsrListFromFile("wsrlist");

        /*new JsonFetcher(result -> {
            try
            {
               List<WsrInfo> wsrList = RadarSiteGetter.getWsrListFromJson(result);
               if (!wsrList.isEmpty())
                   saveToFile(wsrList);
            }
            catch (JSONException e)
            {
                e.printStackTrace();
            }
        })
        .execute("https://api.weather.gov/radar/stations?stationType=WSR-88D");*/
    }

    @Override
    protected void onStart()
    {
        super.onStart();
    }

    protected void startShowingRadar()
    {
        String filepath = getFilesDir().getPath() + "/latest";
        SacwLib.sacwRadarInit(filepath);
    }

    @Override
    protected void onPause()
    {
        super.onPause();
        mSacwMapView.onPause();
    }

    @Override
    protected void onResume()
    {
        super.onResume();
        mSacwMapView.onResume();
    }

    // @todo
    // move me
    float prevX = 0.0f, prevY = 0.0f;
    float scale = 0.00004f;

    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        float x = event.getX();
        float y = event.getY();

        switch(event.getAction())
        {
            case MotionEvent.ACTION_MOVE:

                float w = getWindow().getAttributes().width;
                float h = getWindow().getAttributes().height;

                float dx = (prevX - x) * -scale;
                float dy = (prevY - y) * scale;

                SacwLib.sacwPanMap(dx, dy);

                break;
        }

        prevX = x;
        prevY = y;


        return true;
    }

/*
    private void saveToFile(List<WsrInfo> wsrList)
    {
        // move me later
        String filename = "wsrlist";
        String filepath = getFilesDir().getPath() + "/" + filename;

        StringBuilder output = new StringBuilder();
        for(WsrInfo wsr : wsrList)
        {
            output.append(wsr.toFileLine());
            output.append("\n");
        }

        File file = new File(filepath);
        try(FileOutputStream fos = new FileOutputStream(file);
            BufferedOutputStream bos = new BufferedOutputStream(fos))
        {
            bos.write(output.toString().getBytes());
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }

    private String readWsrFile(String filename)
    {
        String filepath = getFilesDir().getPath() + "/" + filename;
        File file = new File(filepath);

        StringBuilder content = new StringBuilder();

        if (file.exists())
        {
            try(FileReader fr = new FileReader(file);
                BufferedReader br = new BufferedReader(fr))
            {
                String s;
                while ((s = br.readLine()) != null)
                {
                    content.append(s);
                }
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }
        }

        return content.toString();
    }

    private List<WsrInfo> getWsrListFromFile(String filename)
    {
        List<WsrInfo> wsrList = new ArrayList<>();
        String fileContent = readWsrFile(filename);

        String[] lines = fileContent.split("\n");
        for (String line : lines)
        {
            WsrInfo wsr = new WsrInfo();
            String[] s = line.split("\\|");

            wsr.setId(s[WsrInfo.ID_INDEX]);
            wsr.setName(s[WsrInfo.NAME_INDEX]);
            wsr.setLongitude(TryParseDouble(s[WsrInfo.LONGITUDE_INDEX]));
            wsr.setLatitude(TryParseDouble(s[WsrInfo.LATITUDE_INDEX]));
            wsr.setRefCalibration(TryParseDouble(s[WsrInfo.CALIBRATION_INDEX]));

            wsrList.add(wsr);
        }

        return wsrList;
    }

    // @todo
    // need a utils class i guess
    private double TryParseDouble(String d)
    {
        double result;

        try {
            result = Double.parseDouble(d);
        }
        catch (NumberFormatException e)
        {
            result = -1;
        }

        return result;
    }    */
}
