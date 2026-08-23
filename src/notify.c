#include "guardian.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

static void json_escape(const char *s,char *o,size_t n){size_t j=0;for(size_t i=0;s&&s[i]&&j+2<n;i++){char c=s[i];if(c=='"'||c=='\\'){if(j+2>=n)break;o[j++]='\\';}else if(c=='\n'||c=='\r')c=' ';o[j++]=c;}o[j]=0;}
void notify_event(const char *level,const char *title,const char *message){
    mkdir("/var/lib/pi-guardian",0750);
    FILE *f=fopen("/var/log/pi-guardian.log","a");
    if(f){time_t t=time(NULL);fprintf(f,"%ld [%s] %s: %s\n",(long)t,level,title,message);fclose(f);}
    char a[256],b[768],c[128];json_escape(level,c,sizeof(c));json_escape(title,a,sizeof(a));json_escape(message,b,sizeof(b));
    f=fopen("/var/lib/pi-guardian/latest-event.json","w");
    if(f){fprintf(f,"{\"level\":\"%s\",\"title\":\"%s\",\"body\":\"%s\",\"ts\":%ld}\n",c,a,b,(long)time(NULL));fclose(f);}
    if(level && (!strcmp(level,"CRITICAL") || !strcmp(level,"WARNING"))) push_broadcast(title,message,level);
}
