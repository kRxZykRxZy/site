#include "guardian.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_first(const char *path, long *out) {
    FILE *f=fopen(path,"r"); if(!f) return -1;
    if(fscanf(f,"%ld",out)!=1){fclose(f);return -1;} fclose(f); return 0;
}

int monitor_cpu(void){
    static long last_idle=-1,last_total=-1; FILE *f=fopen("/proc/stat","r");
    if(!f)return -1; char c[16]; long user,nice,sys,idle,iowait,irq,softirq,steal;
    if(fscanf(f,"%15s %ld %ld %ld %ld %ld %ld %ld %ld",c,&user,&nice,&sys,&idle,&iowait,&irq,&softirq,&steal)!=9){fclose(f);return -1;}
    fclose(f); long total=user+nice+sys+idle+iowait+irq+softirq+steal, di=idle-last_idle, dt=total-last_total;
    last_idle=idle; last_total=total; return dt>0?(int)(100-(di*100/dt)):-1;
}
int monitor_ram(void){
    FILE *f=fopen("/proc/meminfo","r"); if(!f)return -1; long total=0,avail=0; char k[32]; long v; char u[16];
    while(fscanf(f,"%31s %ld %15s",k,&v,u)==3){if(!strcmp(k,"MemTotal:"))total=v;if(!strcmp(k,"MemAvailable:"))avail=v;}
    fclose(f); return total?(int)((total-avail)*100/total):-1;
}
int monitor_disk(void){
    FILE *p=popen("df -P / | awk 'NR==2 {gsub(/%/,\"\",$5); print $5}'","r"); if(!p)return -1; int v=-1; fscanf(p,"%d",&v);pclose(p);return v;
}
int monitor_temp(void){long v;if(read_first("/sys/class/thermal/thermal_zone0/temp",&v))return -1;return (int)(v/1000);}
