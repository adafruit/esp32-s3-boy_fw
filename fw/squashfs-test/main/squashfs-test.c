#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include "spi_flash_mmap.h"
#include "esp_partition.h"
#include "esp_vfs.h"
#include "squashfs.h"

void show(const char *fn) {
    FILE *f = fopen(fn, "rb");
    if(!f) { perror("fopen"); return; }
    for(int i=0; i<256; i++) {
        if(i % 16 == 0) {
            if(i) { printf("\n"); }
            printf("%04x", i);
        }
        int c = fgetc(f);
        if(c == EOF) break;
        printf(" %02x", c);
    }
    printf("\n");
    fclose(f);
}

void walk(const char *dir) {
    char buf[1024];
    DIR *d = opendir(dir);
    if(!d) {
        perror("opendir");
        return;
    }
    for(struct dirent *ent; (ent = readdir(d)) != NULL;) {
        snprintf(buf, sizeof(buf), "%s/%s", dir, ent->d_name);
        printf("[%2d] %s\n", ent->d_type, buf);
        if(ent->d_type == DT_REG) {
            show(buf);
        }
        if(ent->d_type == DT_DIR) {
            walk(buf);
            printf("\n");
        }
    }
    closedir(d);

}

void app_main(void)
{
    printf("finding partition\n");
    const esp_partition_t *data_partition = esp_partition_find_first(0x40, 0x1, NULL);
    if (!data_partition) {
        printf("failed at finding partition\n"); usleep(1000);
        abort();
    }

    printf("mapping partition\n");
    const static void *data_ptr;
    static spi_flash_mmap_handle_t handle;
    esp_err_t err = esp_partition_mmap(data_partition, 0, data_partition->size, SPI_FLASH_MMAP_DATA, &data_ptr, &handle);
    if (err != ESP_OK) {
        printf("failed at mapping partition\n"); usleep(1000);
      abort();
    }

    printf("mounting image\n"); usleep(1000);
    squash_mount_image("/data", data_ptr);
    printf("mounted image");
    while(1) {
        walk("/data");
        printf("Done!\n");
        usleep(10*1000);
        printf("also Done!\n");
        usleep(10*1000);
    }
}
