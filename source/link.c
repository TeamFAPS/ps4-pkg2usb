#include "ps4.h"
#include "defines.h"
#include "debug.h"
#include "main.h"
#include "elf64.h"
#include "link.h"
#include "utils.h"
#include "pkg.h"

#define BUFFER_SIZE 65536

int file_compare(char *fname1, char *fname2)
{
    long size1, size2;

    int  bytesRead1 = 0,
         bytesRead2 = 0,
         lastBytes = 100,
         res = 0,
         i;

    FILE *file1 = fopen(fname1, "rb"),
         *file2 = fopen(fname2, "rb");

    char *buffer1 = malloc(BUFFER_SIZE),
         *buffer2 = malloc(BUFFER_SIZE);

    if (!file1 || !file2) {
        return res; // res = 0;
    }

    fseek (file1, 0, SEEK_END);
    fseek (file2, 0, SEEK_END);

    size1 = ftell (file1);
    size2 = ftell (file2);

    fseek(file1, 0L, SEEK_SET);
    fseek(file2, 0L, SEEK_SET);

    if (size1 != size2) {
        //printfsocket("Different size > size1: %ld - size2: %ld\n", size1, size2);
        res = 0;
        goto exit;
    }
    //printfsocket("size1: %ld - size2: %ld\n", size1, size2);

    if (size1 < lastBytes) lastBytes = size1;

    fseek(file1, -lastBytes, SEEK_END);
    fseek(file2, -lastBytes, SEEK_END);

    bytesRead1 = fread(buffer1, sizeof(char), lastBytes, file1);
    bytesRead2 = fread(buffer2, sizeof(char), lastBytes, file2);

    if (bytesRead1 > 0 && bytesRead1 == bytesRead2) {
        for ( i = 0; i < bytesRead1; i++) {
            if (buffer1[i] != buffer2[i]) {
                //printfsocket("Different lastBytes %d\n", lastBytes);
                res = 0;
                goto exit;
            }
        }

        //printfsocket("Same lastBytes %d\n", lastBytes);
        res = 1;
    }

    free(buffer1);
    free(buffer2);

    exit:
    //printfsocket("Closing files\n");
    fclose(file1);
    fclose(file2);

    return res;
}

void copy_file(char *sourcefile, char* destfile)
{
    char msg[64];

    FILE *src = fopen(sourcefile, "rb");
    if (src)
    {
        FILE *out = fopen(destfile,"wb");
        if (out)
        {
            size_t bytes, bytes_size, bytes_copied = 0;

            char *buffer = malloc(BUFFER_SIZE);

            if (buffer != NULL)
            {
                fseek(src, 0L, SEEK_END);
                bytes_size = ftell(src);
                fseek(src, 0L, SEEK_SET);

                while (0 < (bytes = fread(buffer, 1, BUFFER_SIZE, src))) {
                    fwrite(buffer, 1, bytes, out);
                    bytes_copied += bytes;

                    if (bytes_copied > bytes_size) bytes_copied = bytes_size;
                    sprintf(notify_buf, "Copying %u%% completed...\n", bytes_copied * 100 / bytes_size);
                    //printfsocket(msg);
                }
                free(buffer);
            }
            fclose(out);
        }
        else {
            sprintf(msg,"write %s err : %s\n", destfile, strerror(errno));
            printfsocket(msg);
            //notify(msg);
        }
        fclose(src);
    }
    else {
        sprintf(msg,"write %s err : %s\n", destfile, strerror(errno));
        printfsocket(msg);
        //notify(msg);
    }
}

