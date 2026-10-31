/* 
 * IlluminateMount - Standalone ISO Mounter
 * Copyright (c) 2026 saintcpmshop-source. All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <strings.h>
#include <sysutil/sysutil.h>
#include <sys/process.h>
#include <sys/memory.h>
#include <sys/timer.h>
#include <sys/fs.h>
#include <sys/storage.h>

#define USB_PATH "/dev_usb000/PS3ISO"
#define HDD_PATH "/dev_hdd0/PS3ISO"

extern int sys_storage_ext_mount_discfile(const char* file, const char* mount_point);
extern int sys_storage_ext_umount_discfile();

char iso_list[30][256]; 
int iso_count = 0;

void scan_for_isos(const char* directory) {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(directory)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            char *ext = strrchr(ent->d_name, '.');
            if (ext && strcasecmp(ext, ".iso") == 0) {
                if (iso_count < 30) {
                    snprintf(iso_list[iso_count], 256, "%s/%s", directory, ent->d_name);
                    iso_count++;
                }
            }
        }
        closedir(dir);
    }
}

void mount_game(int index) {
    if (index < 0 || index >= iso_count) return;
    printf("\n[!] Unmounting any previously mounted game...\n");
    sys_storage_ext_umount_discfile();
    sysUtilSleep(500);
    printf("[!] Mounting: %s\n", iso_list[index]);
    int result = sys_storage_ext_mount_discfile(iso_list[index], "/dev_bdvd");
    if (result == 0) {
        printf("\n[+] SUCCESS! Game mounted.\n");
        printf("[+] Exit app and go to the 'Game' column on the XMB to play.\n");
    } else {
        printf("\n[-] FAILED to mount. Error code: %d\n", result);
        printf("[-] Ensure your CFW is Evilnat (Cobra) and you are not on HEN.\n");
    }
}

int main(int argc, char *argv[]) {
    sysUtilInit();
    sysFsInit();
    printf("\033[2J\033[H");
    printf("==========================================\n");
    printf("          IlluminateMount v1.0            \n");
    printf("==========================================\n\n");
    printf("[!] Scanning for ISO games...\n");
    scan_for_isos(USB_PATH);
    if (iso_count == 0) scan_for_isos(HDD_PATH);

    if (iso_count == 0) {
        printf("[-] No ISO files found on USB or HDD.\n");
        printf("[-] Place ISOs in /dev_usb000/PS3ISO/\n");
    } else {
        printf("[+] Found %d game(s):\n\n", iso_count);
        for (int i = 0; i < iso_count; i++) {
            char *filename = strrchr(iso_list[i], '/');
            if (filename) filename++; 
            else filename = iso_list[i];
            printf("  %d. %s\n", i + 1, filename);
        }
        printf("\nSelect a game to mount (1-%d): ", iso_count);
        int choice;
        if (scanf("%d", &choice) == 1) mount_game(choice - 1);
    }
    printf("\nPress the PS button to exit.\n");
    while(1) { sysUtilCheckCallback(); sysUtilSleep(100); }
    sysFsExit();
    sysUtilExit();
    return 0;
}
