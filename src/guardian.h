#ifndef GUARDIAN_H
#define GUARDIAN_H

typedef struct { int port; char username[64]; char password[128]; } guardian_config;
void guardian_run(const guardian_config *cfg);
int monitor_cpu(void); int monitor_disk(void); int monitor_ram(void); int monitor_temp(void);
int service_is_active(const char *name); int service_restart(const char *name);
void notify_event(const char *level,const char *title,const char *message);
void ai_judge(const char *context,char *decision,int size);
#include "push.h"
#endif
