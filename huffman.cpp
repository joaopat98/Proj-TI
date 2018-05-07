/*Author: Rui Pedro Paiva
Teoria da Informa��o, LEI, 2007/2008*/

#include <cstdlib>
#include "huffman.h"

HuffmanTree* createHFTree()
{
	HuffmanTree *hft = new HuffmanTree;

	hft->root = createHFNode(-1, 0, NULL, NULL);
	hft->curNode = hft->root;

	return hft;
}


HFNode* createHFNode(short index, short level, HFNode *left, HFNode *right)
{
	HFNode *hfn = new HFNode;

	hfn->index = index;  //n� vazio
	hfn->level = level; //n�vel 0 na �rvore
	hfn->left = left;  //n�o tem filhos
	hfn->right = right;

	return hfn;
}

//adiciona n� � �rvore 
//recebe a �rvore onde inserir, a string com o c�digo (representado por string com 0s e 1s), o �ndice no alfabeto e o campo verbose
//devolve 0 se adicionou bem; -1 se o n� j� existe; -2 se o c�digo deixar de ser de prefixo
int addNode(HuffmanTree *hft, char *s, int ind, short verbose)
{
	HFNode *tmp = hft->root;  //n� para atravessar a �rvore

	int lv = 0, len = (int)strlen(s), index;
	int found = 0;
	char direction;
	int pos = -3;
				
	while(lv < len && !found)
	{			
		if (tmp->index != -1)  //tentando criar filho de folha --> deixaria de ser c�digo de prefixo...
		{
			pos = -2;
			found = 1;
		}
		else
		{			
			direction = s[lv];
			
			if (direction == '0')
			{
				if (lv != len-1 && tmp->left != NULL)  //continua descida
				{					
					tmp = tmp->left;
				}
				else if (tmp->left != NULL) // n� j� inserido
				{
					pos = -1;
					found = 1;
				}
				else //cria n� � esquerda
				{
					if (lv == len-1)  //folha						
						index = ind;
					else
						index = -1;

					HFNode *hf = createHFNode(index, lv+1, NULL, NULL);
					tmp->left = hf;
					tmp = tmp->left;
				}
			}
			else if (direction == '1')
			{				
				if (lv != len -1 && tmp->right != NULL)
				{					
					tmp = tmp->right;
				}
				else if (tmp->right != NULL) // n� j� inserido
				{
					  pos = -1;
					  found = 1;
				}
				else //cria n� � direita
				{
					if (lv == len-1)  //folha
						index = ind;
					else
						index = -1;

					HFNode *hf = createHFNode(index, lv+1, NULL, NULL);
					tmp->right = hf;
					tmp = tmp->right;
				}			
			}
		}				
		lv++;				
	}
	
			
	if (!found)
		pos = tmp->index;
		
	if (verbose)
	{
		if (pos == -1)
			printf("C�digo '%s' j� inserido!!!\n", s);
		else if (pos == -2)
			printf("C�digo '%s': tentando extender folha!!!\n", s);
		else
			printf("C�digo '%s' inserido com sucesso!!!\n", s);
	}
	
	return pos;
}				



//actualiza n� corrente na �rvore com base no n� actual e n� pr�ximo bit
//utilizada para pesquisa na �rvore bit a bit (e n�o c�digo inteiro de uma vez) --> situa��o correspodente ao deflate
//devolve index se � folha; -1 se n�o encontrou o n�; -2 se encontrou mas ainda n�o � folha
int nextNode(HuffmanTree *hft, char c)
{
	int pos;
	
	if (isLeaf(hft->curNode))
		pos = -1;
	
	if (c == '0')
		if (hft->curNode->left != NULL)
		{
			hft->curNode = hft->curNode->left;
			if (isLeaf(hft->curNode))
				pos = hft->curNode->index;
			else
				pos = -2;
		}
		else
			pos = -1;
	else // c == '1'
		if (hft->curNode->right != NULL)
		{
			hft->curNode = hft->curNode->right;
			if (isLeaf(hft->curNode))
				pos = hft->curNode->index;
			else
				pos = -2;
		}
		else
			pos = -1;								
	
	return pos;
}



//procura c�digo na �rvore a partir da ra�z (representado por String com 0s e 1s): procura c�digo inteiro
int findNode(HuffmanTree *hft, char* s, short verbose)
{
	return findNode(hft, s, hft->root, verbose);
}



//procura c�digo na �rvore, a partir do n� actual (representado por String com 0s e 1s)
//devolve �ndice no alfabeto, se encontrou; -1 se n�o encontrou; -2 se � prefixo de c�digo existente; 
int findNode(HuffmanTree *hft, char *s, HFNode *cur, short verbose)
{
	HFNode *tmp = cur;
	int lv = 0, len = (int)strlen(s);
	int found = 1;
	int pos;
	
	while(lv < len && found)
	{			
		char direction = s[lv];
		
		if (direction == '0')
		{
			if (tmp->left != NULL)
				tmp = tmp->left;
			else
				found = 0;
		}
		else if (direction == '1')
		{
			if (tmp->right != NULL)
				tmp = tmp->right;
			else
				found = 0;
		}
		
		lv++;
	}
			
	if (!found)
		pos = -1;
	else if(tmp->index == -1)
		pos = -2;
	else
		pos = tmp->index;
		
	if (verbose)
	{
		if (pos == -1)
			printf("C�digo '%s' n�o encontrado!!!\n", s);
		else if (pos == -2)
			printf("C�digo '%s' n�o encontrado, mas prefixo!!!\n", s);
		else
			printf("C�digo '%s' corresponde � posi��o %d do alfabeto\n", s, pos);
	}
					
	return pos;
}	



//verifica se o n� � folha
short isLeaf(HFNode *n)
{
	if (n->left == NULL && n->right == NULL)
		return 1;
	else
		return 0;
}	


//reposicionar n� corrente na ra�z
void resetCurNode (HuffmanTree *hft)
{
	hft->curNode = hft->root;
}

void destroyHFNode(HFNode *n){
    if (n->left)
        destroyHFNode(n->left);
    if (n->right)
        destroyHFNode(n->right);
    free(n);
}

void destroyHFTree(HuffmanTree *hft){
    destroyHFNode(hft->root);
    free(hft);
}