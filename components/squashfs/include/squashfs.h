#pragma once
#include "esp_err.h"
esp_err_t squash_mount_image(const char *path, const uint8_t *image);
