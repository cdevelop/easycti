

#include "easycti.h"
#include "log4cplus/configurator.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cross_platform.h"

int main()
{
   
#if defined(_WIN32) 
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);
#else
    signal(SIGPIPE, SIG_IGN);
#endif

    log4cplus::initialize();

    try
    {
        log4cplus::ConfigureAndWatchThread configureThread(LOG4CPLUS_TEXT("cti.log4cplus.properties"), 10 * 1000);

        EasyCti &cti = EasyCti::getInstance();
        cti.start("127.0.0.1", "", "ClueCon", 8021);

        for (;;) {

            if (!cti.ready()) {
                cross_sleep(2000);
            }
            

            char callerid[256];
            printf("\n请输入被叫号码:\n");
            gets_s(callerid,sizeof(callerid));
            char sipuri[1024];
            //sprintf(sipuri, "sofia/gateway/gw/%s", callerid);
            //sprintf(sipuri, "sofia/external/%s@sipip", callerid);
            sprintf(sipuri, "user/%s", callerid);

            char flowdata[256];
            printf("请输入接通后的流程:\n");
            gets_s(flowdata, sizeof(flowdata));


            cti.call("uniqueID", sipuri, flowdata);


            printf("执行中请等待\n");
            getchar();

        }


    }
    catch (...) {
        printf("main exception");
    }


#if defined(_WIN32) 
    WSACleanup();
#endif 
    return 0;
}

