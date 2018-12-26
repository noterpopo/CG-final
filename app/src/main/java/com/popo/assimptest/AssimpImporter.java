package com.popo.assimptest;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;


public class AssimpImporter {
    private Context context;
    private long ptr=-1;

    public AssimpImporter(Context context) {
        this.context = context;
    }

    public ModelData getModelData(String name) {
        ModelData modelData = new ModelData();
        try {
            ptr=modelimporter(modelData, context.getAssets(), name);

        } catch (Exception e) {
            Log.e("JniExceptionHandler", e.toString());
        }
        finally {
            return modelData;
        }
    }
    public AniMartData getAniData(int aniIndex,boolean isLoop,double tis){
        AniMartData aniMartData = new AniMartData();
        try {
            if(getBoneTransform(aniIndex,isLoop,ptr,tis,aniMartData)){
                return aniMartData;
            }

        } catch (Exception e) {
            Log.e("JniExceptionHandler", e.toString());
        }
        finally {
            return aniMartData;
        }
    }
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native long modelimporter(ModelData modelData, AssetManager manager, String filename);
    public native boolean getBoneTransform(int aniIndex,boolean isLoop,long ptr,double tis,AniMartData aniMartData);

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }


}
