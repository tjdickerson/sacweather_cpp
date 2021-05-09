package com.tjdickerson.sacweather.view;

import android.content.Context;

import android.opengl.GLSurfaceView;
import com.tjdickerson.sacweather.SacwLib;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


public class SacwMapRenderer implements GLSurfaceView.Renderer
{
    public SacwMapRenderer(Context context)
    {

    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config)
    {
        SacwLib.sacwInit();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height)
    {

        SacwLib.sacwUpdateViewport((float)width, (float)height);
    }

    @Override
    public void onDrawFrame(GL10 gl)
    {
        SacwLib.sacwMainLoop();
    }
}
