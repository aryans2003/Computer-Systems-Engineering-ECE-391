/* paging.h - Defines for various paging descriptors, descriptor tables,
 * and selectors
 * vim:ts=4 noexpandtab
 */


#ifndef PAGING_H
#define PAGING_H

#include "types.h"
#include "lib.h"

#define ENTRIES         1024
#define KERNEL_MEMORY   0x400000
#define VIDEO_MEMORY    0xB8000
#define TABLE_INDEX     0
#define KERNEL_INDEX    1
#define USER_MEMORY     0x8000000
#define MB_4            0x400000
#define KB_4            0x1000
#define USERMEM_INDEX   USER_MEMORY/MB_4
#define SHIFT_BYTES     12



/* Calling the setup paging logic defined in setup_paging.c */
extern void setup_paging();


/* Define the data structure for Page Directory Entry */
typedef struct __attribute__((packed)) page_directory_entry_t{
    uint32_t present                :1;
    uint32_t read_write             :1;
    uint32_t user_supervisor        :1;
    uint32_t write_through          :1;
    uint32_t cache_disable          :1;
    uint32_t accessed               :1;
    uint32_t available_1            :1;
    uint32_t page_size              :1;
    uint32_t available_2            :4;
    uint32_t address_bits           :20;
} page_directory_entry_t;

/* Define the data structure for Page Table Entry */
typedef struct __attribute__((packed)) page_table_entry_t {
    uint32_t present                :1;
    uint32_t read_write             :1;
    uint32_t user_supervisor        :1;
    uint32_t write_through          :1;
    uint32_t cache_disable          :1;
    uint32_t accessed               :1;
    uint32_t dirty                  :1;
    uint32_t page_attribute_table   :1;
    uint32_t global                 :1;
    uint32_t available              :3;
    uint32_t address_bits           :20;
} page_table_entry_t;

/* We initialize 1 page directory */
page_directory_entry_t page_directory_single[ENTRIES] __attribute__ ((aligned (4096)));
// 4096 comes from the fact that a page directory is made up of 4096 bytes
// the table contains 1024 4-byte entries
page_table_entry_t page_directory_entry_single[ENTRIES] __attribute__ ((aligned (4096)));

#endif /* PAGING_H */

