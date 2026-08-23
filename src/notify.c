#include "guardian.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

void notify_event(const char *level,const char *title,const char *message){
    FILE *f=fopen("/var/log/pi-guardian.log","a");
    if(f){time_t t=time(NULL);fprintf(f,"%ld [%s] %s: %s\n",(long)t,level,title,message);fclose(f);}
    if(level && (!strcmp(level,"CRITICAL") || !strcmp(level,"WARNING")))
        push_broadcast(title,message,level);
}
