package com.popo.assimptest.common;

enum AnimationType{
    OPEN,
    WALK,
    ATTACK
}
public interface Animation {
    boolean isLoop=false;
    boolean isContinue=false;
}
