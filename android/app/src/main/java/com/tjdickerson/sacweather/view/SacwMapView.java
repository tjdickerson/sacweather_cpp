package com.tjdickerson.sacweather.view;

import android.app.Activity;
import android.content.Context;
import android.graphics.PointF;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import com.tjdickerson.sacweather.SacwMapActivity;
import com.tjdickerson.sacweather.listener.SacTouchListener;

public class SacwMapView extends GLSurfaceView
{
    private SacwMapRenderer mSacwMapRenderer;
    private PointF mLastQueriedPoint;

    public SacwMapView(Context context, AttributeSet attributeSet)
    {
        super(context, attributeSet);

        //setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        setEGLContextClientVersion(2);

        mSacwMapRenderer = new SacwMapRenderer(context);
        setRenderer(mSacwMapRenderer);
    }

    public void init(SacwMapActivity activity)
    {
        setOnTouchListener(new SacTouchListener(activity, this));
    }

    public PointF getLastQueriedPoint()
    {
        return mLastQueriedPoint;
    }

    public void setLastQueriedPoint(float longitude, float latitude)
    {
        setLastQueriedPoint(new PointF(longitude, latitude));
    }

    public void setLastQueriedPoint(PointF point)
    {
        mLastQueriedPoint = point;
    }

    @Override
    public void onPause()
    {
        //super.onPause();
        // @todo
        // Might need to set some flag to stop rendering when application is paused.
    }

    @Override
    public void onResume()
    {
        //super.onResume();
    }
}
