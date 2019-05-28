//
// Created by LONGHE001 on 2018/12/24 0024.
//
#include "LogWriter.h"
#include "ErrorInfo.h"
#include <iostream>
#include <sys/file.h>
#include <string.h>
#include <android/log.h>

LogWriter::LogWriter() {}
ErrInfo *LogWriter::initMmap(JNIEnv *env, std::string logDir,std::string filename) {
    this->logDir = logDir;
    this->buildDate = getDate();
    if (filename.empty()) {
        filename= buildDate + "-mmap";
    }
    this->filePath = logDir + "/" + filename;
     const char *fileRealPath = filePath.c_str();
    if (access(fileRealPath,0)==-1){
        this->fd = open(filePath.c_str(), O_RDWR | O_CREAT, (mode_t) 0600);
    } else{
        this->fd = open(filePath.c_str(), O_RDWR | O_APPEND, (mode_t) 0600);
    }

    if (fd == -1) {
        return new ErrInfo(OPEN_EXIT, "Error opening file");
    }
    this->fileStat.st_size = fd;
    if (fstat(fd, &fileStat) == -1) {
        close(fd);
        return new ErrInfo(FSTAT_EXIT, "Error fstat file");
    }

    this->fileSize = fileStat.st_size;
    this->logPageSize = static_cast<off_t >(ALLOC_PAGE_NUM * sysconf(_SC_PAGE_SIZE));

    // If fileSize is not an integer multiple of logPageSize, let it be complemented to an integer multiple of logPageSize
    if (fileSize < logPageSize || fileSize % logPageSize != 0) {

        off_t increaseSize = logPageSize - (fileSize % logPageSize);

        if (ftruncate(fd, fileSize + increaseSize) == -1) {
            close(fd);
            return new ErrInfo(LSEEK_EXIT, "Error when calling ftruncate() to stretch the file");
        }

        fileSize += increaseSize;

        if (lseek(fd, fileSize - 1, SEEK_SET) == -1) {
            close(fd);
            return new ErrInfo(LSEEK_EXIT, "Error calling lseek() to stretch the file");
        }

        if (write(fd, "", sizeof(char)) == -1) {
            close(fd);
            return new ErrInfo(WRITE_EXIT, "Error writing last byte of the file");
        }

    }

    void *map = mmap(NULL, static_cast<size_t>(logPageSize),
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED, fd,
                     fileSize - logPageSize);
    ////////////////////////////////////////////////////////////////////////////////


    if (map == MAP_FAILED || map == NULL) {
        close(fd);
        return new ErrInfo(MMAP_EXIT, "Error mmaping the file");
    }

    recordPtr = static_cast<char *> (map);

    if (recordPtr == NULL) {
        close(fd);
        return new ErrInfo(MMAP_EXIT, "Error cast char*");
    }

    ErrInfo *errInfo = checkMmapFile();
    if (errInfo != NULL) {
        unixMunmap(fd, static_cast<void *>(recordPtr), logPageSize);
        close(fd);
        return errInfo;
    }

    bool findFlag = false;

    for (off_t i = logPageSize - 1; i >= 0; i--) {
        // Find the first '\0' and stop the search, if not found, then the page is still blank, just back to the beginning of the page
        if (recordPtr[i] != '\0') {
            findFlag = true;
            if (i != logPageSize - 1) {
                recordIndex = i + 1;
            } else {
                recordIndex = logPageSize;
            }
            break;
        }
    }
    if (!findFlag) {
        recordIndex = 0;
    }

    memset(recordPtr + recordIndex, 0, static_cast<size_t>(logPageSize - recordIndex));
    return nullptr;
}

/**
 * @param logDir
 */
ErrInfo *LogWriter::init(JNIEnv *env, std::string logDir,std::string filename) {
    return initMmap(env, logDir,filename);
}

LogWriter::~LogWriter() {
    //now write it to disk
    if (msync(recordPtr, static_cast<size_t>(logPageSize), MS_SYNC) == -1) {
        perror("Could not sync the file to disk");
    }
    //Don't forget to free mmapped memory.
    if (munmap(recordPtr, static_cast<size_t>(logPageSize)) == -1) {
        close(fd);
        perror("Error un-mmaping the file");
        exit(EXIT_FAILURE);
    }
    //Un-mapping doesn't close the file, so we still need to do that.
    close(fd);

    buildDate.shrink_to_fit();
    logDir.shrink_to_fit();
    filePath.shrink_to_fit();
}

ErrInfo *LogWriter::writeLog(JNIEnv *env, const char *logMsg) {
    const size_t textSize = strlen(logMsg);
    return writeLog(env, logMsg, textSize);
}

