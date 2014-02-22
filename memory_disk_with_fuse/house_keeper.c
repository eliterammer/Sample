/*
 * house_keeper.c
 *
 *  Created on: 06-Dec-2013
 *      Author: sriraam
 */

#include "house_keeper.h"
#include "ramdisk.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <errno.h>


void initializeDir(struct DIRS *dirs){
	dirs->files=NULL;
	dirs->nextDir=NULL;
	dirs->subDirs=NULL;
}
static void AddFileSystemSize_dir(FileSystem fileSystem,struct DIRS *dirs){
	fileSystem.currentSize+=strlen(dirs->fullName)+1;
	fileSystem.currentSize+=strlen(dirs->name)+1;

}

static void reduceFileSystemSize_dir(FileSystem fileSystem,struct DIRS *dirs){
	fileSystem.currentSize-=strlen(dirs->fullName)+1;
	fileSystem.currentSize-=strlen(dirs->name)+1;

}
int hk_mkdir(char *dirName,FileSystem fileSystem){
	struct DIRS *dirs = (struct DIRS *)malloc(sizeof(struct DIRS));
	initializeDir(dirs);
	dirs->fullName=malloc(strlen(dirName)+1);
	strcpy(dirs->fullName,dirName);
	setNameOfDir(dirs,dirs->fullName);
	//recover after strtok
	strcpy(dirs->fullName,dirName);
	dirs->stat.mtime=time(NULL);
	AddFileSystemSize_dir(fileSystem,dirs);
	if(fileSystem.currentSize >= fileSystem.maxSize){
			return -ENOSPC;
	}
	struct DIRS *parent = getParentDir_(dirName,fileSystem);
	return addDirToParent(dirs,parent,fileSystem);
}

int hk_rmdir(char *dirName,FileSystem fileSystem){
	struct DIRS *dir = getParentDir_(dirName, fileSystem);
	//printf("parent is %s\n",dir->fullName);
	struct DIRS *prev, *tmp;
	if (dir == NULL) {
		return 1;
	}
	//printf("trying to remove %s\n",dirName);
	struct DIRS *subDir = dir->subDirs;
	if (!strcmp(subDir->fullName, dirName)) {
		//printf("removing first subDir\n");
		tmp = subDir;
		if (subDir->files != NULL) {
			return -ENOTEMPTY;
		}
		if (subDir->subDirs != NULL) {
					return -ENOTEMPTY;
		}
		dir->subDirs = subDir->nextDir;
		reduceFileSystemSize_dir(fileSystem, tmp);
		free(tmp->fullName);
		free(tmp->name);
		free(tmp);
		return 0;
	}
	while (subDir != NULL) {
		if (!strcmp(subDir->fullName, dirName)) {
			if (subDir->files != NULL) {
				return -ENOTEMPTY;
			}
			if (subDir->subDirs != NULL) {
				return -ENOTEMPTY;
			}

			prev->nextDir = subDir->nextDir;
			reduceFileSystemSize_dir(fileSystem, subDir);
			free(subDir->fullName);
			free(subDir->name);
			free(subDir);
			return 0;
		}
		prev = subDir;
		subDir = subDir->nextDir;
	}
	return 0;
}
int addDirToParent(struct DIRS *dirs,struct DIRS *parent,FileSystem fileSystem){
	//printf("parent is %s\n",parent->fullName);
	struct DIRS *iterator = parent->subDirs;
	if(iterator == NULL){
		//printf("first subDir of %s is %s\n",parent->fullName,dirs->fullName);
		parent->subDirs=dirs;
		//printFS(fileSystem.dirs);
		return 0;
	}
	while (iterator->nextDir != NULL) {
		iterator = iterator->nextDir;
	}
	iterator->nextDir = dirs;
	//printf("added %s\n",dirs->fullName);
	//printFS(fileSystem.dirs);
	return 0;
}

