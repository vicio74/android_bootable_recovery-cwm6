#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statfs.h>

#include <signal.h>
#include <sys/wait.h>

#include "bootloader.h"
#include "common.h"
#include "cutils/properties.h"
#include "firmware.h"
#include "install.h"
#include "make_ext4fs.h"
#include "minui/minui.h"
#include "minzip/DirUtil.h"
#include "roots.h"
#include "recovery_ui.h"

#include "../../external/yaffs2/yaffs2/utils/mkyaffs2image.h"
#include "../../external/yaffs2/yaffs2/utils/unyaffs.h"

#include "extendedcommands.h"
#include "nandroid.h"
#include "mounts.h"
#include "flashutils/flashutils.h"
#include "edify/expr.h"
#include <libgen.h>
#include "mtdutils/mtdutils.h"
#include "bmlutils/bmlutils.h"
#include "cutils/android_reboot.h"
#include "kyle.h"

void show_darkside_menu() {
    static char* headers[] = {  "Darkside Tools (BE CAREFUL!)",
                                "",
                                NULL
    };

    char* list[] = { "darkside.cache.wipe",
        "darkside.super.wipe_ext4 (BE CAREFUL!)",
        NULL
    };

    int chosen_item = get_menu_selection(headers, list, 0, 0);
    switch (chosen_item) {
        case 0:
                ensure_path_mounted("/emmc");
                if( access( "/emmc/clockworkmod/.darkside/cachewipe.zip", F_OK ) != -1) {
                install_zip("/emmc/clockworkmod/.darkside/cachewipe.zip");
                } else {
                ui_print("No darkside files found in /emmc/clockworkmod/.darkside");
                }
                break;
        case 1:
                ensure_path_mounted("/emmc");
                if( access( "/emmc/clockworkmod/.darkside/superwipe.zip", F_OK ) != -1) {
                install_zip("/emmc/clockworkmod/.darkside/superwipe.zip");
                } else {
                ui_print("No darkside files found in /emmc/clockworkmod/.darkside");
                }
                break;
    }
}

void show_efs_menu() {
    static char* headers[] = {  "EFS Tools",
                                "",
                                NULL
    };

    char* list[] = { "backup /efs partition to internal",
                     "restore /efs partition from internal",
                     "backup /efs partition to external",
                     "restore /efs partition from external",
                     NULL
    };

    int chosen_item = get_menu_selection(headers, list, 0, 0);
    switch (chosen_item) {
        case 0:
                ensure_path_mounted("/emmc");
                ensure_path_unmounted("/efs");
                __system("backup-efs.sh /emmc");
                ui_print("/emmc/clockworkmod/.efsbackup/efs.img created\n");
                break;
        case 1:
                ensure_path_mounted("/emmc");
                ensure_path_unmounted("/efs");
                if( access("/emmc/clockworkmod/.efsbackup/efs.img", F_OK ) != -1 ) {
                   __system("restore-efs.sh /emmc");
                   ui_print("/emmc/clockworkmod/.efsbackup/efs.img restored to /efs");
                } else {
                   ui_print("No efs.img backup found.\n");
                }
                break;
        case 2:
                ensure_path_mounted("/sdcard");
                ensure_path_unmounted("/efs");
                __system("backup-efs.sh /sdcard");
                ui_print("/sdcard/clockworkmod/.efsbackup/efs.img created\n");
                break;
        case 3:
                ensure_path_mounted("/sdcard");
                ensure_path_unmounted("/efs");
                if( access("/sdcard/clockworkmod/.efsbackup/efs.img", F_OK ) != -1 ) {
                   __system("restore-efs.sh /sdcard");
                   ui_print("/sdcard/clockworkmod/.efsbackup/efs.img restored to /efs");
                } else {
                   ui_print("No efs.img backup found.\n");
                }
                break;
    }
}

