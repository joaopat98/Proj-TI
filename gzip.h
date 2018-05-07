/* Author: Rui Pedro Paiva
Teoria da Informa��o, LEI, 2007/2008*/

#include <stdio.h>
#include "huffman.h"

#define ALPHA_SIZE 286
#define C_ALPHA_SIZE 19
#define D_ALPHA_SIZE 30
#define BLOCK_END 256

#define COPY_LAST 16
#define COPY_ZEROS 17
#define COPY_ZEROS2 18
// Gzip header variables
typedef struct header {
    //elementos fixos
    unsigned char ID1, ID2, CM, XFL, OS;
    unsigned long MTIME;
    unsigned char FLG_FTEXT, FLG_FHCRC, FLG_FEXTRA, FLG_FNAME, FLG_FCOMMENT;   //bits 0, 1, 2, 3 e 4, respectivamente (restantes 3: reservados)

    // FLG_FTEXT --> ignorado deliberadamente (tipicamente igual a 0)
    //se FLG_FEXTRA == 1
    unsigned char xlen;
    unsigned char *extraField;

    //se FLG_FNAME == 1
    char *fName;  //terminada por um byte a 0
    //se FLG_FCOMMENT == 1
    char *fComment; //terminada por um byte a 0

    //se FLG_HCRC == 1
    unsigned char *HCRC;
} gzipHeader;

typedef struct {
    unsigned char HLIT, HDIST, HCLEN;
} HuffInfo;

long getOrigFileSize(FILE *gzFile);

void updateBuffer(char needBits, FILE *ptr);

int getBits(char size, FILE *ptr);

HuffInfo getBHeader(FILE *ptr);

void fillTree(HuffmanTree *huffmanTree, char *codes[], int size);

void getCodes(char *codes[], const char lens[], int size);

void getLens(FILE *ptr, char lens[], int size, HuffmanTree *huffmanTree);

unsigned short nextSymbol(FILE *ptr, HuffmanTree *huffmanTree);

bool
nextSeq(FILE *ptr, char *buf, int *cur, const unsigned short limsf[], char bitsf[], const unsigned int limsb[], char bitsb[],
        HuffmanTree *chftree, HuffmanTree *dChftree);

int getHeader(FILE *gzFile, gzipHeader *gzh);

int isDynamicHuffman(unsigned char rb);

void bits2String(char *strBits, char len, int byte);
