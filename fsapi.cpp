

#include "fsapi.h"


const char LOOPBACK_JOB_UUID_PREFIX[] = "LOOPBACK_";


static const char *cut_path(const char *in)
{
    const char *p, *ret = in;
    char delims[] = "/\\";
    char *i;

    for (i = delims; *i; i++) {
        p = in;
        while ((p = strchr(p, *i)) != 0) {
            ret = ++p;
        }
    }
    return ret;
}


static void default_logger(const char *file, const char *func, int line, int level, const char *fmt, ...)
{
    static log4cplus::Logger logger= log4cplus::Logger::getInstance("esl.evt");
    const char *fp;
    char *data;
    va_list ap;
    int ret;
    fp = cut_path(file);

    va_start(ap, fmt);

    log4cplus::LogLevel logLevel = log4cplus::ALL_LOG_LEVEL;
    switch (level)
    {
    case ESL_LOG_LEVEL_DEBUG:       logLevel = log4cplus::TRACE_LOG_LEVEL;    break;
    case ESL_LOG_LEVEL_INFO:        logLevel = log4cplus::DEBUG_LOG_LEVEL;     break;
    case ESL_LOG_LEVEL_NOTICE:      logLevel = log4cplus::INFO_LOG_LEVEL;     break;
    case ESL_LOG_LEVEL_WARNING:     logLevel = log4cplus::WARN_LOG_LEVEL;     break;
    case ESL_LOG_LEVEL_ERROR:       logLevel = log4cplus::ERROR_LOG_LEVEL;    break;
    case ESL_LOG_LEVEL_CRIT:        logLevel = log4cplus::FATAL_LOG_LEVEL;    break;
    case ESL_LOG_LEVEL_ALERT:       logLevel = log4cplus::FATAL_LOG_LEVEL;    break;
    case ESL_LOG_LEVEL_EMERG:       logLevel = log4cplus::FATAL_LOG_LEVEL;    break;
    }

    LOG4CPLUS_SUPPRESS_DOWHILE_WARNING()

    if (logger.isEnabledFor(logLevel))
    {
        ret = esl_vasprintf(&data, fmt, ap);

        if (ret != -1) {
            //fprintf(stderr, "[%s] %s:%d %s() %s", LEVEL_NAMES[level], fp, line, func, data);
            log4cplus::detail::macro_forced_log(logger, logLevel, data, fp, line, func);
            free(data);
        }
    }
    LOG4CPLUS_RESTORE_DOWHILE_WARNING()


    va_end(ap);

}


FsApi::FsApi(const char* eslHost, const char* eslUsername, const char* eslPassword, unsigned short eslPort)
    :_logger(log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("esl.api")))
    , _workReady(false)
    , _workExit(false)
{

    _cmdQueue = new messagequeue<std::string>(cmdQueueProc, this);

    OnReady = NULL;
    OnOriginateResult = NULL;
    OnCallProgress = NULL;
    OnCallinEvent = NULL;
    OnExecuteCompleteEvent = NULL;
    OnOriginateLoopBackResult = NULL;
    OnDtmf = NULL;

    //_eslHandle.event_lock = true;

    _eslHost = eslHost;
    _eslUsername = eslUsername;
    _eslPassword = eslPassword;
    _eslPort = eslPort;

    esl_global_set_default_logger(0);
    esl_global_set_logger(default_logger);
    pthread_create(&_workThread, NULL, &workThreadProc, this);


}


FsApi::~FsApi(void)
{
    _workExit = true;
    if (_workReady)
    {
        for (int i = 0;i < sizeof(_cmdEslHandle) / sizeof(esl_handle_t);++i)
        {
            esl_send(&_cmdEslHandle[i], "exit");
        }

        esl_send(&_evtEslHandle, "exit");
    }
    void *exitCode;
    pthread_join(_workThread, &exitCode);
    delete _cmdQueue;

}


bool FsApi::ready()
{
    return _workReady;
}

void FsApi::test(int i)
{

    cross_sleep(1000);

}