int addFileToParent(Files *files,struct DIRS *parent,FileSystem fileSystem){
	//printf("parent is %s\n",parent->fullName);
	Files *iterator = parent->files;
	if(iterator == NULL){
		//printf("first file of %s is %s\n",parent->fullName,files->fullName);
		parent->files=files;
		return 0;
	}
	while (iterator->nextFile != NULL) {
		iterator = iterator->nextFile;
	}
	iterator->nextFile = files;
	//printf("added %s\n",files->fullName);
	return 0;
}

static struct DIRS *searchForTokenInDir(const char* dirName,struct DIRS *dirs){
	if(dirs == NULL){
		return NULL;
	}
	struct DIRS *iterator=dirs;
	while (iterator != NULL) {
		//printf("compare with %s\n",iterator->name);
		if(!strcmp(iterator->name,dirName)){
			return iterator;
		}
			iterator = iterator->nextDir;
	}
	return NULL;
}

void setNameOfDir(struct DIRS *dirs,char *fullName){
	char *token;
	token = strtok(fullName, "/");
	char *prev = token;
	while (token != NULL) {
		//printf("token is %s\n",token);
		prev = token;
		token = strtok(NULL,"/");
	}
	//printf("setting name to %s\n",prev);
	dirs->name=malloc(strlen(prev)+1);
	strcpy(dirs->name,prev);
}

void setNameOfFile(Files *files,const char *fullName){
	char *tmpName;
	char *token;
	tmpName = malloc(strlen(fullName)+1);
	strcpy(tmpName,fullName);
	token = strtok(tmpName, "/");
	char *prev = token;
	while (token != NULL) {
		//printf("token is %s\n",token);
		prev = token;
		token = strtok(NULL,"/");
	}
	//printf("setting name to %s\n",prev);
	files->name=malloc(strlen(prev)+1);
	strcpy(files->name,prev);
}


Files *getFile(const char* fileName,FileSystem fs){
	/*char *tmpName;
	tmpName = malloc(strlen(fileName)+1);
	strcpy(tmpName,fileName);
	*/
	struct DIRS *dir = getParentDir_(fileName,fs);
	Files *iterator = dir->files;
	while(iterator != NULL){
		if(!strcmp(fileName,iterator->fullName)){
			return iterator;
		}
		iterator=iterator->nextFile;
	}
	return NULL;
}

int parsePath(char *pathName,char *paths[]){
	char *token;
	char *tmpName;
	int count=0;
	tmpName = malloc(strlen(pathName)+strlen("ROOT")+1);
	strcpy(tmpName,"ROOT");
	strcat(tmpName,pathName);
	//printf("parsePath parsing %s\n",tmpName);
	token = strtok(tmpName, "/");
	while (token != NULL) {
		paths[count]=malloc(strlen(token)+1);
		strcpy(paths[count++],token);
		//printf("level %s\n", token);
		token = strtok(NULL, "/");
	}
	free(tmpName);
	return count;
}


struct DIRS* getParentDir_(const char* pathName,FileSystem fs){
	char *paths[50];
	int count=0,i=0;
	char *path;
	struct DIRS *dir=fs.dirs;
	//memset(paths,'\0',sizeof(paths));
	count = parsePath(pathName,paths);
	//printf("count is %d\n",count);
	while(i<count-1){
		path = paths[i++];
		dir = searchForTokenInDir(path,dir);
		if(i < count-1){
			dir = dir->subDirs;
		}
	}
	for(i=0;i<count;i++){
		free(paths[i]);
	}
	//printf("---parent dir is %s\n",dir->fullName);
	return dir;
}



struct DIRS *getDir(const char *pathName,FileSystem fs){
	char *paths[50];
	int count=0,i=0;
	char *path;
	struct DIRS *dir=fs.dirs;
	//memset(paths,'\0',sizeof(paths));
	count = parsePath(pathName,paths);
	//printf("count is %d\n",count);
	while(i<count-1){
		path = paths[i++];
		dir = searchForTokenInDir(path,dir);
		if(i < count-1){
			dir = dir->subDirs;
		}
	}
	//printf("parent dir is %s\n",dir->fullName);
	if(!strcmp(dir->fullName,pathName)){
		return dir;
	}
	dir = dir->subDirs;
	if(dir == NULL){
		return NULL;
	}
	while(strcmp(dir->fullName,pathName)){
		dir=dir->nextDir;
		if(dir == NULL){
			return NULL;
		}
		//printf("check with %s\n",dir->fullName);
	}
	//printf("actual dir is %s\n",dir->fullName);
	return dir;
}

