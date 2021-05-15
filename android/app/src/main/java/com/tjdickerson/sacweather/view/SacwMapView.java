package com.tjdickerson.sacweather.view;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import com.tjdickerson.sacweather.listener.SacTouchListener;

public class SacwMapView extends GLSurfaceView
{
    private SacwMapRenderer mSacwMapRenderer;

    public SacwMapView(Context context, AttributeSet attributeSet)
    {
        super(context, attributeSet);

        //setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        setEGLContextClientVersion(2);

        mSacwMapRenderer = new SacwMapRenderer(context);
        setRenderer(mSacwMapRenderer);
    }

    public void init(Activity activity)
    {
        setOnTouchListener(new SacTouchListener(activity));
    }
}
