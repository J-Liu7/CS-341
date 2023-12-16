/**
 * finding_filesystems
 * CS 341 - Fall 2023
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string,
             "Free blocks: %zd\n"
             "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);


void* blok(file_system* fs, inode* parent, uint64_t bindex) {
  data_block_number* block;
  
  if (bindex < NUM_DIRECT_BLOCKS) {
    block = parent -> direct;
  } else {
    block = (data_block_number*)(fs -> data_root + parent -> indirect);
    bindex -= NUM_DIRECT_BLOCKS;
  }
  void* result = (void*) (fs -> data_root + block[bindex]);
  return result;
}

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    // Thar she blows!
    inode* node = get_inode(fs, path);
    if (node == NULL){
        errno = ENOENT;
        return -1;
    }
    uint16_t reserve = node->mode >> RWX_BITS_NUMBER;
    node -> mode = new_permissions | (reserve << RWX_BITS_NUMBER);
    clock_gettime(CLOCK_REALTIME, &(node -> ctim));
    return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!
    inode* node = get_inode(fs, path);
    if (node == NULL) {
        errno = ENOENT;
        return -1;
    }
    if (owner != ((uid_t)-1))
        node -> uid = owner;
    
    if (group != ((gid_t)-1))
        node -> gid = group;
    
    clock_gettime(CLOCK_REALTIME, &(node -> ctim));
    return 0;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    // Land ahoy!
    inode* node = get_inode(fs, path);
    if (node != NULL)
        return NULL;
    
    const char* file_name;
    inode* parent = parent_directory(fs, path, &file_name);
    if (!valid_filename(file_name))
        return NULL;
    
    if (parent == NULL || !is_directory(parent))
        return NULL;
    
    inode_number index = first_unused_inode(fs);
    if (index == -1)
        return NULL;
    
    inode* new = fs -> inode_root + index;
    init_inode(parent, new);
    minixfs_dirent d;
    d.inode_num = index;
    d.name = (char*) file_name;
    if ((parent -> size / sizeof(data_block)) >= NUM_DIRECT_BLOCKS)
        return NULL;
    
    int offset = parent -> size % sizeof(data_block);
    if (!offset && (add_data_block_to_inode(fs, parent) == -1))
        return NULL;
    
    void* start = blok(fs, parent, (parent -> size / sizeof(data_block))) + offset;
    memset(start, 0, FILE_NAME_ENTRY);
    make_string_from_dirent(start, d);
    parent -> size += MAX_DIR_NAME_LEN;
    return new;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
        size_t number_used = 0;
        char *map = GET_DATA_MAP(fs->meta);
        uint64_t i = 0;
        for (; i < fs->meta->dblock_count; i++) {
            if (map[i] != 0)
                number_used++;
        }
        char* info_string = block_info_string(number_used);
        size_t size = strlen(info_string);
        if (*off > (long) size)
            return 0;
        
        if (count > size - *off)
            count = size - *off;
        
        memmove(buf, info_string + *off, count);
        *off += count;
        return count;
    }

    errno = ENOENT;
    return -1;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
    inode* node = get_inode(fs, path);
    if (node == NULL) {
        node = minixfs_create_inode_for_path(fs, path);
        if (node == NULL) {
            errno = ENOSPC;
            return -1;
        }
    }
    size_t max = sizeof(data_block) * (NUM_DIRECT_BLOCKS + NUM_INDIRECT_BLOCKS);
    if (count + *off > max) {
        errno = ENOSPC;
        return -1;
    }
    int require_block = (count + *off + sizeof(data_block) - 1) / sizeof(data_block);
    if (minixfs_min_blockcount(fs, path, require_block) == -1) {
        errno = ENOSPC;
        return -1;
    }
    size_t bindex = *off / sizeof(data_block);
    size_t boffset = *off % sizeof(data_block);
    uint64_t size = MIN((sizeof(data_block) - boffset), count);
    void* block = blok(fs, node, bindex) + boffset;
    memcpy(block, buf, size);
    *off += size;
    size_t wcount = size;
    bindex++;
    while (wcount < count) {
        size = MIN((count - wcount), sizeof(data_block));
        block = blok(fs, node, bindex);
        memcpy(block, buf + wcount, size);
        bindex++;
        wcount += size;
        *off += size;
    }
    if (count + *off > node -> size) {
        node -> size  = count + *off;
    }
    clock_gettime(CLOCK_REALTIME, &(node->mtim));
    clock_gettime(CLOCK_REALTIME, &(node->atim));
    return wcount;
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *vpath = is_virtual_path(path);
    if (vpath)
        return minixfs_virtual_read(fs, vpath, buf, count, off);
    // 'ere be treasure!
    inode* node = get_inode(fs, path);
    if (node == NULL) {
        errno = ENOENT;
        return -1;
    }
    if ((uint64_t)*off > node -> size)
        return 0;
    
    size_t bindex = *off / sizeof(data_block);
    size_t boffset = *off % sizeof(data_block);
    count = MIN(count, node -> size - *off);
    uint64_t size = MIN(count, (sizeof(data_block) - boffset));
    void* block = blok(fs, node, bindex) + boffset;
    memcpy(buf, block, size);
    *off += size;
    size_t rcount = size;
    bindex++;
    while (rcount < count) {
        size = MIN((count - rcount), sizeof(data_block));
        block = blok(fs, node, bindex);
        memcpy(buf + rcount, block, size);
        bindex++;
        rcount += size;
        *off += size;
    }
    clock_gettime(CLOCK_REALTIME, &(node -> atim));
    return rcount;
}
