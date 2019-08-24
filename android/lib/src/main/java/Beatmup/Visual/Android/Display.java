package Beatmup.Visual.Android;

import android.content.Context;
import android.graphics.PointF;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

import Beatmup.Visual.GestureListener;

/**
 * GL display with basic TUI functionality
 */
public class Display extends BasicDisplay {
    private GestureListener gestureListener;
    private TouchListener touchListener;

    public Display(Context context, AttributeSet attrs) {
        super(context, attrs);
        setOnTouchListener(touchListener = new TouchListener());
    }


    public void setGestureListener(GestureListener newListener) {
        gestureListener = newListener;
    }


    public GestureListener getGestureListener() {
        return this.gestureListener;
    }


    private class TouchListener implements OnTouchListener {
        static final int INVALID_POINTER_ID = -1;

        PointF
            mainStartPos,                   //!< initial position of the main pointer
            secondStartPos,                 //!< initial position of the second pointer
            mainLastPos,
            secondLastPos;

        int mainPtrId, secondPtrId;         //!< pointer IDs
        boolean userActionsEnabled;

        public TouchListener() {
            userActionsEnabled = true;
            mainStartPos = new PointF();
            mainLastPos = new PointF();
            secondStartPos = new PointF();
            secondLastPos = new PointF();
            mainPtrId = secondPtrId = INVALID_POINTER_ID;
        }

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if (!userActionsEnabled || gestureListener == null)
                return true;

            switch (event.getActionMasked()) {
                // main pointer down: new gesture
                case MotionEvent.ACTION_DOWN: {
                    final int ptrIdx = event.getActionIndex();
                    if (ptrIdx < 0)
                        break;
                    mainStartPos.set(event.getX(ptrIdx) / getWidth(), event.getY(ptrIdx) / getWidth());
                    mainLastPos.set(mainStartPos.x, mainStartPos.y);
                    mainPtrId = event.getPointerId(ptrIdx);
                    gestureListener.pointerDown(mainStartPos.x, mainStartPos.y);
                    break;
                }

                // additional pointer down
                case MotionEvent.ACTION_POINTER_DOWN: {
                    final int ptrIdx = event.getActionIndex();
                    if (ptrIdx < 0)
                        break;
                    float x = event.getX(ptrIdx) / getWidth(), y = event.getY(ptrIdx) / getWidth();
                    if (secondPtrId == INVALID_POINTER_ID) {
                        secondPtrId = event.getPointerId(ptrIdx);
                        secondStartPos.set(x, y);
                        secondLastPos.set(secondStartPos.x, secondStartPos.y);
                        mainStartPos.set(mainLastPos.x, mainLastPos.y);
                        gestureListener.cumulateGesture();
                    }
                    gestureListener.pointerDown(x, y);
                    break;
                }

                case MotionEvent.ACTION_MOVE: {
                    final int mainIdx = event.findPointerIndex(mainPtrId);
                    if (mainIdx < 0) {
                        // invalidation of the main pointer
                        mainPtrId = INVALID_POINTER_ID;
                        break;
                    }

                    mainLastPos.set(event.getX(mainIdx) / getWidth(), event.getY(mainIdx) / getWidth());

                    // if both pointers got, rotation/scaling
                    if (secondPtrId != INVALID_POINTER_ID) {
                        final int secondIdx = event.findPointerIndex(secondPtrId);
                        if (secondIdx >= 0) {
                            secondLastPos.set(event.getX(secondIdx) / getWidth(), event.getY(secondIdx) / getWidth());
                            gestureListener.gestureUpdated(
                                    mainStartPos.x, mainStartPos.y, secondStartPos.x, secondStartPos.y,
                                    mainLastPos.x, mainLastPos.y, secondLastPos.x, secondLastPos.y
                            );
                        }
                    }
                    // otherwise just translation
                    else
                        gestureListener.gestureUpdated(
                                mainStartPos.x, mainStartPos.y,
                                Float.NaN, Float.NaN,
                                mainLastPos.x, mainLastPos.y,
                                Float.NaN, Float.NaN
                        );
                    break;
                }

                case MotionEvent.ACTION_UP:
                    mainPtrId = INVALID_POINTER_ID;
                    secondPtrId = INVALID_POINTER_ID;
                    gestureListener.pointerUp();
                    gestureListener.cancelGesture();    // sometimes some pointers remain down
                    break;

                case MotionEvent.ACTION_CANCEL:
                    mainPtrId = INVALID_POINTER_ID;
                    secondPtrId = INVALID_POINTER_ID;
                    gestureListener.cancelGesture();
                    break;

                case MotionEvent.ACTION_POINTER_UP: {
                    final int
                            ptrIdx = event.getActionIndex(),
                            pointerId = event.getPointerId(ptrIdx);
                    if (pointerId == mainPtrId && secondPtrId != INVALID_POINTER_ID) {
                        mainPtrId = secondPtrId;
                        mainLastPos.set(secondLastPos.x, secondLastPos.y);
                    }
                    mainStartPos.set(mainLastPos.x, mainLastPos.y);
                    secondPtrId = INVALID_POINTER_ID;
                    gestureListener.pointerUp();
                    gestureListener.cumulateGesture();
                    break;
                }
            }

            return true;
        }
    }


    public void setUIActionsEnabled(boolean enabled) {
        touchListener.userActionsEnabled = enabled;
    }


    public boolean getUIActionsEnabled() {
        return touchListener.userActionsEnabled;
    }
}
