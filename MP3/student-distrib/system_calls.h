#ifndef _system_calls_H
#define _system_calls_H

#include "lib.h"
#include "filesys.h"
#include "paging.h"
#include "terminal.h"
#include "x86_desc.h"
#include "rtc.h"
#include "keyboard.h"

#define NUM_OPEN_FILES  8
#define MAX_NUM_ARGS    3
#define MAX_ARGS_LEN    100
#define MAX_PCBs        6
#define ELF_LEN         4
#define FLAG_ON         1
#define FLAG_OFF        0
#define STDIO_OFFSET    2
#define PAGING_OFFSET   4

#define MAGICNUM1       0x7F
#define MAGICNUM2       0x45
#define MAGICNUM3       0x4C
#define MAGICNUM4       0x46

#define MB_8            0x800000
#define KB_8            MB_8/1024
#define USERMEM_OFFSET  0x48000
#define ELF_OFFSET      24
#define CLEAR_EBX       0x000F

#define VIDMAP_START    0x8000000  /* 0x8000000 represents the start of the user address */
#define VIDMAP_END      0x8400000  /* 0x8400000 represents the end of the user address */
#define SCREENSTART     0x8800000  /* Reprsents 132 MB */
#define VIDMAP          SCREENSTART/MB_4 // index for video mapping
#define BLOCK_SIZE      4096 // 4KB block sizes (4096 bytes)        

typedef struct file_operations_t {
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
} file_operations_t;

typedef struct fd_t {
    file_operations_t file_operations;
    uint32_t inode; // 4B size
    uint32_t file_pos; // 4B size
    uint32_t flags; // 4B reserved
} fd_t;

typedef struct pcb_t {
    uint32_t pid;
    uint32_t parent_pid;
    uint32_t esp;
    uint32_t ebp;
    uint8_t args[MAX_ARGS_LEN];
    fd_t fd_table[NUM_OPEN_FILES];
    uint32_t curr_term;
} pcb_t;

void split(const uint8_t* command, uint8_t* fname, uint8_t* args);
int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t get_pcb_count();
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);

#endif /* _system_calls_H */
