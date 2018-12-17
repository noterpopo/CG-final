package com.popo.assimptest;

import android.content.Context;
import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;

import com.popo.assimptest.common.rendering.ObjectRenderer;

import org.junit.Test;
import org.junit.runner.RunWith;

import static org.junit.Assert.*;

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
@RunWith(AndroidJUnit4.class)
public class ExampleInstrumentedTest {
    @Test
    public void useAppContext() throws Exception{
        // Context of the app under test.
        Context appContext = InstrumentationRegistry.getTargetContext();

        assertEquals("com.popo.assimptest", appContext.getPackageName());

        ObjectRenderer virtualObject=new ObjectRenderer();
        try {
            virtualObject.createOnGlThread(/*context=*/ appContext, "models/man/model.dae", "models/man/diffuse.png");
            virtualObject.testAni();
        }catch (Exception e){
            e.printStackTrace();
            throw new Exception(e);
        }
        virtualObject.setMaterialProperties(0.0f, 2.0f, 0.5f, 6.0f);
    }
}
