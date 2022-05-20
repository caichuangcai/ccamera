package com.we.ccamera.renderer;

import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.opengl.GLES30;
import android.opengl.GLUtils;
import android.opengl.Matrix;
import android.util.Log;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import static android.opengl.GLES11Ext.GL_TEXTURE_EXTERNAL_OES;


public class CameraRenderer extends BaseRenderer {

    private final String TAG = this.getClass().getSimpleName();

    private IOesTexture oesTextureListener;

    public final String VERTEX_SHADER = "#version 300 es\n" +
            "layout(location=0) in vec3 aPos;\n" +
            "layout(location=1) in vec2 aTexCoord;\n" +
            "uniform mat4 model_matrix;\n" +
            "out vec2 texCoord;" +
            "void main()\n" +
            "{\n" +
            "texCoord = aTexCoord;\n" +
            "gl_Position = model_matrix * vec4(aPos, 1.0f);\n" +
            "}";

    public final String FRAGMENT_SHADER = "#version 300 es\n" +
            "#extension GL_OES_EGL_image_external_essl3 : require\n" +
            "layout(location = 0) out vec4 outColor;\n" +
            "in vec2 texCoord;\n" +
            "uniform samplerExternalOES texture_image;\n" +
            "void main()\n" +
            "{\n" +
            "outColor = texture(texture_image, texCoord);\n" +
            "}\n";

    private final float modelMatrix[] = new float[16];

    public CameraRenderer() {
        super();
    }

    public void setOesTextureListener(IOesTexture listener) {
        this.oesTextureListener = listener;
    }

    private int programId, oesTextureHandle, modelMatrixHandle;
    private SurfaceTexture surfaceTexture;
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        gl.glClearColor(0.5f, 0.5f, 0.5f, 0.5f);

        programId = GLHelper.compileShaderAndLink(VERTEX_SHADER, FRAGMENT_SHADER);

        oesTextureHandle = GLES30.glGetUniformLocation(programId, "texture_image");
        modelMatrixHandle = GLES30.glGetUniformLocation(programId, "model_matrix");

        createOESSurfaceTexture();

        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.rotateM(modelMatrix, 0, -90, 0f, 0.0f, 1f);
    }

    private void createOESSurfaceTexture() {
        int oesTextureId = createOESTexture();
        surfaceTexture = new SurfaceTexture(oesTextureId);
        Log.e(TAG, "oesTextureId: "+oesTextureId);

        oesTextureListener.onCreateOesTexture(surfaceTexture);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        gl.glViewport(0, 0, width, height);

    }

    @Override
    public void onDrawFrame(GL10 gl) {

        if(surfaceTexture != null) {
            surfaceTexture.updateTexImage();
        }

        GLES30.glUseProgram(programId);

        // 顶点
        GLES30.glEnableVertexAttribArray(0);
        GLES30.glVertexAttribPointer(0, 3, GLES30.GL_FLOAT, false, 0, vertexBuffer);

        // 纹理坐标
        GLES30.glEnableVertexAttribArray(1);
        GLES30.glVertexAttribPointer(1, 2, GLES30.GL_FLOAT, false, 0, texVertexBuffer);

        // 旋转矩阵
        GLES20.glUniformMatrix4fv(modelMatrixHandle, 1, false, modelMatrix, 0);

        //绑定纹理
        GLES30.glActiveTexture(GLES30.GL_TEXTURE0 + OES_TEXTURE_ID[0]);
        GLES30.glBindTexture(GL_TEXTURE_EXTERNAL_OES, OES_TEXTURE_ID[0]);
        GLES30.glUniform1i(oesTextureHandle, OES_TEXTURE_ID[0]);

        GLES30.glDrawElements(GLES30.GL_TRIANGLES, VERTEX_INDEX.length, GLES30.GL_UNSIGNED_SHORT, indicesBuffer);
    }

    public interface IOesTexture {

        void onCreateOesTexture(SurfaceTexture surfaceTexture);

    }

}