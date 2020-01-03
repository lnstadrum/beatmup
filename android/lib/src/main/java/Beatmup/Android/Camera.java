package Beatmup.Android;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.graphics.Rect;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.Face;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.util.Size;
import android.view.Display;
import android.view.Surface;

import java.util.Arrays;

import Beatmup.Task;

public class Camera {
    public interface Callback {
        void onAccessError(Camera camera);
        void onConfigureFailed(Camera camera);
        void onCameraError(Camera camera, int code);
        void onCameraOpened(Camera camera);
        void onCameraClosed(Camera camera);
        void onFacesDetected(Camera camera, Face[] faces);
        void onFrameReceived(Camera camera);
    }

    /**
     * Automatic preview resolution selection policy
     */
    public enum ResolutionSelectionPolicy {
        SMALLEST_COVERAGE,      //!< pick smallest available that covers a given size
        BIGGEST                 //!< pick the biggest available resolution
    };

    public enum Facing {
        FRONT,
        BACK,
        EXTERNAL
    };

    private Beatmup.Android.Context context;
    private ExternalBitmap image;

    private String[] cameraIds;
    private int selectedCameraIdx;
    private Size resolution;
    private CameraCharacteristics characteristics;

    private DeviceStateCallback stateCallback;
    private CaptureRequest.Builder captureRequestBuilder;
    private CameraManager manager;
    private Handler backgroundHandler;
    private HandlerThread backgroundThread;

    private Callback callback;


    @TargetApi(Build.VERSION_CODES.M)
    public Camera(Beatmup.Android.Context beatmup, Context context) throws CameraAccessException {
        this.context = beatmup;
        image = new ExternalBitmap(beatmup);
        stateCallback = new DeviceStateCallback();

        manager = (CameraManager) context.getSystemService(android.content.Context.CAMERA_SERVICE);
        assert manager != null;
        cameraIds = manager.getCameraIdList();
        selectCamera(0);

        backgroundThread = new HandlerThread("Beatmup camera thread");
        backgroundThread.start();
        backgroundHandler = new Handler(backgroundThread.getLooper());
    }

    /**
     * Retrieves an image captured by the camera.
     * @return bitmap object.
     */
    public ExternalBitmap getImage() {
        return image;
    }


    public void setCallback(Callback callback) {
        this.callback = callback;
    }


    /**
     * @return number of available cameras
     */
    public int getNumberOfCameras() {
        return cameraIds.length;
    }


    /**
     * Select a camera by its number.
     * @param number     Zero-based camera number
     */
    public void selectCamera(int number) {
        selectedCameraIdx = number;
        characteristics = null;
        try {
            characteristics = manager.getCameraCharacteristics(cameraIds[selectedCameraIdx]);
        } catch (CameraAccessException e) {
            if (callback == null)
                e.printStackTrace();
            else
                callback.onAccessError(this);
        }
    }


    /**
     * Tries to find a camera having a specified facing.
     * @param facing    The target camera facing
     * @return `true` if found and selected, `false` otherwise (the selected camera remains
     * unchanged).
     */
    public boolean selectCamera(Facing facing) {
        CameraCharacteristics chars;
        for (int i = 0; i < cameraIds.length; i++) {
            try {
                chars = manager.getCameraCharacteristics(cameraIds[i]);
            } catch (CameraAccessException e) {
                if (callback == null)
                    e.printStackTrace();
                else
                    callback.onAccessError(this);
                return false;
            }

            Integer facingGot = chars.get(CameraCharacteristics.LENS_FACING);
            if (facingGot != null)
                switch (facing) {
                    case FRONT:
                        if (facingGot == CameraCharacteristics.LENS_FACING_FRONT) {
                            selectCamera(i);
                            return true;
                        }
                        break;

                    case BACK:
                        if (facingGot == CameraCharacteristics.LENS_FACING_BACK) {
                            selectCamera(i);
                            return true;
                        }
                        break;

                    case EXTERNAL:
                        if (facingGot == CameraCharacteristics.LENS_FACING_EXTERNAL) {
                            selectCamera(i);
                            return true;
                        }
                        break;
                }
        }

        // not found
        return false;
    }


    public int getCameraNumber() {
        return selectedCameraIdx;
    }


    /**
     * Performs automatic search of the preview resolution according to a specified criterion.
     * @param width     Desired preview width
     * @param height    Desired preview height
     * @param policy    Criterion used to choose the resolution
     * @return selected resolution.
     */
    public Size chooseResolution(int width, int height, ResolutionSelectionPolicy policy) {
        if (!checkAccess())
            return null;

        StreamConfigurationMap map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        assert map != null;
        Size[] sizes = map.getOutputSizes(SurfaceTexture.class);

        switch (policy) {
            case SMALLEST_COVERAGE:
                for (Size size : sizes) {
                    if (resolution == null || (
                            size.getWidth() >= width &&
                            size.getHeight() >= height &&
                            size.getWidth() * size.getHeight() < resolution.getWidth() * resolution.getHeight()
                    ))
                        resolution = size;
                }
                break;

            case BIGGEST:
                for (Size size : sizes) {
                    if (resolution == null ||
                            (size.getWidth() * size.getHeight() >= resolution.getWidth() * resolution.getHeight()))
                        resolution = size;
                }
                break;
        }

        return resolution;
    }


    /**
     * @return currently used resolution.
     */
    public Size getResolution() {
        return resolution;
    }


    public Rect getSensorSpaceSize()  {
        return characteristics.get(CameraCharacteristics.SENSOR_INFO_ACTIVE_ARRAY_SIZE);
    }


