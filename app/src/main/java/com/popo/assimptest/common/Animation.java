package com.popo.assimptest.common;

public class Animation {
    public boolean isLoop=false;
    public boolean isContinue=true;
    public boolean isFirst=true;
    public AnimationType type;
    public float[] walkMartix={1,0,0,0,
                        0,1,0,-1,
                        0,0,1,0,
                        0,0,0,1};

    public Animation(AnimationType type) {
        this.type = type;
        if(type==AnimationType.WALK){
            isLoop=true;
        }
    }
}
