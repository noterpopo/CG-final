package com.popo.assimptest.common;

import java.util.Vector;

public class AniMartData {
    private Vector<float[]> aniMatrixArray=new Vector<>();

    public Vector<float[]> getAniMatrixArray() {
        return aniMatrixArray;
    }

    public void addMatrix(float[] data){
        aniMatrixArray.add(data);
    }
}
