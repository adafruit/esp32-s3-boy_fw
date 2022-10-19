#include "fatfs.h"
#include "cli.h"
#include "sd.h"
#include "spi_flash_mmap.h"

#ifdef _USE_HW_FATFS
#include "esp_vfs.h"
#include "squashfs.h"


#define lock()      xSemaphoreTake(mutex_lock, portMAX_DELAY);
#define unLock()    xSemaphoreGive(mutex_lock);


#ifdef _USE_HW_CLI
static void cliFatfs(cli_args_t *args);
#endif

static bool fatfsMount(void);
static bool fatfsUnMount(void) __attribute__((unused));


static bool is_init = false;
static bool is_mounted = false;

static SemaphoreHandle_t mutex_lock;
const static char *base_path = "/sdcard";

const static esp_partition_t *data_partition;
const static void *data_ptr;
static spi_flash_mmap_handle_t handle;

bool fatfsInit(void)
{
  mutex_lock = xSemaphoreCreateMutex();
 
  data_partition = esp_partition_find_first(0x40, 0x1, NULL);
  if (!data_partition) return false;

  esp_err_t err = esp_partition_mmap(data_partition, 0, data_partition->size, SPI_FLASH_MMAP_DATA, &data_ptr, &handle);
  if (err != ESP_OK) {
      return false;
  }
  is_init = true;

  fatfsMount();
  #ifdef _USE_HW_CLI
  cliAdd("fatfs", cliFatfs);
  #endif

  return true;
}

bool fatfsIsInit(void)
{
  return is_init;
}

bool fatfsIsMounted(void)
{
  return is_mounted;
}

bool fatfsLock(void)
{
  lock();
  return true;
}

bool fatfsUnLock(void)
{
  unLock();
  return true;
}

bool fatfsMount(void)
{
  if (!is_init) { return false; }
  if (is_mounted) { return true; }
  squash_mount_image(base_path, data_ptr);
  is_mounted = true;
  return true;
}

bool fatfsUnMount(void)
{
  return false;
}


#ifdef _USE_HW_CLI
void cliFatfs(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    cliPrintf("fatfs init      : %d\n", is_init);
    cliPrintf("fatfs mount     : %d\n", is_mounted);
    

    ret = true;
  }

  if (args->argc >= 1 && args->isStr(0, "dir") == true)
  {
    DIR * d = opendir(base_path); 
    struct dirent * dir;
    char buffer[300];

    if (args->argc == 1)
      snprintf(buffer, 256, "%s", base_path);
    else
      snprintf(buffer, 256, "%s/%s", base_path, args->getStr(1));

    cliPrintf("\n");
    cliPrintf("dir %s\n", buffer);
    if (d != NULL)
    {
      while ((dir = readdir(d)) != NULL) 
      {
        if (dir-> d_type != DT_DIR) 
        {
          FILE *f;
          int file_len = 0;

          sprintf(buffer, "%s/%s", base_path, dir->d_name);
          f = fopen(buffer, "r");
          if (f)
          {
            fseek(f, 0, SEEK_END);
            file_len = ftell(f);
            fclose(f);
          }
          if (strlen(dir->d_name) >= 28)
          {
            dir->d_name[28] = '.';
            dir->d_name[29] = '.';
            dir->d_name[30] = '.';
            dir->d_name[31] = 0;
          }
          cliPrintf("%-32s %dKB\n", dir->d_name, file_len/1024);
        }
        else if (dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) 
        {
          cliPrintf("[%s]\n", dir->d_name); 
        }
      }
      closedir(d);
    }
    ret = true;
  }

  if (ret != true)
  {
    cliPrintf("fatfs info\n");
    cliPrintf("fatfs dir [name]\n");
  }
}
#endif

#endif
