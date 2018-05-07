/*Author: Rui Pedro Paiva
Teoria da Informação, LEI, 2007/2008*/

#include <cstdlib>
#include <climits>
#include <sys/types.h>

#include "gzip.h"

//#define DEBUG

char availBits = 0;
unsigned long rb = 0;  //último byte lido (poderá ter mais que 8 bits, se tiverem sobrado alguns de leituras anteriores)

//função principal, a qual gere o processo de descompactação
int main(int argc, char **argv) {
    HuffmanTree *CChftree;
    HuffmanTree *Chftree;
    HuffmanTree *DChftree;
    unsigned short limsf[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115,
                              131, 163,
                              195, 227, 258};

    char bitsf[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};

    unsigned int limsb[] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537,
                            2049, 3073,
                            4097, 6145, 8193, 12289, 16385, 24577};

    char bitsb[] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

    //--- Gzip file management variables
    FILE *gzFile;  //ponteiro para o ficheiro a abrir
    FILE *fp;
    FILE *fp_log = fopen("log.txt", "w");
    long fileSize;
    long origFileSize;
    int numBlocks = 0;
    gzipHeader gzh;
    HuffInfo huffInfo;
    int index = 0;
    unsigned char byte;  //variável temporária para armazenar um byte lido directamente do ficheiro
    char needBits = 0;
    char order[] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
    char cLens[C_ALPHA_SIZE], *cCodes[C_ALPHA_SIZE], lens[ALPHA_SIZE], *codes[ALPHA_SIZE], dLens[D_ALPHA_SIZE], *dCodes[D_ALPHA_SIZE];
    char str[15];
    char bit;
    char *f_buffer;


    for (int k = 0; k < C_ALPHA_SIZE; cLens[k++] = 0);

    //--- obter ficheiro a descompactar
    //char fileName[] = "FAQ.txt.gz";

    if (argc != 2) {
        printf("Linha de comando inválida!!!");
        return -1;
    }
    char *fileName = argv[1];

    //--- processar ficheiro
    gzFile = fopen(fileName, "rb");
    fseek(gzFile, 0L, SEEK_END);
    fileSize = ftell(gzFile);
    fseek(gzFile, 0L, SEEK_SET);


    //ler tamanho do ficheiro original e definir Vector com símbolos
    origFileSize = getOrigFileSize(gzFile);
    f_buffer = (char *) malloc(origFileSize * sizeof(char));


    //--- ler cabeçalho
    int erro = getHeader(gzFile, &gzh);
    if (erro != 0) {
        printf("Formato inválido!!!");
        return -1;
    }

    fp = fopen("nome.mp3", "wb");

    //--- Para todos os blocos encontrados
    char BFINAL;

    do {
        //inicializar os vetores de comprimentos e de códigos de Huffman a 0/NULL

        for (int i = 0; i < C_ALPHA_SIZE; ++i) {
            cLens[i] = 0;
            cCodes[i] = NULL;
        }

        for (int i = 0; i < ALPHA_SIZE; ++i) {
            lens[i] = 0;
            codes[i] = NULL;
        }

        for (int i = 0; i < D_ALPHA_SIZE; ++i) {
            dLens[i] = 0;
            dCodes[i] = NULL;
        }

        numBlocks++;
        #ifdef DEBUG
        printf("BLOCO #%d\n", numBlocks);
        #endif

        BFINAL = getBits(1, gzFile); //primeiro bit é o menos significativo

        #ifdef DEBUG
        printf("BFINAL = %d\n", BFINAL);
        #endif

        if (!isDynamicHuffman(getBits(2, gzFile)))  //ignorar bloco se não for Huffman dinâmico
            continue;

        //obter header do bloco
        huffInfo = getBHeader(gzFile);

        #ifdef DEBUG
        printf("HLIT = %d\nHDIST = %d\nHCLEN = %d\n", huffInfo.HLIT, huffInfo.HDIST, huffInfo.HCLEN);
        #endif

        //obter comprimentos dos códigos de comprimento de código
        for (int i = 0; i < huffInfo.HCLEN + 4; ++i) {
            cLens[order[i]] = getBits(3, gzFile);
        }

        #ifdef DEBUG
        printf("Comprimentos dos códigos de comprimento de código:\n");
        for (int i = 0; i < C_ALPHA_SIZE; ++i) {
            printf("%d\t--> %d\n", i, cLens[i]);
        }
        #endif

        //calcular códigos de Huffman de comprimento de código
        getCodes(cCodes, cLens, C_ALPHA_SIZE);

        #ifdef DEBUG
        printf("Códigos de comprimento de código:\n");
        for (int i = 0; i < C_ALPHA_SIZE; ++i) {
            printf("%d\t--> %s\n", i, cCodes[i]);
        }
        #endif

        //criar e preencher a árvore de Huffman de códigos de comprimento de código
        CChftree = createHFTree();
        fillTree(CChftree, cCodes, C_ALPHA_SIZE);

        //obter comprimentos dos códigos de literais e de comprimento
        getLens(gzFile, lens, huffInfo.HLIT + 257, CChftree);

        #ifdef DEBUG
        printf("Comprimentos dos códigos de literais e de comprimento:\n");
        for (int i = 0; i < ALPHA_SIZE; ++i) {
            printf("%d\t--> %d\n", i, lens[i]);
        }
        #endif

        //calcular códigos de Huffman de literais e de comprimento
        getCodes(codes, lens, ALPHA_SIZE);

        #ifdef DEBUG
        printf("Códigos de literais e de comprimento:\n");
        for (int i = 0; i < ALPHA_SIZE; ++i) {
            printf("%d\t--> %s\n", i, codes[i]);
        }
        #endif

        //criar e preencher a árvore de Huffman de literais e de comprimento
        Chftree = createHFTree();
        fillTree(Chftree, codes, ALPHA_SIZE);

        //obter comprimentos dos códigos de distância a recuar
        getLens(gzFile, dLens, huffInfo.HDIST + 1, CChftree);

        #ifdef DEBUG
        printf("Comprimentos dos códigos de distância a recuar:\n");
        for (int i = 0; i < D_ALPHA_SIZE; ++i) {
            printf("%d\t--> %d\n", i, dLens[i]);
        }
        #endif

        //calcular códigos de Huffman de distância a recuar
        getCodes(dCodes, dLens, D_ALPHA_SIZE);

        #ifdef DEBUG
        printf("Códigos de distância a recuar:\n");
        for (int i = 0; i < D_ALPHA_SIZE; ++i) {
            printf("%d\t--> %s\n", i, dCodes[i]);
        }
        #endif

        //criar e preencher a árvore de Huffman de distância a recuar
        DChftree = createHFTree();
        fillTree(DChftree, dCodes, D_ALPHA_SIZE);

        //copiar sequências/literais até chegar ao fim do bloco;
        while (nextSeq(gzFile, f_buffer, &index, limsf, bitsf, limsb, bitsb, Chftree, DChftree));



        //limpar árvores de Huffman
        destroyHFTree(CChftree);
        destroyHFTree(Chftree);
        destroyHFTree(DChftree);
    } while (BFINAL == 0);


    //terminações
    fwrite(f_buffer, 1, origFileSize, fp);
    fclose(fp);
    fclose(fp_log);
    fclose(gzFile);
    printf("End: %d bloco(s) analisado(s).\n", numBlocks);
}

