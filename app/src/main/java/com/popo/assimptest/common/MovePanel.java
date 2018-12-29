package com.popo.assimptest.common;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

public class MovePanel extends View {
    private Paint paint;
    private float btnX;
    private float btnY;

    private float radiusBack;
    private float radiusFront;

    private float circleX;
    private float circleY;

    private MoveListener moveListener;

    public void setMoveListener(MoveListener moveListener) {
        this.moveListener = moveListener;
    }

    public MovePanel(Context context){
        this(context,null);
    }

    public MovePanel(Context context, AttributeSet attrs){
        this(context,attrs,0);
    }

    public MovePanel(Context context,AttributeSet attrs,int defStyleAttr){
        super(context,attrs,defStyleAttr);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        canvas.drawColor(Color.TRANSPARENT);

        radiusBack=getWidth()/3;
        radiusFront=getWidth()/8;
        circleX=getWidth()/2;
        circleY=getHeight()/2;

        if(paint==null){
            paint=new Paint();
        }
        paint.setAntiAlias(true);
        paint.setStyle(Paint.Style.FILL);
        paint.setColor(0x7f111111);
        canvas.drawCircle(circleX,circleY,radiusBack,paint);
        if(btnX<=0&&btnY<=0){
            paint.setColor(0xFFF10159);
            canvas.drawCircle(circleX,circleY,radiusFront,paint);
            canvas.save();
            return;
        }
        paint.setColor(0xFF744041);
        canvas.drawCircle(btnX,btnY,radiusFront,paint);
        canvas.save();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        switch (event.getAction()){
            case MotionEvent.ACTION_MOVE:
                btnX=event.getX();
                btnY=event.getY();
                if((btnY-circleY)*(btnY-circleY)+(btnX-circleX)*(btnX-circleY)>=radiusBack*radiusBack){
                    moveListener.onMove(getRad(btnX-circleX,btnY-circleY));
                    changeBtnLocation();
                }
                this.invalidate();
                break;
            case MotionEvent.ACTION_UP:
                moveListener.onUp();
                btnX=0;
                btnY=0;
                this.invalidate();
                break;
        }
        return true;
    }


    private void changeBtnLocation(){
        double similarity=Math.sqrt((radiusBack*radiusBack)/((btnY-circleY)*(btnY-circleY)+(btnX-circleX)*(btnX-circleX)));
        btnX=(float) ((btnX-circleX)*similarity+circleX);
        btnY=(float) ((btnY-circleY)*similarity+circleY);
    }

    private float getRad(float deltaX,float deltaY) {
        //算出斜边长
        float hyp = (float) Math.sqrt(Math.pow(deltaX, 2) + Math.pow(deltaY, 2));
        //得到这个角度的余弦值（通过三角函数中的定理 ：邻边/斜边=角度余弦值）
        float cosAngle = deltaY / hyp;
        //通过反余弦定理获取到其角度的弧度
        float rad = (float) Math.acos(cosAngle);
        //注意：当触屏的位置Y坐标<摇杆的Y坐标我们要取反值-0~-180
        if (btnX < circleX) {
            rad = -rad;
        }
        return rad;
    }
}
