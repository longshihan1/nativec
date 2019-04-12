#include <jni.h>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include "JNIHelp.h"
#include "LogWriter.h"


#ifdef __cplusplus
extern "C" {
#endif

std::string jstring2string(JNIEnv *env, jstring jStr) {
    const jclass stringClass = env->GetObjectClass(jStr);
    const jmethodID getBytes = env->GetMethodID(stringClass, "getBytes", "(Ljava/lang/String;)[B");
    const jstring utf = env->NewStringUTF("UTF-8");
    const jbyteArray stringJbytes = (jbyteArray) env->CallObjectMethod(jStr, getBytes, utf);

    size_t length = (size_t) env->GetArrayLength(stringJbytes);
    jbyte *pBytes = env->GetByteArrayElements(stringJbytes, NULL);

    std::string ret = std::string((char *) pBytes, length);

    env->DeleteLocalRef(stringClass);
    env->DeleteLocalRef(utf);
    env->ReleaseByteArrayElements(stringJbytes, pBytes, JNI_COMMIT);

    return ret;
}


jlong Java_com_longshihan_mmap_MMAP_nativeInit(JNIEnv *env, jobject instance,
                                                  jstring dir,
                                                  jstring filename) {
    //////////////////////////////////////
    std::string methodName = "nativeInit";
    print(env, methodName.c_str());
    ////////////////////////////////////
    LogWriter *logWriter = new LogWriter();
    std::string logDir;
    if (dir != NULL) {
        logDir = jstring2string(env, dir);
        env->DeleteLocalRef(dir);
    }
    std::string currentfilename;
    if (filename != NULL) {
        currentfilename = jstring2string(env, filename);
        env->DeleteLocalRef(filename);
    }
    ErrInfo *errInfo = logWriter->init(env, logDir,currentfilename);
    if (errInfo != NULL) {
        throwExceptionIfNeed(env, errInfo);
        delete errInfo;
        errInfo = NULL;
    }
    return reinterpret_cast<jlong>(logWriter);
}


jlong Java_com_longshihan_mmap_MMAP_nativeWrite(JNIEnv *env, jobject instance,
                                                   jlong log_writer_object,
                                                   jstring msg_content) {
    LogWriter *logWriter = reinterpret_cast<LogWriter *>(log_writer_object);
    if (msg_content != NULL) {
        const char *msg = env->GetStringUTFChars(msg_content, JNI_FALSE);
        ErrInfo *errInfo = logWriter->writeLog(env, msg);
        env->ReleaseStringUTFChars(msg_content, msg);
        if (errInfo != NULL) {
            throwExceptionIfNeed(env, errInfo);
            delete errInfo;
            errInfo = NULL;
        }
    }
    return reinterpret_cast<jlong>(logWriter);
}

void Java_com_longshihan_mmap_MMAP_nativeCloseAndRenew(JNIEnv *env, jobject instance,
                                                           jlong logWriterObject) {
    std::string methodName = "nativeCloseAndRenew";
    print(env, methodName.c_str());
    LogWriter *logWriter = reinterpret_cast<LogWriter *>(logWriterObject);
    logWriter->closeAndRenew(env);
}

void Java_com_longshihan_mmap_MMAP_nativeReadLog(JNIEnv *env, jclass type, jlong logWriterObject) {
    LogWriter *logWriter = reinterpret_cast<LogWriter *>(logWriterObject);
    logWriter->readLog(logWriterObject);
}

#ifdef __cplusplus
};
#endif