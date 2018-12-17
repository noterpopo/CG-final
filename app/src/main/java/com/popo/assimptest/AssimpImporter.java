package com.popo.assimptest;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;


public class AssimpImporter {
    private Context context;

    public AssimpImporter(Context context) {
        this.context = context;
    }

    public ModelData getModelData(String name) {
        ModelData modelData = new ModelData();
        try {
            modelimporter(modelData, context.getAssets(), name);

        } catch (Exception e) {
            Log.e("JniExceptionHandler", e.toString());
        }
        finally {
            return modelData;
        }
    }
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native boolean modelimporter(ModelData modelData, AssetManager manager, String filename);
    public native void getBoneTransform(double tis,AniMartData aniMartData);

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }


}