void FsApi::call(const char *job_uuid, const char *url, const char *dest, const char *variable)
{
    assert(job_uuid);
    assert(url);
    assert(dest);
    assert(variable);
    

    char arg[1024] = "\0";
    snprintf(arg, sizeof(arg) - 1, "{originate_job_uuid=%s,%s}%s %s", job_uuid, variable, url, dest);
    bgapi("originate", arg, job_uuid);
}

void FsApi::originate(const char *job_uuid, const char *callername, const char *callernum, const char *called, const char *trunkfromat,const char *variable)
{
    assert(job_uuid);
    assert(called);
    assert(called);
    assert(trunkfromat);
    const char* addr=strstr(trunkfromat, "%s");
    if (addr==NULL)
    {
        return;
    }
    else if(strstr(addr+2,"%s"))
    {
        return;
    }


    char line[512]="\0";
    snprintf(line,sizeof(line)-1,trunkfromat, called);

    char arg[1024]="\0";
    snprintf(arg, sizeof(arg)-1, "{park_after_bridge=true,uuid_bridge_park_on_cancel=true,originate_job_uuid=%s,origination_caller_id_name=%s,origination_caller_id_number=%s,sip_contact_user=%s,%s}%s &park", job_uuid,callername,callernum,callernum,variable,line);
    bgapi("originate", arg, job_uuid);
}

void FsApi::originate_loopback(int id)
{
    char jobUuid[64]="\0";
    sprintf(jobUuid, "%s%d", LOOPBACK_JOB_UUID_PREFIX,id);
    char arg[1024] = "\0";
    snprintf(arg, sizeof(arg) - 1, "{park_after_bridge=true,uuid_bridge_park_on_cancel=true,park_after_early_bridge=true,continue_on_fail=true,originate_job_uuid=%s}loopback/answer\\,park/default/inline answer,park inline",jobUuid);

    //variable_other_loopback_leg_uuid
    bgapi("originate",arg , jobUuid);
}


void FsApi::hangup(const char *channel_uuid)
{
    assert(channel_uuid);
    api("uuid_kill", channel_uuid);
}


void FsApi::bridge(const char *src_channel_uuid, const char *dest_channel_uuid)
{
    assert(src_channel_uuid);
    assert(dest_channel_uuid);
    char arg[256] = "\0";
    snprintf(arg, 255, "%s %s", src_channel_uuid, dest_channel_uuid);
    api("uuid_bridge", arg);
}

void FsApi::eavesdrop(const char *src_channel_uuid, const char *dest_channel_uuid)
{
    app("eavesdrop", dest_channel_uuid, src_channel_uuid);
}

void FsApi::park(const char *channel_uuid)
{
    api("uuid_park", channel_uuid);
}



void FsApi::answer(const char *channel_uuid)
{
    api("uuid_answer", channel_uuid);
}

void FsApi::preanswer(const char *channel_uuid)
{
    api("uuid_pre_answer", channel_uuid);
}

void FsApi::playback(const char *channel_uuid, const char *file, const char *customFlag)
{
    //playback_terminators=none
    char arg[4096]="\0";
    snprintf(arg,sizeof(arg)-1, "{%s}%s", customFlag, file);
    app("playback", arg, channel_uuid);
}

void FsApi::_break(const char *channel_uuid)
{
    //app("break","",channel_uuid);
    api("uuid_break", channel_uuid);
}

void FsApi::play_and_get_digits(const char *channel_uuid, int min, int max, int tries, int timeout, const char *terminators, const char *file,const char *customFlag)
{
    char arg[512] = "\0";
    snprintf(arg, sizeof(arg)-1, "{%s}%d %d %d %d '%s' '%s' '' read_dtmf", customFlag,min, max, tries, timeout, terminators, file);
    app("play_and_get_digits", arg, channel_uuid);
}


void FsApi::send_dtmf(const char *channel_uuid, const char *dtmf)
{
    app("send_dtmf", dtmf, channel_uuid);
}

void FsApi::read(const char *channel_uuid, int min, int max, const char* soundfile, int timeout, const char* terminators, const char *customFlag)
{
    char arg[512] = "\0";
    snprintf(arg, sizeof(arg)-1, "{%s}%d %d '%s' read_dtmf %d '%s'", customFlag,min, max, soundfile, timeout, terminators);
    app("read", arg, channel_uuid);
}