void link_pkg(char *title_id, char *usb_path)
{
    char app_pkg[64];
    char app_pkg_usb_root_path[64];
    char app_pkg_hdd_base_path[64];
    char app_pkg_usb_base_path[64];
    char app_pkg_hdd_path[64];
    char app_pkg_usb_path[64];
    char msg[512];

    sprintf(app_pkg, "app.pkg");
    sprintf(app_pkg_usb_root_path, "%s/PS4", usb_path);
    sprintf(app_pkg_hdd_base_path, "/user/app/%s", title_id);
    sprintf(app_pkg_usb_base_path, "%s/%s", app_pkg_usb_root_path, title_id);

    sprintf(app_pkg_hdd_path, "%s/%s", app_pkg_hdd_base_path, app_pkg);
    sprintf(app_pkg_usb_path, "%s/%s", app_pkg_usb_base_path, app_pkg);

    sprintf(msg, "Checking app.pkg in %s...\n", app_pkg_hdd_path);
    printfsocket(msg);

#ifndef DEBUG_SOCKET
    notify(msg);
    sceKernelSleep(5);
#endif

    if (!file_exists(app_pkg_hdd_path)) {
        sprintf(msg, "Error: app.pkg in %s not found!\n", app_pkg_hdd_path);
        printfsocket(msg);

#ifndef DEBUG_SOCKET
        notify(msg);
        sceKernelSleep(5);
#endif
        return;
    }

    if (symlink_exists(app_pkg_hdd_path)) {
        sprintf(msg, "Error: app.pkg in %s is yet moved!\n", app_pkg_hdd_base_path);
        printfsocket(msg);

#ifndef DEBUG_SOCKET
        notify(msg);
        sceKernelSleep(5);
#endif
        return;
    }

    if (isfpkg(app_pkg_hdd_path) != 0) {
        sprintf(msg, "Error: %s is not a valid fpkg!\n", app_pkg_hdd_path);
        printfsocket(msg);

#ifndef DEBUG_SOCKET
        notify(msg);
        sceKernelSleep(5);
#endif
        return;
    }
    
    sprintf(msg, "Checking app.pkg in %s ...\n", app_pkg_usb_path);
    printfsocket(msg);

    if (!file_exists(app_pkg_usb_root_path)) {
        sprintf(msg, "Creating root folder %s ...\n", app_pkg_usb_root_path);
        printfsocket(msg);

        mkdir(app_pkg_usb_root_path, 0777);
    }

    if (file_exists(app_pkg_usb_path)) {
        sprintf(msg, "App found in %s\n", app_pkg_usb_path);
        printfsocket(msg);

        if (!file_compare(app_pkg_hdd_path, app_pkg_usb_path)) {
            sprintf(msg, "%s and %s are different!\n", app_pkg_hdd_path, app_pkg_usb_path);
            printfsocket(msg);

#ifndef DEBUG_SOCKET
            notify(msg);
            sceKernelSleep(5);
#endif
            return;
        }
    } else {
        sprintf(msg, "App not found in %s\n", app_pkg_usb_path);
        printfsocket(msg);
        //notify(msg);

        if (!dir_exists(app_pkg_usb_base_path)) {
            sprintf(msg, "Creating %s ...\n", app_pkg_usb_base_path);
            printfsocket(msg);

            mkdir(app_pkg_usb_base_path, 0777);
        }

        sprintf(msg, "Copying app.pkg from %s to %s ...\n", app_pkg_hdd_path, app_pkg_usb_path);
        printfsocket(msg);

#ifndef DEBUG_SOCKET
        notify(msg);
        sceKernelSleep(5);
#endif

        copy_file(app_pkg_hdd_path, app_pkg_usb_path);

        sprintf(msg, "Copying completed!\n");
        printfsocket(msg);

#ifndef DEBUG_SOCKET
        notify(msg);
        sceKernelSleep(5);
#endif

        if (!file_compare(app_pkg_hdd_path, app_pkg_usb_path)) {
            sprintf(msg, "%s and %s are different!\n", app_pkg_hdd_path, app_pkg_usb_path);
            printfsocket(msg);

#ifndef DEBUG_SOCKET
            notify(msg);
            sceKernelSleep(5);
#endif
            return;
        }
    }

    unlink(app_pkg_hdd_path);

    sprintf(msg, "Removing %s completed!\n", app_pkg_hdd_path);
    printfsocket(msg);

    int link = symlink(app_pkg_usb_path, app_pkg_hdd_path);

#ifdef DEBUG_SOCKET
    if (link != -1) {
        sprintf(msg, "Linking %s to %s completed!\n", app_pkg_hdd_path, app_pkg_usb_path);
    } else {
        sprintf(msg, "Linking %s to %s failed!\n", app_pkg_hdd_path, app_pkg_usb_path);
    }
    printfsocket(msg);
#endif

    if (file_exists(app_pkg_hdd_path)) {
        sprintf(msg, "Checking app.pkg in %s successed!\n", app_pkg_hdd_path);
        printfsocket(msg);

#ifndef DEBUG_SOCKET
        notify(msg);
        sceKernelSleep(5);
#endif
    }
}