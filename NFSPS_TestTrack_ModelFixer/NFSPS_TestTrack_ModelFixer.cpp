// NFSPS_TestTrack_ModelFixer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <string.h>

#define ESOLID_LIST_CHUNK1 0x80134000
#define ESOLID_LIST_CHUNK2 0x80134010
#define ESOLID_LIST_CHUNK3 0x80134100
#define ESOLID_LIST_MAT_CHUNK 0x00134B02

char *OutputFilename;
char FileExt[16];

struct stat st = { 0 };

bool bFixupMats = true;

struct ProtoMatStruct
{
	int Pointer1;
	int Pointer2;
	int Pointer3;
	int Unk1;
	int Unk2;
	int Unk3;
	char unkbytes[8];
	int Unk4;
	int Unk5;
	int Unk6;
	short ProtoChange1;
	short ProtoChange2;
	int Unk7[6];
	int Unk8;
	char Unk9[4];
	int Unk10[10];
	int Unk11;
	int Unk12;
}ProtoMat;

struct FinalMatStruct
{
	int Pointer1; // 0
	int Pointer2; // 4
	int Pointer3; // 8
	int Unk1; // C
	int Unk2; // 10
	int Unk3; // 14
	char unkbytes[8]; // 18
	int Unk4; // 20
	int Unk5; // 24
	int Unk6; // 28
	int ProtoChange1; // 2C
	int ProtoChange2; // 30
	int Unk7[6]; // 34
	int Unk8; // 3A
	char Unk9[4]; // 3E
	int Unk10[10]; // 42
	int Unk11;
}FinalMat;

void *OutFile;

void ProtoToFinalBridge()
{
	FinalMat.Pointer1 = ProtoMat.Pointer1;
	FinalMat.Pointer2 = ProtoMat.Pointer2;
	FinalMat.Pointer3 = ProtoMat.Pointer3;
	FinalMat.Unk1 = ProtoMat.Unk1;
	FinalMat.Unk2 = ProtoMat.Unk2;
	FinalMat.Unk3 = ProtoMat.Unk3;
	memcpy(&FinalMat.unkbytes, &ProtoMat.unkbytes, sizeof(ProtoMat.unkbytes));
	FinalMat.Unk4 = ProtoMat.Unk4;
	FinalMat.Unk5 = ProtoMat.Unk5;
	FinalMat.Unk6 = ProtoMat.Unk6;
	FinalMat.ProtoChange1 = ProtoMat.ProtoChange1;
	FinalMat.ProtoChange2 = ProtoMat.ProtoChange2;
	memcpy(&FinalMat.Unk7, &ProtoMat.Unk7, sizeof(ProtoMat.Unk7));
	FinalMat.Unk8 = ProtoMat.Unk8;
	memcpy(&FinalMat.Unk9, &ProtoMat.Unk9, sizeof(ProtoMat.Unk9));
	memcpy(&FinalMat.Unk10, &ProtoMat.Unk10, sizeof(ProtoMat.Unk10));
	FinalMat.Unk11 = ProtoMat.Unk11;
}

int ParseAndFixMaterial(FILE *f, int RelativeEnd, int ChunkSize)
{
	bool bFinishedPadding = false;
	int MatsOffset = 0;
	int PaddingStart = 0;
	int PaddingSize = 0;
	int TrueChunkSize = 0;
	int NumMaterial = 0;
	//printf("Fixing material here!!\n");
	PaddingStart = ftell(f);
	while (ftell(f) < RelativeEnd)
	{

		if (!bFinishedPadding)
		{
			if (fgetc(f) != 0x11)
			{
				bFinishedPadding = true;
				fseek(f, -1, SEEK_CUR);
				MatsOffset = ftell(f);
				PaddingSize = MatsOffset - PaddingStart;
				TrueChunkSize = ChunkSize - PaddingSize;
				NumMaterial = TrueChunkSize / sizeof(ProtoMatStruct);
				break;
			}
		}
		else
		{
			fseek(f, 4, SEEK_CUR);
		}
	}

	for (unsigned int i = 0; i < NumMaterial; i++)
	{
		memset(&ProtoMat, 0, sizeof(ProtoMatStruct));
		memset(&FinalMat, 0, sizeof(FinalMatStruct));
		fread(&ProtoMat, sizeof(ProtoMatStruct), 1, f);
		
		if (ProtoMat.ProtoChange2)
			ProtoToFinalBridge();
		else
			memcpy(&FinalMat, &ProtoMat, 128);
		


		if ((*(int*)((int)(&(FinalMat)) + 0x4) > 4) && bFixupMats)
			*(int*)((int)(&(FinalMat)) + 0x4) = *(int*)((int)(&(FinalMat)) + 0x4) + 1;


		printf("Patching output memory image at @ 0x%X !\n", MatsOffset);
		// patch memory image of the file now
		memcpy((void*)((int)OutFile + MatsOffset), &FinalMat, sizeof(FinalMatStruct));
		MatsOffset += sizeof(ProtoMatStruct);
		fseek(f, MatsOffset, SEEK_SET);
	}

	//ProtoToFinalBridge();
	//printf("Patching output memory image at @ 0x%X !\n", MatsOffset);
	//// patch memory image of the file now
	//memcpy((void*)((int)OutFile + MatsOffset), &FinalMat, sizeof(FinalMatStruct));

	fseek(f, PaddingStart, SEEK_SET);
	fseek(f, ChunkSize, SEEK_CUR);

	//memcpy((void*)((int)OutFile + MatsOffset), &PreDefMat, sizeof(FinalMatStruct));

	return 0;
}

