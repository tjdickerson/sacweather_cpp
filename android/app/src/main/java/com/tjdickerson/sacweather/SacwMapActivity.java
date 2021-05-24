package com.tjdickerson.sacweather;


import android.Manifest;
import android.app.AlertDialog;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.view.*;
import android.widget.PopupMenu;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import com.tjdickerson.sacweather.data.NexradProductInfo;
import com.tjdickerson.sacweather.data.RadarView;
import com.tjdickerson.sacweather.data.WsrInfo;
import com.tjdickerson.sacweather.task.FileDownloader;
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
    private TextView mProductNameTv;
    private Handler mHandler;

    private List<WsrInfo> mWsrList;
    private List<NexradProductInfo> mProducts;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        //
/*        if (checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) !=
                PackageManager.PERMISSION_GRANTED)
        {
            requestPermissions(new String[] { Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
        }*/

        setContentView(R.layout.activity_sacwmap);
        mSacwMapView = findViewById(R.id.surfaceView);
        mSacwMapView.init(this);

        registerForContextMenu(mSacwMapView);

        scanTimeTv = findViewById(R.id.scanTime);
        mProductNameTv = findViewById(R.id.productName);

        mHandler = new Handler(Looper.myLooper());

        // get site data
        mWsrList = getWsrListFromFile("data/wsrlist");
        mProducts = createProductList();

        // get thing
        mRadarView = new RadarView();

        WsrInfo selectedRda = RadarView.findWsrInfoById(mWsrList, "KTLX");
        NexradProductInfo product = RadarView.findProductByCode(mProducts, (short) 94, (short) 0);
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

    public void setProgressVisible(boolean isVisible)
    {
        View view = findViewById(R.id.fetchingScanProgress);
        view.setVisibility(isVisible ? View.VISIBLE : View.INVISIBLE);
    }

    protected void SetRadarView(WsrInfo selectedRda, NexradProductInfo product)
    {
        if (selectedRda != null && product != null)
        {
            setProgressVisible(true);
            mRadarView.setWsrInfo(selectedRda);
            mRadarView.setProductInfo(product);

            // set info panel stuff?
            TextView rdaName = findViewById(R.id.rdaName);
            rdaName.setText(String.format("%s - %s", selectedRda.getId(), selectedRda.getName()));

            TextView productName = findViewById(R.id.productName);
            productName.setText(product.getDisplayName());

            // init radar stuff
            /*RadarExecutorService.getInstance().run(
                    new FileFetcher(
                            getFilesDir().getPath(),
                            mRadarView.getUrl(),
                            this::startShowingRadar));*/

           /* FileDownloader fd = new FileDownloader(this);
            fd.requestDownload(mRadarView.getUrl(), "latest", this::startShowingRadar);*/

            RadarExecutorService.getInstance().start(this, mRadarView, this::startShowingRadar);
        }
        else
        {
            // error happened
        }
    }

    @Override
    protected void onStart()
    {
        super.onStart();
    }


    protected void startShowingRadar(String result)
    {
        String dir = getFilesDir().getPath();
        String filepath = dir + "/latest";

        String dateString = "";

        boolean radarStatus =
                SacwLib.sacwRadarInit(filepath, mRadarView.getProductInfo().getProductCode());

        if (!radarStatus)
        {
            Toast.makeText(this, "Error rendering radar data.", Toast.LENGTH_LONG).show();
        }

        else
        {
            // number of seconds after midnight GMT
            long scanTime = SacwLib.sacwScanTime();

            TimeZone tz = TimeZone.getTimeZone("UTC");
            Calendar calendar = Calendar.getInstance(tz);
            calendar.setTimeInMillis(scanTime);

            long time = calendar.getTimeInMillis();
            Date d = new Date(time);
            dateString = d.toString();
        }

        String finalDateString = dateString;
        mHandler.post(() ->
        {
            scanTimeTv.setText(finalDateString);
            mProductNameTv.setText(mRadarView.getProductInfo().getFullName());
            setProgressVisible(false);
        });
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
        // @todo
        // Use itemId...
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

    private List<NexradProductInfo> createProductList()
    {
        List<NexradProductInfo> npi = new ArrayList<>();

        npi.add(new NexradProductInfo((short) 99, (short) 0, "Velocity", "p99v0"));
        npi.add(new NexradProductInfo((short) 99, (short) 1, "Velocity", "p99v1"));
        npi.add(new NexradProductInfo((short) 99, (short) 2, "Velocity", "p99v2"));
        npi.add(new NexradProductInfo((short) 99, (short) 3, "Velocity", "p99v3"));

        npi.add(new NexradProductInfo((short) 94, (short) 0, "Reflectivity", "p94r0"));
        npi.add(new NexradProductInfo((short) 94, (short) 1, "Reflectivity", "p94r1"));
        npi.add(new NexradProductInfo((short) 94, (short) 2, "Reflectivity", "p94r2"));
        npi.add(new NexradProductInfo((short) 94, (short) 3, "Reflectivity", "p94r3"));

        npi.add(new NexradProductInfo((short) 37, (short) 0, "Composite", "p37cr"));


        return npi;
    }

    public void showQuickMenu(View view)
    {
        PopupMenu popup = new PopupMenu(this, view);
        popup.inflate(R.menu.menu_quick);

        MenuItem products = popup.getMenu().getItem(0);
        Menu productList = products.getSubMenu();

        mProducts.forEach(p -> {
            MenuItem newItem = productList.add(0, p.getGuid(), 0, p.getFullName());
            newItem.setOnMenuItemClickListener(item -> {
                SetRadarView(
                        mRadarView.getWsrInfo(),
                        RadarView.findProductByGuid(mProducts, p.getGuid()));
                return true;
            });
        });

        popup.show();
    }

}
