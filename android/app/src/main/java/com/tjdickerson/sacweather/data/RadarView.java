package com.tjdickerson.sacweather.data;

import android.graphics.Point;
import android.graphics.PointF;
import android.location.Location;

import java.util.List;
import java.util.Locale;

public class RadarView
{
    private final String URL_TEMPLATE =
            "https://tgftp.nws.noaa.gov/SL.us008001/DF.of/DC.radar/DS.%s/SI.%s/sn.last";

    NexradProductInfo productInfo;
    WsrInfo wsrInfo;

    public static WsrInfo findWsrInfoById(List<WsrInfo> siteList, String id)
    {
        return siteList.stream()
                .filter(info -> info.getId().equalsIgnoreCase(id))
                .findFirst()
                .orElse(null);
    }

    public static NexradProductInfo findProductByCode(
            List<NexradProductInfo> productList,
            short productCode,
            short angle)
    {
        return productList.stream()
                .filter(p ->
                    p.getProductCode() == productCode && p.getAngle() == angle
                )
                .findFirst()
                .orElse(null);
    }

    public static NexradProductInfo findProductByGuid(
            List<NexradProductInfo> productList,
            int guid)
    {
        // @todo
        // probably return a default product instead of null...
        return productList.stream().filter(p -> p.getGuid() == guid).findFirst().orElse(null);
    }


    public static WsrInfo findClosestRda(List<WsrInfo> siteList, float lon, float lat)
    {
        WsrInfo closestRda = null;
        PointF touchPoint = new PointF(lon, lat);
        PointF sitePoint;
        double distance;
        double lowestDistance = Double.MAX_VALUE;

        for (WsrInfo wsrInfo : siteList)
        {
            sitePoint = new PointF(wsrInfo.getLongitude(), wsrInfo.getLatitude());

            // @todo
            // This calculation doesn't work for polar coordinates. Need to rewrite this using
            // sperical trig, perhaps a function in the C lib because this might be useful
            // in several places
            /*distance = Math.abs(
                    Math.sqrt(
                            Math.pow((touchPoint.x - touchPoint.y), 2) +
                                    Math.pow((sitePoint.x - sitePoint.y), 2)
                    )
            );*/

            float[] result = new float[1];
            Location.distanceBetween(lat, lon, sitePoint.y, sitePoint.x, result);

            distance = result[0];

            if (distance < lowestDistance)
            {
                lowestDistance = distance;
                closestRda = wsrInfo;
            }
        }

        return closestRda;
    }

    public String getUrl()
    {
        String temp = this.productInfo.getUrlPart();
        return String.format(URL_TEMPLATE, temp, wsrInfo.getId().toLowerCase(Locale.ROOT));
    }

    public NexradProductInfo getProductInfo()
    {
        return productInfo;
    }

    public void setProductInfo(NexradProductInfo productInfo)
    {
        this.productInfo = productInfo;
    }

    public WsrInfo getWsrInfo()
    {
        return wsrInfo;
    }

    public void setWsrInfo(WsrInfo wsrInfo)
    {
        this.wsrInfo = wsrInfo;
    }
}
