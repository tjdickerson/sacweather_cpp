package com.tjdickerson.sacweather.listener;

import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;
import androidx.core.view.GestureDetectorCompat;
import com.tjdickerson.sacweather.SacwLib;
import com.tjdickerson.sacweather.SacwMapActivity;
import com.tjdickerson.sacweather.view.SacwMapView;

public class SacTouchListener implements
        GestureDetector.OnGestureListener,
        GestureDetector.OnDoubleTapListener,
        View.OnTouchListener
{
    float prevX = 0.0f, prevY = 0.0f;
    boolean isDoubleTap = false;

    private final GestureDetectorCompat mDetector;
    private final SacwMapActivity mActivity;
    private final SacwMapView mView;

    public SacTouchListener(SacwMapActivity activity, SacwMapView view)
    {
        mDetector = new GestureDetectorCompat(activity, this);
        mDetector.setOnDoubleTapListener(this);

        mActivity = activity;
        mView = view;
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
        if (isDoubleTap) return;

        float[] polarCoords = SacwLib.sacwGetPolarFromScreen(e.getX(), e.getY());
        float x = polarCoords[0];
        float y = polarCoords[1];

        mView.setLastQueriedPoint(x, y);
        mActivity.openContextMenu(mView);
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY)
    {
        return false;
    }

    @Override
    public boolean onTouch(View v, MotionEvent event)
    {
        if (mDetector.onTouchEvent(event)) {
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
                    SacwLib.sacwZoomMap(dy * -1);
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
