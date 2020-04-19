package Beatmup.Visual.Android;

import android.content.Context;
import android.graphics.PointF;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

import Beatmup.Rendering.SceneRenderer;
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

        float scaleX, scaleY;    //!< scaling factors to map gestures to Renderer space
        int shiftX, shiftY;      //!< shift values to map gestures to Renderer space

        int mainPtrId, secondPtrId;         //!< pointer IDs
        boolean userActionsEnabled;

        public TouchListener() {
            userActionsEnabled = true;
            mainStartPos = new PointF();
            mainLastPos = new PointF();
            secondStartPos = new PointF();
            secondLastPos = new PointF();
            mainPtrId = secondPtrId = INVALID_POINTER_ID;
            scaleX = scaleY = 1;
            shiftX = shiftY = 0;
        }


        private void updateGestureMapping() {
            SceneRenderer renderer = Display.this.getRenderer();
            if (renderer != null)
                switch (renderer.getOutputMapping()) {
                    case FIT_WIDTH_TO_TOP:
                        scaleX = scaleY = 1.0f / getWidth();
                        shiftX = shiftY = 0;
                        return;
                    case FIT_WIDTH:
                        scaleX = scaleY = 1.0f / getWidth();
                        shiftX = 0;
                        shiftY = (getWidth() - getHeight()) / 2;
                        return;
                    case FIT_HEIGHT:
                        scaleX = scaleY = 1.0f / getHeight();
                        shiftX = (getHeight() - getWidth()) / 2;
                        shiftY = 0;
                        return;
                }
            scaleX = scaleY = 1.0f / getWidth();
            shiftX = shiftY = 0;
        }


        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if (!userActionsEnabled || gestureListener == null)
                return true;

            switch (event.getActionMasked()) {
                // main pointer down: new gesture
                case MotionEvent.ACTION_DOWN: {
                    updateGestureMapping();
                    final int ptrIdx = event.getActionIndex();
                    if (ptrIdx < 0)
                        break;
                    mainStartPos.set(
                            scaleX * (event.getX(ptrIdx) + shiftX),
                            scaleY * (event.getY(ptrIdx) + shiftY)
                    );
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
                    float x = scaleX * (event.getX(ptrIdx) + shiftX);
                    float y = scaleY * (event.getY(ptrIdx) + shiftY);
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

                    mainLastPos.set(
                            scaleX * (event.getX(mainIdx) + shiftX),
                            scaleY * (event.getY(mainIdx) + shiftY)
                    );

                    // if both pointers got, rotation/scaling
                    if (secondPtrId != INVALID_POINTER_ID) {
                        final int secondIdx = event.findPointerIndex(secondPtrId);
                        if (secondIdx >= 0) {
                            secondLastPos.set(
                                    scaleX * (event.getX(secondIdx) + shiftX),
                                    scaleY * (event.getY(secondIdx) + shiftY)
                            );
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
