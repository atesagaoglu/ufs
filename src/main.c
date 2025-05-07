#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <mntent.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>

#include <blkid/blkid.h>

#include "log.h"

#define streq(a, b) (strcmp(a, b) == 0)

struct mntent *get_partition_info(const char *path) {
    FILE *mtab = setmntent("/etc/mtab", "r");
    struct mntent *mnt;

    if (mtab == NULL) {
        perror("setmntent");
        return NULL;
    }

    while ((mnt = getmntent(mtab)) != NULL) {
        if (strcmp(mnt->mnt_fsname, path) == 0) {
            endmntent(mtab);
            return mnt;
        }
    }

    endmntent(mtab);
    return NULL;
}

// NOTE: return boolean for now, might return fd in the future
int check_meta(const char *path, struct dirent *partition_entry) {
    if (access(path, F_OK) == 0) {
        logger(L_INFO, "Meta file found on %s\n", partition_entry->d_name);
        return 1;
    } else {
        // this is ok, not an error
        if (errno == ENOENT) {
            logger(L_WARN,
                   "No metafile found of partition %s\n",
                   partition_entry->d_name);
            return 0;
        } else {
            const char *errdesc, *errname;
            errname = strerrorname_np(errno);
            errdesc = strerror(errno);
            logger(L_ERROR,
                   "[%s] Couldn't open .ufs file at partion %s. %s\n",
                   errname,
                   partition_entry->d_name,
                   errdesc);
            return 0;
        }
    }

    return 0;
}

int mount_and_check(struct dirent *entry, struct mntent *info) {
    if (entry == NULL || info == NULL) {
        logger(L_ERROR, "Null entry pointer.\n");
        return 0;
    }

    char device_path[512];
    snprintf(device_path, sizeof(device_path), "/dev/%s", entry->d_name);
    logger(L_INFO, "Trying to mount %s...\n", device_path);
    int ret;
    ret = mkdir("/tmp/ufs", 0744);

    if (ret < 0) {
        perror("mkdir");
        // TODO: Ignore directry exists
    }

    ret = mount(device_path,
                "/tmp/ufs",
                info->mnt_type,
                MS_RDONLY | MS_NOATIME,
                NULL);
    if (ret < 0) {
        perror("mount");
        exit(0);
    }

    return 0;
}

int main(int argc, char **argv) {

    // surpress warnings for now
    // TODO: use these for something
    argc = argc;
    argv = argv;

    DIR *sblock;
    DIR *current_device;
    struct dirent *entry;
    struct dirent *partition_entry;

    sblock = opendir("/sys/block");
    if (sblock == NULL) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(sblock)) != NULL) {
        if (streq(entry->d_name, ".") || streq(entry->d_name, ".."))
            continue;

        char device_path[512];
        snprintf(device_path,
                 sizeof(device_path),
                 "/sys/block/%s",
                 entry->d_name);

        current_device = opendir(device_path);
        if (current_device == NULL) {
            perror("opendir");
            return -1;
        }

        while ((partition_entry = readdir(current_device)) != NULL) {
            if (streq(entry->d_name, ".") || streq(entry->d_name, ".."))
                continue;

            // printf("Partition: %s\n", partition_entry->d_name);
            // printf("entry: %s, partition_entry: %s\n",
            //        entry->d_name,
            //        partition_entry->d_name);
            if (strncmp(partition_entry->d_name,
                        entry->d_name,
                        strlen(entry->d_name)) == 0) {

                char partition_path[512];
                snprintf(partition_path,
                         sizeof(partition_path),
                         "/dev/%s",
                         partition_entry->d_name);

                struct mntent *partition_info =
                    get_partition_info(partition_path);
                int is_mounted = (partition_info == NULL) ? 0 : 1;
                int meta_exists;

                if (is_mounted) {
                    // MAYBE: look for metadata file, this
                    // could be json
                    logger(L_INFO,
                           "Found mounted partition: %s "
                           "at: %s with type %s\n",
                           partition_entry->d_name,
                           partition_info->mnt_dir,
                           partition_info->mnt_type);

                    char meta_file[512];
                    snprintf(meta_file,
                             sizeof(meta_file),
                             "%s/.ufs",
                             partition_info->mnt_dir);

                    meta_exists = check_meta(meta_file, partition_entry);

                } else {
                    // MAYBE: mount the partition
                    // temporarily and check the meta file
                    logger(L_INFO,
                           "Found un-mouted partition: %s\n",
                           partition_entry->d_name);

                    meta_exists =
                        mount_and_check(partition_entry, partition_info);
                }
            }
        }
    }

    return 0;
}
