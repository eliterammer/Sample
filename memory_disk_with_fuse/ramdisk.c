#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include "ramdisk.h"
#include "house_keeper.h"
#include <strings.h>
#include <inttypes.h>
#include <math.h>

static FileSystem fileSystem;

static int isDirPresent(const char *path,struct DIRS *dir){
	struct DIRS *require = getDir(path,fileSystem);
	return (require != NULL);
}

static int isFilePresent(const char *path,struct DIRS *dir){
	Files *files = getFile(path,fileSystem);
	return (files != NULL);
}

static int ramdisk_getattr(const char *path, struct stat *stbuf)
{
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		stbuf->st_size=4096;
		stbuf->st_uid=getuid();
		stbuf->st_gid=getgid();
		struct DIRS *dir = getDir(path,fileSystem);
		stbuf->st_mtim.tv_sec=dir->stat.mtime;
		return 0;
	}else{
		if(isDirPresent(path,fileSystem.dirs)){
			stbuf->st_mode = S_IFDIR | 0755;
			stbuf->st_nlink = 2;
			stbuf->st_size=4096;
			stbuf->st_uid=getuid();
			stbuf->st_gid=getgid();
			struct DIRS *dir = getDir(path,fileSystem);
			stbuf->st_mtim.tv_sec=dir->stat.mtime;
			return 0;
		}
		if(isFilePresent(path,fileSystem.dirs)){
			stbuf->st_mode = S_IFREG | 0755;
			stbuf->st_nlink = 1;
			stbuf->st_uid=getuid();
			stbuf->st_gid=getgid();
			Files *file = getFile(path,fileSystem);
			////printf("file size is %d\n",file->stat.st_size);
			stbuf->st_size=file->stat.st_size;
			stbuf->st_mtim.tv_sec=file->stat.mtime;
			return 0;
		}
		return -ENOENT;
	}
}

void ramdisk_ls(struct DIRS *dir,void *buf,fuse_fill_dir_t filler){
	struct DIRS *subDir=NULL;
	Files *files=NULL;
	if (dir == NULL) {
		return;
	}
	subDir = dir->subDirs;
	while(subDir != NULL){
		////printf("listing %s\n",subDir->fullName);
		filler(buf, subDir->name, NULL, 0);
		subDir=subDir->nextDir;
	}
	////printf("dirs listed\n");
	files = dir->files;
	while(files != NULL){
		////printf("listing %s\n",files->name);;
		filler(buf, files->name, NULL, 0);
		files=files->nextFile;
	}
}

static int ramdisk_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi){
	////printf("called with %s\n",path);
	struct DIRS *dir = getDir(path,fileSystem);
	(void) offset;
	(void) fi;
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	ramdisk_ls(dir,buf,filler);
	return 0;
}


static int ramdisk_read(const char *pathName, char *buffer, size_t size, off_t offset,
		      struct fuse_file_info *fi){
	Files *file = getFile(pathName,fileSystem);
	////printf("file size is %d\n",file->stat.st_size);
	////printf("requested %d from %d\n",size,offset);
	////printf("returinig %d\n",file->stat.st_size-offset);
	if(size > (file->stat.st_size-offset)){
		memcpy(buffer,file->contents+offset,file->stat.st_size-offset);
		return file->stat.st_size-offset;
	}else{
		memcpy(buffer,file->contents+offset,size);
		return size;
	}
}

static int ramdisk_truncate (const char *pathName, off_t offset){
	Files *file = getFile(pathName,fileSystem);
	fileSystem.currentSize-=file->stat.st_size;
	file->stat.st_size=0;
	free(file->contents);
	file->contents=NULL;
	return 0;
}




static int ramdisk_write(const char *pathName, const char *buffer, size_t size, off_t offset,
	      struct fuse_file_info *info){
	//printf("current size is %ld vs %ld\n",fileSystem.currentSize,fileSystem.maxSize);

	if(fileSystem.currentSize + size > fileSystem.maxSize){
		return -ENOSPC;
	}
	fileSystem.currentSize+=size;
	Files *file = getFile(pathName,fileSystem);
	//printf("current file size is %d offset is %d size is %d\n",file->stat.st_size,offset,size);

	size_t newFileSize = 0;
	if(file->contents == NULL){
		file->contents=(char *)malloc(size);
		newFileSize=size;
	}else{
		//printf("else\n");
		if(offset > 0){
			newFileSize+=file->stat.st_size+size;
		}else{
			newFileSize=size;
		}
		////printf("reallocing to %zd\n",newFileSize);
		file->contents=realloc(file->contents,newFileSize);
	}
	memcpy((file->contents)+offset,buffer,size);
	//file->contents[size+offset]='\0';
	file->stat.st_size=newFileSize;
	return size;
}




