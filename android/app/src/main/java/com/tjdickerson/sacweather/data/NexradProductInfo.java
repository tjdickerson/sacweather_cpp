package com.tjdickerson.sacweather.data;

public class NexradProductInfo
{
    private short productCode;
    private short angle;
    private String displayName;

    public NexradProductInfo(short productCode, short angle, String displayName)
    {
        init(productCode, angle, displayName);
    }

    public NexradProductInfo(short productCode, String displayName)
    {
        init(productCode, (short) 0, displayName);
    }

    private void init(short productCode, short angle, String displayName)
    {
        this.productCode = productCode;
        this.displayName = displayName;
        this.angle = angle;
    }

    public short getProductCode()
    {
        return productCode;
    }

    public void setProductCode(short productCode)
    {
        this.productCode = productCode;
    }

    public String getDisplayName()
    {
        return displayName;
    }

    public void setDisplayName(String displayName)
    {
        this.displayName = displayName;
    }

    public short getAngle()
    {
        return angle;
    }

    public void setAngle(short angle)
    {
        this.angle = angle;
    }
}
