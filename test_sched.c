#include "types.h"
#include "stat.h"
#include "user.h"

#define PRIO_BASE 20
#define INNER_LOOP 1000
#define OUTER_LOOP 20
#define PI 3.14 

void set_priority(int priority) {
    setnice(getpid(), priority);
}

int get_priority() {
    return getnice(getpid());
}

int main(int argc, char **argv)
{
    int pid = getpid();
    float dummy = 0;
    int i,j;
    printf(1, "=== TEST START ===\n");
    set_priority(PRIO_BASE);

    pid = fork();
    if (pid == 0)   /* Child */
    {
        if (get_priority() >= 15) {
            set_priority(get_priority() / 2);
        } else {
            set_priority(get_priority() + 1);
        }

        for (i=0; i<OUTER_LOOP/2; i++)
        {
            for (j=0; j<INNER_LOOP; j++)
                dummy += PI*j;
            printf(1, "In (HI), i = %d, dummy = %x, priority = %d\n", i, (unsigned int)dummy, get_priority());
        }
        
        pid = fork();
        if (pid == 0) /* child-of-child */
        {
            if (get_priority() >= 15) {
                set_priority(get_priority() / 2);
            } else {
                set_priority(get_priority() + 1);
            }

            for (i=0; i<OUTER_LOOP; i++)
            {
                for (j=0; j<INNER_LOOP; j++)
                    dummy += PI*j;
                printf(1, "In (MID), i = %d, dummy = %x, priority = %d\n", i, (unsigned int)dummy, get_priority());
            }
        }
        else    /* Child */
        {
            if (get_priority() >= 15) {
                set_priority(get_priority() / 2);
            } else {
                set_priority(get_priority() + 1);
            }

            for (i=0; i<OUTER_LOOP/2; i++)
            {
                for (j=0; j<INNER_LOOP; j++)
                    dummy += PI*j;
                printf(1, "In (HI), i = %d, dummy = %x, priority = %d\n", i, (unsigned int)dummy, get_priority());
            }
            wait();
        }
    }
    else    /* Parent */
    {
        for (i=0; i<OUTER_LOOP; i++)
        {
            for (j=0; j<INNER_LOOP; j++)
                dummy += PI*j;
            printf(1, "In (LO), i = %d, dummy = %x, priority = %d\n", i, (unsigned int)dummy, get_priority());
        }
        wait();
        printf(1, "=== TEST DONE ===\n");
    }
    exit();
}
