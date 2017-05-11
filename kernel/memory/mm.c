#include "common.h"
#include "string.h"
#include "memory.h"
#include "mmu.h"

#define NR_PHY_PTE (PHY_MEM/PTSIZE)

typedef struct PTE_INFO{
	uint32_t physbase;
	struct PTE_INFO *prev;
	struct PTE_INFO *next;
}PTE_INFO;

static PTE_INFO pte_info[NR_PHY_PTE];
static PTE_INFO *_used_pte;
static PTE_INFO *_free_pte;

void init_pte_info(void){
	int i;
	for(i=0;i<KERNEL_SIZE/PTSIZE;++i){
		pte_info[i].physbase=i*PTSIZE;
		if(i>0) pte_info[i].prev=&pte_info[i-1];
		else pte_info[i].prev=NULL;
		if(i<KERNEL_SIZE/PTSIZE-1) pte_info[i].next=&pte_info[i+1];
		else pte_info[i].next=NULL;
	}
	for(i=KERNEL_SIZE/PTSIZE;i<PHY_MEM/PTSIZE;++i){
		pte_info[i].physbase=i*PTSIZE;
		if(i>KERNEL_SIZE/PTSIZE) pte_info[i].prev=&pte_info[i-1];
		else pte_info[i].prev=NULL;
		if(i<PHY_MEM/PTSIZE-1) pte_info[i].next=&pte_info[i+1];
		else pte_info[i].next=NULL;
	}
	_used_pte=&pte_info[0];
	_free_pte=&pte_info[KERNEL_SIZE/PTSIZE];
}

uint32_t get_pte(){
	Assert(_free_pte!=NULL,"There is not free pte left!\n");
	PTE_INFO* temp=_free_pte;
	_free_pte=temp->next;
	_free_pte->prev=NULL;
	_used_pte->next=temp;
	temp->prev=NULL;
	temp->next=_used_pte;
	_used_pte=temp;
	return _used_pte->physbase;
}

void free_pte(int pte_idx){
	assert(pte_idx>=0 && pte_idx<NR_PHY_PTE);
	PTE_INFO *temp=&pte_info[pte_idx];
	if(temp->prev) temp->prev->next=temp->next;
	if(temp->next) temp->next->prev=temp->prev;
	temp->prev=NULL;
	temp->next=_free_pte;
	_free_pte=temp;
}

int find_pte(int address){
	int index=0;
	for(index=0;index<NR_PHY_PTE;index++){
		if((address>=pte_info[index].physbase)&&(address<pte_info[index].physbase+PTSIZE)){
			return index;
		}
	}
	panic("find_pte: YOU should find one pte!\n");
	return 0;
}

void free_address(int address){
	free_pte(find_pte(address));
}
