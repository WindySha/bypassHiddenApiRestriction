package com.wind.example.bypass

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.view.View
import com.wind.hiddenapi.bypass.HiddenApiBypass
import java.lang.Exception

class MainActivity : AppCompatActivity() {
    init {
        System.loadLibrary("test-lib")
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
    }

    fun onClick(view: View) {
        if (view.id == R.id.bypass) {
            HiddenApiBypass.startBypass()
        }
        if (view.id == R.id.bypass_test) {
            try {
                // api_level >= 29
                // $ adb shell settings delete global hidden_api_policy
                val clazz = Class.forName("dalvik.system.VMRuntime")
                val method =
                    clazz.getDeclaredMethod("setHiddenApiExemptions", Array<String>::class.java)
                Log.e(
                    "MainActivity",
                    " clazz = $clazz   exem_method= $method"
                )
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
        if (view.id == R.id.bypass_dlfcn_test) {
            testBypassDlfcn()
        }
    }

    companion object {
        @JvmStatic
        private external fun  testBypassDlfcn()
    }
}