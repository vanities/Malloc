# Malloc
My attempt at malloc

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
