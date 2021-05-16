package com.tjdickerson.sacweather.data;

public class WsrInfo
{
    public static final String SEPARATE_TOKEN = "|";

    public static int ID_INDEX = 0;
    public static int NAME_INDEX = 1;
    public static int LONGITUDE_INDEX = 2;
    public static int LATITUDE_INDEX = 3;
    public static int CALIBRATION_INDEX = 4;

    private String id;
    private String name;
    private float longitude;
    private float latitude;
    private double refCalibration;

    public String toFileLine()
    {
        return  id + SEPARATE_TOKEN +
                name + SEPARATE_TOKEN +
                longitude + SEPARATE_TOKEN +
                latitude + SEPARATE_TOKEN +
                refCalibration;
    }

    public String getId()
    {
        return id;
    }

    public void setId(String id)
    {
        this.id = id;
    }

    public String getName()
    {
        return name;
    }

    public void setName(String name)
    {
        this.name = name;
    }

    public float getLongitude()
    {
        return longitude;
    }

    public void setLongitude(float longitude)
    {
        this.longitude = longitude;
    }

    public float getLatitude()
    {
        return latitude;
    }

    public void setLatitude(float latitude)
    {
        this.latitude = latitude;
    }

    public double getRefCalibration()
    {
        return refCalibration;
    }

    public void setRefCalibration(double refCalibration)
    {
        this.refCalibration = refCalibration;
    }
}