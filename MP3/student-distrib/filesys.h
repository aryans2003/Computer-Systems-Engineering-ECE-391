/* filesys.c - Functions headers and structs for supporting operations on the file system image
 * vim:ts=4 noexpandtab
 */

#include "lib.h"
#include "system_calls.h"
#ifndef FILESYS_H
#define FILESYS_H

#define NUM_FILES             63
#define BOOT_RES              52
#define DENTRY_RES            24
#define MAX_FILENAME_LENGTH   32
#define DATA_BLOCK_LENGTH     1023
#define DATA_BLOCK_SIZE       4096

/* Defining File System Structure from Week 9 Discussion Slide 13 */
/* Check if unsigned or signed for data block */
/* File system structure, bottom middle */
typedef struct directory_entry_t{
    char filename[MAX_FILENAME_LENGTH]; // 32B size comes from the maximum possible length of a line
    uint32_t file_type; // 4B size
    uint32_t inode_number; // inode #, 4B size
    uint8_t reserve[DENTRY_RES]; // 24B reserved
} directory_entry_t;

/* Boot block structure, left */
typedef struct boot_block_t{
    uint32_t dir_entries_count; // # dir. entries, 4B size
    uint32_t inodes; // # inodes (N), 4B size
    uint32_t data_blocks; // # data blocks (D), 4B size
    uint8_t reserved[BOOT_RES]; // 52B reserved
    directory_entry_t dir_entries[NUM_FILES]; // dir. entries, 63B size (first 64 bytes is not for directory entries)
} boot_block_t;

/* inode structure, right */
typedef struct inode_t{
    uint32_t length; // length in B, 4B size
    uint32_t data_blocks[DATA_BLOCK_LENGTH]; // length of all data blocks, 1023B size
    /* 1023 comes from the ((4096B total size / 4kB per block) - 1 to be inclusive) */
} inode_t;

void init_file_sys(uint32_t filesys_ptr);

/* File function headers */
int32_t open_file(const uint8_t* filename);
int32_t close_file(int32_t fd);
int32_t write_file(int32_t fd, const void* buffer, int32_t size);
int32_t read_file(int32_t fd, void* buffer, int32_t size);

/* Directory function headers */
int32_t open_directory(const uint8_t* file_directory);
int32_t close_directory(int32_t file_directory);
int32_t write_directory(int32_t file_directory, const void* buffer, int32_t size);
int32_t read_directory(int32_t file_directory, void* buffer, int32_t size);

/* read  data functions */
int32_t read_dentry_by_name(const uint8_t* fname, directory_entry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, directory_entry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t empty_read (int32_t fd, void* buf, int32_t nbytes);
int32_t empty_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t empty_open (const uint8_t* filename);
int32_t empty_close (int32_t fd);

directory_entry_t* dentry_block;
boot_block_t* boot_block;
inode_t* inode_block;


#endif //FILESYS_H
