#include "push.h"
#include <curl/curl.h>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/pem.h>
#include <openssl/ecdsa.h>
#include <openssl/bn.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DATA_DIR "/var/lib/pi-guardian"
#define KEY_FILE DATA_DIR "/vapid-private.pem"
#define PUB_FILE DATA_DIR "/vapid-public.b64"
#define SUB_FILE DATA_DIR "/push-subscriptions"
static const char B64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static char *b64url(const unsigned char *in,size_t n,char *out,size_t cap){size_t j=0;unsigned int v=0,b=0;for(size_t i=0;i<n;i++){v=(v<<8)|in[i];b+=8;while(b>=6){b-=6;if(j+1>=cap)return NULL;out[j++]=B64[(v>>b)&63];}}if(b){if(j+1>=cap)return NULL;out[j++]=B64[(v<<(6-b))&63];}out[j]=0;return out;}
static int read_pub(char*out,int size){FILE*f=fopen(PUB_FILE,"r");if(!f)return -1;if(!fgets(out,size,f)){fclose(f);return -1;}fclose(f);out[strcspn(out,"\r\n")]=0;return 0;}
static int make_keys(void){mkdir(DATA_DIR,0750);FILE*f=fopen(KEY_FILE,"r");if(f){fclose(f);return 0;}EVP_PKEY_CTX*ctx=EVP_PKEY_CTX_new_id(EVP_PKEY_EC,NULL);if(!ctx)return -1;EVP_PKEY*p=NULL;if(EVP_PKEY_keygen_init(ctx)<=0||EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx,NID_X9_62_prime256v1)<=0||EVP_PKEY_keygen(ctx,&p)<=0){EVP_PKEY_CTX_free(ctx);return -1;}f=fopen(KEY_FILE,"w");if(!f){EVP_PKEY_free(p);EVP_PKEY_CTX_free(ctx);return -1;}chmod(KEY_FILE,0600);PEM_write_PrivateKey(f,p,NULL,NULL,0,NULL,NULL);fclose(f);
 EC_KEY*e=EVP_PKEY_get1_EC_KEY(p);const EC_POINT*pt=EC_KEY_get0_public_key(e);const EC_GROUP*g=EC_KEY_get0_group(e);unsigned char raw[65];size_t n=EC_POINT_point2oct(g,pt,POINT_CONVERSION_UNCOMPRESSED,raw,sizeof(raw),NULL);char pub[128];b64url(raw,n,pub,sizeof(pub));f=fopen(PUB_FILE,"w");if(f){fputs(pub,f);fputc('\n',f);fclose(f);chmod(PUB_FILE,0640);}EC_KEY_free(e);EVP_PKEY_free(p);EVP_PKEY_CTX_free(ctx);return 0;}