int SearchChunk4(FILE *f, int RelativeEnd)
{
	int ChunkType;
	int ChunkSize;

	while (ftell(f) < RelativeEnd)
	{
		fread(&ChunkType, sizeof(int), 1, f);
		fread(&ChunkSize, sizeof(int), 1, f);

		if (ChunkType == ESOLID_LIST_MAT_CHUNK)
		{
			printf("Found esolid mat chunk 0x%X size 0x%X @ 0x%X\n", ChunkType, ChunkSize, ftell(f) - sizeof(int) * 2);
			ParseAndFixMaterial(f, ftell(f) + ChunkSize, ChunkSize);
		}
		else
			fseek(f, ChunkSize, SEEK_CUR);
	}
	return 0;
}

int SearchChunk3(FILE *f, int RelativeEnd)
{
	int ChunkType;
	int ChunkSize;
	
	while (ftell(f) < RelativeEnd)
	{
		fread(&ChunkType, sizeof(int), 1, f);
		fread(&ChunkSize, sizeof(int), 1, f);

		if (ChunkType == ESOLID_LIST_CHUNK3)
		{
			printf("Found esolid chunk 3 0x%X size 0x%X @ 0x%X\n", ChunkType, ChunkSize, ftell(f) - sizeof(int) * 2);
			SearchChunk4(f, ftell(f) + ChunkSize);
		}
		else
			fseek(f, ChunkSize, SEEK_CUR);
	}
	return 0;
}

int SearchChunk2(FILE *f, int RelativeEnd)
{
	int ChunkType;
	int ChunkSize;

	while (ftell(f) < RelativeEnd)
	{
		fread(&ChunkType, sizeof(int), 1, f);
		fread(&ChunkSize, sizeof(int), 1, f);

		if (ChunkType == ESOLID_LIST_CHUNK2)
		{
			printf("Found esolid chunk 2 0x%X size 0x%X @ 0x%X\n", ChunkType, ChunkSize, ftell(f) - sizeof(int) * 2);

			//printf("Patching output memory image at @ 0x%X !\n", ftell(f) - sizeof(int) * 2);
			//*(int*)((int)OutFile + ftell(f) - sizeof(int) * 2) = 0;
			//fseek(f, ChunkSize, SEEK_CUR);

			SearchChunk3(f, ftell(f) + ChunkSize);
		}
		else
			fseek(f, ChunkSize, SEEK_CUR);
	}
	return 0;
}


int FixMaterialChunks(const char* InFilename, const char* OutFilename)
{
	if (stat(InFilename, &st) == -1)
	{

		printf("ERROR: Can't open file %s for reading!\n", InFilename);
		perror("ERROR");
		return -1;
	}

	FILE *f = fopen(InFilename, "rb");
	
	int ChunkType;
	int ChunkSize;
	OutFile = malloc(st.st_size); // loading the entire file into memory, idc atm.
	if (!f)
	{
		printf("ERROR: Can't open file %s for reading!\n", InFilename);
		perror("ERROR");
		return -1;
	}

	FILE *fout = fopen(OutFilename, "wb");
	if (!fout)
	{
		printf("ERROR: Can't open file %s for writing!\n", OutFilename);
		perror("ERROR");
		return -1;
	}

	printf("Loading file %s into memory (size: %d bytes / 0x%X) ...\n", InFilename, st.st_size, st.st_size);
	fread(OutFile, st.st_size, 1, f);
	fseek(f, 0, SEEK_SET);

	while (ftell(f) < st.st_size)
	{

		fread(&ChunkType, sizeof(int), 1, f);
		fread(&ChunkSize, sizeof(int), 1, f);

		if (ChunkType == ESOLID_LIST_CHUNK1)
		{
			printf("Found esolid chunk 1 0x%X size 0x%X @ 0x%X\n", ChunkType, ChunkSize, ftell(f) - sizeof(int) * 2);
			SearchChunk2(f, ftell(f) + ChunkSize);
		}
		else
		{
			printf("Skipping chunk 0x%X size 0x%X @ 0x%X\n", ChunkType, ChunkSize, ftell(f) - sizeof(int) * 2);
			fseek(f, ChunkSize, SEEK_CUR);
		}

	}
	// dump the modded memory image...
	printf("Writing %s (size: %d bytes / 0x%X) to disk!\n", OutFilename, st.st_size, st.st_size);
	fwrite(OutFile, st.st_size, 1, fout);

	free(OutFile);
	fclose(fout);
	fclose(f);
	return 0;
}

int main(int argc, char *argv[])
{
	printf("NFS ProStreet Proto Material to Final patcher\n\n");
	if (argc < 2)
	{
		printf("ERROR: Too few arguments.\nUSAGE: %s ESOLIDBUNDLE [OUTFILENAME]\n", argv[0]);
		return -1;
	}
	if (argc == 2)
	{
		char* PatchPoint;
		OutputFilename = (char*)calloc(strlen(argv[1]), sizeof(char) + 8);
		strcpy(OutputFilename, argv[1]);
		PatchPoint = strrchr(OutputFilename, '.');
		strcpy(FileExt, PatchPoint);
		sprintf(PatchPoint, "_patched%s", FileExt);
	}
	else
	{
		OutputFilename = argv[2];
	}
	FixMaterialChunks(argv[1], OutputFilename);
    return 0;
}

