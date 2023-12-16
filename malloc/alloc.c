/**
 * malloc
 * CS 341 - Fall 2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define min(a, b) (((a) < (b)) ? (a) : (b))

typedef struct _metadata_entry{
  void* ptr;
  size_t size;
  int free;
  struct _metadata_entry* next;
  struct _metadata_entry* prev;
} entry_t;

static entry_t* head = NULL;
static size_t free_memory_size = 0;

void merge_with_previous(entry_t* entry){
    entry_t* previous_entry = entry -> prev;
    entry->size += previous_entry ->size + sizeof(entry_t);
    if (previous_entry -> prev) {
        previous_entry -> prev -> next = entry;
        entry->prev = previous_entry->prev;
    } else {
        entry -> prev = NULL;
        head = entry;
    }
}

void split(entry_t* entry, size_t size){
    entry_t* ex_spce = (void*) entry->ptr + size;
    ex_spce->ptr = ex_spce + 1;
    ex_spce->size = entry->size - size - sizeof(entry_t);
    ex_spce->free = 1;
    free_memory_size += ex_spce->size;

    entry->size = size;

    ex_spce->next = entry;
    ex_spce->prev = entry->prev;
    if(entry->prev)
        entry->prev->next = ex_spce;
    else
        head = ex_spce;
    
    entry->prev = ex_spce;

    if(ex_spce->prev && ex_spce->prev->free) 
        merge_with_previous(ex_spce);
    
}


/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    // implement calloc!
     void* ptr = malloc(num * size);
    
    if (ptr == NULL)
        return NULL;
    
    memset(ptr, 0, num*size);
    return ptr;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
    // implement malloc!
    entry_t* ptr = head;
    entry_t* picked = NULL;
    
    if (free_memory_size >= size) {
        while(ptr) {
            if (ptr -> free && ptr -> size >= size) {
                free_memory_size -= size;
                picked = ptr;
                break;
            }
            ptr = ptr -> next;
        }

        if (picked) {
            if((picked->size - size >= size) && (picked->size - size >= sizeof(entry_t)))
                split(picked,size);
            
            picked -> free = 0;
            return picked -> ptr;
        }
    }
    picked = sbrk(sizeof(entry_t));
    picked -> ptr = sbrk(0);
    if (sbrk(size) == (void*) -1)
        return NULL;

    picked -> size = size;
    picked -> free = 0;

    entry_t* og_head = head;
    
    if (og_head)
        og_head -> prev = picked;

    picked -> next = og_head;
    picked -> prev = NULL;
    head = picked;

    return picked -> ptr;
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    // implement free!
    if (ptr == NULL)
        return;
    
    entry_t* entry = ((entry_t*) ptr) - 1;

    entry -> free = 1;
    free_memory_size += (entry -> size + sizeof(entry_t));
    
    if(entry->prev && entry->prev->free == 1) 
        merge_with_previous(entry);
    
    if(entry->next && entry->next->free == 1)
        merge_with_previous(entry->next);
    
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    // implement realloc!
     if (ptr == NULL)
        return malloc(size);
    
    if (size == 0) {
        free(ptr);
        return NULL;
    }

    entry_t* entry = ((entry_t*) ptr) - 1;

    if (entry -> size >= size) {
        if(entry->size - size >= sizeof(entry_t)){
            split(entry,size);
            return entry->ptr;
        }
        return ptr;
    }

    void* new_ptr = malloc(size);
    if (new_ptr == NULL)
        return NULL;
    
    memcpy(new_ptr, ptr, min(size, entry -> size));
    free(ptr);
    return new_ptr;
}