int push_init(void){return make_keys();}
int push_public_key(char*out,int size){return read_pub(out,size);}
int push_subscribe(const char*endpoint){if(!endpoint||strncmp(endpoint,"https://",8))return -1;mkdir(DATA_DIR,0750);FILE*f=fopen(SUB_FILE,"a+");if(!f)return -1;rewind(f);char line[4096];while(fgets(line,sizeof(line),f)){line[strcspn(line,"\r\n")]=0;if(!strcmp(line,endpoint)){fclose(f);return 0;}}fprintf(f,"%s\n",endpoint);fclose(f);return 0;}
int push_unsubscribe(const char*endpoint){FILE*f=fopen(SUB_FILE,"r");if(!f)return 0;FILE*t=fopen(SUB_FILE".tmp","w");if(!t){fclose(f);return -1;}char line[4096];while(fgets(line,sizeof(line),f)){line[strcspn(line,"\r\n")]=0;if(strcmp(line,endpoint))fprintf(t,"%s\n",line);}fclose(f);fclose(t);rename(SUB_FILE".tmp",SUB_FILE);return 0;}
static int jwt_for(const char*endpoint,char*out,int cap){FILE*f=fopen(KEY_FILE,"r");if(!f)return -1;EVP_PKEY*p=PEM_read_PrivateKey(f,NULL,NULL,NULL);fclose(f);char pub[128];if(!p||read_pub(pub,sizeof(pub))<0){EVP_PKEY_free(p);return -1;}const char*s=strstr(endpoint,"https://");if(!s){EVP_PKEY_free(p);return -1;}s+=8;const char*slash=strchr(s,'/');size_t hn=slash?(size_t)(slash-s):strlen(s);char host[512];if(hn>=sizeof(host)){EVP_PKEY_free(p);return -1;}memcpy(host,s,hn);host[hn]=0;char aud[600];snprintf(aud,sizeof(aud),"https://%s",host);time_t exp=time(NULL)+3600*12;char h1[128],p1[1024];b64url((const unsigned char*)"{\"typ\":\"JWT\",\"alg\":\"ES256\"}",30,h1,sizeof(h1));int pn=snprintf(p1,sizeof(p1),"{\"aud\":\"%s\",\"exp\":%ld,\"sub\":\"mailto:pi-guardian@localhost\"}",aud,(long)exp);char hb[128],pb[1400];b64url((const unsigned char*)h1,strlen(h1),hb,sizeof(hb));/* h1 is already encoded; avoid double encoding below */
 (void)pn;(void)hb;(void)pb;
 const char *header="eyJ0eXAiOiJKV1QiLCJhbGciOiJFUzI1NiJ9";char payload[1400];pn=snprintf(payload,sizeof(payload),"{\"aud\":\"%s\",\"exp\":%ld,\"sub\":\"mailto:pi-guardian@localhost\"}",aud,(long)exp);char ep[1900];b64url((const unsigned char*)payload,pn,ep,sizeof(ep));char signing[2100];snprintf(signing,sizeof(signing),"%s.%s",header,ep);EVP_MD_CTX*md=EVP_MD_CTX_new();size_t sl=0;EVP_DigestSignInit(md,NULL,EVP_sha256(),NULL,p);EVP_DigestSignUpdate(md,signing,strlen(signing));EVP_DigestSignFinal(md,NULL,&sl);unsigned char der[128];EVP_DigestSignFinal(md,der,&sl);EVP_MD_CTX_free(md);EVP_PKEY_free(p);const unsigned char*q=der;ECDSA_SIG*sig=d2i_ECDSA_SIG(NULL,&q,sl);if(!sig)return -1;unsigned char rawsig[64];if(BN_bn2binpad(ECDSA_SIG_get0_r(sig),rawsig,32)<0||BN_bn2binpad(ECDSA_SIG_get0_s(sig),rawsig+32,32)<0){ECDSA_SIG_free(sig);return -1;}ECDSA_SIG_free(sig);char sb[128];b64url(rawsig,64,sb,sizeof(sb));snprintf(out,cap,"%s.%s",signing,sb);return 0;}
static int send_one(const char*endpoint){char jwt[2600],pub[128];if(jwt_for(endpoint,jwt,sizeof(jwt))<0||read_pub(pub,sizeof(pub))<0)return -1;CURL*c=curl_easy_init();if(!c)return -1;struct curl_slist*h=NULL;char auth[3000];snprintf(auth,sizeof(auth),"Authorization: vapid t=%s, k=%s",jwt,pub);h=curl_slist_append(h,auth);h=curl_slist_append(h,"TTL: 86400");h=curl_slist_append(h,"Content-Length: 0");curl_easy_setopt(c,CURLOPT_URL,endpoint);curl_easy_setopt(c,CURLOPT_POST,1L);curl_easy_setopt(c,CURLOPT_POSTFIELDS,"");curl_easy_setopt(c,CURLOPT_POSTFIELDSIZE,0L);curl_easy_setopt(c,CURLOPT_HTTPHEADER,h);curl_easy_setopt(c,CURLOPT_TIMEOUT,10L);curl_easy_setopt(c,CURLOPT_USERAGENT,"Pi-Guardian/1.0");CURLcode rc=curl_easy_perform(c);long code=0;curl_easy_getinfo(c,CURLINFO_RESPONSE_CODE,&code);curl_slist_free_all(h);curl_easy_cleanup(c);if(rc!=CURLE_OK)return -1;if(code==404||code==410){push_unsubscribe(endpoint);return -2;}return (code>=200&&code<300)?0:-1;}
int push_broadcast(const char*title,const char*body,const char*level){(void)title;(void)body;(void)level;FILE*f=fopen(SUB_FILE,"r");if(!f)return 0;curl_global_init(CURL_GLOBAL_DEFAULT);char line[4096];int sent=0;while(fgets(line,sizeof(line),f)){line[strcspn(line,"\r\n")]=0;if(*line&&send_one(line)==0)sent++;}fclose(f);curl_global_cleanup();return sent;}
