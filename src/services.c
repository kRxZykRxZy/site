#include "guardian.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int service_is_active(const char *name){
    char cmd[256]; snprintf(cmd,sizeof(cmd),"systemctl is-active --quiet -- '%s'",name);
    return system(cmd)==0;
}
int service_restart(const char *name){
    if(!name || !*name || strchr(name,'/') || strchr(name,';') || strchr(name,'`')) return -1;
    char cmd[256]; snprintf(cmd,sizeof(cmd),"systemctl restart -- '%s'",name);
    return system(cmd)==0?0:-1;
}
