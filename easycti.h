#ifndef EASYCTI_H_
#define EASYCTI_H_

#include "fsapi.h"


/*!
 * \class EasyCti
 *
 * \brief 简单的 fs ESL接口CTI例子
 *
 * \author xu(微信：cdevelop)
 * \date 2015/10/28
 */

class EasyCti
{
private:
    EasyCti();
    ~EasyCti();
public:
    static EasyCti& getInstance();

    bool start(const char* eslHost, const char* eslUsername, const char* eslPassword, unsigned short eslPort);
    bool ready();

    bool call(const char *uuid, const char *callerid, const char *flowdata);


    static void OnFsReady(bool ready);
    static void OnOriginateLoopBackResult(int id, const char *aleg, const char *bleg);
    static void OnOriginateResult(const char *job_uuid, bool result, const char *param);
    static void OnCallProgress(const char *channel_uuid, FsApi::CALL_PROGRESS progress, char *param);
    static void OnCallinEvent(const char *channel_uuid, const char *caller_number, const char *destination_number, char*shortdata);

    static void OnExecuteCompleteEvent(esl_event_t *event, FsApi::APPTYPE type);

    static void OnDtmf(const char *channel_uuid, const char *digit);

private:
    PthreadMutex            _mutex;
    log4cplus::Logger       _logger;
    FsApi                  *_fsapi;

    bool                    _ready;
};

#endif