void FsApi::record(const char *channel_uuid, const char *file, int time_limit_secs, int silence_hits, const char *customFlag)
{
    char arg[512] = "\0";
    if (silence_hits>0)
    {
        snprintf(arg, sizeof(arg) - 1, "{%s}'%s' %d 200 %d", customFlag, file, time_limit_secs, silence_hits);
    }
    else
    {
        snprintf(arg, sizeof(arg) - 1, "{%s}'%s' %d 0 7200", customFlag, file, time_limit_secs);
    }
    app("record", arg, channel_uuid);
}

void FsApi::system(const char *commands)
{
    api("system", commands);
}

void FsApi::conference(const char *channel_uuid, int confid, const char* flag)
{
    char arg[512] = "\0";
    snprintf(arg, sizeof(arg) - 1, "%d@cti++flags{%s}",confid,flag);
    app("conference", arg, channel_uuid);
}

void FsApi::conference_kick_all(int confid)
{
    char arg[512] = "\0";
    snprintf(arg, sizeof(arg) - 1, "%d kick all", confid);
    api("conference", arg);
}

void FsApi::hupall()
{
    api("hupall");
}

void FsApi::reloadxml()
{
    api("reloadxml");
}




void FsApi::set(const char *channel_uuid, const char* variable_name, const char* variable_value)
{
    char arg[512] = "\0";
    snprintf(arg, sizeof(arg) - 1, "%s %s %s", channel_uuid, variable_name,variable_value);
    api("uuid_setvar", arg);
}
void FsApi::get(const char *channel_uuid, const char* variable_name)
{
    char arg[512] = "\0";
    snprintf(arg, sizeof(arg) - 1, "%s %s", channel_uuid,variable_name);

    api("uuid_getvar", arg);
}

void FsApi::transfer(const char *channel_uuid, const char *dest, const char *dialplan , const char* context)
{
    char arg[512] = "\0";
    snprintf(arg, sizeof(arg) - 1, "%s %s %s", dest,dialplan,context);
    app("transfer", arg, channel_uuid);

}


void FsApi::fsstatus()
{
    api("status");
}

const char* FsApi::printCallProgress(CALL_PROGRESS progress)
{
    switch (progress)
    {
    case FsApi::CALL_RINGING:
        return "CALL_RINGING";
    case FsApi::CALL_ANSWER:
        return "CALL_ANSWER";
    case FsApi::CALL_HANGUP:
        return "CALL_HANGUP";
    }
    return "UNKNOWN";
}


void* FsApi::workThreadProc(void *param)
{
    FsApi* fsApi = (FsApi*)param;
    for (;;)
    {
        if (fsApi->eslConnect())
        {
            if (fsApi->_workExit)
                break;

            esl_events(&(fsApi->_evtEslHandle), ESL_EVENT_TYPE_PLAIN, "CHANNEL_EXECUTE_COMPLETE BACKGROUND_JOB CHANNEL_ORIGINATE CHANNEL_PROGRESS_MEDIA CHANNEL_PROGRESS CHANNEL_ANSWER CHANNEL_HANGUP HEARTBEAT DTMF CUSTOM callin");

            fsApi->_workReady = true;
            if (fsApi->OnReady)
                fsApi->OnReady(true);


            for (;;)
            {
                if (!fsApi->_workReady)
                {
                    break;
                }

                esl_status_t status = esl_recv_event_timed(&(fsApi->_evtEslHandle), 40000, 1, NULL);
                if (status == ESL_SUCCESS)
                {
                    esl_event_t *e = fsApi->_evtEslHandle.last_ievent ? fsApi->_evtEslHandle.last_ievent : fsApi->_evtEslHandle.last_event;
                    fsApi->eslEventProc(e);
                    continue;
                }
                else if (status == ESL_BREAK)
                {
                    LOG4CPLUS_WARN(fsApi->_logger, __FUNCTION__ << " evtEslHandle esl_recv_event_timed timeout");
                }
                break;
            }
        }

        fsApi->_workReady = false;

        if (fsApi->OnReady)
            fsApi->OnReady(false);

        if (fsApi->_workExit)
            break;

        fsApi->eslDisConnect();
        cross_sleep(5000);
    }

    fsApi->eslDisConnect();

    return 0;
}



