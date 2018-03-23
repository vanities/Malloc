#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void heap_init(int num_pages_for_heap);
void *heap_alloc(int num_bytes_to_allocate);
void heap_free(void *pointer_to_area_to_free);  // not used in this test
void print_thing(void*, int);

int main(int argc, char *argv[])
{
    char *p1, *p2, *p3, *p4, *p5, *p6;

    heap_init(1);

    p1 = (char *) heap_alloc(10);
    if ((long int)p1 % 16 != 0)
    {
        printf("p1 bad %p  pmod16 %ld\n",p1,((long int)p1)%16);
        exit(-1);
    }
    memset(p1,'A',10);
    print_thing(p1,10);
    
    p2 = (char *) heap_alloc(10);
    if ((long int)p2 % 16 != 0)
    {
        printf("p2 bad %p  pmod16 %ld\n",p2,((long int)p2)%16);
        exit(-1);
    }
    memset(p2,'A',10);
    print_thing(p2,10);

    
    p3 = (char *) heap_alloc(10);
    if ((long int)p3 % 16 != 0)
    {
        printf("p3 bad %p  pmod16 %ld\n",p3,((long int)p3)%16);
        exit(-1);
    }
    memset(p3,'A',10);
    print_thing(p3,10);

    p4 = (char *) heap_alloc(10);
    if ((long int)p4 % 16 != 0)
    {
        printf("p4 bad %p  pmod16 %ld\n",p4,((long int)p4)%16);
        exit(-1);
    }
    memset(p4,'A',10);
    print_thing(p4,10);

    p5 = (char *) heap_alloc(10);
    if ((long int)p5 % 16 != 0)
    {
        printf("p5 bad %p  pmod16 %ld\n",p5,((long int)p5)%16);
        exit(-1);
    }
    memset(p5,'A',10);
    print_thing(p5,10);

    heap_free( (void *)p1 );


    heap_free( (void *)p3 );

    heap_free( (void *)p2 );

    printf("DONE\n");
    return 0;
}
