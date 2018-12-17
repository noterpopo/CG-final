package com.popo.assimptest;


public class ModelData {
    private float[] vertexArray;
    private float[] normalArray;
    private float[] textureCoordArray;
    private short[] indexArray;
    private float[] boneIds;
    private float[] weights;
    private boolean hasAni;

    public boolean isHasAni() {
        return hasAni;
    }

    public void setHasAni(boolean hasAni) {
        this.hasAni = hasAni;
    }

    public float[] getBoneIds() {
        return boneIds;
    }

    public void setBoneIds(float[] boneIds) {
        this.boneIds = boneIds;
    }

    public void setVertexArray(float[] vertexArray) {
        this.vertexArray=vertexArray;
    }

    public void setNormalArray(float[] normalArray) {
        this.normalArray=normalArray;
    }

    public void setTextureCoordArray(float[] texturecoordArray) {
        this.textureCoordArray=texturecoordArray;
    }

    public void setIndexArray(short[] indexArray) {
        this.indexArray=indexArray;
    }

    public float[] getVertexArray() {
        return vertexArray;
    }

    public float[] getNormalArray() {
        return normalArray;
    }

    public float[] getTextureCoordArray() {
        return textureCoordArray;
    }

    public short[] getIndexArray() {
        return indexArray;
    }

    public float[] getWeights() {
        return weights;
    }

    public void setWeights(float[] weights) {
        this.weights = weights;
    }
}