bool FsApi::eslConnect()
{
    LOG4CPLUS_INFO(_logger, __FUNCTION__ << "eslHost:" << _eslHost << " eslPort:" << _eslPort << " eslUsername:" << _eslUsername << " eslPassword:" << _eslPassword);
    _evtEslHandle.destroyed = 0;
    esl_status_t status = esl_connect_timeout(&_evtEslHandle, _eslHost.c_str(), _eslPort, _eslUsername.c_str(), _eslPassword.c_str(), 3000);
    if (status == ESL_SUCCESS)
    {
        int i = 0;
        bool failed = false;
        for (;i < sizeof(_cmdEslHandle) / sizeof(esl_handle_t);++i)
        {
            _cmdEslHandle[i].destroyed = 0;
            status = esl_connect_timeout(&_cmdEslHandle[i], _eslHost.c_str(), _eslPort, _eslUsername.c_str(), _eslPassword.c_str(), 3000);
            if (status != ESL_SUCCESS)
            {
                LOG4CPLUS_WARN(_logger, __FUNCTION__ << " cmdEslHandle:"<<i<<" last_reply:" << _cmdEslHandle[i].last_reply << " err:" << _cmdEslHandle[i].err);
                failed = true;
                break;
            }
        }
        if (failed)
        {
            for (int e = 0;e <= i;++e)
                esl_disconnect(&_cmdEslHandle[e]);
        }
        else
        {
            LOG4CPLUS_INFO(_logger, __FUNCTION__ << " esl connected ");
            return true;
        }

    }
    else
    {
        LOG4CPLUS_WARN(_logger, __FUNCTION__ << "evtEslHandle last_reply:" << _evtEslHandle.last_reply << " err:" << _evtEslHandle.err);
    }
    esl_disconnect(&_evtEslHandle);
    return false;
}

void FsApi::eslDisConnect()
{
    LOG4CPLUS_INFO(_logger, __FUNCTION__);
    for (int i = 0;i < sizeof(_cmdEslHandle)/sizeof(esl_handle_t);++i)
    {
        esl_disconnect(&_cmdEslHandle[i]);
    }
    esl_disconnect(&_evtEslHandle);
}

int FsApi::getReplyResult(char *replyText, char **reason)
{
    char *result = replyText;
    *reason = NULL;
    while (*replyText)
    {
        if (*reason == NULL)
        {
            if (*replyText == ' ' || *replyText == '\n')
            {
                *replyText = 0;
                *reason = replyText + 1;
                break;
            }
        }
        ++replyText;
    }
    if (*reason)
    {
        int len = strlen(*reason);
        if (len > 0)
        {
            len -= 1;
            if ((*reason)[len] == '\n')
            {
                (*reason)[len] = 0;
            }
        }
    }
    if (strncmp(result, "+OK", 3) == 0)
    {
        return 0;
    }
    else if (strncmp(result, "-ERR", 4) == 0)
    {
        return 1;
    }

    return -1;
}

void FsApi::api(const char *cmd, const char *arg)
{
    char cmd_buf[1024] = "\0";
    snprintf(cmd_buf, sizeof(cmd_buf), "api %s %s\n\n", cmd, arg ? arg : "");
    send(cmd_buf);
}

void FsApi::bgapi(const char *cmd, const char *arg, const char *job_uuid)
{
    char cmd_buf[1024] = "\0";
    if (job_uuid) {
        snprintf(cmd_buf, sizeof(cmd_buf), "bgapi %s%s%s\nJob-UUID: %s\n\n", cmd, arg ? " " : "", arg ? arg : "", job_uuid);
    }
    else {
        snprintf(cmd_buf, sizeof(cmd_buf), "bgapi %s%s%s\n\n", cmd, arg ? " " : "", arg ? arg : "");
    }
    send( cmd_buf);
}

