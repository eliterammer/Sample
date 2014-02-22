/*
 * house_keeper.h
 *
 *  Created on: 06-Dec-2013
 *      Author: sriraam
 */

#ifndef HOUSE_KEEPER_H_
#define HOUSE_KEEPER_H_

#include "ramdisk.h"
int hk_mkdir(char *dirName,FileSystem fileSystem);
int hk_rmdir(char *dirName,FileSystem fileSystem);

void renameFile(Files *file,char *from,char *to,FileSystem fs);
void renameDir(struct DIRS *dir,char *from,char *to,FileSystem fs);

void initializeDir(struct DIRS *dirs);
struct DIRS* getParentDir_(const char* pathName,FileSystem fs);
//struct DIRS* getParentDir(const char* dirName,FileSystem fs);
void setNameOfDir(struct DIRS *dirs,char *fullName);
void setNameOfFile(Files *files,const char *fullName);
int addFileToParent(Files *files,struct DIRS *parent,FileSystem fileSystem);
int addDirToParent(struct DIRS *dirs,struct DIRS *parent,FileSystem fileSystem);
Files *getFile(const char* fileName,FileSystem fs);
struct DIRS *getDir(const char *pathName,FileSystem fs);
int unlinkFile(const char*,FileSystem);
#endif /* HOUSE_KEEPER_H_ */
