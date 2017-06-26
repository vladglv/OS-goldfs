#pragma once

// - Standard C
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// - Provided code
#include "disk_emu.h"

// - Require C11 to compile the code
#if __STDC_VERSION__ < 201112L
#error "C11 compiler is required"
#endif

// - Macro to inhibit unused variable warnings
#define UNUSED(expr)  \
    do {              \
        (void)(expr); \
    } while(0)

// - Defines for file system geometry
#define BLOCK_SIZE 1024
#define NUM_BLOCKS BLOCK_SIZE
#define BLOCKS_PER_INODE 14
#define MAX_FN_LEN 11
#define MAGIC 0XDEADBEEF
#define MAX_FILES 256
#define MAX_OPEN_FILES 32
#define INDIRECT_BLOCK_ENTRY_SIZE 4
#define INDIRECT_BLOCKS (BLOCK_SIZE / INDIRECT_BLOCK_ENTRY_SIZE)
#define FILE_SIZE_MAX_DIRECT (BLOCKS_PER_INODE * BLOCK_SIZE)
#define FILE_SIZE_MAX (FILE_SIZE_MAX_DIRECT + INDIRECT_BLOCKS * BLOCK_SIZE)
#define MAX_BLOCKS_PER_FILE (BLOCKS_PER_INODE + INDIRECT_BLOCKS)

// - Defines for file system entry sizes
#define DIR_ENTRY_SIZE 16
#define INODE_ENTRY_SIZE 64

// - Defines for table entry states
#define ENTRY_TAKEN 0
#define ENTRY_FREE 1
#define ENTRY_INVALID -1

// - Defines of error codes
#define MY_OK 0
#define MY_ERR -1

static char MY_NAME[] = "goldfs";

/**
 * @class _inode
 * @file sfs_api.h
 * @brief I-node structure for storage of file data. This structure is
 * stored on disk and cached in memory for faster access.
 */
typedef struct __attribute__((packed)) _inode {
    uint32_t size;                 //!< Size of the file
    int32_t ptr[BLOCKS_PER_INODE]; //!< Data block locations
    int16_t next;                  //!< Next block to look for I-nodes
    int16_t free;                  //!< State of an I-node
} inode_t;

#if 1
_Static_assert(sizeof(inode_t) == INODE_ENTRY_SIZE,
               "iNode size must be INODE_ENTRY_SIZE");
#endif

/**
 * @class _super_block
 * @file sfs_api.h
 * @brief Super-block structure for the header of the disk. This structure
 * is stored on the disk and cached in memory for faster access.
 */
typedef struct __attribute__((packed)) _super_block {
    uint32_t magic;          //!< Magic
    uint32_t blocks;         //!< Number of blocks
    uint32_t blocks_size;    //!< Size of a block
    int32_t sb_block_idx;    //!< Super-block starting index
    int32_t sb_block_num;    //!< Super-block block count
    int32_t fbm_block_idx;   //!< Free bit map starting index
    int32_t fbm_block_num;   //!< Free bit map block count
    int32_t dir_block_idx;   //!< Directory starting block index
    int32_t dir_block_num;   //!< Directory blocks count
    int32_t inode_block_idx; //!< I-node starting block index
    int32_t inode_block_num; //!< I-node blocks count
} super_block_t;

#if 1
_Static_assert(sizeof(super_block_t) <= BLOCK_SIZE,
               "super block size must be smaller or equal to BLOCK_SIZE");
#endif

/**
 * @class _fbm_table
 * @file sfs_api.h
 * @brief Free bit map table used for block allocation. This structure is
 * stored on the disk and cached in memory for faster access.
 */
typedef struct __attribute__((packed)) _fbm_table {
    uint8_t block[NUM_BLOCKS]; //!< State of a block: ENTRY_TAKEN or ENTRY_FREE
} fbm_table_t;

/**
 * @class _dir_entry
 * @file sfs_api.h
 * @brief Directory entry used for mapping file-names to I-nodes. This
 * structure is stored on the disk and cached in memory for faster access.
 */
typedef struct __attribute__((packed)) _dir_entry {
    char fn[MAX_FN_LEN];  //!< File name
    int8_t free;          //!< State of an entry: ENTRY_TAKEN or ENTRY_FREE
    int32_t linked_inode; //!< I-node associated with this file name
} dir_entry_t;

#if 1
_Static_assert(sizeof(dir_entry_t) == DIR_ENTRY_SIZE,
               "directory entry size must be DIR_ENTRY_SIZE");
#endif

/**
 * @class _file_entry
 * @file sfs_api.h
 * @brief File descriptor entry used for keeping track of open files. It
 * maps file descriptor to I-nodes. This structure is only stored in memory.
 */
