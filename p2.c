/*	Author: 	Adam Mischke
	Class:		Intro to Computer Systems (3240)
	Teacher:	rbutler
	Project:	p2
	Date:		Friday, September 22, 2017

	I feel dirty writing this. Nevertheless, this is my attempt at a contiguous heap allocator in C.

	Basically, I had to write an implementation for a linked list:
	(creating a list, adding to the list, deleting the list, and deleting a node in the list, finding an element in the list)

	I then went on to create a struct for a region:
	a begin and end address, the size of the region, and a pointer to the next region
 
	I start by creating one big block determined by the heat_init(PAGE SIZE WANTED) function with malloc

	A user can then *heap_alloc(A SIZE YOU WANT) => pointer to the region with free

	the user can also heap_free(POINTER TO REGION)
	a feature I added was the coalesce(FREE'd POINTER), which combines contiguous memory slots together.
	by recursivelly calling coalesce(), it can handle up to 3 contiguous free spots
*/

/* INCLUDES	*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

/* SETTING SOME GLOBALS */
int pgsz=0, max_bytes=0, memory_allocated=0;


/* TYPEDEFFING OUR REGION LIST */
typedef struct s_region{
        void* begin_address;
        void* end_address;
        int size;
        struct s_region* next;
} Region;

/* PROTOTYPES FOR LIST */
Region* find_region(void*);
Region* create_region(void*, void*, int);
Region* add_region(Region*, void*, void*, int);
void delete_region(Region*);
void delete_node(Region *head, Region *n);

/* PROTOTYPES FOR HEAP */
void heap_init(int);
void *heap_alloc(int);
void heap_free(void*);
void coalesce(Region*);

/* PROTOTYPES FOR PRINTING/DEBUGGING */
void print_thing(void*, int);
void print_regions(Region*);

/* PROTOTYPES FOR UTILITY */
int align_16(int);
Region* get_best_fit(int);

/* GLOBAL LISTS	*/
Region* free_memory;
Region* taken_memory=NULL;


// initializes the heap
// INPUT:  an integer for the number of pages allowed for the heap
// OUTPUT: none
void heap_init(int num_pages_for_heap){
	// get page size
	pgsz = getpagesize();
	int i;

	// initialize the maximum bytes allowed for this program
	max_bytes = num_pages_for_heap * pgsz;
	
	// create block mmap
	void *region = mmap(NULL, max_bytes, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);

    if (region == ((void *) -1)) {
        perror("mmap");
        return;
    }
    //printf("memory allocated for initial heap: %d @ reigon: %p-%p\n",max_bytes, region, region+max_bytes);
	
	// create list of free memory
    free_memory = create_region(region, region+max_bytes, max_bytes);
    //print_regions(free_memory);

	return;
}