void FsApi::app(const char *app, const char *arg, const char *channel_uuid)
{
    char cmd_buf[128] = "sendmsg";
    char app_buf[512] = "";
    char arg_buf[4096] = "";
    const char *el_buf = "event-lock: true\n";
    const char *bl_buf = "async: true\n";
    char send_buf[5120] = "";

    if (channel_uuid) {
        snprintf(cmd_buf, sizeof(cmd_buf), "sendmsg %s", channel_uuid);
    }

    if (app) {
        snprintf(app_buf, sizeof(app_buf), "execute-app-name: %s\n", app);
    }

    if (arg) {
        snprintf(arg_buf, sizeof(arg_buf), "execute-app-arg: %s\n", arg);
    }

    snprintf(send_buf, sizeof(send_buf), "%s\ncall-command: execute\n%s%s%s%s\n",
        cmd_buf, app_buf, arg_buf, _cmdEslHandle[0].event_lock ? el_buf : "", _cmdEslHandle[0].async_execute ? bl_buf : "");

    send(send_buf);

}

void FsApi::send(const char *cmd)
{
    std::string eslcommand(cmd);
    _cmdQueue->post(&eslcommand);
}


void FsApi::cmdQueueProc(messagequeue<std::string>::MSG *msg, void *p)
{
    FsApi *pThis = (FsApi*)p;
    if (!pThis->ready())
    {
        return;
    }

    static unsigned int  count = 0;

    int index = count++%(sizeof(pThis->_cmdEslHandle)/sizeof(esl_handle_t));

    LOG4CPLUS_DEBUG(pThis->_logger, __FUNCTION__<<" send:"<<msg->data);
    int r = ::send(pThis->_cmdEslHandle[index].sock, msg->data.c_str(), msg->data.length(),0);
    if (r != -1)
    {
        for (;;)
        {

            int activity = esl_wait_sock(pThis->_cmdEslHandle[index].sock, 0, (esl_poll_t)(ESL_POLL_READ));
            if (activity == 0)
            {
                break;
            }
            else if (activity > 0) {
                char buf[8192];
                activity =recv(pThis->_cmdEslHandle[index].sock, buf, sizeof(buf)-1, 0);
                if (activity > 0)
                {
                    buf[activity] = 0;
                    LOG4CPLUS_DEBUG(pThis->_logger, __FUNCTION__ << " recv:" << buf);
                    if(activity==sizeof(buf))
                        continue;
                    else
                        break;
                }
            }
            LOG4CPLUS_WARN(pThis->_logger, __FUNCTION__ << "cmd:" << msg->data << " activity:" << activity);
            pThis->_workReady = false;
            break;
        }

    }
    else
    {
        LOG4CPLUS_WARN(pThis->_logger, __FUNCTION__ << "cmd:" << msg->data << " r:" << r);
        pThis->_workReady = false;
    }
}

