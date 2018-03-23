#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void heap_init(int num_pages_for_heap);
void *heap_alloc(int num_bytes_to_allocate);
void heap_free(void *pointer_to_area_to_free);
void print_thing(void*, int);


int main(int argc, char *argv[])
{
    char *p1, *p2, *p3, *p4, *p5, *p6;

    heap_init(1);

    p1 = (char *) heap_alloc(1000);
    if ((long int)p1 % 16 != 0)
    {
        printf("p1 bad %p  pmod16 %d\n",p1,((long int)p1)%16);
        exit(-1);
    }
    memset(p1,'A',1000);
    print_thing(p1, 1000);


    p2 = (char *) heap_alloc(1000);
    if ((long int)p2 % 16 != 0)
    {
        printf("p2 bad %p  pmod16 %d\n",p2,((long int)p2)%16);
        exit(-1);
    }
    memset(p2,'B',1000);
    print_thing(p2, 1000);

    p3 = (char *) heap_alloc(1000);
    if ((long int)p3 % 16 != 0)
    {
        printf("p3 bad %p  pmod16 %d\n",p3,((long int)p3)%16);
        exit(-1);
    }
    memset(p3,'C',1000);
    print_thing(p3, 1000);

    p4 = (char *) heap_alloc(1000);
    if ((long int)p4 % 16 != 0)
    {
        printf("p4 bad %p  pmod16 %d\n",p4,((long int)p4)%16);
        exit(-1);
    }
    memset(p4,'D',1000);
    print_thing(p4, 1000);

    p5 = (char *) heap_alloc(1600);    // 1st try should fail
    if (p5 != NULL)
    {
        printf("p5 should have been NULL, but is %p\n",p5);
        exit(-1);
    }

    heap_free( (void *)p2 );
    p5 = (char *) heap_alloc(1600);    // 2nd try should fail
    if (p5 != NULL)
    {
        printf("p5 should have been NULL, but is %p\n",p5);
        exit(-1);
    }

    heap_free( (void *)p3 );
    p5 = (char *) heap_alloc(1600);    // 3rd try should succeed
    if (p5 == NULL)
    {
        printf("p5 should NOT have been NULL here %p\n",p5);
        exit(-1);
    }
    if ((long int)p5 % 16 != 0)
    {
        printf("p4 bad %p  pmod16 %d\n",p4,((long int)p4)%16);
        exit(-1);
    }
    if ( *(p5+32) != 'B')     //  first few bytes should be B
    {
        printf("p5 (%p) first byte should be B but is %c\n",p5,*(p5+32));
        exit(-1);
    }
    if ( *(p5+1200) != 'C')  // bytes near the end should be C
    {
        printf("p5 (%p) end bytes should be C but are %c\n",p5,*(p5+1200));
        exit(-1);
    }

    printf("DONE\n");

    return 0;
}
