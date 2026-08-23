#include "guardian.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static void quote(char*out,size_t n,const char*in){size_t j=0;for(size_t i=0;in[i]&&j+4<n;i++){if(in[i]=='\''){out[j++]='\'';out[j++]='\\';out[j++]='\'';out[j++]='\'';}else out[j++]=in[i];}out[j]=0;}
void ai_judge(const char*context,char*decision,int size){if(!context||!decision||size<2)return;char q[1200];quote(q,sizeof(q),context);char cmd[1500];snprintf(cmd,sizeof(cmd),"curl -fsS --max-time 8 --get --data-urlencode 'prompt=You are Pi Guardian. Analyze this diagnostic and answer with exactly one action: observe, notify, or restart_service. Never suggest shell commands. Diagnostic: %s' https://text.pollinations.ai/",q);FILE*p=popen(cmd,"r");if(!p){snprintf(decision,size,"observe");return;}char b[256]={0};fread(b,1,sizeof(b)-1,p);pclose(p);if(strstr(b,"restart_service"))snprintf(decision,size,"restart_service");else if(strstr(b,"notify"))snprintf(decision,size,"notify");else snprintf(decision,size,"observe");}