int create_customzip(const char* custompath)
{
    char command[PATH_MAX];
    sprintf(command, "create_update_zip.sh %s", custompath);
    __system(command);
    return 0;
}

void show_extras_menu()
{
    static char* headers[] = {  "Extras Menu",
                                "",
                                NULL
    };

    static char* list[] = { "disable install-recovery.sh",
                            "enable/disable one confirm",
                            "hide/show backup & restore progress",
			    "aroma file manager",
			    "darkside tools",
			    "/efs tools",
			    "create custom zip",
			    "recovery info",
                            NULL
    };

    for (;;)
    {
        int chosen_item = get_filtered_menu_selection(headers, list, 0, 0, sizeof(list) / sizeof(char*));
        if (chosen_item == GO_BACK)
            break;
        switch (chosen_item)
        {
            case 0:
                if (ensure_path_mounted("/system") != 0)
                return 0;
                int ret = 0;
                struct stat st;
                if (0 == lstat("/system/etc/install-recovery.sh", &st)) {
                if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
                ui_show_text(1);
                ret = 1;
                if (confirm_selection("ROM may flash stock recovery on boot. Fix?", "Yes - Disable recovery flash")) {
                __system("chmod -x /system/etc/install-recovery.sh");
            }
        }
    }
                ensure_path_unmounted("/system");
                return ret;
                break;
	    case 1:
		ensure_path_mounted("/emmc");
                if( access("/emmc/clockworkmod/.one_confirm", F_OK ) != -1 ) {
                   __system("rm -rf /emmc/clockworkmod/.one_confirm");
                   ui_print("one confirm disabled\n");
                } else {
                   __system("touch /emmc/clockworkmod/.one_confirm");
                   ui_print("one confirm enabled\n");
                }
		break;
	    case 2:
                ensure_path_mounted("/emmc");
                if( access("/emmc/clockworkmod/.hidenandroidprogress", F_OK ) != -1 ) {
                   __system("rm -rf /emmc/clockworkmod/.hidenandroidprogress");
                   ui_print("nandroid progress will be shown\n");
                } else {
                   __system("touch /emmc/clockworkmod/.hidenandroidprogress");
                   ui_print("nandroid progress will be hidden\n");
                }
                break;
	    case 3:
		ensure_path_mounted("/emmc");
		if( access( "/emmc/clockworkmod/.aromafm/aromafm.zip", F_OK ) != -1) {
                install_zip("/emmc/clockworkmod/.aromafm/aromafm.zip");
		} else {
                ui_print("No aroma files found in /emmc/clockworkmod/.aromafm");
		}
		break;
	    case 4:
		show_darkside_menu();
		break;
	    case 5:
		show_efs_menu();
		break;
	    case 6:
		ensure_path_mounted("/system");
		ensure_path_mounted("/emmc");
                if (confirm_selection("Create a zip from system and boot?", "Yes - Create custom zip")) {
		ui_print("Creating custom zip...\n");
		ui_print("This may take a while. Be Patient.\n");
                    char custom_path[PATH_MAX];
                    time_t t = time(NULL);
                    struct tm *tmp = localtime(&t);
                    if (tmp == NULL)
                    {
                        struct timeval tp;
                        gettimeofday(&tp, NULL);
                        sprintf(custom_path, "/emmc/clockworkmod/zips/%d", tp.tv_sec);
                    }
                    else
                    {
                        strftime(custom_path, sizeof(custom_path), "/emmc/clockworkmod/zips/%F.%H.%M.%S", tmp);
                    }
                    create_customzip(custom_path);
		ui_print("custom zip created in /emmc/clockworkmod/zips/\n");
	}
		ensure_path_unmounted("/system");
		break;
	    case 7:
		ui_print("ClockworkMod Recovery 6.0.1.3 Touch v13\n");
		ui_print("Created By: sk8erwitskil (Kyle Laplante)\n");
		ui_print("Build Date: 08/31/2012 9:30 pm\n");
	}
    }
}
