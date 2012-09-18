#ifndef NANDROID_H
#define NANDROID_H

int nandroid_main(int argc, char** argv);
int nandroid_backup(const char* backup_path);
int nandroid_restore(const char* backup_path, int restore_boot, int restore_system, int restore_data, int restore_cache, int restore_sdext, int restore_wimax);
void nandroid_dedupe_gc(const char* blob_dir);
void nandroid_force_backup_format(const char* fmt);

#define NANDROID_BACKUP_FORMAT_FILE "/emmc/clockworkmod/.default_backup_format"

#endif