// allocates a number of bytes on the heap, returns the pointer (address space) of where the memory byte starts
// uses mmap (usually used for large values because it is initialized with # of pages)
// INPUT:  an integer for the number of bytes to allocate
// OUTPUT: returns the pointer to the initial memory address for the heap
void *heap_alloc(int num_bytes_to_allocate){
	//printf("num of bytes wanted to allocate: %d\n", num_bytes_to_allocate);
	
	// aligns the bytes to a x % 16 == 0
	num_bytes_to_allocate = align_16(num_bytes_to_allocate);

	// increase the memory allocated for our heap
	memory_allocated += num_bytes_to_allocate;
	//printf("total num allocated: %d\n", memory_allocated);


	// if the memory allocated is bigger than the maximum allowed bytes,
	if (memory_allocated >= max_bytes){
		// return NULL and REMEMBER TO REDUCE THE MEMORY ALLOCATED
		//printf("No room to allocate! memory attempted to allocate: %d, max bytes allowed on the heap: %d\n", memory_allocated, max_bytes);
		memory_allocated -= num_bytes_to_allocate;
		return NULL;
	}

	// create an iterator to the best fit region
	Region* iter = get_best_fit(num_bytes_to_allocate);


	// unmap that data from the the free array up to the number of bytes needed to allocate
	munmap(iter->begin_address,num_bytes_to_allocate);
	//printf("unmapped: %p-%p\n", iter->begin_address,iter->begin_address+num_bytes_to_allocate);


	// remap that position and range
	void *alloc = mmap(iter->begin_address, num_bytes_to_allocate, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	//printf("mapped: %p-%p\n", iter->begin_address,iter->begin_address+num_bytes_to_allocate);

	// if this is the first memory allocated, create a new region
	if(taken_memory==NULL){
		// create list of free and taken memory
    	taken_memory = create_region(iter->begin_address,iter->begin_address+num_bytes_to_allocate,num_bytes_to_allocate);

	}
	// else add one
	else{
		// add the new region to the taken list
		taken_memory = add_region(taken_memory,iter->begin_address,iter->begin_address+num_bytes_to_allocate,num_bytes_to_allocate);
		//print_regions(taken_memory);
	}

	// initialize a new region with the begin address
	Region* begin = iter->begin_address;
	// change the free pointer
	iter->begin_address = iter->begin_address+num_bytes_to_allocate;
	// reduce the size
	iter->size -= num_bytes_to_allocate;

	// return the void * back for allocation
	//printf("begin: %p returning: %p\n", begin, alloc);
	return begin;

}

// destroys the allocated memory from mmap by using mummap
// INPUT:  the pointer to the area that we're trying to free
// OUTPUT: none, but releases the memory from the heap and coalesces the free'd memory
void heap_free(void *pointer_to_area_to_free){
	
	// find the area in the list to free
	Region* found = find_region(pointer_to_area_to_free);
	

	if (found == NULL){
		//printf("could not find: %p\n", pointer_to_area_to_free);
		return;
	}


	//printf("memory freeing: %p - %p size: %d\n", found->begin_address, found->end_address, found->size);
	// unmap the area
	munmap(pointer_to_area_to_free, found->size);

	// add the unmapped area to the free list
	free_memory = add_region(free_memory,found->begin_address, found->end_address, found->size);

	
	// reduce the total memory allocated by the size 
	memory_allocated -= found->size;


	//printf("free list after heap free\n");
	// coalesce memory blocks if there are adjacent slots
	if(free_memory->next != NULL){

		//printf("attempting to coalesce memory\n");
		coalesce(found);
        //print_regions(free_memory);


	}

        // delete the node in the taken_memory list
    delete_node(taken_memory,found);
	//printf("memory allocated: %d\n", memory_allocated);
	return;
}

/* LIST FUNCTIONS */
// finds the beginning of a region and returns the pointer to it
// INPUT:	a beginning address to the region to find
// OUTPUT:	the pointer to that region, NULL if not found
Region* find_region(void* find){
    Region* iter;
    for (iter = taken_memory; NULL != iter; iter = iter->next) {
        if(iter->begin_address == find){
        	return iter;
        }
    }
    return NULL;
}

// creates a new list of regions
// INPUT:	a begin and end address, and its size
// OUTPUT:	the list
Region* create_region(void* begin, void* end, int sz) {
    Region* new_region = malloc(sizeof(Region));
    if (NULL != new_region){
            new_region->begin_address = begin;
            new_region->end_address = end;
            new_region->size = sz;
            new_region->next = NULL;
    }
    return new_region;
}

// recursively deletes the region list
// INPUT:	the list
// OUTPUT:	NONE
void delete_region(Region* old_region) {
        if (NULL != old_region->next) {
                delete_region(old_region->next);
        }
        free(old_region);
}

// adds a region to a list
// INPUT:	the list to be updated, a begin and end address, and its size
// OUTPUT:	the update list
Region* add_region(Region* region_list, void* begin, void* end, int sz) {
    Region* new_region = create_region(begin, end, sz);
    if (NULL != new_region) {
      	new_region->next = region_list;
    }
    return new_region;
}

// deletes a particular node in a list
// INPUT:	the head of the list, and n, the node to be deleted
// OUTPUT:	none, but updates the list
void delete_node(Region *head, Region *n){	

    // when node to be deleted is head node
    if(head->begin_address == n->begin_address){
        if(head->next == NULL){
            free(head);
            free_memory=NULL;
            return;
        }
 
        // copy the data of next node to head 
        head->begin_address = head->next->begin_address;
        head->end_address = head->next->end_address;
        head->size = head->next->size;

 
        // store address of next node
        n = head->next;
 
        // remove the link of next node
        head->next = head->next->next;
 
        // free memory
        free(n);
 
        return;
    }
 

    // when not first node, follow the normal deletion process
 
    // find the previous node
    Region *prev = head;
    while(prev->next != NULL && prev->next->begin_address != n->begin_address)
        prev = prev->next;
 
    // check if node really exists in list
    if(prev->next == NULL){
        //printf("given node is not present in list\n");
        return;
    }
 
    // remove node from list
    prev->next = prev->next->next;
    
    // free memory
    free(n);
 
    return; 
}
 
// prints a region of memory by characters
// INPUT:	the region and the number of bytes that are allocated to that region
// OUTPUT:	NONE, but prints to stout
void print_thing(void* region, int bytes_allocated){
	unsigned char *cp = (unsigned char *)region;
    int i;
    for (i=0; i < bytes_allocated; i++){
        printf("%02x ", *(cp+i));
    }printf("\n");
}

// prints a list
// INPUT:	the head of the list
// OUTPUT:	NONE, but prints to stout
void print_regions(Region* head){
    Region* iter;
    for (iter = head; NULL != iter; iter = iter->next) {
        printf("LL: begin: %p end: %p size: %i \n", iter->begin_address, iter->end_address, iter->size);
    }
}

// coalesces two or three contiguous free memory slots
// INPUT:	a region that was recently free'd
// OUTPUT:	NONE, but deletes nodes from the free memory list and adds a coalesced one
void coalesce(Region* found){
	Region *iter;

    for (iter = free_memory; NULL != iter; iter = iter->next) {

    	// case when free'd memory is to the left of the memory to be coalesced
        if(iter->end_address == found->begin_address ){


			// initialize a begin, end, and size
			void* b=iter->begin_address, *e=found->end_address; 
        	int s=(long int)found->end_address - (long int)iter->begin_address;

        	// delete the two nodes
        	delete_node(free_memory,found);
        	delete_node(free_memory,iter);


        	// if free memory is null, create a new list
        	if (free_memory == NULL)
        		free_memory = create_region(b,e,s);
        	// else just add a new region
        	else
        		free_memory = add_region(free_memory, b, e, s);


    		// recursively call coalesce for simplicity
    		coalesce(free_memory);
    		return;
        
        }
        
        // case when free'd memory is to the right of the memory to be coalesced
        if (iter->begin_address == found->end_address){

                                        printf("test\n");

			// initialize a begin, end, and size
        	void* b=found->begin_address, *e=iter->end_address; 
        	int s=(long int)iter->end_address - (long int)found->begin_address;


        	// delete the two nodes
        	delete_node(free_memory,found);
        	delete_node(free_memory,iter);

        	// if free memory is null, create a new list
        	if (free_memory == NULL)
        		free_memory = create_region(b,e,s);
        	// else just add a new region
        	else
        		free_memory = add_region(free_memory, b, e, s);

    		// recursively call coalesce for simplicity
    		coalesce(free_memory);
    		return;
        }
    }
}

// aligns a memory slot to 16 bytes, if less than 16, add until 16
// INPUT:	an size
// OUTPUT:	NONE, but the input size is referenced
int align_16(int in){

	if (in < 16){
		while (in < 16)
			(in)++;
		// add the number of bytes left over to make it work
		//printf("rounded up to 16 bytes\n");

	}

	// check to see if the bytes are aligned by 16, if not,
	if (in % 16 != 0){
		// add the number of bytes left over to make it work
		//printf("num of bytes to add: %d\n", in % 16);
		in += in % 16;
		return in;
	}
	//printf("aligned: %d\n",in );
	return in;
}

// gets the best fit in the free memory list
// INPUT:	the size of the memory that is wanted to allocate
// OUTPUT:	the pointer to the region in the free list that best fits
Region* get_best_fit(int sz) {

    if (free_memory == NULL) return NULL; // empty list!

    Region* temp = free_memory;

    while (temp != NULL) {
        if (temp->size >= sz) {
            return temp; // found a fit!
        }
        temp = temp->next;
    }
    return NULL; // no fit!
}
