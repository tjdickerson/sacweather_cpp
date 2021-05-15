package com.tjdickerson.sacweather.listener;

import android.app.Activity;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;
import androidx.core.view.GestureDetectorCompat;
import com.tjdickerson.sacweather.SacwLib;

public class SacTouchListener implements
        GestureDetector.OnGestureListener,
        GestureDetector.OnDoubleTapListener,
        View.OnTouchListener
{
    float prevX = 0.0f, prevY = 0.0f;
    boolean isDoubleTap = false;

    private GestureDetectorCompat mDetector;

    public SacTouchListener(Activity activity)
    {
        mDetector = new GestureDetectorCompat(activity, this);
        mDetector.setOnDoubleTapListener(this);
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e)
    {
        return false;
    }

    @Override
    public boolean onDoubleTap(MotionEvent e)
    {
        isDoubleTap = true;
        return true;
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e)
    {
        return false;
    }

    @Override
    public boolean onDown(MotionEvent e)
    {
        return false;
    }

    @Override
    public void onShowPress(MotionEvent e)
    {

    }

    @Override
    public boolean onSingleTapUp(MotionEvent e)
    {
        return false;
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY)
    {
        return false;
    }

    @Override
    public void onLongPress(MotionEvent e)
    {

    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY)
    {
        return false;
    }

    @Override
    public boolean onTouch(View v, MotionEvent event)
    {
        if (this.mDetector.onTouchEvent(event)) {
            return true;
        }

        float x = event.getX();
        float y = event.getY();

        switch(event.getAction())
        {
            case MotionEvent.ACTION_UP:
                isDoubleTap = false;
                break;

            case MotionEvent.ACTION_MOVE:

                float dx = (prevX - x);
                float dy = (prevY - y);

                if (isDoubleTap)
                {
                    SacwLib.sacwZoomMap(dy);
                }
                else
                {
                    SacwLib.sacwPanMap(dx, dy);
                }
                break;
        }

        prevX = x;
        prevY = y;

        return true;
    }
}
