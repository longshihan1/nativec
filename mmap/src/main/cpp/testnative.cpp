//
// Created by longshihan on 2018/12/23.
//
#include <jni.h>
#include <string>

extern "C"
JNIEXPORT jstring JNICALL
Java_com_longshihan_nativec_TestNative_stringFromJNI(JNIEnv *env, jobject instance) {

    // TODO

    std::string hello = "Hello from cdsdscsdcdscs";
    return  env->NewStringUTF(hello.c_str());
}