static void reduceFileSystemSize_file(FileSystem fileSystem,Files *files){
	fileSystem.currentSize-=strlen(files->fullName)+1;
	fileSystem.currentSize-=strlen(files->name)+1;

}

int unlinkFile(const char* fileName,FileSystem fileSystem){
	struct DIRS *dir = getParentDir_(fileName, fileSystem);
	//printf("parent is %s file is %s\n", dir->fullName,fileName);
	Files *prev, *tmp;
	if (dir == NULL) {
		return 1;
	}
	Files *files = dir->files;
	if (!strcmp(files->fullName, fileName)) {
		//printf("removing first file %s\n",files->fullName);
		tmp = files;
		fileSystem.currentSize-=files->stat.st_size;
		dir->files = tmp->nextFile;
		reduceFileSystemSize_file(fileSystem,tmp);
		free(tmp->fullName);
		free(tmp->name);
		if(tmp->stat.st_size != 0){
			free(tmp->contents);
		}
		free(tmp);
		return 0;
	}
	while (files != NULL) {
		if (!strcmp(files->fullName, fileName)) {
			fileSystem.currentSize-=files->stat.st_size;
			prev->nextFile = files->nextFile;
			reduceFileSystemSize_file(fileSystem,files);
			free(files->fullName);
			free(files->name);
			free(files->contents);
			free(files);
			return 0;
		}
		prev = files;
		files = files->nextFile;
	}
	return 0;

}


void renameFile(Files *file,char *from,char *to,FileSystem fs){
	Files *old = getFile(from,fs);
	Files *cpy=(Files *)malloc(sizeof(Files));
	cpy->fullName=malloc(strlen(to)+1);

	strcpy(cpy->fullName,to);
	setNameOfFile(cpy,to);

	cpy->nextFile=NULL;
	if(old->contents != NULL){
		cpy->contents=malloc(old->stat.st_size);
		memcpy(cpy->contents,old->contents,old->stat.st_size);
	}

	cpy->stat.st_size=file->stat.st_size;
	cpy->stat.mtime=file->stat.mtime;
	unlinkFile(from,fs);

	struct DIRS *toDir = getParentDir_(to,fs);
	//printf("to dir is %s\n",toDir->fullName);
	addFileToParent(cpy,toDir,fs);
}


static void renameRecursively(struct DIRS *dir,char *from,char *to,FileSystem fs){

	hk_mkdir(to,fs);
	char *newFileName=NULL;
	char *newDirName =NULL;
	Files *iterator = dir->files;
	if(iterator != NULL){
		Files *oldFile = getFile(iterator->fullName, fs);
		while (iterator != NULL) {
			newFileName = malloc(strlen(to) + strlen(oldFile->name) + 2); //1 for /
			strcpy(newFileName, to);
			strcat(newFileName, "/");
			strcat(newFileName, oldFile->name);
			//printf("rename %s to %s\n", oldFile->fullName, newFileName);
			renameFile(oldFile, oldFile->fullName, newFileName, fs);
			iterator = iterator->nextFile;
		}
	}
	if(dir->subDirs != NULL){
		newDirName=malloc(strlen(to)+strlen(dir->subDirs->name)+2);//1 for /
		strcpy(newDirName,to);
		strcat(newDirName,"/");
		strcat(newDirName,dir->subDirs->name);
		//printf("rename %s to %s\n",dir->subDirs->fullName,newDirName);
		renameRecursively(dir->subDirs,dir->subDirs->fullName,newDirName,fs);
	}

	hk_rmdir(dir->fullName,fs);

}
void renameDir(struct DIRS *dir,char *from,char *to,FileSystem fs){
	renameRecursively(dir,from,to,fs);
}
