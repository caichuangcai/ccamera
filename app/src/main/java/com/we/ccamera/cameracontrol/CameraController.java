package com.we.ccamera.cameracontrol;

import android.annotation.SuppressLint;
import android.graphics.SurfaceTexture;
import android.util.Log;
import android.util.Size;
import android.view.Surface;
import com.google.common.util.concurrent.ListenableFuture;
import com.we.ccamera.presenter.CCameraBuffer;
import com.we.ccamera.presenter.OnFrameAvailableListener;
import com.we.ccamera.presenter.OnSurfaceTextureListener;
import com.we.ccamera.presenter.PreviewCallback;
import com.we.ccamera.util.ImageConvertUtil;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.camera.core.Camera;
import androidx.camera.core.CameraSelector;
import androidx.camera.core.ImageAnalysis;
import androidx.camera.core.ImageProxy;
import androidx.camera.core.Preview;
import androidx.camera.lifecycle.ProcessCameraProvider;
import androidx.core.content.ContextCompat;


public class CameraController implements ICameraController {

    private final int DEFAULT_WIDTH = 720;      // 实际宽度
    private final int DEFAULT_HEIGHT = 1280;    // 实际高度

    private final int PREVIEW_WIDTH = 720;      // 预览宽度
    private final int PREVIEW_HEIGHT = 1280;    // 预览高度

    // Camera提供者
    private ProcessCameraProvider mCameraProvider;
    // Camera接口
    private Camera mCamera;
    // 预览配置
    private Preview mPreview;
    // Camera分析
    private ImageAnalysis mPreviewAnalyzer;

    private Executor mExecutor = Executors.newSingleThreadExecutor();

    // 预览角度
    private int mRotation = 0;
    // 是否打开前置摄像头
    private boolean mFacingFront = false;

    private AppCompatActivity context;

    public CameraController(AppCompatActivity context) {
        this.context = context;
    }

    @Override
    public void openCamera(SurfaceTexture surfaceTexture) {
        ListenableFuture<ProcessCameraProvider> cameraProviderFuture = ProcessCameraProvider.getInstance(context);
        cameraProviderFuture.addListener(() -> {
            try {
                mCameraProvider = cameraProviderFuture.get();
                bindCameraUseCases(surfaceTexture);
            } catch (ExecutionException | InterruptedException e) {
                e.printStackTrace();
            }
        }, ContextCompat.getMainExecutor(context));
    }

    @Override
    public void closeCamera() {
        try {
            if (mCameraProvider != null) {
                mCameraProvider.unbindAll();
                mCameraProvider = null;
            }
            releaseSurfaceTexture();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 初始化相机配置
     */
    private void bindCameraUseCases(SurfaceTexture surfaceTexture) {
        if (mCameraProvider == null) {
            return;
        }

        // 解除绑定
        mCameraProvider.unbindAll();

        // 预览画面
        mPreview = new Preview
                .Builder()
                .setTargetResolution(new Size(PREVIEW_WIDTH, PREVIEW_HEIGHT))
                .build();

        // 预览绑定SurfaceTexture
        mPreview.setSurfaceProvider(surfaceRequest ->
        {
            surfaceTexture.setDefaultBufferSize(surfaceRequest.getResolution().getWidth(), surfaceRequest.getResolution().getHeight());
            Surface surface = new Surface(surfaceTexture);
            surfaceRequest.provideSurface(surface, mExecutor, result ->
            {
                surface.release();
            });
            if (onSurfaceTextureListener != null)
            {
                onSurfaceTextureListener.onSurfaceTexturePrepared(surfaceRequest.getResolution().getWidth(), surfaceRequest.getResolution().getHeight());
            }
            surfaceTexture.setOnFrameAvailableListener((SurfaceTexture texture)->
            {
                onFrameAvailableListener.onFrameAvailable();
            });
        });

        // 预览帧回调
        mPreviewAnalyzer = new ImageAnalysis.Builder()
                .setTargetResolution(new Size(PREVIEW_WIDTH, PREVIEW_HEIGHT))
                .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
                .build();
        mPreviewAnalyzer.setAnalyzer(mExecutor, new PreviewCallbackAnalyzer(mPreviewCallback));

        // 前后置摄像头选择器
        CameraSelector cameraSelector = new CameraSelector.Builder()
                .requireLensFacing(mFacingFront?CameraSelector.LENS_FACING_FRONT : CameraSelector.LENS_FACING_BACK).build();

        // 绑定输出
        mCamera = mCameraProvider.bindToLifecycle(context, cameraSelector, mPreview, mPreviewAnalyzer);
    }

    static class PreviewCallbackAnalyzer implements ImageAnalysis.Analyzer{

        private PreviewCallback previewCallback;
        private CCameraBuffer buffer;

        private final boolean VERBOSE = false;

        public PreviewCallbackAnalyzer(PreviewCallback callback) {
            this.previewCallback = callback;
            this.buffer = new CCameraBuffer();
        }

        @SuppressLint("UnsafeExperimentalUsageError")
        @Override
        public void analyze(@NonNull ImageProxy image) {
            long start = System.currentTimeMillis();
            if (VERBOSE) {
                Log.d("Image", "analyze: timestamp - " + image.getImageInfo().getTimestamp() + ", " +
                        "orientation - " + image.getImageInfo().getRotationDegrees() + ", imageFormat" +
                        " - " + image.getFormat());
            }

            if(previewCallback != null && image.getImage() != null)
            {
                buffer.frameWidth = image.getImage().getWidth();
                buffer.frameHeight = image.getImage().getHeight();
                ImageConvertUtil.convertCameraImage(image.getImage(), buffer);
                previewCallback.onPreviewFrame(buffer);
            }

            image.close();

            if (VERBOSE) {
                Log.d("Image", "convert cost time - " + (System.currentTimeMillis() - start));
            }
        }

    }

    /**
     * 释放输出的SurfaceTexture，防止内存泄露
     */
    private void releaseSurfaceTexture() {

    }

    @Override
    public void setFront(boolean font) {
        this.mFacingFront = font;
    }

    @Override
    public boolean isFront() {
        return mFacingFront;
    }

    @Override
    public int getPreviewWidth() {
        if(mRotation == 90 || mRotation == 270) {
            return PREVIEW_HEIGHT;
        }
        return PREVIEW_WIDTH;
    }

    @Override
    public int getPreviewHeight() {
        if(mRotation == 90 || mRotation == 270) {
            return PREVIEW_WIDTH;
        }
        return PREVIEW_HEIGHT;
    }

    private OnSurfaceTextureListener onSurfaceTextureListener;
    private PreviewCallback mPreviewCallback;
    private OnFrameAvailableListener onFrameAvailableListener;

    @Override
    public void setOnSurfaceTextureListener(OnSurfaceTextureListener listener) {
        this.onSurfaceTextureListener = listener;
    }

    @Override
    public void setOnPreviewCallback(PreviewCallback callback) {
        this.mPreviewCallback = callback;
    }

    @Override
    public void setOnFrameAvailableListener(OnFrameAvailableListener listener) {
        this.onFrameAvailableListener = listener;
    }

}