int ramdisk_unlink(const char *fileName){
	return unlinkFile(fileName,fileSystem);
}


static int ramdisk_rmdir (const char *dirName){
	return hk_rmdir(dirName,fileSystem);
}



static int ramdisk_mkdir(const char *dirName, mode_t mode){
	return hk_mkdir(dirName,fileSystem);
}





static void AddFileSystemSize_file(FileSystem fileSystem,Files *files){
	fileSystem.currentSize+=strlen(files->fullName)+1;
	fileSystem.currentSize+=strlen(files->name)+1;

}






static int ramdisk_create (const char *fileName, mode_t mode, struct fuse_file_info *info){
	////printf("filename %s\n",fileName);
	struct DIRS *parent = getParentDir_(fileName,fileSystem);
	Files *files=(Files *)malloc(sizeof(Files));
	files->fullName=malloc(strlen(fileName)+1);
	strcpy(files->fullName,fileName);
	setNameOfFile(files,fileName);
	strcpy(fileName,files->fullName);
	AddFileSystemSize_file(fileSystem,files);
	if(fileSystem.currentSize >= fileSystem.maxSize){
		return -ENOSPC;
	}
	files->nextFile=NULL;
	files->contents=NULL;
	files->stat.st_size=0;
	files->stat.mtime=time(NULL);
	return addFileToParent(files,parent,fileSystem);
}

static void *ramdisk_init (struct fuse_conn_info *conn){
	struct DIRS *dirs = (struct DIRS *)malloc(sizeof(struct DIRS));
	dirs->fullName = malloc(strlen("/")+1);
	dirs->name=malloc(strlen("ROOT")+1);
	strcpy(dirs->fullName,"/");
	strcpy(dirs->name,"ROOT");
	dirs->stat.mtime=time(NULL);
	initializeDir(dirs);
	fileSystem.dirs=dirs;
}

static int ramdisk_open (const char *pathName, struct fuse_file_info *info){
	return 0;
}

static int ramdisk_rename (const char *from, const char *to){
	////printf("to move %s from %s\n",from,to);
	Files *files = getFile(from,fileSystem);
	if(files != NULL){
		renameFile(files,from,to,fileSystem);
		return 0;
	}
	renameDir(getDir(from,fileSystem),from,to,fileSystem);
	return 0;
}


static int ramdisk_utimens (const char *path, const struct timespec tv[2]){
	return 0;
}
int ramdisk_opendir (const char *path, struct fuse_file_info *info){
	return 0;
}
/*int ramdisk_flush (const char *path, struct fuse_file_info *info){
	return 0;
}*/
int ramdisk_chmod (const char *path, mode_t mode){
	return 0;
}

static struct fuse_operations hello_oper = {
	.init		= ramdisk_init,
	.getattr	= ramdisk_getattr,
	.readdir	= ramdisk_readdir,
	.open		= ramdisk_open,
	.read		= ramdisk_read,
	.write		= ramdisk_write,
	.create 	= ramdisk_create,
	.truncate	= ramdisk_truncate,
	.mkdir		= ramdisk_mkdir,
	.rmdir		= ramdisk_rmdir,
	.unlink		= ramdisk_unlink,
	.rename		= ramdisk_rename,
	.utimens	= ramdisk_utimens,
	.opendir	= ramdisk_opendir,
//	.flush		= ramdisk_flush,
	.chmod		= ramdisk_chmod,

};

int main(int argc, char *argv[]){
	////printf("argc is %d\n",argc);
	if(argc <= 2){
		printf("format is ramdisk <MOUNT_POINT> SIZE_OF_FILE_SYSTEM\n");
		exit(0);
	}
	if(atol(argv[argc-1]) <= 0){
		printf("SIZE_OF_FILE_SYSTEM must be > 0\n");
	}
	fileSystem.maxSize=atol(argv[argc-1]);
	fileSystem.maxSize*=(long)pow(2,20);
	fileSystem.currentSize=0;
	argc--;
	////printf("maxSize is %ld\n",fileSystem.maxSize);
	return fuse_main(argc, argv, &hello_oper, NULL);
}

