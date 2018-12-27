package com.popo.assimptest.common;

public class AnimationHelper {
    public static Animation getAnimation(AnimationType type){
        switch (type){
            case OPEN:
                return new Animation(AnimationType.OPEN);
            case BEFORE_WALK:
                return new Animation(AnimationType.BEFORE_WALK);
            case WALK:
                return new Animation(AnimationType.WALK);
            case AFTER_WALK:
                return new Animation(AnimationType.AFTER_WALK);
            case ATTACK:
                return new Animation(AnimationType.ATTACK);
        }
        return null;
    }

}
