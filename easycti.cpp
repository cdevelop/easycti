#include "easycti.h"


#include "log4cplus/logger.h"
#include "log4cplus/loggingmacros.h"

EasyCti::EasyCti()
: _logger(log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("cti")))
, _mutex(false)
, _fsapi(NULL)
, _ready(false)
{
}


EasyCti::~EasyCti()
{
}

EasyCti& EasyCti::getInstance()
{
    static EasyCti cti;
    return cti;
}


bool EasyCti::start(const char* eslHost, const char* eslUsername, const char* eslPassword, unsigned short eslPort)
{
    if (!_fsapi) {
        _fsapi = new FsApi(eslHost, eslUsername, eslPassword, eslPort);
        _fsapi->OnReady = &OnFsReady;
        _fsapi->OnOriginateLoopBackResult = &OnOriginateLoopBackResult;

        _fsapi->OnOriginateResult = &OnOriginateResult;
        _fsapi->OnCallProgress = &OnCallProgress;
        _fsapi->OnCallinEvent = &OnCallinEvent;

        _fsapi->OnExecuteCompleteEvent = &OnExecuteCompleteEvent;
        _fsapi->OnDtmf = &OnDtmf;
        return  true;
   }
   return false;
}

bool EasyCti::ready()
{
    PthreadMutexScope lock(_mutex);
    return _ready;
}


bool EasyCti::call(const char *uuid,const char *url, const char *dest)
{
    PthreadMutexScope lock(_mutex);

    if (_ready) {

        _fsapi->call(uuid, url ,dest,"");
        return true;

    }

    return false;
}


void EasyCti::OnFsReady(bool ready)
{
    EasyCti &cti = getInstance();
    PthreadMutexScope lock(cti._mutex);
    cti._ready = ready;
    LOG4CPLUS_INFO(cti._logger, __FUNCTION__ << "ready:"<<ready);

}



void EasyCti::OnOriginateLoopBackResult(int id, const char *aleg, const char *bleg)
{
    EasyCti &cti = getInstance();
    PthreadMutexScope lock(cti._mutex);

    if ((aleg == NULL) || (bleg == NULL))
    {
        LOG4CPLUS_ERROR(cti._logger, __FUNCTION__ << "PARAM id:" << id << " param error");
        return;
    }

    LOG4CPLUS_DEBUG(cti._logger, __FUNCTION__ << "PARAM id:" << id << " aleg:" << aleg << " bleg:" << bleg);




}


void EasyCti::OnOriginateResult(const char *job_uuid, bool result, const char *param)
{
    assert(job_uuid && param);
    if (job_uuid == NULL || param == NULL)
        return;

    EasyCti &cti = getInstance();
    PthreadMutexScope lock(cti._mutex);
    LOG4CPLUS_INFO(cti._logger, __FUNCTION__ << "PARAM job_uuid:" << job_uuid << " result:" << result << " param:" << param);


}


void EasyCti::OnCallProgress(const char *channel_uuid, FsApi::CALL_PROGRESS progress, char *param)
{
    EasyCti &pti = getInstance();
    PthreadMutexScope lock(pti._mutex);
    LOG4CPLUS_INFO(pti._logger, __FUNCTION__ << "PARAM channel_uuid:" << channel_uuid << " progress:" << FsApi::printCallProgress(progress) << " param:" << (param ? param : ""));

    switch (progress)
    {
    case FsApi::CALL_RINGING:
    {

    }
    break;
    case FsApi::CALL_ANSWER:
    {


    }
    break;
    case FsApi::CALL_HANGUP:
    {


    }
    break;
    }
}




void EasyCti::OnCallinEvent(const char*channel_uuid, const char *caller_number, const char *destination_number, char*shortdata)
{
    assert(channel_uuid && caller_number && destination_number);
    if (channel_uuid == NULL || caller_number == NULL || destination_number == NULL)
        return;

    EasyCti &pti = getInstance();
    PthreadMutexScope lock(pti._mutex);
    LOG4CPLUS_INFO(pti._logger, __FUNCTION__ << " PARAM channel_uuid:" << channel_uuid << " caller_number:" << caller_number << " destination_number:" << destination_number);

    //转入放音流程 play1流程

    pti._fsapi->transfer(channel_uuid, "play1");

}






void EasyCti::OnExecuteCompleteEvent(esl_event_t *event, FsApi::APPTYPE apptype)
{
    EasyCti &pti = getInstance();
    PthreadMutexScope lock(pti._mutex);

    const char *uuid = esl_event_get_header(event, "Unique-ID");
    if (!uuid)
    {
        return;
    }


}




void EasyCti::OnDtmf(const char *channel_uuid, const char *digit)
{
    EasyCti &pti = getInstance();
    PthreadMutexScope lock(pti._mutex);
    LOG4CPLUS_INFO(pti._logger, __FUNCTION__ << " PARAM channel_uuid:" << channel_uuid << " digit:" << digit);


}
