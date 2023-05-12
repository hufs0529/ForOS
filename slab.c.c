#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "slab.h"

struct {
  struct spinlock lock;
  struct slab slab[NSLAB];
} stable;

void slabinit(){
  initlock(&stable.lock, "stable");
  acquire(&stable.lock);
  
  for(int i = 0; i < NSLAB; i++){
    stable.slab[i].size = 0;
    stable.slab[i].num_pages = 0;
    stable.slab[i].num_free_objects = 0;
    stable.slab[i].num_used_objects = 0;
    stable.slab[i].bitmap = kalloc();
    for(int j = 0; j < NPAGES; j++){
      stable.slab[i].page[j] = kalloc();
    }
    memset(stable.slab[i].bitmap, 0, PGSIZE);
  }
  
  release(&stable.lock);
}

char *kmalloc(int size){
  acquire(&stable.lock);
  
  // This is a naive implementation, you should use a best-fit or first-fit algorithm here.
  for(int i = 0; i < NSLAB; i++){
    if(stable.slab[i].size >= size){
      for(int j = 0; j < NPAGES; j++){
        if(!stable.slab[i].bitmap[j]){
          stable.slab[i].bitmap[j] = 1;
          stable.slab[i].size -= size;
          release(&stable.lock);
          return stable.slab[i].page[j];
        }
      }
    }
  }
  
  // If no free space found
  release(&stable.lock);
  return 0x00;
}

void kmfree(char *addr, int size){
  acquire(&stable.lock);
  
  for(int i = 0; i < NSLAB; i++){
    for(int j = 0; j < NPAGES; j++){
      if(addr == stable.slab[i].page[j]){
        if(stable.slab[i].bitmap[j]){
          stable.slab[i].bitmap[j] = 0;
          stable.slab[i].size += size;
        }
        break;
      }
    }
  }
  
  release(&stable.lock);
}
