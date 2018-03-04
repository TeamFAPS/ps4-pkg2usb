#ifndef UTILS_H
#define UTILS_H

#include "types.h"

int symlink(const char *pathname, const char *slink);

int symlinkat(const char *pathname, int newdirfd, const char *slink);

int lstat(const char *pathname, struct stat *buf);

off_t file_size(const char *filename);

int file_exists(char *fname);

int dir_exists(char *dname);

int symlink_exists(const char* fname);

int wait_for_usb(char *usb_name, char *usb_path);

char* chop(char *string);

#endif