ErrInfo *LogWriter::writeLog(JNIEnv *env, const char *logMsg, size_t textSize) {
    if (logMsg == NULL || textSize <= 0) {
        return nullptr;
    }

    if (recordPtr == NULL) {
        close(fd);
        return new ErrInfo(WRITE_EXIT, "Error writing log");
    }

    ErrInfo *errInfo = checkMmapFile();
    if (errInfo != NULL) {
        unixMunmap(fd, static_cast<void *>(recordPtr), logPageSize);
        close(fd);
        return errInfo;
    }

    size_t msgIndex = 0;

    while (1) {
        for (; msgIndex < textSize && recordIndex < logPageSize; msgIndex++) {
            recordPtr[recordIndex] = logMsg[msgIndex];
            recordIndex++;
        }

        //当开辟的mmap内存被写满时,需要再开辟一页mmap内存
        if (recordIndex >= logPageSize) {

            ErrInfo *errInfo = unixMunmap(fd, recordPtr, (size_t) logPageSize);
            if (errInfo != NULL) {
                close(fd);
                return errInfo;
            }

            recordPtr = NULL;

            if (access(filePath.c_str(), 0) != 0) {
                close(fd);
                return new ErrInfo(ACCESS_EXIT, "Error calling access file");
            }

            //扩展文件大小
            if (ftruncate(fd, fileSize + logPageSize) == -1) {
                close(fd);
                return new ErrInfo(LSEEK_EXIT, "Error calling ftruncate() to stretch file");
            }

            //移动到文件末尾
            if (lseek(fd, fileSize + logPageSize - 1, SEEK_SET) == -1) {
                close(fd);
                return new ErrInfo(LSEEK_EXIT, "Error calling lseek() to stretch the file");
            }

            //在文件末尾写入一个字符，达到扩展文件大小的目的
            if (write(fd, "", 1) == -1) {
                close(fd);
                return new ErrInfo(WRITE_EXIT, "Error writing last byte of the file");
            }

            this->fileStat.st_size = 0;

            if (fstat(fd, &fileStat) == -1) {
                close(fd);
                return new ErrInfo(FSTAT_EXIT, "Error fstat file");
            }

            if (fileStat.st_size - logPageSize != this->fileSize &&
                fileStat.st_size % logPageSize != 0) {
                close(fd);
                return new ErrInfo(WRITE_EXIT, "Error stretch file when writing");
            }

            this->fileSize = fileStat.st_size;

            void *map = mmap(NULL, static_cast<size_t>(logPageSize), PROT_READ | PROT_WRITE,
                             MAP_SHARED, fd,
                             fileSize - logPageSize);

            if (map == MAP_FAILED || map == NULL) {
                close(fd);
                return new ErrInfo(MMAP_EXIT, "Error mmaping the file");
            }

            recordPtr = static_cast<char *> (map);

            if (recordPtr == NULL) {
                close(fd);
                return new ErrInfo(MMAP_EXIT, "Error cast char*");
            }

            memset(recordPtr, 0, static_cast<size_t >(logPageSize));

            recordIndex = 0;
        } else {
            break;
        }
    }

    return nullptr;
}

void *LogWriter::closeAndRenew(JNIEnv *env) {

    //首先取消映射
    ErrInfo *errInfo = unixMunmap(fd, recordPtr, static_cast<size_t >(logPageSize));
    if (errInfo != NULL) {
        close(fd);
        return errInfo;
    }
    recordPtr = NULL;
    //然后关闭文件
    close(fd);
}

std::string LogWriter::getDate() {
    time_t now = time(0);
    tm localTime = *localtime(&now);
    std::string *date;
    size_t bufSize = sizeof(char) * 20;
    char *buf = (char *) malloc(bufSize);
    strftime(buf, bufSize, "%Y-%m-%d", &localTime);
    date = new std::string(buf);
    free(buf);
    return *date;
}

ErrInfo *LogWriter::unixMunmap(int fd, void *map, size_t map_size) {
    if (msync(map, map_size, MS_SYNC) == -1) {
        return new ErrInfo(UNMMAP_EXIT, "Error sync the file to disk");
    }
    if (munmap(map, map_size) == -1) {
        return new ErrInfo(UNMMAP_EXIT, "Error un-mmapping the file");
    }
    return NULL;
}

ErrInfo *LogWriter::checkMmapFile() {
    if (access(filePath.c_str(), 0) != 0) {
        return new ErrInfo(WRITE_EXIT, "Error access log file");
    }
    this->fileStat.st_size = 0;
    if (fstat(fd, &fileStat) == -1 || this->fileStat.st_size != this->fileSize) {
        return new ErrInfo(FSTAT_EXIT, "Error read file size");
    }
    return NULL;
}

void LogWriter::readLog(jlong i1) {
    /*把文件映射成虚拟内存地址*/
    __android_log_print(ANDROID_LOG_INFO,"高级","这是啥啥啥");
    long size = lseek(fd, 0, SEEK_END);
    char *pp= (char *)mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    char *pp1=new char[size];
    memcpy(pp1,pp,size);
    __android_log_print(ANDROID_LOG_INFO,"高级","---------------------------\n   %s",pp1);
}
