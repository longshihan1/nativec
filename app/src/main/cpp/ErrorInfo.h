//
// Created by LONGHE001 on 2018/12/24 0024.
//

#ifndef TROJAN_ERRINFO_H
#define TROJAN_ERRINFO_H

#define OPEN_EXIT -1
#define FSTAT_EXIT -2
#define LSEEK_EXIT -3
#define WRITE_EXIT -4
#define MMAP_EXIT -5
#define UNMMAP_EXIT -6
#define LOCK_EXIT -7
#define UNLOCK_EXIT -8
#define ACCESS_EXIT -9

#define SUCCESS 0

class ErrInfo {

public:
    ErrInfo(int errCode, const char *errMsg);

    const char *errMsg;
    int errCode;
};

#endif //TROJAN_ERRINFO_H