HuffInfo getBHeader(FILE *ptr) {
    HuffInfo huffInfo;
    huffInfo.HLIT = getBits(5, ptr);
    huffInfo.HDIST = getBits(5, ptr);
    huffInfo.HCLEN = getBits(4, ptr);
    return huffInfo;
}

int getBits(char size, FILE *ptr) {
    int mask = (((unsigned int) 1) << size) - 1;
    int r_value;
    updateBuffer(size, ptr);
    r_value = rb & mask;
    rb = rb >> size;
    availBits -= size;
    return r_value;
}

void fillTree(HuffmanTree *huffmanTree, char *codes[], int size) {
    for (int i = 0; i < size; ++i) {
        if (codes[i]) {
            addNode(huffmanTree, codes[i], i, 0);
        }
    }
}

void updateBuffer(char needBits, FILE *ptr) {
    unsigned short byte;
    while (availBits < needBits) { //enquanto não houver bits suficientes vai obtendo bytes e colocando-os à esquerda no rb
        fread(&byte, 1, 1, ptr);
        rb = (byte << availBits) | rb;
        availBits += 8;
    }
}

void getCodes(char *codes[], const char lens[], int size) {
    int min = INT_MAX;
    int max = 0;
    for (int k = 0; k < size; ++k) { //obtem o comprimento màximo e mínimo
        if (lens[k] && lens[k] < min)
            min = lens[k];
        if (lens[k] > max)
            max = lens[k];
    }
    unsigned int code = 0;
    for (int i = 0; i < size; ++i) {
        if (lens[i] == min) {
            char *c = (char *) malloc((min + 1) * sizeof(char)); //enquanto está num comprimento vai somando 1
            bits2String(c, min, code++);
            codes[i] = c;
        }
        if (i == size - 1 && min < max) {   //quando não há mais códigos com esse comprimento
            i = -1;                         //multiplica por 2 e passa ao próximo
            min++;
            code = code << 1;
        }
    }
}

