/*
 * ramdisk.h
 *
 *  Created on: 04-Dec-2013
 *      Author: sriraam
 */
#ifndef RAMDISK_H_
#define RAMDISK_H_
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include<sys/types.h>

struct RAMDISK_STAT {
          mode_t    st_mode;    /* protection */
          nlink_t   st_nlink;   /* number of hard links */
          uid_t     st_uid;     /* user ID of owner */
          gid_t     st_gid;     /* group ID of owner */
          off_t     st_size;    /* total size, in bytes */
          time_t    atime;   /* time of last access */
          time_t    mtime;   /* time of last modification */
          time_t    ctime;   /* time of last status change */
};

struct FILES{
	char *name;
	char *fullName;
	char *contents;
	struct RAMDISK_STAT stat;
	struct FILES *nextFile;
};
struct DIRS {
	char *fullName;
	char *name;
	struct FILES *files;
	struct DIRS *subDirs;
	struct DIRS *nextDir;
	struct RAMDISK_STAT stat;
};
struct FILE_SYSTEM{
	size_t currentSize;
	size_t maxSize;
	struct DIRS *dirs;
};


typedef struct FILE_SYSTEM FileSystem;
typedef struct FILES Files;
#endif /* RAMDISK_H_ */
