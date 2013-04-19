///////////////////////////////////////////////////////////////////////////////
//	
//						Copyright 2013 xxxxxxx, xxxxxxx
//	File:	elfmerge.c
//	Author:	Bala bhat (bhat.balasubramanya@gmail.com)
//
//	Description: This tool merges the two elf executable files.
//		Basically the LOADable segments from both elf files are merged.
//		The resulting file only has elf header and program header. Section headers 
//		are dropped.
//		The entry point is set to be the entry point of the first elf file.
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
#include "elf.h"

typedef struct Elf32_Segment
{
	Elf32_Phdr 				programHdr;
	void 					* programData;
	struct Elf32_Segment 	* next;	
} Elf32_Segment;

typedef struct Elf32_Program
{
	Elf32_Ehdr				fileHdr;
	struct Elf32_Segment 	* segmentHead;
	struct Elf32_Segment 	* segmentTail;
	int						segmentCount;
} Elf32_Program;

Elf32_Program elfProg1;
Elf32_Program elfProg2;
Elf32_Program elfProgOut;

int readElfProgram( int elf, Elf32_Program * program);
int readElfHeader( int elf, Elf32_Ehdr *ehdr );
int readProgramHeader( int elf, off_t off, Elf32_Phdr *phdr );
int readElfData( int elf, off_t off, size_t size, char *buf );
void freeElfProgram(Elf32_Program *program);
void freeElfSegment(Elf32_Segment *segment);
int mergeElfPrograms(Elf32_Program *prog1, Elf32_Program *prog2, Elf32_Program *out);
int writeElfProgram(int elf, Elf32_Program *out);

#define	PF_R		0x4		/* p_flags */
#define	PF_W		0x2
#define	PF_X		0x1

char phFpags [][4] = { 	"---",
						"--x",
						"-w-",
						"-wx",
						"r--",
						"r-x",
						"rw-",
						"rwx" };

int writeElfProgram(int elf, Elf32_Program *out)
{
	if(!out) return -1;
	
	// First write the elf header
	write(elf, &out->fileHdr, sizeof(Elf32_Ehdr));

	// Now copy program headers	
	Elf32_Segment * segment = out->segmentHead;
	while(segment) {
		
		// Write the section headers
		write(elf, &segment->programHdr, sizeof(Elf32_Phdr));
	
		// Go to next segment		
		segment = segment->next;
	}
	
	// Now the segment data
	segment = out->segmentHead;
	while(segment) {
		
		// Write the section headers
		write(elf, segment->programData, segment->programHdr.p_filesz);
	
		// Go to next segment		
		segment = segment->next;
	}
	
	return 0;
}

