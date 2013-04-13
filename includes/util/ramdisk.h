///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	ramdisk.h
//	Author:	Bala B. (bhat.balasubramanya@gmail.com)
//	Description: ramdisk header file
//
//	Note: This ramdisk is a Read-Only file system. This is not optimized for writes.
//	
///////////////////////////////////////////////////////////////////////////////

#ifndef _RAMDISK_H
#define _RAMDISK_H

#include "os_types.h"
#include "os_config.h"

#define RAMDISK_IDENT_STRING	("chARMrd")
#define RAMDISK_IDENT_LENGTH	8


// The ramdisk starts with the following structure
typedef struct
{
	INT8	ident[RAMDISK_IDENT_LENGTH];		// Unique identifier for the ramdisk file
	UINT32	size;								// Total size of the ramdisk file including this header
	UINT32	rootOffset;							// Offset of the first file header
	UINT32	crc;								// CRC of the ramdisk file. The CRC computation not include 
												// ramdisk header data. It includes everything after this header
	
} FS_RamdiskHdr;

// There are zero or more file headers in the ramdisk
typedef struct 
{
	INT8 			fileName[MAX_FILE_NAME_SIZE];
	UINT32 			flags;						// file / dir attributes and Permissions
	UINT32			offset;						// Offset where the file contents begin
	union
	{
		UINT32		length;						// Size of the file, if this is not a folder
		UINT32		fileCount;					// Number of files in the folder
	};
	
	UINT32			parent;						// Offset of the parent folder
	
} FS_FileHdr;


// Memory object for Ramdisk node
typedef struct
{
	FS_RamdiskHdr rdHdr;
	struct Node_File *root;	

} Node_Ramdisk;

// Memory object for File node
typedef struct Node_File
{
	FS_FileHdr fileHdr;
	INT8 * data;
	struct Node_File *parent;
	struct Node_File *child;
	struct Node_File *next;
	
} Node_File;

enum 
{
	F_DIR_MASK 	= 0x10000000,
	F_FILE 		= 0x00000000,
	F_DIR 		= 0x10000000,
};

#ifndef DESKTOP_TARGET

INT32 open(INT8 * filename, INT32 flags);
INT32 close(INT32 fd);
INT32 read(INT32 fd, void * ptr, INT32 numbytes);
INT32 lseek(INT32 fd, INT32 position, INT32 startpoint);
	
enum 
{
	S_ISUID   	= 0x0004000,    // set user id on execution
	S_IRUSR   	= 0x0000400,    // protection: readable by owner
	S_IWUSR   	= 0x0000200,    // writable by owner
	S_IXUSR   	= 0x0000100,    // executable by owner
	S_IRGRP   	= 0x0000040,    // readable by group
	S_IWGRP   	= 0x0000020,    // writable by group
	S_IXGRP   	= 0x0000010,    // executable by group
	S_IROTH   	= 0x0000004,    // readable by all
	S_IWOTH   	= 0x0000002,    // writable by all
	S_IXOTH   	= 0x0000001     // executable by all
};
enum
{
	// File access modes for open()
	O_RDONLY          = 0,    // open(name, O_RDONLY) opens read only
	O_WRONLY          = 1,    // open(name, O_WRONLY) opens write only
	O_RDWR            = 2     // open(name, O_RDWR) opens read/write
};

enum 
{
	SEEK_SET = 0,
	SEEK_CUR = 1,
	SEEK_END = 2
}

#endif // DESKTOP_TARGET

#endif // _RAMDISK_H
