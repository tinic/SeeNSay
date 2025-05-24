#include <lfs.h>
#include <pfs.h>
#include <stdio.h>

#include "music_file_lfs.h"
#include "pico/stdlib.h"

#define ROOT_SIZE 0x100000
#define ROOT_OFFSET 0x100000

static struct pfs_pfs *pfs = 0;
static struct lfs_config cfg{};
static music_file_lfs mf{};

#define WORKING_SIZE 16000
#define RAM_BUFFER_LENGTH 6000

static int16_t d_buff[RAM_BUFFER_LENGTH];
static unsigned char working[WORKING_SIZE];

void load_sample_file(const char *filename) {
    musicFileCreate(&mf, filename, (unsigned char *)&working[0], WORKING_SIZE);

    uint32_t sample_rate = musicFileGetSampleRate(&mf);
    uint16_t num_channels =  musicFileGetChannels(&mf);
    printf("sample_rate %d num_channels %d!\n", sample_rate, num_channels);

    uint32_t read = 0;
    for (; musicFileRead(&mf, d_buff, RAM_BUFFER_LENGTH, &read);) {
    }
}

int main() {
    stdio_init_all();
    printf("Raspberry Pi Pico LFS Music File System Demo\n");

    // Initialize the LFS filesystem
    ffs_pico_createcfg(&cfg, ROOT_OFFSET, ROOT_SIZE);
    pfs = pfs_ffs_create(&cfg);
    int mount_result = pfs_mount(pfs, "/");

    if (mount_result == 0) {
        printf("LFS filesystem mounted successfully at /\n");
    } else {
        printf("Failed to mount LFS filesystem: %d\n", mount_result);
    }

    // Test the music file implementation
    printf("LFS-based music file implementation ready!\n");
    printf("Functions available:\n");
    printf("- musicFileCreate()\n");
    printf("- musicFileRead()\n");
    printf("- musicFileClose()\n");
    printf("Uses standard C file I/O with PFS/LFS underneath\n");

    load_sample_file("test.mp3");

    while (1) {
        printf("LFS Music File System Ready\n");
        sleep_ms(5000);
    }

    return 0;
}
