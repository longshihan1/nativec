//
// Created by LONGHE001 on 2018/12/24 0024.
//

#include "ErrorInfo.h"

ErrInfo::ErrInfo(int errCode, const char *errMsg) {
    this->errCode=errCode;
    this->errMsg=errMsg;
}