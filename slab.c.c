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

void slabinit() {
    initlock(&stable.lock, "stable");
    acquire(&stable.lock);

    for (int i = 0; i < NSLAB; i++) {
        stable.slab[i].size = 0; // size of a slab allocator
        stable.slab[i].num_pages = 0; // number of pages of a slab allocator
        stable.slab[i].num_free_objects = 0; // number of free objects of a slab allocator
        stable.slab[i].num_used_objects = 0; // number of used objects of a slab allocator
        stable.slab[i].bitmap = kalloc(); // allocation bitmap for a slab allocator
        stable.slab[i].page = kalloc(); // page array for a slab allocator

        // Initialize bitmap to 0
        memset(stable.slab[i].bitmap, 0, PGSIZE);
    }

    release(&stable.lock);
}

char *kmalloc(int size) {
    acquire(&stable.lock);

    for (int i = 0; i < NSLAB; i++) {
        if (stable.slab[i].num_free_objects * stable.slab[i].size >= size) {
            for (int j = 0; j < PGSIZE; j++) {
                if (!stable.slab[i].bitmap[j]) {
                    stable.slab[i].bitmap[j] = 1;
                    stable.slab[i].num_used_objects++;
                    stable.slab[i].num_free_objects--;

                    release(&stable.lock);
                    return stable.slab[i].page + j * size;
                }
            }
        }
    }

    // If no free space found
    release(&stable.lock);
    return 0x00;
}

void kmfree(char *addr, int size) {
    acquire(&stable.lock);

    for (int i = 0; i < NSLAB; i++) {
        if (addr >= stable.slab[i].page && addr < stable.slab[i].page + PGSIZE) {
            int index = (addr - stable.slab[i].page) / size;
            if (stable.slab[i].bitmap[index]) {
                stable.slab[i].bitmap[index] = 0;
                stable.slab[i].num_used_objects--;
                stable.slab[i].num_free_objects++;
            }
            break;
        }
    }

    release(&stable.lock);
}