void FsApi::eslEventProc(esl_event_t *event )
{


    bool hit = false;
    switch (event->event_id)
    {

    case ESL_EVENT_CHANNEL_EXECUTE_COMPLETE:
    {
        const char *application = esl_event_get_header(event, "Application");
        if (application)
        {
            APPTYPE type = apptype_none;
            if ((strcmp(application, "play_and_get_digits") == 0))
            {
                type = apptype_play_and_get_digits;
            }
            else if ((strcmp(application, "playback") == 0))
            {
                type = apptype_playback;
            }
            else if ((strcmp(application, "record") == 0))
            {
                type = apptype_record;
            }
            else if ((strcmp(application, "read") == 0))
            {
                type = apptype_read;
            }
            else if ((strcmp(application, "send_dtmf") == 0))
            {
                type = apptype_send_dtmf;
            }

            if (type != apptype_none && OnExecuteCompleteEvent)
            {
                OnExecuteCompleteEvent(event, type);
                hit = true;
            }

        }

    }
    break;
    case ESL_EVENT_BACKGROUND_JOB:
    {
        const char *job_uuid = esl_event_get_header(event, "Job-UUID");
        const char *job_command = esl_event_get_header(event, "Job-Command");
        const char *job_command_arg = esl_event_get_header(event, "Job-Command-Arg");
        if (job_uuid && job_command)
        {
            char *body = event->body;
            char *reason;
            int r = getReplyResult(body, &reason);
            if (!esl_safe_strcasecmp(job_command, "originate"))
            {

                if (strncmp(job_uuid, LOOPBACK_JOB_UUID_PREFIX, sizeof(LOOPBACK_JOB_UUID_PREFIX) - 1))
                {
                    if (r == 1 && OnOriginateResult)
                    {
                        OnOriginateResult(job_uuid, false, reason);
                        hit = true;
                    }
                }
            }
        }
    }
    break;
    case ESL_EVENT_CHANNEL_ORIGINATE:
    {
        const char* originate_job_uuid = esl_event_get_header(event, "variable_originate_job_uuid");
        const char* unique_id = esl_event_get_header(event, "Unique-ID");
        if (unique_id && originate_job_uuid)
        {
            if (strncmp(originate_job_uuid, LOOPBACK_JOB_UUID_PREFIX, sizeof(LOOPBACK_JOB_UUID_PREFIX) - 1))
            {
                if (OnOriginateResult)
                {
                    OnOriginateResult(originate_job_uuid, true, unique_id);
                    hit = true;
                }
            }
            else
            {
                const char* variable_other_loopback_leg_uuid = esl_event_get_header(event, "variable_other_loopback_leg_uuid");

                if (OnOriginateLoopBackResult)
                {
                    OnOriginateLoopBackResult(strtol(originate_job_uuid + sizeof(LOOPBACK_JOB_UUID_PREFIX) - 1, NULL, 10), unique_id, variable_other_loopback_leg_uuid);
                    hit = true;
                }
            }
        }

    }
    break;
    case ESL_EVENT_CHANNEL_PROGRESS_MEDIA:
    case ESL_EVENT_CHANNEL_PROGRESS:
    {
        const char* unique_id = esl_event_get_header(event, "Unique-ID");
        if (unique_id)
        {
            if (OnCallProgress)
            {
                OnCallProgress(unique_id, CALL_RINGING, NULL);
                hit = true;
            }
        }
    }
    break;
    case ESL_EVENT_CHANNEL_ANSWER:
    {
        const char* unique_id = esl_event_get_header(event, "Unique-ID");
        if (unique_id)
        {
            if (OnCallProgress)
            {
                OnCallProgress(unique_id, CALL_ANSWER, NULL);
                hit = true;
            }
        }
    }
    break;
    case ESL_EVENT_CHANNEL_HANGUP:
    {
        const char* unique_id = esl_event_get_header(event, "Unique-ID");
        if (unique_id)
        {
            const char *sip_bye_content_type = esl_event_get_header(event, "variable_sip_bye_content_type");
            const char *sip_bye_payload = esl_event_get_header(event, "variable_sip_bye_payload");

            const char *shortdta = NULL;
            if (sip_bye_content_type && (strcmp(sip_bye_content_type,"application/shortdata") == 0))
            {
                shortdta = sip_bye_payload;
            }

            if (OnCallProgress)
            {
                OnCallProgress(unique_id, CALL_HANGUP, (char*)shortdta);
                hit = true;
            }
        }
    }
    break;

    case ESL_EVENT_DTMF:
    {
        const char* unique_id = esl_event_get_header(event, "Unique-ID");
        if (unique_id)
        {
            const char *digit = esl_event_get_header(event, "DTMF-Digit");
            if (OnDtmf && digit)
            {
                OnDtmf(unique_id, digit);
            }
        }

    }
    break;

    case ESL_EVENT_CUSTOM:
    {
        const char* unique_id = esl_event_get_header(event, "Unique-ID");
        const char* event_subclass = esl_event_get_header(event, "Event-Subclass");
        const char* caller_number = esl_event_get_header(event, "Caller-Caller-ID-Number");
        const char* destination_number = esl_event_get_header(event, "Caller-Destination-Number");
        char* user_to_user = esl_event_get_header(event, "variable_sip_h_User-to-User");

        if (unique_id && event_subclass)
        {
            if (strcmp(event_subclass, "callin") == 0)
            {
                char* shortdata = NULL;
                if (user_to_user && strncmp(user_to_user, "shortdata=", 10)==0)
                {
                    shortdata = user_to_user + 10;
                }
                if (OnCallinEvent && caller_number && destination_number)
                {
                    OnCallinEvent(unique_id, caller_number, destination_number, shortdata);
                    hit = true;
                }
            }
        }

    }

    }

}