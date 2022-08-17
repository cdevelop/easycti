#ifndef FSAPI_H_
#define FSAPI_H_

#include "pthread_mutex.h"
#include "cross_platform.h"
#include "log4cplus/loggingmacros.h"
#include "log4cplus/logger.h"

#include "esl.h"
#include "esl_oop.h"


#include <string>

#include "messagequeue.h"


/*!
 * \class FsApi
 *
 * \brief Freeswitch esl接口的抽象
 *
 * \author xu(微信：cdevelop)
 * \date 2015/10/28
 */
class FsApi
{
private:
    FsApi(){}
public:
    FsApi(const char* eslHost,const char* eslUsername,const char* eslPassword,unsigned short eslPort);
    ~FsApi(void);

    bool ready();

    void test(int i);

    void call(const char *job_uuid, const char *url, const char *dest, const char *variable);

    void originate(const char *job_uuid,const char *callername,const char *callernum,const char *called,const char *trunkfromat,const char *variable);
    
    void originate_loopback(int id);

    void hangup(const char *channel_uuid);
    void bridge(const char *src_channel_uuid,const char *dest_channel_uuid);
    void eavesdrop(const char *src_channel_uuid,const char *dest_channel_uuid);
    void park(const char *channel_uuid);
    void answer(const char *channel_uuid);
    void preanswer(const char *channel_uuid);
    void playback(const char *channel_uuid,const char *file,const char *customFlag);

    void _break(const char *channel_uuid);
    void play_and_get_digits(const char *channel_uuid,int min,int max,int tries,int timeout,const char *terminators, const char *file, const char *customFlag);
    void send_dtmf(const char *channel_uuid,const char *dtmf);
    void read(const char *channel_uuid,int min,int max,const char* soundfile,int timeout,const char* terminators, const char *customFlag);

    void record(const char *channel_uuid, const char *file, int time_limit_secs, int silence_hits, const char *customFlag);

    void system(const char *commands);

    void conference(const char *channel_uuid,int confid,const char* flag);
    void conference_kick_all(int confid);

    void hupall();
    void reloadxml();

    void set(const char *channel_uuid, const char* variable_name, const char* variable_value);
    void get(const char *channel_uuid, const char* variable_name);


    void transfer(const char *channel_uuid, const char *dest, const char *dialplan = "XML", const char* context = "default");

    void fsstatus();

    void(*OnReady)(bool ready);

    void(*OnOriginateLoopBackResult)(int id,const char *aleg,const char *bleg);

    void (*OnOriginateResult)(const char *job_uuid,bool result,const char *param);
    enum CALL_PROGRESS{CALL_RINGING,CALL_ANSWER,CALL_HANGUP};
    //CALL_HANGUP 时 param为shoftdata
    void(*OnCallProgress)(const char *channel_uuid, CALL_PROGRESS progress, char *param);
    static const char* printCallProgress(CALL_PROGRESS progress);
    void (*OnCallinEvent)(const char *unique_id,const char *caller_number,const char *destination_number, char*shortdata);

    void(*OnDtmf)(const char *channel_uuid,const char *digit);

    enum APPTYPE
    {
        apptype_none,
        apptype_play_and_get_digits,
        apptype_playback,
        apptype_record,
        apptype_read,
        apptype_send_dtmf,
    };
    void (*OnExecuteCompleteEvent)(esl_event_t *event,APPTYPE type);

    


    static void* workThreadProc(void *param);

private:
    bool eslConnect();
    void eslDisConnect();
    //0(+OK) 1(-ERR) -1(other)
    int getReplyResult(char* replyText,char **reason);

    void api(const char *cmd, const char *arg = NULL);
    void bgapi(const char *cmd, const char *arg=NULL, const char *job_uuid=NULL);

    void app(const char *app, const char *arg, const char *channel_uuid);

    void send(const char *cmd);

    void eslEventProc(esl_event_t *event);

    static void cmdQueueProc(messagequeue<std::string>::MSG *msg,void *p);

private:
    log4cplus::Logger    _logger;
    esl_handle_t         _cmdEslHandle[4];
    esl_handle_t         _evtEslHandle;


    std::string          _eslHost;
    std::string          _eslUsername;
    std::string          _eslPassword;
    unsigned short       _eslPort;

    pthread_t            _workThread;
    volatile bool        _workReady;
    volatile bool        _workExit;

    messagequeue<std::string>*   _cmdQueue;
};

#endif //FSPAI_H_
