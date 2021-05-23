package com.tjdickerson.sacweather.data;

import java.util.UUID;

public class NexradProductInfo
{
    private short productCode;
    private short angle;
    private String displayName;
    private int guid;
    private String urlPart;

    public NexradProductInfo(short productCode, short angle, String displayName, String urlPart)
    {
        init(productCode, angle, displayName, urlPart);
    }

    private void init(short productCode, short angle, String displayName, String urlPart)
    {
        this.productCode = productCode;
        this.displayName = displayName;
        this.angle = angle;
        this.urlPart = urlPart;

        // @todo
        // probably change how this works....
        this.guid = (this.displayName + String.valueOf(this.angle)).hashCode();
    }

    public String getFullName()
    {
        return getDisplayName() + " t" + getAngle() + "**";
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

    public int getGuid()
    {
        return guid;
    }

    public String getUrlPart()
    {
        return urlPart;
    }
}
