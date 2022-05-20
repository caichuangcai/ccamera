package com.we.ccamera.util;

import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.media.Image;
import android.util.Log;
import com.we.ccamera.presenter.CCameraBuffer;
import java.nio.ByteBuffer;


public class ImageConvertUtil {

    public static final boolean VERBOSE = false;
    public static final String TAG = "ImageConvertUtil";

    private static byte[] rowData = null;

    public static void convertCameraImage(Image image, CCameraBuffer ccameraBuffer) {
        Rect crop = image.getCropRect();
        int format = image.getFormat();
        int width = crop.width();
        int height = crop.height();
        Image.Plane[] planes = image.getPlanes();

        byte[] data = ccameraBuffer.initBuffer(width, height, format);
        if(rowData == null || rowData.length != planes[0].getRowStride()) {
            rowData = new byte[planes[0].getRowStride()];
        }

        if (VERBOSE) {
            Log.d(TAG, "get data from " + planes.length + " planes");
        }
        int yLength = 0;
        int stride = 1;
        for (int i = 0; i < planes.length; i++) {
            switch (i) {
                case 0: {
                    yLength = 0;
                    stride = 1;
                    break;
                }
                case 1: {
                    yLength = width * height + 1;
                    stride = 2;
                    break;
                }
                case 2: {
                    yLength = width * height;
                    stride = 2;
                    break;
                }
            }

            ByteBuffer buffer = planes[i].getBuffer();
            int rowStride = planes[i].getRowStride();
            int pixelStride = planes[i].getPixelStride();
            if (VERBOSE) {
                Log.e(TAG, "pixelStride " + pixelStride);
                Log.e(TAG, "rowStride " + rowStride);
                Log.e(TAG, "width " + width);
                Log.e(TAG, "height " + height);
                Log.e(TAG, "buffer size " + buffer.remaining());
            }

            int shift = (i == 0) ? 0 : 1;
            int w = width >> shift;
            int h = height >> shift;
            buffer.position(rowStride * (crop.top >> shift) + pixelStride * (crop.left >> shift));
            for (int row = 0; row < h; row++) {
                int length;
                if (pixelStride == 1 && stride == 1) {
                    length = w;
                    buffer.get(data, yLength, length);
                    yLength += length;
                } else {
                    length = (w - 1) * pixelStride + 1;
                    buffer.get(rowData, 0, length);
                    for (int col = 0; col < w; col++) {
                        data[yLength] = rowData[col * pixelStride];
                        yLength += stride;
                    }
                }
                if (row < h - 1) {
                    buffer.position(buffer.position() + rowStride - length);
                }
            }
            if (VERBOSE) {
                Log.e(TAG, "Finished reading data from plane " + i);
            }
        }

    }

}