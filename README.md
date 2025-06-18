# Virtual Memory Testing - Lazy Allocations & Caching

## What we test:

#### Lazy allocations and memory caching - how to always optimize an array's first-write execution time 

### We test 3 (4) execution times:
* init
* first write
* second write
* third write (same as second write - sanity check)

### We have 3 ways to allocate:
* malloc
* calloc
* malloc + page touching

#### page touching
Virtual memory is allocated during malloc, but only mapped to physical memory during first read/write.  
So right after allocation, we write 1 byte to each allocated page, forcing mapping of array.  
This makes sure first-write is fast, as it doesn't need to map the memory.