void getLens(FILE *ptr, char lens[], int size, HuffmanTree *huffmanTree) {
    for (int i = 0; i < size; ++i) {
        short len = nextSymbol(ptr, huffmanTree);
        int rep;
        char c;
        switch (len) {
            case COPY_LAST:    //copia o código anterior 3 + rep vezes
                c = lens[i - 1];
                rep = 3 + getBits(2, ptr);
                for (int lim = i + rep; i < lim; ++i) {
                    lens[i] = c;
                }
                i--;
                break;
            case COPY_ZEROS:    //os 3 + rep vezes códigos seguintes são 0
                i += 3 + getBits(3, ptr) - 1;

                break;
            case COPY_ZEROS2:    //o mesmo que o anterior mas com mais códigos
                i += 11 + getBits(7, ptr) - 1;
                break;
            default:    //o normal, o comprimento é o literal
                lens[i] = len;
                break;
        }
    }
}

unsigned short nextSymbol(FILE *ptr, HuffmanTree *huffmanTree) {
    resetCurNode(huffmanTree);
    unsigned char bit;
    unsigned short pos = 0;
    while (!isLeaf(huffmanTree->curNode)) {     //enquanto não chegar a uma folha da àrvore vai
        bit = getBits(1, ptr);                  //descendo para o ramo adequado tendo lido 0 ou 1
        pos = nextNode(huffmanTree, bit + '0');
    }
    resetCurNode(huffmanTree);
    return pos;
}

bool
nextSeq(FILE *ptr, char *buf, int *cur, const unsigned short limsf[], char bitsf[], const unsigned int limsb[], char bitsb[],
        HuffmanTree *chftree, HuffmanTree *dChftree) {
    unsigned int back;
    unsigned int forward;
    unsigned short pos;
    pos = nextSymbol(ptr, chftree);
    if (pos == BLOCK_END)   //chegou ao fim do bloco
        return false;
    if (pos < 256) {        //obteu um literal e copia-o
        buf[*cur] = pos;
        #ifdef DEBUG
        printf("A copiar '%c'\n", pos);
        #endif
        *cur += 1;
        return true;
    } else {                                                            //obteu um comprimento x, vai obter a distância a recuar y,
        forward = limsf[pos - 257] + getBits(bitsf[pos - 257], ptr);    // recuar y e copiar x bytes a partir daí
        pos = nextSymbol(ptr, dChftree);
        back = limsb[pos] + getBits(bitsb[pos], ptr);
        #ifdef DEBUG
        printf("A copiar '");
        #endif
        for (int i = *cur; i < *cur + forward; ++i) {
            buf[i] = buf[i - back];
            #ifdef DEBUG
            printf("%c", buf[i]);
            #endif
        }
        #ifdef DEBUG
        printf("'...\n");
        #endif
        *cur += forward;
        return true;
    }
}

