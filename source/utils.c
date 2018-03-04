#include "ps4.h"
#include "defines.h"
#include "debug.h"
#include "utils.h"
#include "main.h"
#include "elf64.h"
#include "pkg.h"

#define TRUE 1
#define FALSE 0

#define BUFFER_SIZE 65536

extern int run;

int symlink(const char *pathname, const char *slink) {
    return syscall(57, pathname, slink);
}

int symlinkat(const char *pathname, int newdirfd, const char *slink) {
    return syscall(502, pathname, newdirfd, slink);
}

int lstat(const char *pathname, struct stat *buf) {
    return syscall(190, pathname, buf); //40 old syscall
}

int wait_for_usb(char *usb_name, char *usb_path)
{
    FILE *out = fopen("/mnt/usb0/.probe", "wb");
    if (!out)
    {
        out = fopen("/mnt/usb1/.probe", "wb");
        if (!out)
        {
            return 0;
        }
        else
        {
            unlink("/mnt/usb1/.probe");
            sprintf(usb_name, "%s", "USB1");
            sprintf(usb_path, "%s", "/mnt/usb1");
        }
    }
    else
    {
        unlink("/mnt/usb0/.probe");
        sprintf(usb_name, "%s", "USB0");
        sprintf(usb_path, "%s", "/mnt/usb0");
    }
    fclose(out);

    return 1;
}

off_t file_size(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}

int file_exists(char *fname)
{
    FILE *file = fopen(fname, "rb");
    if (file)
    {
        fclose(file);
        return 1;
    }
    return 0;
}

int dir_exists(char *dname)
{
    DIR *dir = opendir(dname);

    if (dir)
    {
        /* Directory exists. */
        closedir(dir);
        return 1;
    }
    return 0;
}

int symlink_exists(const char* fname)
{
    struct stat statbuf;

    if (lstat(fname, &statbuf) < 0) {
        //error occured
        return -1;
    }

    if (S_ISLNK(statbuf.st_mode) == 1) {
        //symbolic link;
        return 1;
    } else {
        //NOT a symbolic link;
        return 0;
    }
}

char* chop(char *string)
{
    char *ptr;

    ptr = strrchr(string, '\r');
    if (ptr) *ptr = '\0';

    ptr = strrchr(string, '\n');
    if (ptr) *ptr = '\0';

    return string;
}

/*
void _mkdir(const char *dir)
{
    char tmp[256];
    char *p = NULL;

    snprintf(tmp, sizeof(tmp), "%s", dir);
    for (p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = 0;
            mkdir(tmp, 0777);
            *p = '/';
        }
    }
}

char *read_string(FILE* f)
{
    char *string = malloc(sizeof(char) * 256);
    int c;
    int length = 0;
    if (!string) return string;
    while((c = fgetc(f)) != EOF)
    {
        string[length++] = c;
    }
    string[length++] = '\0';

    return realloc(string, sizeof(char) * length);
}

static inline int fgetc(FILE *fp)
{
  char c;

  if (fread(&c, 1, 1, fp) == 0)
    return (EOF);
  return (c);
}
*/