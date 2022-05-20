package com.we.ccamera.renderer;

import android.opengl.GLES30;
import android.util.Log;

public class GLHelper {

    private static final String TAG = GLHelper.class.getSimpleName();

    public static int compileShaderAndLink(String vertexShader, String fragmentShader)
    {
        int vertexId = compileShader(GLES30.GL_VERTEX_SHADER, vertexShader);
        int fragmentId = compileShader(GLES30.GL_FRAGMENT_SHADER, fragmentShader);

        Log.e(TAG, "vertexId: "+vertexId+", fragmentId: "+fragmentId);

        final int programId = GLES30.glCreateProgram();
        if (programId != 0) {
            //将顶点着色器加入到程序
            GLES30.glAttachShader(programId, vertexId);
            //将片元着色器加入到程序中
            GLES30.glAttachShader(programId, fragmentId);
            //链接着色器程序
            GLES30.glLinkProgram(programId);

            final int[] linkStatus = new int[1];

            GLES30.glGetProgramiv(programId, GLES30.GL_LINK_STATUS, linkStatus, 0);
            if (linkStatus[0] == 0) {
                String logInfo = GLES30.glGetProgramInfoLog(programId);
                Log.e(TAG, "Link program == "+logInfo+"  linkStatus == "+linkStatus[0]);
                GLES30.glDeleteProgram(programId);
                return 0;
            }
            Log.e(TAG, "Link program success, programId: "+programId);
            return programId;
        } else {
            //创建失败
            return 0;
        }
    }

    public static int compileShader(int type, String shader) {
        final int shaderId = GLES30.glCreateShader(type);
        if (shaderId != 0) {
            GLES30.glShaderSource(shaderId, shader);
            GLES30.glCompileShader(shaderId);
            //检测状态
            final int[] compileStatus = new int[1];
            GLES30.glGetShaderiv(shaderId, GLES30.GL_COMPILE_STATUS, compileStatus, 0);
            if (compileStatus[0] == 0) {
                String logInfo = GLES30.glGetShaderInfoLog(shaderId);
                Log.e(TAG, "compileShader: type:"+type+", logInfo:"+logInfo);
                //创建失败
                GLES30.glDeleteShader(shaderId);
                return 0;
            }
            return shaderId;
        } else {
            //创建失败
            return 0;
        }
    }

}