int mergeElfPrograms(Elf32_Program *prog1, Elf32_Program *prog2, Elf32_Program *out)
{
	if(!prog1 || !prog2 || !out) return -1;

	// Copy the e_ident from the first file
	for(int i = 0; i < EI_NIDENT; i++) {
		out->fileHdr.e_ident[i] = prog1->fileHdr.e_ident[i];
	}
	
	out->fileHdr.e_type = ET_EXEC;
	out->fileHdr.e_machine = EM_ARM;
	out->fileHdr.e_version = EV_CURRENT;
	out->fileHdr.e_entry = prog1->fileHdr.e_entry;
	out->fileHdr.e_phoff = sizeof(Elf32_Ehdr);
	out->fileHdr.e_shoff = 0;	// No section headers
	out->fileHdr.e_flags = prog1->fileHdr.e_flags;
	out->fileHdr.e_ehsize = sizeof(Elf32_Ehdr);
	out->fileHdr.e_phentsize = sizeof(Elf32_Phdr);

	// Move the whole section list from the first input program
	out->segmentHead = prog1->segmentHead;
	out->segmentTail = prog1->segmentTail;
	out->segmentCount = prog1->segmentCount;
	prog1->segmentHead = prog1->segmentTail = NULL;
	prog1->segmentCount = 0;
		
	printf("* segmentCount = %d\n", out->segmentCount);
		
	// Add the segment list from second program also to the output program
	if(out->segmentHead) {
		out->segmentTail->next = prog2->segmentHead;
		out->segmentTail =  prog2->segmentTail;
	}
	else {
		out->segmentHead = prog2->segmentHead;
		out->segmentTail = prog2->segmentTail;
	}
	
	out->segmentCount += prog2->segmentCount;
	prog2->segmentHead = prog2->segmentTail = NULL;
	prog2->segmentCount = 0;
	
	out->fileHdr.e_phnum = out->segmentCount;
	
	printf("* segmentCount = %d\n", out->segmentCount);
	
	// Now we have to correct the virtual addresses of individual segments 
	// and correct file offsets of data pointers
	Elf32_Segment * segment = out->segmentHead;
	int offset = sizeof(Elf32_Ehdr) + out->segmentCount * sizeof(Elf32_Phdr);
	while(segment) {
		
		// TODO: Validate the virtual addresses
	
		// Correct file offsets
		segment->programHdr.p_offset = offset;
		offset += segment->programHdr.p_filesz;

		// Go to next segment		
		segment = segment->next;
	}
		
	return 0;
}

int readElfProgram( int elf, Elf32_Program * program)
{
	if(elf < 0) return -1;
	if(program == NULL) return -1;
	
	// Read elf file header
	if( readElfHeader(elf, &program->fileHdr) < 0 ) {
		return -1;
	}
	
	// Step through each program header
	for(int i = 0; i < program->fileHdr.e_phnum; i++) {
		
		Elf32_Segment * segment = (Elf32_Segment *)malloc(sizeof(Elf32_Segment));
		if(!segment)
		{
			fprintf(stderr,"malloc(%ld): %s\n", sizeof(Elf32_Segment), strerror(errno));
		}
		
		// Clear the Elf32_Segment
		memset(segment, 0, sizeof(Elf32_Segment));
		
		// Attach to the program structure
		if(program->segmentTail)	
		{
			program->segmentTail->next = segment;
		}
		else
		{
			program->segmentHead = segment;
		} 
		program->segmentTail = segment;
		program->segmentCount++;
		
		if(readProgramHeader(elf, (program->fileHdr.e_phoff) + (sizeof(Elf32_Phdr) * i), &segment->programHdr) < 0 ) {
			freeElfProgram(program);
			return -1;
		}
		
		printf("* p[%d] 0x%08x [%8d] %s\n", i, segment->programHdr.p_paddr, segment->programHdr.p_memsz, 
											phFpags[segment->programHdr.p_flags & 0x7]);
		
		if(segment->programHdr.p_memsz > 0)
		{
			// Get a buffer for this Program
			if( (segment->programData = (void*)malloc(segment->programHdr.p_memsz)) == NULL ) {
				fprintf(stderr,"malloc(%d): %s\n", segment->programHdr.p_memsz, strerror(errno));
				freeElfProgram(program);
				return -1;
			}
		
			// Read in this block of data
			if( readElfData(elf, segment->programHdr.p_offset, segment->programHdr.p_filesz, segment->programData) < 0 ) {
				freeElfProgram(program);
				return -1;
			}
		}
	}
	
	printf("* entry=0x%08x\n",program->fileHdr.e_entry);

	return 0;
}

void freeElfSegment(Elf32_Segment *segment)
{
	if(!segment) return;
	free(segment->programData);
	free(segment);
}

void freeElfProgram(Elf32_Program *program)
{
	if(program == NULL) return;
	
	// Free all segments
	while(program->segmentHead)
	{
		Elf32_Segment * freeseg = program->segmentHead;
		program->segmentHead = program->segmentHead->next;
		freeElfSegment(freeseg);
		program->segmentCount--;
	}
	program->segmentTail = NULL;
}

