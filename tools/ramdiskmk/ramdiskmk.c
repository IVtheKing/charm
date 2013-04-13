///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	elfmerge.c
//	Author:	Bala bhat (bhat.balasubramanya@gmail.com)
//
//	Description: This tool creates a ramdisk image and adds a new file to the ramdisk
//	If the ramdisk file already exists, then it just appends a new file to te ramdisk.
//	Otherwise it creates a ramdisk file containing the file to be added.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stddef.h>

#include "ramdisk.h"

Node_Ramdisk ramdisk;

int makeRamdiskNode(int rdfile, Node_Ramdisk *rd);
int readRamdiskHdr( int rdfile, FS_RamdiskHdr *rd );
int readFileData( int rdfile, off_t off, size_t size, char *buf );
int readFileHeader( int rdfile, off_t off, FS_FileHdr *fileHdr );

int readRamdiskHdr( int rdfile, FS_RamdiskHdr *rd )
{
	// Rewind the file first
	if( lseek(rdfile,0,SEEK_SET) == (off_t)-1 ) {
		fprintf(stderr,"lseek(elf,0,SEEK_SET): %s\n",
						strerror(errno));
		return -1;
	}
	
	// Now read in our structure
	if( read(rdfile, rd, sizeof(FS_RamdiskHdr)) == -1 ) {
		fprintf(stderr,"read(rd, rd, sizeof(FS_RamdiskHdr)): %s\n",
						strerror(errno));
		return -2;
	}
	
	return 0;
}

int readFileHeader( int rdfile, off_t off, FS_FileHdr *fileHdr )
{
	// Rewind the file first
	if( lseek(rdfile, off, SEEK_SET) == (off_t)-1 ) {
		fprintf(stderr,"lseek(elf,%lld,SEEK_SET): %s\n",
						off,
						strerror(errno));
		return -1;
	}
	
	// now read in our structure
	if( read(rdfile, fileHdr, sizeof(*fileHdr)) == -1 ) {
		fprintf(stderr,"read(rdfile, fileHdr, sizeof(*fileHdr)): %s\n",
						strerror(errno));
		return -2;
	}
	
	return 0;
}

// read in a block of data from file to specified buffer
int readFileData( int rdfile, off_t off, size_t size, char *buf )
{
	// Rewind the file first
	if( lseek(rdfile,off,SEEK_SET) == (off_t)-1 ) {
		fprintf(stderr,"lseek(elf,%lld,SEEK_SET): %s\n",
						off,
						strerror(errno));
		return -1;
	}
	
	// now read in our structure
	if( read(rdfile, buf, size) == -1 ) {
		fprintf(stderr,"read(rdfile, buf, %zu): %s\n",
						size,
						strerror(errno));
		return -2;
	}	
	
	return 0;
}

Node_File * readFileNode(int rdfile, int offset, Node_File *parent)
{
	Node_File * file = (Node_File *)malloc(sizeof(Node_File));
	if(!file)
	{
		fprintf(stderr,"malloc(%ld): %s\n", sizeof(Node_File), strerror(errno));
	}
		
	// Clear the Elf32_Segment
	memset(file, 0, sizeof(Node_File));
		
	// Attach the file to the parent
	if(parent)
	{
		if(parent->child)
		{
			file->next = parent->child;
		}
		
		parent->child = file;
		file->parent = parent;					
	}
	
	if(readFileHeader(rdfile, offset, &file->fileHdr) < 0 ) {
		return NULL;
	}
	
	if((file->fileHdr.flags & F_DIR_MASK) == F_DIR) 
	{
		for(int i = 0; i < file->fileHdr.fileCount; i++)
		{
			Node_File * child = readFileNode(rdfile, file->fileHdr.offset + i * sizeof(Node_File), 	file);
			
			if(!child) 
			{
				fprintf(stderr,"ERROR: Error while reading folder - %s\n", file->fileHdr.fileName);
				break;
			}
			
			if(file->child) 
			{
				child->next = file->child;
			}
			
			file->child = child;
		}
	}
	else if(file->fileHdr.length > 0)
	{

		file->data = (INT8 *) malloc(file->fileHdr.length);
		if(readFileData(rdfile, file->fileHdr.offset, file->fileHdr.length, file->data) < 0)
		{
			fprintf(stderr,"ERROR: Reading data for file - %s\n", file->fileHdr.fileName);
		}
	}
	
	return file;
}

