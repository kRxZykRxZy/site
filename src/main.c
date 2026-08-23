#include "guardian.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    guardian_config cfg = {0};
    cfg.port = 81;
    strncpy(cfg.username, "admin", sizeof(cfg.username)-1);
    strncpy(cfg.password, "Hm361485%", sizeof(cfg.password)-1);
    printf("Pi Guardian starting on port %d\n", cfg.port);
    guardian_run(&cfg);
    return 0;
}