typedef struct __attribute__((packed)) _file_entry {
    int8_t free;          //!< State of an entry: ENTRY_TAKEN or ENTRY_FREE
    int32_t ptr_read;     //!< Absolute position of the read pointer
    int32_t ptr_write;    //!< Absolute position of the write pointer
    int32_t linked_inode; //!< I-node associated with this file descriptor
} file_entry_t;

// - Defines for file system special blocks
#define SB_BLOCK 0
#define SB_BLOCK_NUM 1
#define DIR_BLOCK (SB_BLOCK + SB_BLOCK_NUM)
#define DIR_BLOCK_NUM (MAX_FILES / (BLOCK_SIZE / DIR_ENTRY_SIZE))
#define INODE_BLOCK (DIR_BLOCK + DIR_BLOCK_NUM)
#define INODE_BLOCK_NUM (MAX_FILES / (BLOCK_SIZE / INODE_ENTRY_SIZE))
#define FBM_BLOCK (INODE_BLOCK + INODE_BLOCK_NUM)
#define FBM_BLOCK_NUM 1

// - Super block management

/**
 * @brief Reads the Super-block from disk.
 * @param sb Super-block
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t sb_read(super_block_t* sb);

/**
 * @brief Updates the Super-block on disk.
 * @param sb Super-block
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t sb_update(const super_block_t sb);

/**
 * @brief Initialises the Super-block in memory.
 * @param sb Super-block
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t sb_init(super_block_t* sb);

// - Free bit map management

/**
 * @brief Reads the free bit map from disk.
 * @param fbm Free bit map table
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t fbm_read(fbm_table_t* fbm_table);

/**
 * @brief Updates the free bit map on disk.
 * @param fbm Free bit map table
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t fbm_update(const fbm_table_t fbm_table);

/**
 * @brief Initialises the free bit map in memory.
 * @param fbm Free bit map table
 * @return MY_OK is returned on success and MY_ERR otherwise
 */

int32_t fbm_init(fbm_table_t* fbm_table);

// - Block management (updates the free bit map table)

/**
 * @brief Allocates a free block and returns its index. If the requested
 * index is taken, then, the function fails.
 * @param fbm Free bit map table
 * @param idx A suggested index is considered only if the value of idx is >= 0
 * @return Index of the block or MY_ERR otherwise
 */
int32_t block_allocate(fbm_table_t* fbm_table, int32_t idx);

/**
 * @brief Marks a block as free.
 * @param fbm Free bit map table
 * @param idx Block index in free bit map
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t block_deallocate(fbm_table_t* fbm_table, int32_t idx);

// - Directory management

/**
 * @brief Finds the I-node associated with the file-name.
 * @param d Pointer to the directory structure
 * @param size Size of the directory
 * @param name File name
 * @return Index of the entry associated with name or MY_ERR otherwise
 */
int32_t dir_find(dir_entry_t* d, uint32_t size, const char* name);

/**
 * @brief Removes the association of the I-node and a file-name.
 * @param d Pointer to the directory structure
 * @param size Size of the directory
 * @param name File name
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t dir_remove(dir_entry_t* d, uint32_t size, const char* name);

/**
 * @brief Adds the association between the file-name and an I-node.
 * @param d Pointer to the directory structure
 * @param size Size of the directory
 * @param name File name
 * @param node I-node index
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t dir_add(dir_entry_t* d, uint32_t size, const char* name, uint32_t node);

/**
 * @brief Reads the directory from disk.
 * @param d Pointer to the directory structure
 * @param size Size of the directory
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t dir_read(dir_entry_t* d, uint32_t size);

/**
 * @brief Updates the directory on disk.
 * @param d Pointer to the directory structure
 * @param size Size of the directory
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t dir_update(const dir_entry_t* d, uint32_t size);

/**
 * @brief Initialises the directory on disk.
 * @param d Pointer to the directory structure
 * @param size Size of the directory
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t dir_init(dir_entry_t* d, uint32_t size);

// - File descriptor management

/**
 * @brief Removes the file descriptor.
 * @param f Pointer to the file descriptor table
 * @param size Size of the file descriptor table
 * @param fd File descriptor to remove
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t fdt_remove(file_entry_t* f, uint32_t size, int fd);

/**
 * @brief Find the first free file descriptor.
 * @param f Pointer to the file descriptor table
 * @param size Size of the file descriptor table
 * @param inode_idx The I-node bound to the file
 * @return A new file descriptor or MY_ERR otherwise
 */

int32_t fdt_add(file_entry_t* f, uint32_t size, uint32_t inode_idx);