int makeRamdiskNode(int rdfile, Node_Ramdisk *rd)
{
	if(rdfile < 0) return -1;
	if(rd == NULL) return -1;
	
	// Read ramdisk file header
	if(readRamdiskHdr(rdfile, &rd->rdHdr) < 0) {
		return -1;
	}

	// Validate the ramdisk file
	if(strcmp(rd->rdHdr.ident, RAMDISK_IDENT_STRING) != 0) {
		fprintf(stderr,"The input file is not a valid ramdisk image\n");
		return -1;
	}
	
	// Read the root folder
	rd->root = readFileNode(rdfile, rd->rdHdr.rootOffset, NULL);
	
	if(!rd->root)
	{
		fprintf(stderr,"ERROR: Root file system is empty\n");
	}

	return 0;
}

int main( int argc, char *argv[] ) 
{
	int rdfile;
	int addfile;
	
	if(argc != 3) {
		fprintf(stderr,"\nSYNTAX:\n%s <ramdisk file> <new file>\n",argv[0]);
		fprintf(stderr,"\n \
			This tool creates a ramdisk image and adds a new file to the ramdisk \
			If the ramdisk file already exists, then it just appends a new file to te ramdisk. \
			Otherwise it creates a ramdisk file containing the file to be added. \n\n");
		return -1;
	}
	
	// Open input files
	if((addfile = open(argv[1],O_RDONLY)) < 0) {
		fprintf(stderr,"open(\"%s\",O_RDONLY): %s\n",
						argv[1],
						strerror(errno));
		return addfile;
	}

	rdfile=open(argv[1],O_RDONLY);
	
	//////////////////////////////////////////////////////////////////////////////////////
	// Process the first file
	//////////////////////////////////////////////////////////////////////////////////////
	
	if(rdfile < 0) {	
		fprintf(stderr,"Ramdisk file \"%s\" does not exist. Creating a new ramdisk file.\n", argv[1]);
		
		memset(&ramdisk, 0, sizeof(Node_Ramdisk));
		strncpy(ramdisk.rdHdr.ident, RAMDISK_IDENT_STRING, RAMDISK_IDENT_LENGTH);
		ramdisk.rdHdr.rootOffset = sizeof(FS_RamdiskHdr);
		ramdisk.rdHdr.size = sizeof(FS_RamdiskHdr);
		
		// Add a root folder
		ramdisk.root = (Node_File *) malloc(sizeof(Node_File));
		memset(&ramdisk.root, 0, sizeof(Node_File));
		
		strcpy(ramdisk.root->fileHdr.fileName, "/");
		ramdisk.root->fileHdr.flags = F_DIR | (S_IRUSR | S_IWUSR | S_IXUSR) | S_IRGRP | S_IROTH;

		ramdisk.root->fileHdr.fileCount = 0;		
		ramdisk.root->parent = NULL;
	}
	else { 
		if(makeRamdiskNode(rdfile, &ramdisk) < 0) {
			goto Exit;
		}
		else {
			// We are done reading the ramdisk. Close the file
			close(rdfile);		
		}
	}
	
//	ramdiskAddFile(&ramdisk, &newfile);


	// Open ramdisk file for output
	if((rdfile = open(argv[1],O_WRONLY)) < 0) {
		fprintf(stderr,"open(\"%s\",O_WRONLY): %s\n",
						argv[1],
						strerror(errno));
		return rdfile;
	}
	
//	ramdiskWriteFile(rdfile, &ramdisk);
	
Exit:
	
	return 0;
}
