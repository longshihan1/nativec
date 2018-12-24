//
// Created by LONGHE001 on 2018/12/24 0024.
//

#ifndef NATIVEC_LOGWRITER_H
#define NATIVEC_LOGWRITER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <jni.h>
#include <string>
#include <vector>
#include <fstream>
#include "JNIHelp.h"
#include "ErrorInfo.h"

//一次性分配的页数,目前测试取值为1,2,3,4时seekOffset使用logPageSize-1可以
#define ALLOC_PAGE_NUM 40

class LogWriter {
public:
    LogWriter();

    ~LogWriter();

    ErrInfo *init(JNIEnv *, std::string logDir,std::string filename);

    ErrInfo *writeLog(JNIEnv *, const char *logMsg);

    void *closeAndRenew(JNIEnv *env);

private:
    struct stat fileStat;

    int fd;

    off_t fileSize;

    off_t logPageSize;

    std::string buildDate;
    std::string filePath;

    std::string logDir;

    char *recordPtr = NULL;

    off_t recordIndex = 0;

    std::string getDate();

    ErrInfo *initMmap(JNIEnv *, std::string logDir,std::string filename);

    ErrInfo *writeLog(JNIEnv *, const char *logMsg, size_t textSize);

    ErrInfo *unixMunmap(int fd, void *map, size_t map_size);

    ErrInfo *checkMmapFile();

};

#endif //TROJAN_WRITER_H
