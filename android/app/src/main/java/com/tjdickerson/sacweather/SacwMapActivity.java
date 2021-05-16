package com.tjdickerson.sacweather;


import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.ContextMenu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import com.tjdickerson.sacweather.data.NexradProductInfo;
import com.tjdickerson.sacweather.data.RadarView;
import com.tjdickerson.sacweather.data.WsrInfo;
import com.tjdickerson.sacweather.task.FileFetcher;
import com.tjdickerson.sacweather.task.RadarExecutorService;
import com.tjdickerson.sacweather.view.SacwMapView;

import java.io.*;
import java.util.*;

public class SacwMapActivity extends AppCompatActivity
{
    private SacwMapView mSacwMapView;
    private RadarView mRadarView;

    private TextView scanTimeTv;
    private Handler mHandler;

    private List<WsrInfo> mWsrList;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_sacwmap);
        mSacwMapView = findViewById(R.id.surfaceView);
        mSacwMapView.init(this);

        registerForContextMenu(mSacwMapView);

        scanTimeTv = findViewById(R.id.scanTime);

        mHandler = new Handler(Looper.myLooper());

        // get site data
        mWsrList = getWsrListFromFile("data/wsrlist");
        List<NexradProductInfo> products = createProductList();

        // get thing
        mRadarView = new RadarView();

        WsrInfo selectedRda = RadarView.findWsrInfoById(mWsrList, "KPAH");
        NexradProductInfo product = RadarView.findProductByCode(products, (short) 94);
        SetRadarView(selectedRda, product);

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

    protected void SetRadarView(WsrInfo selectedRda, NexradProductInfo product)
    {
        if (selectedRda != null && product != null)
        {
            mRadarView.setWsrInfo(selectedRda);
            mRadarView.setProductInfo(product);

            // set info panel stuff?
            TextView rdaName = findViewById(R.id.rdaName);
            rdaName.setText(String.format("%s - %s", selectedRda.getId(), selectedRda.getName()));

            TextView productName = findViewById(R.id.productName);
            productName.setText(product.getDisplayName());

            // init radar stuff
            RadarExecutorService.getInstance().run(
                    new FileFetcher(
                            getFilesDir().getPath(),
                            mRadarView.getUrl(),
                            result -> startShowingRadar()));

        }
        else
        {
            // error happened
            alertDialog("Failed to find data.", "ERROR");
        }
    }

    @Override
    protected void onStart()
    {
        super.onStart();
    }


    protected void startShowingRadar()
    {
        String filepath = getFilesDir().getPath() + "/latest";
        SacwLib.sacwRadarInit(filepath, mRadarView.getProductInfo().getProductCode());

        // number of seconds after midnight GMT
        long scanTime = SacwLib.sacwScanTime();

        TimeZone tz = TimeZone.getTimeZone("UTC");
        Calendar calendar = Calendar.getInstance(tz);
        calendar.setTimeInMillis(scanTime);

        long time = calendar.getTimeInMillis();
        Date d = new Date(time);
        String what = d.toString();

        mHandler.post(() -> scanTimeTv.setText(what));

    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v,
                                    ContextMenu.ContextMenuInfo menuInfo)
    {
        super.onCreateContextMenu(menu, v, menuInfo);

        float latitude = mSacwMapView.getLastQueriedPoint().y;
        float longitude = mSacwMapView.getLastQueriedPoint().x;
        String template = "Info for %2.4f, %2.4f";
        WsrInfo wsrInfo = RadarView.findClosestRda(mWsrList, longitude, latitude);

        menu.clear();
        menu.setHeaderTitle(String.format(Locale.getDefault(), template, latitude, longitude));
        menu.add(0, v.getId(), 0, wsrInfo.getId());

        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu_location, menu);
    }

    @Override
    public boolean onContextItemSelected(MenuItem item)
    {
        String rda = item.getTitle().toString();
        WsrInfo wsrInfo = RadarView.findWsrInfoById(mWsrList, rda);

        SetRadarView(wsrInfo, mRadarView.getProductInfo());
        return true;
    }

    public void showLocationMenu(float lon, float lat)
    {
        //
/*        String template = "%2.4f, %2.4f";


        MenuItem rdaItem = findViewById(R.id.closestRda);
        rdaItem.setTitle(wsrInfo.getId());

        openContextMenu(mSacwMapView);*/
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

    //float scale = 0.00004f;
/*
    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        float x = event.getX();
        float y = event.getY();
        long firstTouchTime;

        switch(event.getAction())
        {
            case MotionEvent.ACTION_MOVE:

                float dx = (prevX - x);
                float dy = (prevY - y);

                SacwLib.sacwPanMap(dx, dy);

                break;
        }

        prevX = x;
        prevY = y;


        return true;
    }*/

    private void saveToFile(List<WsrInfo> wsrList)
    {
        // move me later
        String filename = "data/wsrlist";
        String filepath = getFilesDir().getPath() + "/" + filename;

        StringBuilder output = new StringBuilder();
        for (WsrInfo wsr : wsrList)
        {
            output.append(wsr.toFileLine());
            output.append("\n");
        }

        File file = new File(filepath);
        try (FileOutputStream fos = new FileOutputStream(file);
             BufferedOutputStream bos = new BufferedOutputStream(fos))
        {
            bos.write(output.toString().getBytes());
        } catch (IOException e)
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
            try (FileReader fr = new FileReader(file);
                 BufferedReader br = new BufferedReader(fr))
            {
                String s;
                while ((s = br.readLine()) != null)
                {
                    content.append(s).append("\n");
                }
            } catch (IOException e)
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

            if (s.length < 5) break;

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
    private float TryParseDouble(String d)
    {
        float result;

        try
        {
            result = Float.parseFloat(d);
        } catch (NumberFormatException e)
        {
            result = -1;
        }

        return result;
    }


    private void alertDialog(String message, String title)
    {
        AlertDialog.Builder dialog = new AlertDialog.Builder(this);
        dialog.setMessage(message);
        dialog.setTitle(title);

        AlertDialog alertDialog = dialog.create();
        alertDialog.show();
    }

    private List<NexradProductInfo> createProductList()
    {
        List<NexradProductInfo> npi = new ArrayList<>();

        npi.add(new NexradProductInfo((short) 94, "Reflectivity"));
        npi.add(new NexradProductInfo((short) 99, "Velocity"));

        return npi;
    }
}