int readElfHeader( int elf, Elf32_Ehdr *ehdr )
{
	// rewind the file first
	if( lseek(elf,0,SEEK_SET) == (off_t)-1 ) {
		fprintf(stderr,"lseek(elf,0,SEEK_SET): %s\n",
						strerror(errno));
		return -1;
	}
	
	// now read in our structure
	if( read(elf, ehdr, sizeof(*ehdr)) == -1 ) {
		fprintf(stderr,"read(elf, ehdr, sizeof(*ehdr)): %s\n",
						strerror(errno));
		return -2;
	}
	
	return 0;
}

int readProgramHeader( int elf, off_t off, Elf32_Phdr *phdr )
{
	// rewind the file first
	if( lseek(elf,off,SEEK_SET) == (off_t)-1 ) {
		fprintf(stderr,"lseek(elf,%lld,SEEK_SET): %s\n",
						off,
						strerror(errno));
		return -1;
	}
	
	// now read in our structure
	if( read(elf, phdr, sizeof(*phdr)) == -1 ) {
		fprintf(stderr,"read(elf, phdr, sizeof(*phdr)): %s\n",
						strerror(errno));
		return -2;
	}
	
	return 0;
}

// read in a block of data from file to specified buffer
int readElfData( int elf, off_t off, size_t size, char *buf )
{
	// rewind the file first
	if( lseek(elf,off,SEEK_SET) == (off_t)-1 ) {
		fprintf(stderr,"lseek(elf,%lld,SEEK_SET): %s\n",
						off,
						strerror(errno));
		return -1;
	}
	
	// now read in our structure
	if( read(elf, buf, size) == -1 ) {
		fprintf(stderr,"read(elf, buf, %zu): %s\n",
						size,
						strerror(errno));
		return -2;
	}	
	
	return 0;
}

int main( int argc, char *argv[] ) 
{
	int elf1;
	int elf2;
	int elfOut;
		
	if( argc != 4 ) {
		fprintf(stderr,"\nSYNTAX:\n%s <elf file 1> <elf file 2> <output elf file>\n",argv[0]);
		fprintf(stderr,"\n \
			This tool merges the two elf executable files. \n \
			Basically the LOADable segments from both elf files are merged. \n \
			The resulting file only has elf header and program header. \n \
			Section headers are dropped. \n \
			The entry point is set to be the entry point of the first elf file. \n\n");
		return -1;
	}
	
	// Open input files
	if( (elf1=open(argv[1],O_RDONLY)) < 0 ) {
		fprintf(stderr,"open(\"%s\",O_RDONLY): %s\n",
						argv[1],
						strerror(errno));
		return elf1;
	}
	if( (elf2=open(argv[2],O_RDONLY)) < 0 ) {
		fprintf(stderr,"open(\"%s\",O_RDONLY): %s\n",
						argv[2],
						strerror(errno));
		return elf2;
	}
	if( (elfOut=open(argv[3],O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0 ) {
		fprintf(stderr,"open(\"%s\",O_WRONLY|O_CREAT): %s\n",
						argv[3],
						strerror(errno));
		return elfOut;
	}
	
	//////////////////////////////////////////////////////////////////////////////////////
	// Process the first file
	//////////////////////////////////////////////////////////////////////////////////////
	
	if(readElfProgram(elf1, &elfProg1) < 0) {
		goto Exit;
	}
	
	if(readElfProgram(elf2, &elfProg2) < 0) {
		goto Exit;
	}
	
	mergeElfPrograms(&elfProg1, &elfProg2, &elfProgOut);
	
	writeElfProgram(elfOut, &elfProgOut);
	
Exit:
	close(elf1);
	close(elf2);
	close(elfOut);

	freeElfProgram(&elfProg1);
	freeElfProgram(&elfProg2);
	freeElfProgram(&elfProgOut);
	return 0;
}