//---------------------------------------------------------------
//Lê o cabeçalho do ficheiro gzip: devolve erro (-1) se o formato for inválidodevolve, ou 0 se ok
int getHeader(FILE *gzFile, gzipHeader *gzh) //obtém cabeçalho
{
    unsigned char byte;

    //Identicação 1 e 2: valores fixos
    fread(&byte, 1, 1, gzFile);
    (*gzh).ID1 = byte;
    if ((*gzh).ID1 != 0x1f) return -1; //erro no cabeçalho

    fread(&byte, 1, 1, gzFile);
    (*gzh).ID2 = byte;
    if ((*gzh).ID2 != 0x8b) return -1; //erro no cabeçalho

    //Método de compressão (deve ser 8 para denotar o deflate)
    fread(&byte, 1, 1, gzFile);
    (*gzh).CM = byte;
    if ((*gzh).CM != 0x08) return -1; //erro no cabeçalho

    //Flags
    fread(&byte, 1, 1, gzFile);
    unsigned char FLG = byte;

    //MTIME
    char lenMTIME = 4;
    fread(&byte, 1, 1, gzFile);
    (*gzh).MTIME = byte;
    for (int i = 1; i <= lenMTIME - 1; i++) {
        fread(&byte, 1, 1, gzFile);
        (*gzh).MTIME = (byte << 8) + (*gzh).MTIME;
    }

    //XFL (not processed...)
    fread(&byte, 1, 1, gzFile);
    (*gzh).XFL = byte;

    //OS (not processed...)
    fread(&byte, 1, 1, gzFile);
    (*gzh).OS = byte;

    //--- Check Flags
    (*gzh).FLG_FTEXT = (char) (FLG & 0x01);
    (*gzh).FLG_FHCRC = (char) ((FLG & 0x02) >> 1);
    (*gzh).FLG_FEXTRA = (char) ((FLG & 0x04) >> 2);
    (*gzh).FLG_FNAME = (char) ((FLG & 0x08) >> 3);
    (*gzh).FLG_FCOMMENT = (char) ((FLG & 0x10) >> 4);

    //FLG_EXTRA
    if ((*gzh).FLG_FEXTRA == 1) {
        //ler 2 bytes XLEN + XLEN bytes de extra field
        //1º byte: LSB, 2º: MSB
        char lenXLEN = 2;

        fread(&byte, 1, 1, gzFile);
        (*gzh).xlen = byte;
        fread(&byte, 1, 1, gzFile);
        (*gzh).xlen = (byte << 8) + (*gzh).xlen;

        (*gzh).extraField = new unsigned char[(*gzh).xlen];

        //ler extra field (deixado como está, i.e., não processado...)
        for (int i = 0; i <= (*gzh).xlen - 1; i++) {
            fread(&byte, 1, 1, gzFile);
            (*gzh).extraField[i] = byte;
        }
    } else {
        (*gzh).xlen = 0;
        (*gzh).extraField = 0;
    }

    //FLG_FNAME: ler nome original
    if ((*gzh).FLG_FNAME == 1) {
        (*gzh).fName = new char[1024];
        unsigned int i = 0;
        do {
            fread(&byte, 1, 1, gzFile);
            if (i <= 1023)  //guarda no máximo 1024 caracteres no array
                (*gzh).fName[i] = byte;
            i++;
        } while (byte != 0);
        if (i > 1023)
            (*gzh).fName[1023] = 0;  //apesar de nome incompleto, garantir que o array termina em 0
    } else
        (*gzh).fName = 0;

    //FLG_FCOMMENT: ler comentário
    if ((*gzh).FLG_FCOMMENT == 1) {
        (*gzh).fComment = new char[1024];
        unsigned int i = 0;
        do {
            fread(&byte, 1, 1, gzFile);
            if (i <= 1023)  //guarda no máximo 1024 caracteres no array
                (*gzh).fComment[i] = byte;
            i++;
        } while (byte != 0);
        if (i > 1023)
            (*gzh).fComment[1023] = 0;  //apesar de comentário incompleto, garantir que o array termina em 0
    } else
        (*gzh).fComment = 0;


    //FLG_FHCRC (not processed...)
    if ((*gzh).FLG_FHCRC == 1) {
        (*gzh).HCRC = new unsigned char[2];
        fread(&byte, 1, 1, gzFile);
        (*gzh).HCRC[0] = byte;
        fread(&byte, 1, 1, gzFile);
        (*gzh).HCRC[1] = byte;
    } else
        (*gzh).HCRC = 0;

    return 0;
}


//---------------------------------------------------------------
//Analisa block header e vê se é huffman dinâmico
int isDynamicHuffman(unsigned char rb) {
    unsigned char BTYPE = rb & 0x03;

    if (BTYPE == 0) //--> sem compressão
    {
        printf("Ignorando bloco: sem compactação!!!\n");
        return 0;
    } else if (BTYPE == 1) {
        printf("Ignorando bloco: compactado com Huffman fixo!!!\n");
        return 0;
    } else if (BTYPE == 3) {
        printf("Ignorando bloco: BTYPE = reservado!!!\n");
        return 0;
    } else
        return 1;
}


//---------------------------------------------------------------
//Obtém tamanho do ficheiro original
long getOrigFileSize(FILE *gzFile) {
    //salvaguarda posição actual do ficheiro
    long fp = ftell(gzFile);

    //últimos 4 bytes = ISIZE;
    fseek(gzFile, -4, SEEK_END);

    //determina ISIZE (só correcto se cabe em 32 bits)
    unsigned long sz = 0;
    unsigned char byte;
    fread(&byte, 1, 1, gzFile);
    sz = byte;
    for (int i = 0; i <= 2; i++) {
        fread(&byte, 1, 1, gzFile);
        sz = (byte << 8 * (i + 1)) + sz;
    }


    //restaura file pointer
    fseek(gzFile, fp, SEEK_SET);

    return sz;
}


//---------------------------------------------------------------
void bits2String(char *strBits, char len, int byte) {
    char mask = 0x01;  //get LSbit

    strBits[len] = 0;
    for (char bit, i = len - 1; i >= 0; i--) {
        bit = byte & mask;
        strBits[i] = bit + 48; //converter valor numérico para o caracter alfanumérico correspondente
        byte = byte >> 1;
    }
}
