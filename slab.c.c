#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "slab.h"

struct
{
    struct spinlock lock;
    struct slab slab[NSLAB];
} stable;

void slabinit()
{
    initlock(&stable.lock, "stable");
    acquire(&stable.lock);

    for (int i = 0; i < NSLAB; i++)
    {
        stable.slab[i].size = 0;
        stable.slab[i].bitmap = kalloc();
        stable.slab[i].cache = kalloc();
        memset(stable.slab[i].bitmap, 0, PGSIZE);
    }

    release(&stable.lock);
}

char *kmalloc(int size)
{
    acquire(&stable.lock);

    for (int i = 0; i < NSLAB; i++)
    {
        if (stable.slab[i].size >= size)
        {
            for (int j = 0; j < PGSIZE; j++)
            {
                if (!stable.slab[i].bitmap[j])
                {
                    stable.slab[i].bitmap[j] = 1;
                    stable.slab[i].size -= size;
                    release(&stable.lock);
                    return stable.slab[i].cache + j * size;
                }
            }
        }
    }

    // if no free space found
    release(&stable.lock);
    return 0x00;
}

void kmfree(char *addr, int size)
{
    acquire(&stable.lock);

    for (int i = 0; i < NSLAB; i++)
    {
        if (addr >= stable.slab[i].cache && addr < stable.slab[i].cache + PGSIZE)
        {
            int index = (addr - stable.slab[i].cache) / size;
            if (stable.slab[i].bitmap[index])
            {
                stable.slab[i].bitmap[index] = 0;
                stable.slab[i].size += size;
            }
            break;
        }
    }

    release(&stable.lock);
}

void slabdump()
{
    cprintf("__slabdump__\n");

    struct slab *s;

    cprintf("size\tnum_pages\tused_objects\tfree_objects\n");

    for (s = stable.slab; s < &stable.slab[NSLAB]; s++)
    {
        cprintf("%d\t%d\t\t%d\t\t%d\n",
                s->size, s->num_pages, s->num_used_objects, s->num_free_objects);
    }
}

int numobj_slab(int slabid)
{
    return stable.slab[slabid].num_used_objects;
}
