package com.we.ccamera.renderer;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import static android.opengl.GLES11Ext.GL_TEXTURE_EXTERNAL_OES;
import static android.opengl.GLES20.GL_TEXTURE_2D;

public class BaseRenderer implements GLSurfaceView.Renderer {

    /**
     * 顶点坐标
     * */
    private float[] VERTEX = new float[]{
            0f, 0f, 0f,     //顶点坐标V0
            1f, 1f, 0f,     //顶点坐标V1
            -1f, 1f, 0f,    //顶点坐标V2
            -1f, -1f, 0f,   //顶点坐标V3
            1f, -1f, 0f     //顶点坐标V4
    };

    /**
     * 纹理坐标
     * */
    private static final float[] TEX_VERTEX = {
            0.5f, 0.5f, //纹理坐标V0
            1f, 0f,     //纹理坐标V1
            0f, 0f,     //纹理坐标V2
            0f, 1.0f,   //纹理坐标V3
            1f, 1.0f    //纹理坐标V4
    };

    /**
     * 索引
     */
    public static final short[] VERTEX_INDEX = {
            0, 1, 2,  //V0,V1,V2 三个顶点组成一个三角形
            0, 2, 3,  //V0,V2,V3 三个顶点组成一个三角形
            0, 3, 4,  //V0,V3,V4 三个顶点组成一个三角形
            0, 4, 1   //V0,V4,V1 三个顶点组成一个三角形
    };

    public FloatBuffer vertexBuffer, texVertexBuffer;
    public ShortBuffer indicesBuffer;
    public BaseRenderer() {
        vertexBuffer = ByteBuffer.allocateDirect(VERTEX.length * 4).order(ByteOrder.nativeOrder()).asFloatBuffer().put(VERTEX);
        vertexBuffer.position(0);
        texVertexBuffer = ByteBuffer.allocateDirect(TEX_VERTEX.length * 4).order(ByteOrder.nativeOrder()).asFloatBuffer().put(TEX_VERTEX);
        texVertexBuffer.position(0);
        indicesBuffer = ByteBuffer.allocateDirect(VERTEX_INDEX.length * 2).order(ByteOrder.nativeOrder()).asShortBuffer().put(VERTEX_INDEX);
        indicesBuffer.position(0);
    }

    public int[] OES_TEXTURE_ID = new int[1];
    public int createOESTexture()
    {
        GLES20.glGenTextures(1, OES_TEXTURE_ID, 0);
        GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, OES_TEXTURE_ID[0]);
        GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER,GLES20.GL_LINEAR);
        GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_NONE);
        return OES_TEXTURE_ID[0];
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {

    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {

    }

    @Override
    public void onDrawFrame(GL10 gl) {

    }
}