/**
 * @brief Initialises the file descriptor table.
 * @param f Pointer to the file descriptor table
 * @param size Size of the file descriptor table
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t fdt_init(file_entry_t* f, uint32_t size);

// - I-node management

/**
 * @brief Finds the data block where an I-node is located.
 * @param p Pointer to the I-node table
 * @param size Size of the I-node table
 * @param idx Index of the I-node to look for
 * @return Index of the block where the I-node resides
 */
int32_t inode_find(inode_t* p, uint32_t size, int32_t idx);

/**
 * @brief Removes the I-node from the I-node table.
 * @param p Pointer to the I-node table
 * @param size Size of the I-node table
 * @param idx Index of the I-node to look for
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t inode_remove(inode_t* p, uint32_t size, int32_t idx);

/**
 * @brief Allocates a new I-node to the I-node table.
 * @param p Pointer to the I-node table
 * @param size Size of the I-node table
 * @return Index of I-node in the table on success and MY_ERR otherwise
 */
int32_t inode_allocate(inode_t* p, uint32_t size);

/**
 * @brief Initialises the I-node table.
 * @param p Pointer to the I-node table
 * @param size Size of the I-node table
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t inode_init(inode_t* p, uint32_t size);

/**
 * @brief Reads the I-node table from disk.
 * @param p Pointer to the I-node table
 * @param size Size of the I-node table
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t inode_read(inode_t* p, uint32_t size);

/**
 * @brief Updates the I-nodes table to disk.
 * @param p Pointer to the I-node table
 * @param size Size of the I-node table
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t inode_update(const inode_t* p, uint32_t size);

/**
 * @brief Obtains a list of blocks associated with an I-node.
 * @param p I-node
 * @param size Size of the block table
 * @return Address of the block table is returned on success and NULL otherwise
 */
int32_t* inode_get_block_list(const inode_t p, uint32_t* size);

/**
 * @brief Sets a list of blocks associated with an I-node.
 * @param p Pointer to the I-node
 * @param block_list List of blocks to set
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t inode_set_block_list(inode_t* p, int32_t* block_list);

/**
 * @brief Frees memory allocated by 'inode_get_block_list' call.
 * @param block_list Pointer to the block list
 * @return MY_OK is returned on success and MY_ERR otherwise
 */
int32_t inode_free_block_list(int32_t* block_list);

// - ssfs

/**
 * @brief Creates a new file system or opens an existing one.
 * @param fresh If 'fresh' is non zero, then a new file system is created.
 * Otherwise, an existing one is opened.
 */
void mkssfs(int fresh);

/**
 * @brief Opens a file specified by 'name'. If a file exists, then, it is opened
 * in append mode. Otherwise, a new file is created if there is a free entry in
 * the directory table. If the file is already opened and thus located in file
 * descriptor table, an old file handle is returned.
 * @param name File name
 * @return -1 on error or a file handle on success
 */
int ssfs_fopen(char* name);

/**
 * @brief Closes a file and frees a space in the file descriptor table.
 * @param fileID File handle given by a call to 'ssfs_fopen'
 * @return -1 on error or 0 on success
 */
int ssfs_fclose(int fileID);

/**
 * @brief Repositions the read pointer.
 * @param fileID File handle given by a call to 'ssfs_fopen'
 * @param loc Absolute position within the file on [0, size]
 * @return -1 on error or 0 on success
 */
int ssfs_frseek(int fileID, int loc);

/**
 * @brief Repositions the write pointer.
 * @param fileID File handle given by a call to 'ssfs_fopen'
 * @param loc Absolute position within the file on [0, size]
 * @return -1 on error or 0 on success
 */
int ssfs_fwseek(int fileID, int loc);

/**
 * @brief Writes a given amount of data to the file.
 * @param fileID File handle given by a call to 'ssfs_fopen'
 * @param buf Pointer to the data to write
 * @param length Length of the data to write
 * @return Number of bytes written or -1 on error. If all data cannot be
 * written, then, the data that could fit until maximum size is written and -1
 * is returned.
 */
int ssfs_fwrite(int fileID, char* buf, int length);

/**
 * @brief Reads a given amount of data to the file.
 * @param fileID File handle given by a call to 'ssfs_fopen'
 * @param buf Pointer to the data to read
 * @param length Length of the data to read
 * @return Number of bytes read or -1 on error.
 */
int ssfs_fread(int fileID, char* buf, int length);

/**
 * @brief Removes a file from the file system.
 * @param file Name of the file to be removed
 * @return -1 on error or 0 on success
 */
int ssfs_remove(char* file);

// - Bonus
int ssfs_commit();
int ssfs_restore(int cnum);
