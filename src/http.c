#include "guardian.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
static guardian_config G;
static void reply(int c,const char*s,const char*t,const char*b,const char*x){char h[768];int n=(int)strlen(b);snprintf(h,sizeof(h),"HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %d\r\nCache-Control: no-store\r\n%sConnection: close\r\n\r\n",s,t,n,x?x:"");send(c,h,strlen(h),0);send(c,b,n,0);}
static int auth(const char*r){return strstr(r,"Cookie: pg_session=1")!=NULL;}
static void static_file(int c,const char*p,const char*t){FILE*f=fopen(p,"rb");if(!f){reply(c,"404 Not Found","text/plain","Not found",0);return;}fseek(f,0,SEEK_END);long z=ftell(f);rewind(f);char*b=malloc(z+1);fread(b,1,z,f);b[z]=0;fclose(f);reply(c,"200 OK",t,b,0);free(b);}
static void handle(int c){char r[8192]={0};if(recv(c,r,sizeof(r)-1,0)<=0){close(c);return;}char method[8],path[256];sscanf(r,"%7s %255s",method,path);
 if(!strcmp(path,"/manifest.json")){static_file(c,"web/manifest.json","application/manifest+json");close(c);return;}if(!strcmp(path,"/sw.js")){static_file(c,"web/sw.js","application/javascript");close(c);return;}
 if(!strcmp(path,"/api/login")&&!strcmp(method,"POST")){char*b=strstr(r,"\r\n\r\n");b=b?b+4:"";char u[64]={0},p[128]={0};sscanf(b,"username=%63[^&]&password=%127s",u,p);if(!strcmp(u,G.username)&&!strcmp(p,G.password))reply(c,"200 OK","application/json","{\"ok\":true}","Set-Cookie: pg_session=1; Path=/; HttpOnly; SameSite=Strict\r\n");else reply(c,"401 Unauthorized","application/json","{\"ok\":false}",0);close(c);return;}
 if(!strcmp(path,"/api/logout")){reply(c,"200 OK","application/json","{\"ok\":true}","Set-Cookie: pg_session=; Max-Age=0; Path=/; HttpOnly; SameSite=Strict\r\n");close(c);return;}
 if(!strcmp(path,"/api/status")){if(!auth(r)){reply(c,"401 Unauthorized","application/json","{\"error\":\"login required\"}",0);close(c);return;}char j[512];snprintf(j,sizeof(j),"{\"cpu\":%d,\"ram\":%d,\"disk\":%d,\"temp\":%d,\"port\":81}",monitor_cpu(),monitor_ram(),monitor_disk(),monitor_temp());reply(c,"200 OK","application/json",j,0);close(c);return;}
 static_file(c,"web/index.html","text/html; charset=utf-8");close(c);}
static void*watch(void*x){(void)x;for(;;){int cpu=monitor_cpu(),ram=monitor_ram(),disk=monitor_disk(),temp=monitor_temp();if(cpu>95||ram>95||disk>90||temp>80){char m[160];snprintf(m,sizeof(m),"CPU %d%% RAM %d%% disk %d%% temperature %dC",cpu,ram,disk,temp);notify_event("CRITICAL","System pressure",m);}sleep(15);}return 0;}
void guardian_run(const guardian_config*cfg){G=*cfg;pthread_t th;pthread_create(&th,0,watch,0);int s=socket(AF_INET,SOCK_STREAM,0),one=1;setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));struct sockaddr_in a={0};a.sin_family=AF_INET;a.sin_port=htons(cfg->port);a.sin_addr.s_addr=INADDR_ANY;if(bind(s,(struct sockaddr*)&a,sizeof(a))<0||listen(s,32)<0){perror("bind/listen");exit(1);}for(;;){int c=accept(s,0,0);if(c>=0)handle(c);}}
