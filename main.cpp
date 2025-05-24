#include "pico/stdlib.h"

#include <stdio.h>
#include <pfs.h>
#include <lfs.h>

#define ROOT_SIZE 0x100000
#define ROOT_OFFSET 0x100000

static struct pfs_pfs *pfs = 0;
static struct lfs_config cfg {};

int main() {
    stdio_init_all();

    ffs_pico_createcfg (&cfg, ROOT_OFFSET, ROOT_SIZE);
    pfs = pfs_ffs_create (&cfg);
    pfs_mount (pfs, "/");

    while (1) {
        printf("Hello from Pico!\n");
        sleep_ms(1000);
    }
    
    return 0;
}
