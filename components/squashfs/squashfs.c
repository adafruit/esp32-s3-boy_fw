#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>

#include "esp_vfs.h"

#include "squashfs.h"

#include "squash.h"
#include "squash/fs.h"
#include "squash/fdtable.h"
#include "squash/mutex.h"

static sqfs fs;
static _lock_t lock;

typedef struct {
    DIR base;
    SQUASH_DIR *real_dir;
} SQ_DIR_WRAP;

#define RETURN_ERROR(err) do { _lock_release(&lock); errno = err; return -1; } while(0)
#define RETURN_ERRNO() do { _lock_release(&lock); return -1; } while(0)
#define RETURN_ERRNO_NULL() do { _lock_release(&lock); return NULL; } while(0)
#define RETURN_SUCCESS() do { _lock_release(&lock); return 0; } while(0)
#define RETURN_ARG(arg) do { _lock_release(&lock); return arg; } while(0)
#define RETURN_VOID() do { _lock_release(&lock); return; } while(0)
#define RETURN_RES() do { _lock_release(&lock); if(res < 0) return -1; return res; } while(0)
#define RETURN_PTR() do { _lock_release(&lock); return res; } while(0)

dev_t sqfs_makedev(unsigned int maj, unsigned int min) {
    return (maj << 24) | min;
}

static int squash_fs_open(const char *path, int flags, int mode) {
    _lock_acquire(&lock);

    if((mode & O_ACCMODE) != O_RDONLY) { RETURN_ERROR(EROFS); }

    int res = squash_open(&fs, path);
    RETURN_RES();
}

static int squash_fs_fstat(int fd, struct stat *st) {
    _lock_acquire(&lock);
    int res = squash_fstat(fd, st);
    RETURN_RES();
}

static int squash_fs_stat(const char *path, struct stat *st) {
    _lock_acquire(&lock);
    int res = squash_stat(&fs, path, st);
    RETURN_RES();
}

static int squash_fs_close(int fd) {
    _lock_acquire(&lock);
    int res = squash_close(fd);
    RETURN_RES();
}

static ssize_t squash_fs_read(int fd, void *buf, size_t count) {
    _lock_acquire(&lock);
    ssize_t res = squash_read(fd, buf, count);
    RETURN_RES();
}

static off_t squash_fs_lseek(int fd, off_t offset, int whence) {
    _lock_acquire(&lock);
    off_t res = squash_lseek(fd, offset, whence);
    RETURN_RES();
}

static long squash_fs_telldir(DIR *pdir) {
    _lock_acquire(&lock);
    long res = squash_telldir((SQUASH_DIR*)pdir);
    RETURN_RES();
}

static void squash_fs_seekdir(DIR *pdir, long offset) {
    _lock_acquire(&lock);
    squash_seekdir((SQUASH_DIR*)pdir, offset);
    RETURN_VOID();
}

static int squash_fs_closedir(DIR *pdir) {
    if(!pdir) { errno = -EINVAL; return -1; }
    _lock_acquire(&lock);
    SQ_DIR_WRAP *wrap_dir = (SQ_DIR_WRAP*) pdir;
    int res = squash_closedir(wrap_dir->real_dir);
    free(wrap_dir);
    RETURN_RES();
}

static DIR *squash_fs_opendir(const char *name) {
    _lock_acquire(&lock);
    SQUASH_DIR *res = squash_opendir(&fs, name);
    if (!res) { RETURN_ARG(NULL); }

    SQ_DIR_WRAP *wrap_dir = malloc(sizeof(SQ_DIR_WRAP));
    wrap_dir->real_dir = res;
    RETURN_ARG((DIR*)wrap_dir);
}

static struct dirent *squash_fs_readdir(DIR *pdir) {
    _lock_acquire(&lock);
    SQ_DIR_WRAP *wrap_dir = (SQ_DIR_WRAP*) pdir;
    struct dirent *res = squash_readdir(wrap_dir->real_dir);
    RETURN_PTR();
}

#define SLOT(x) .x = & squash_fs_ ## x
static esp_vfs_t squash_fs = {
    .flags = ESP_VFS_FLAG_DEFAULT,
    SLOT(open),
    SLOT(close),
    SLOT(read),
    SLOT(fstat),
    SLOT(stat),
    SLOT(lseek),
    SLOT(readdir),
    SLOT(opendir),
    SLOT(closedir),
    SLOT(telldir),
    SLOT(seekdir),
};

static esp_err_t map_error(sqfs_err e) {
    if(e == SQFS_OK) { return ESP_OK; }
    return ESP_FAIL;
}

esp_err_t squash_mount_image(const char *path, const uint8_t *image) {
    ESP_ERROR_CHECK(map_error(squash_start()));
    ESP_ERROR_CHECK(map_error(sqfs_open_image(&fs, image, 0)));
    ESP_ERROR_CHECK(esp_vfs_register(path, &squash_fs, NULL));
    _lock_init(&lock);
    return ESP_OK;
}

#define FD_PENDING ((void*)-1)

int dup(int unused) {
    (void) unused;
    MUTEX_LOCK(&squash_global_mutex);
    size_t i;
    for(i=3; i<squash_global_fdtable.nr; i++) {
        printf("fds[%2zd] = %p\n", i, squash_global_fdtable.fds[i]);
        if(!squash_global_fdtable.fds[i]) { break; }
    }
    MUTEX_UNLOCK(&squash_global_mutex);
    printf("Note: dup() returning %d\n", i);
    return i; // can be equal to nr!
}
