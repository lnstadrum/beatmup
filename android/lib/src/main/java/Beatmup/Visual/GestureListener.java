package Beatmup.Visual;

import Beatmup.Geometry.AffineMapping;

/**
 * Processing of basic UI events related to a scene
 */
public class GestureListener {
    private static final int
            TAP_TIME_MILLISEC = 200,        //!< difference in time between down and up events to be considered as a tapping
            HOLD_TIME_MILLISEC = 500;

    private AffineMapping
            cumulatedGesture,
            lastMovement,
            currentGesture;                 //!< affine mapping representing entire current gesture (cumulated * last)

    private float squaredPointerTolerance;
    private int pointerCount;
    private long mainTouchTime;                     //!< timestamp when main pointer goes down
    private boolean tapping;
    private float mainX, mainY;
    private float
            minGestureScale,
            maxGestureScale;

    private Animator animator;                      //!< used to handle timed events (like hold)
    private Animator.Event holdEvent;               //!< hold event
    private int holdEventTicket;                    //!< ticket number of current hold event for Animator


    public GestureListener(float pointerTolerance) {
        this.squaredPointerTolerance = pointerTolerance * pointerTolerance;
        cumulatedGesture = new AffineMapping();
        lastMovement = new AffineMapping();
        currentGesture = new AffineMapping();
        tapping = false;
        minGestureScale = 0.0f;
        maxGestureScale = Float.POSITIVE_INFINITY;
        animator = null;
        holdEventTicket = Animator.INVALID_TICKET;
        holdEvent = new Animator.Event() {
            @Override
            public long run() {
                if (tapping)
                    onHold(mainX, mainY);
                return -1;
            }
        };
    }


    private void cancelPlannedEvents() {
        if (animator != null)
            animator.cancel(holdEvent, holdEventTicket);
    }


    public void cumulateGesture() {
        cumulatedGesture.assign(currentGesture);
        lastMovement.setIdentity();
    }


    /**
     * Retrieves a mapping from two pointers displacement. The default implementation assumes
     * orthogonal mappings in image plane.
     * @param target    the retrieved mapping (output)
     * @param X1        first pointer initial horizontal coordinate
     * @param Y1        first pointer initial vertical coordinate
     * @param X2        second pointer initial horizontal coordinate
     * @param Y2        second pointer initial vertical coordinate
     * @param newX1     first pointer final horizontal coordinate
     * @param newY1     first pointer final vertical coordinate
     * @param newX2     second  pointer final horizontal coordinate
     * @param newY2     second pointer final vertical coordinate
     */
    protected void resolveGesture(
            AffineMapping target,
            float X1, float Y1,
            float X2, float Y2,
            float newX1, float newY1,
            float newX2, float newY2
    ) {
        if (Float.isNaN(X2) || Float.isNaN(Y2))
            // convention: if point 2 shows NaNs, there is only one pointer given
            target.setTranslation(newX1 - X1, newY1 - Y1);
        else {
            float scale = (float) Math.sqrt(cumulatedGesture.det());
            target.resolve(
                    X1, Y1, X2, Y2,
                    newX1, newY1, newX2, newY2,
                    minGestureScale / scale, maxGestureScale / scale
            );
        }
    }


    /**
     * To be called when a two-pointer gesture is performed
     * @param X1        first pointer initial horizontal coordinate
     * @param Y1        first pointer initial vertical coordinate
     * @param X2        second pointer initial horizontal coordinate
     * @param Y2        second pointer initial vertical coordinate
     * @param newX1     first pointer final horizontal coordinate
     * @param newY1     first pointer final vertical coordinate
     * @param newX2     second  pointer final horizontal coordinate
     * @param newY2     second pointer final vertical coordinate
     */
    public void gestureUpdated(
            float X1, float Y1,
            float X2, float Y2,
            float newX1, float newY1,
            float newX2, float newY2
    ) {
        resolveGesture(lastMovement,
                X1, Y1, X2, Y2,
                newX1, newY1, newX2, newY2
        );

        // if tapping, check main pointer displacement
        if (tapping) {
            float dx = Math.abs(mainX - newX1), dy = Math.abs(mainY - newY1);
            if (dx*dx + dy*dy > squaredPointerTolerance) {
                // stop tapping
                tapping = false;
                cancelPlannedEvents();
            }
        }

        AffineMapping.compose(currentGesture, lastMovement, cumulatedGesture);
        if (!onGesture(currentGesture))
            AffineMapping.compose(lastMovement, currentGesture, cumulatedGesture.getInverse());
    }


    public final void cancelGesture() {
        if (pointerCount > 0)
            onCancel();
        pointerCount = 0;
        tapping = false;
    }


    public final void pointerDown(float x, float y) {
        cancelPlannedEvents();
        if (pointerCount == 0) {
            mainTouchTime = System.currentTimeMillis();
            onFirstTouch(x,y);
            mainX = x;
            mainY = y;
            tapping = true;
            // planify hold event
            if (animator != null) {
                holdEventTicket = animator.planify(holdEvent, HOLD_TIME_MILLISEC);
            }
        } else
            tapping = false;
        onPointerDown(pointerCount, x, y);
        pointerCount++;
    }


    public final void pointerUp() {
        cancelPlannedEvents();
        if (pointerCount == 1  &&  System.currentTimeMillis() < mainTouchTime + TAP_TIME_MILLISEC && tapping)
            onTap(mainX, mainY);
        else
            if (pointerCount == 1)
                onRelease(currentGesture);
        pointerCount = Math.max(pointerCount-1, 0);
    }


    /**
     * A quick touch-release (click-like)
     * @param x         width-normalized horizontal coordinate
     * @param y         width-normalized vertical coordinate
     */
    public void onTap(float x, float y) {
        // nothing to do by default
    }


    /**
     * A holdEvent touch. Invoked only if enabled by calling enableHoldEvent
     * @param x         width-normalized horizontal coordinate
     * @param y         width-normalized vertical coordinate
     */
    public void onHold(float x, float y) {
        // nothing to do by default
    }


    /**
     * A first touch
     */
    public void onFirstTouch(float x, float y) {
        // nothing to do by default
    }


    /**
     * Permanently called when a gesture is in progress
     * @param gesture   the mapping of the current gesture
     * @return  `true` if the gesture is accepted as is; if it is truncated (modified), should
     *          return `false` so that the modification is taken into account by the listener
     */
    public boolean onGesture(AffineMapping gesture) {
        // nothing to do by default
        return true;
    }


    public void onRelease(AffineMapping gesture) {
        // nothing to do by default
    }


    public void onCancel() {
        // nothing to do by default
    }


    public void onPointerDown(int pointerNumber, float x, float y) {
        // nothing to do by default
    }


    public void setGesture(AffineMapping gesture, float minScale, float maxScale) {
        if (gesture == null)
            currentGesture.setIdentity();
        else
            currentGesture.assign(gesture);
        this.minGestureScale = minScale;
        this.maxGestureScale = maxScale;
        cumulateGesture();
    }


    /**
     * Enables hold event.
     * @param animator an Animator object used to handle the event
     */
    public void enableHoldEvent(Animator animator) {
        this.animator = animator;
    }


    /**
     * Reinitializes gesture listener
     */
    public void reset() {
        pointerCount = 0;
        tapping = false;
        cancelPlannedEvents();
    }
}