    /**
     * Retrieves camera image orientation with respect to a given display
     * @param display The display
     * @return camera image orientation in degrees within [0..360) range.
     */
    public int getOrientation(Display display) {
        if (!checkAccess())
            return 0;

        // get display orientation
        int displayOrientation = display.getRotation();
        if (displayOrientation == android.view.OrientationEventListener.ORIENTATION_UNKNOWN)
            return 0;
        displayOrientation = (displayOrientation + 45) / 90 * 90;

        // Reverse device orientation for front-facing cameras
        Integer facing = characteristics.get(CameraCharacteristics.LENS_FACING);
        boolean facingFront = facing != null && facing == CameraCharacteristics.LENS_FACING_FRONT;
        if (facingFront)
            displayOrientation = -displayOrientation;

        Integer sensorOrientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
        if (sensorOrientation == null)
            sensorOrientation = 0;
        return (sensorOrientation + displayOrientation + 360) % 360;
    }


    public boolean enableFaceDetection() {
        if (captureRequestBuilder == null)
            return false;

        int modes[] = characteristics.get(CameraCharacteristics.STATISTICS_INFO_AVAILABLE_FACE_DETECT_MODES);
        if (modes == null || modes.length == 0 ||
                (modes.length == 1 && modes[0] == CaptureRequest.STATISTICS_FACE_DETECT_MODE_OFF))
            return false;
        captureRequestBuilder.set(CaptureRequest.STATISTICS_FACE_DETECT_MODE, modes[modes.length - 1]);

        return true;
    }


    /**
     * Initiates preview.
     */
    @SuppressLint("MissingPermission")
    public void open() {
        if (resolution == null)
            chooseResolution(640, 480, ResolutionSelectionPolicy.SMALLEST_COVERAGE);
        try {
            manager.openCamera(cameraIds[selectedCameraIdx], stateCallback, backgroundHandler);
        } catch (CameraAccessException e) {
            if (callback == null)
                e.printStackTrace();
            else
                callback.onAccessError(this);
        }
    }


    public void close() {
        stateCallback.close();
    }


    protected void stopBackgroundThread() {
        backgroundThread.quitSafely();
        try {
            backgroundThread.join();
            backgroundThread = null;
            backgroundHandler = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }


    private boolean checkAccess() {
        if (characteristics == null) {
            // characteristics are null if camera access is denied (occurs in selectCamera)
            if (callback != null)
                callback.onAccessError(this);
            return false;
        }
        return true;
    }


    private class DeviceStateCallback extends CameraDevice.StateCallback {
        private CameraDevice device;
        private CaptureSessionStateCallback stateCallback;

        DeviceStateCallback() {
            stateCallback = new CaptureSessionStateCallback();
        }

        void close() {
            if (device != null)
                device.close();
        }

        @Override
        public void onOpened(CameraDevice cameraDevice) {
            this.device = cameraDevice;
            SurfaceTexture texture = image.getSurfaceTexture();
            assert texture != null;
            texture.setDefaultBufferSize(resolution.getWidth(), resolution.getHeight());
            Surface surface = new Surface(texture);
            try {
                captureRequestBuilder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            } catch (CameraAccessException e) {
                if (callback == null)
                    e.printStackTrace();
                else
                    callback.onAccessError(Camera.this);
                return;
            }
            captureRequestBuilder.addTarget(surface);
            try {
                cameraDevice.createCaptureSession(Arrays.asList(surface), stateCallback, backgroundHandler);
            } catch (CameraAccessException e) {
                if (callback == null)
                    e.printStackTrace();
                else
                    callback.onAccessError(Camera.this);
                return;
            }

            if (callback != null)
                callback.onCameraOpened(Camera.this);
        }

        @Override
        public void onDisconnected(CameraDevice cameraDevice) {
            device.close();
            if (callback != null)
                callback.onCameraClosed(Camera.this);
        }

        @Override
        public void onError(CameraDevice cameraDevice, int code) {
            device.close();
            device = null;
            if (callback != null)
                callback.onCameraError(Camera.this, code);
        }

        private class CaptureSessionStateCallback extends CameraCaptureSession.StateCallback {
            private CaptureCallback captureCallback;

            CaptureSessionStateCallback() {
                captureCallback = new CaptureCallback();
            }

            @Override
            public void onConfigured(CameraCaptureSession cameraCaptureSession) {
                //The camera is already closed
                if (null == device)
                    return;
                // When the session is ready, we start displaying the preview.
                captureRequestBuilder.set(CaptureRequest.CONTROL_MODE, CameraMetadata.CONTROL_MODE_AUTO);
                try {
                    cameraCaptureSession.setRepeatingRequest(captureRequestBuilder.build(), captureCallback, backgroundHandler);
                } catch (CameraAccessException e) {
                    if (callback == null)
                        e.printStackTrace();
                    else
                        callback.onAccessError(Camera.this);
                }
            }


            @Override
            public void onConfigureFailed(CameraCaptureSession cameraCaptureSession) {
                if (callback != null)
                    callback.onConfigureFailed(Camera.this);
            }


            private class CaptureCallback extends CameraCaptureSession.CaptureCallback {
                @Override
                public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request, TotalCaptureResult result) {
                    image.notifyUpdate(resolution.getWidth(), resolution.getHeight());

                    if (callback != null) {
                        Face[] faces = result.get(CaptureResult.STATISTICS_FACES);
                        if (faces != null)
                            callback.onFacesDetected(Camera.this, faces);

                        callback.onFrameReceived(Camera.this);
                    }
                }
            }
        }
    }
}