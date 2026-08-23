#include "guardian.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
static guardian_config G;
static void reply(SSL*c,const char*s,const char*t,const char*b,const char*x){char h[1024];int n=(int)strlen(b);snprintf(h,sizeof(h),"HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %d\r\nCache-Control: no-store\r\nStrict-Transport-Security: max-age=31536000\r\n%sConnection: close\r\n\r\n",s,t,n,x?x:"");SSL_write(c,h,strlen(h));SSL_write(c,b,n);}
static int auth(const char*r){return strstr(r,"Cookie: pg_session=1")!=NULL;}
static void static_file(SSL*c,const char*p,const char*t){FILE*f=fopen(p,"rb");if(!f){reply(c,"404 Not Found","text/plain","Not found",0);return;}fseek(f,0,SEEK_END);long z=ftell(f);rewind(f);char*b=malloc(z+1);if(!b){fclose(f);reply(c,"500 Internal Server Error","text/plain","OOM",0);return;}size_t n=fread(b,1,z,f);b[n]=0;fclose(f);reply(c,"200 OK",t,b,0);free(b);}
static int endpoint_from_json(const char*b,char*out,size_t n){const char*k=strstr(b,"\"endpoint\"");if(!k)return -1;k=strchr(k,':');if(!k)return -1;k++;while(*k&&(*k==' '||*k=='\"'))k++;size_t j=0;while(*k&&*k!='\"'&&j+1<n){if(*k=='\\'&&k[1])k++;out[j++]=*k++;}out[j]=0;return j?0:-1;}
static void handle(SSL*c){char r[16384]={0};if(SSL_read(c,r,sizeof(r)-1)<=0)return;char method[8],path[256];sscanf(r,"%7s %255s",method,path);
 if(!strcmp(path,"/manifest.json")){static_file(c,"web/manifest.json","application/manifest+json");return;}if(!strcmp(path,"/sw.js")){static_file(c,"web/sw.js","application/javascript");return;}if(!strcmp(path,"/icon.svg")){static_file(c,"web/icon.svg","image/svg+xml");return;}
 if(!strcmp(path,"/api/login")&&!strcmp(method,"POST")){char*b=strstr(r,"\r\n\r\n");b=b?b+4:"";char u[64]={0},p[128]={0};sscanf(b,"username=%63[^&]&password=%127s",u,p);if(!strcmp(u,G.username)&&!strcmp(p,G.password))reply(c,"200 OK","application/json","{\"ok\":true}","Set-Cookie: pg_session=1; Path=/; HttpOnly; Secure; SameSite=Strict\r\n");else reply(c,"401 Unauthorized","application/json","{\"ok\":false}",0);return;}
 if(!strcmp(path,"/api/logout")){reply(c,"200 OK","application/json","{\"ok\":true}","Set-Cookie: pg_session=; Max-Age=0; Path=/; HttpOnly; Secure; SameSite=Strict\r\n");return;}
 if(!auth(r)){reply(c,"401 Unauthorized","application/json","{\"error\":\"login required\"}",0);return;}
 if(!strcmp(path,"/api/status")){char j[512];snprintf(j,sizeof(j),"{\"cpu\":%d,\"ram\":%d,\"disk\":%d,\"temp\":%d,\"port\":81,\"https\":true}",monitor_cpu(),monitor_ram(),monitor_disk(),monitor_temp());reply(c,"200 OK","application/json",j,0);return;}
 if(!strcmp(path,"/api/push/vapid-public-key")){char key[128]={0};if(push_public_key(key,sizeof(key))<0)reply(c,"500 Internal Server Error","application/json","{\"error\":\"vapid unavailable\"}",0);else{char j[192];snprintf(j,sizeof(j),"{\"publicKey\":\"%s\"}",key);reply(c,"200 OK","application/json",j,0);}return;}
 if(!strcmp(path,"/api/push/subscribe")&&!strcmp(method,"POST")){char*b=strstr(r,"\r\n\r\n");char ep[4096]={0};if(b&&endpoint_from_json(b+4,ep,sizeof(ep))==0&&push_subscribe(ep)==0)reply(c,"201 Created","application/json","{\"ok\":true}",0);else reply(c,"400 Bad Request","application/json","{\"ok\":false}",0);return;}
 if(!strcmp(path,"/api/push/unsubscribe")&&!strcmp(method,"POST")){char*b=strstr(r,"\r\n\r\n");char ep[4096]={0};if(b&&endpoint_from_json(b+4,ep,sizeof(ep))==0&&push_unsubscribe(ep)==0)reply(c,"200 OK","application/json","{\"ok\":true}",0);else reply(c,"400 Bad Request","application/json","{\"ok\":false}",0);return;}
 if(!strcmp(path,"/api/notifications/latest")){static_file(c,"/var/lib/pi-guardian/latest-event.json","application/json");return;}
 static_file(c,"web/index.html","text/html; charset=utf-8");}
static void*watch(void*x){(void)x;for(;;){int cpu=monitor_cpu(),ram=monitor_ram(),disk=monitor_disk(),temp=monitor_temp();if(cpu>95||ram>95||disk>90||temp>80){char m[160];snprintf(m,sizeof(m),"CPU %d%% RAM %d%% disk %d%% temperature %dC",cpu,ram,disk,temp);notify_event("CRITICAL","System pressure",m);}sleep(15);}return 0;}
void guardian_run(const guardian_config*cfg){G=*cfg;if(push_init()<0)fprintf(stderr,"warning: VAPID initialization failed\n");SSL_library_init();SSL_load_error_strings();OpenSSL_add_ssl_algorithms();SSL_CTX*ctx=SSL_CTX_new(TLS_server_method());if(!ctx){ERR_print_errors_fp(stderr);exit(1);}if(SSL_CTX_use_certificate_file(ctx,"/var/lib/pi-guardian/tls/server.crt",SSL_FILETYPE_PEM)<=0||SSL_CTX_use_PrivateKey_file(ctx,"/var/lib/pi-guardian/tls/server.key",SSL_FILETYPE_PEM)<=0){ERR_print_errors_fp(stderr);exit(1);}SSL_CTX_set_min_proto_version(ctx,TLS1_2_VERSION);pthread_t th;pthread_create(&th,0,watch,0);int s=socket(AF_INET,SOCK_STREAM,0),one=1;setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));struct sockaddr_in a={0};a.sin_family=AF_INET;a.sin_port=htons(cfg->port);a.sin_addr.s_addr=INADDR_ANY;if(bind(s,(struct sockaddr*)&a,sizeof(a))<0||listen(s,32)<0){perror("bind/listen");exit(1);}for(;;){int c=accept(s,0,0);if(c<0)continue;SSL*ssl=SSL_new(ctx);SSL_set_fd(ssl,c);if(SSL_accept(ssl)>0)handle(ssl);SSL_shutdown(ssl);SSL_free(ssl);close(c);}}
