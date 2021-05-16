package com.tjdickerson.sacweather;

public class SacwLib
{
    static
    {
        System.loadLibrary("native-lib");
    }

    public static native void sacwInit();
    public static native void sacwMainLoop();
    public static native void sacwUpdateViewport(float width, float height);
    public static native void sacwPanMap(float x, float y);
    public static native void sacwZoomMap(float zoom);
    public static native long sacwScanTime();
    public static native void sacwRadarInit(String filename, short productCode);
    public static native float[] sacwGetPolarFromScreen(float x, float y);
}
