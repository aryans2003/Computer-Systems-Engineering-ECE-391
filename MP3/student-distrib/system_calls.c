#include "system_calls.h"
/* Define global vars and external functions here */
static int num_pcb = 0;
extern void flush_tlb();
extern void parent_return(uint32_t ebp, uint32_t esp, uint8_t status);
extern void context_switch(uint32_t d, uint32_t c, uint32_t b, uint32_t a);

/* halt()
 *  Functionality: Halts the current process, closes file descriptors, restores parent process data and paging, and returns control to parent process.
 *  Arguments: status - the status code indicating the reason for halting.
 *  Return: -1 (fails if it reaches here)
 *  References: Discussion 11
 ***********************************************************************************
 *  IMPORTANT NOTICE FOR READER
 */
int32_t halt(uint8_t status) {
    cli();
    int i;

    if (num_pcb == 1 || num_pcb == 2 || num_pcb == 3) { // if first three pcbs
        cli();
        const uint8_t* command = (uint8_t*)"shell";
        directory_entry_t dentry;
        uint8_t fname[MAX_FILENAME_LENGTH];
        uint8_t args[MAX_ARGS_LEN];
        split(command, fname, args); // Parse the arguments passed in
        uint8_t copyhelp_buffer[4];
        uint32_t i; //loop variable

        //Checking file validity
        if (read_dentry_by_name(fname, &dentry) == -1) {
            sti();
            return -1;
        } else if(read_data(dentry.inode_number, 0, copyhelp_buffer, 4) == -1) { // Check if the filename from the argument is valid and also an executable file
            sti();
            return -1;
        } else if(!(copyhelp_buffer[0] == MAGICNUM1 && copyhelp_buffer[1] == MAGICNUM2 && copyhelp_buffer[2] == MAGICNUM3 && copyhelp_buffer[3] == MAGICNUM4)) {
            sti();
            return -1;
        } 
        int curr_terminal = get_curr_terminal();
        //Setting up paging
        uint32_t pa = MB_8 + ((curr_terminal-1) * MB_4);
        page_directory_single[USERMEM_INDEX].address_bits = pa >> SHIFT_BYTES;
        flush_tlb();
        
        //Loading file into memory
        if(read_data(dentry.inode_number, 0, (uint8_t*)(USER_MEMORY+USERMEM_OFFSET), ((inode_t*)inode_block + dentry.inode_number)->length) == -1) {
            sti();
            return -1;
        } 

        //Creating pcb
        pcb_t* pcb = (pcb_t*)(MB_8 - (KB_8*curr_terminal));
        pcb->pid = curr_terminal-1;
        if(curr_terminal == 1 || curr_terminal == 2 || curr_terminal == 3) {
            pcb->parent_pid = -1;
        } else {
            pcb->parent_pid = curr_terminal;
        }
        strncpy((int8_t*) pcb->args, (int8_t*) args, MAX_ARGS_LEN);
        if (read_data(dentry.inode_number, ELF_OFFSET, copyhelp_buffer, ELF_LEN) == -1) {
            sti();
            return -1;
        }

        //Opening FDs
        /*stdin*/
        pcb->fd_table[0].file_operations.read = terminal_read;
        pcb->fd_table[0].file_operations.write = terminal_write;
        pcb->fd_table[0].file_operations.open = terminal_open;
        pcb->fd_table[0].file_operations.close = terminal_close;
        pcb->fd_table[0].flags = FLAG_ON; 

        /*stdout*/
        pcb->fd_table[1].file_operations.read = terminal_read;
        pcb->fd_table[1].file_operations.write = terminal_write;
        pcb->fd_table[1].file_operations.open = terminal_open;
        pcb->fd_table[1].file_operations.close = terminal_close;
        pcb->fd_table[1].flags = FLAG_ON; 

        for(i = STDIO_OFFSET; i < NUM_OPEN_FILES; i++) {
            pcb->fd_table[i].file_operations.read = empty_read;
            pcb->fd_table[i].file_operations.write = empty_write;
            pcb->fd_table[i].file_operations.open = empty_open;
            pcb->fd_table[i].file_operations.close = empty_close;
        }

        //context switch
        uint32_t esp;
        uint32_t ebp;
        __asm__("movl %%esp, %0" : "=r"(esp));
        __asm__("movl %%ebp, %0" : "=r"(ebp));
        pcb->esp = esp;
        pcb->ebp = ebp;
        uint32_t user_eip = *((uint32_t*)copyhelp_buffer);
        uint32_t user_esp = USER_MEMORY + MB_4 - PAGING_OFFSET;
        tss.ss0 = KERNEL_DS;
        tss.esp0 = MB_8 - (KB_8*(curr_terminal-1))-PAGING_OFFSET;
        context_switch(user_esp, USER_CS, USER_DS, user_eip);
        sti();
        return 0;
    }

    //Closing FDs
    pcb_t* pcb = (pcb_t*)(MB_8 - (KB_8*(num_pcb)));
    for (i = 0; i < NUM_OPEN_FILES; i++) {
        pcb->fd_table[i].flags = FLAG_OFF;
        close(i);
    }

    //Restoring parent data
    num_pcb--;
    uint32_t ebp = pcb->ebp;
    uint32_t esp = pcb->esp;
    tss.ss0 = KERNEL_DS;
    tss.esp0 = (MB_8-(KB_8*(pcb->parent_pid))) - PAGING_OFFSET;
    pcb_t* parent_pcb = (pcb_t*)(MB_8 - (KB_8*(pcb->parent_pid + 1)));

    //Restoring parent paging
    page_directory_single[USERMEM_INDEX].address_bits = (MB_8 + MB_4*(parent_pcb->pid)) >> SHIFT_BYTES;
    flush_tlb();

    parent_return(ebp, esp, status);
    sti();
    return -1; //fails if it reaches here
}

/* execute()
 *  Functionality: Executes a program specified by the command.
 *  Arguments: command - the command containing the program to execute and its arguments.
 *  Return: 
 *            -1: If program does not exist, filename is not an executable, or maximum supported PCBs reached.
 *           256: If program dies by an exception.
 *              0: If execution is successful.
 *  References: Discussion 11
 ***********************************************************************************
 *  IMPORTANT NOTICE FOR READER
 */
int32_t execute(const uint8_t* command) {
    // return -1 if program does not exist or filename is not an executable
    // return 256 if program dies by an exception
    // if call halt, then return what halt returns
    cli();
    directory_entry_t dentry;
    uint8_t fname[MAX_FILENAME_LENGTH];
    uint8_t args[MAX_ARGS_LEN];
    split(command, fname, args); // Parse the arguments passed in
    uint8_t copyhelp_buffer[4];
    uint32_t i; //loop variable

    if(num_pcb >= MAX_PCBs) {
        sti();
        return -1;
    }

    //Checking file validity
    if (read_dentry_by_name(fname, &dentry) == -1) {
        sti();
        return -1;
    } else if(read_data(dentry.inode_number, 0, copyhelp_buffer, ELF_LEN) == -1) { // Check if the filename from the argument is valid and also an executable file
        sti();
        return -1;
    } else if(!(copyhelp_buffer[0] == MAGICNUM1 && copyhelp_buffer[1] == MAGICNUM2 && copyhelp_buffer[2] == MAGICNUM3 && copyhelp_buffer[3] == MAGICNUM4)) {
        sti();
        return -1;
    } else if(num_pcb == MAX_PCBs) { // Check if we reached the maximum supported PCB’s (Max programs).
        sti();
        return -1;
    }
    num_pcb++;
    //Setting up paging
    uint32_t pa = MB_8 + ((num_pcb-1) * MB_4);
    page_directory_single[USERMEM_INDEX].address_bits = pa >> SHIFT_BYTES;
    flush_tlb();
    
    //Loading file into memory
    if(read_data(dentry.inode_number, 0, (uint8_t*)(USER_MEMORY+USERMEM_OFFSET), ((inode_t*)inode_block + dentry.inode_number)->length) == -1) {
        sti();
        return -1;
    } 

    //Creating pcb
    pcb_t* pcb = (pcb_t*)(MB_8 - (KB_8*num_pcb));
    pcb->pid = num_pcb-1;
    if(num_pcb == 1 || num_pcb == 2 || num_pcb == 3) {
        pcb->parent_pid = -1;
    } else {
        pcb->parent_pid = get_curr_terminal()-1;
    }
    strncpy((int8_t*) pcb->args, (int8_t*) args, MAX_ARGS_LEN);
    if (read_data(dentry.inode_number, ELF_OFFSET, copyhelp_buffer, ELF_LEN) == -1) {
        sti();
        return -1;
    }

    //Opening FDs
    /*stdin*/
    pcb->fd_table[0].file_operations.read = terminal_read;
    pcb->fd_table[0].file_operations.write = terminal_write;
    pcb->fd_table[0].file_operations.open = terminal_open;
    pcb->fd_table[0].file_operations.close = terminal_close;
    pcb->fd_table[0].flags = FLAG_ON; 

    /*stdout*/
    pcb->fd_table[1].file_operations.read = terminal_read;
    pcb->fd_table[1].file_operations.write = terminal_write;
    pcb->fd_table[1].file_operations.open = terminal_open;
    pcb->fd_table[1].file_operations.close = terminal_close;
    pcb->fd_table[1].flags = FLAG_ON; 

    for(i = STDIO_OFFSET; i < NUM_OPEN_FILES; i++) {
        pcb->fd_table[i].file_operations.read = empty_read;
        pcb->fd_table[i].file_operations.write = empty_write;
        pcb->fd_table[i].file_operations.open = empty_open;
        pcb->fd_table[i].file_operations.close = empty_close;
    }

    //context switch
    uint32_t esp;
    uint32_t ebp;
    __asm__("movl %%esp, %0" : "=r"(esp));
    __asm__("movl %%ebp, %0" : "=r"(ebp));
    pcb->esp = esp;
    pcb->ebp = ebp;
    uint32_t user_eip = *((uint32_t*)copyhelp_buffer);
    uint32_t user_esp = USER_MEMORY + MB_4 - PAGING_OFFSET;
    tss.ss0 = KERNEL_DS;
    tss.esp0 = MB_8 - (KB_8*(num_pcb-1))-PAGING_OFFSET;
    context_switch(user_esp, USER_CS, USER_DS, user_eip);
    sti();
    return 0;
}

/* split(const uint8_t* command, uint8_t* fname, uint8_t* args)
 *  Functionality: Parses the command string to extract the filename and its arguments.
 *  Arguments:
 *              command - the command string containing the filename and its arguments.
 *              fname - a buffer to store the extracted filename.
 *              arg1 - a buffer to store the first argument.
 *              arg2 - a buffer to store the second argument.
 *              arg3 - a buffer to store the third argument.
 *  Return: None
 *  References: Discussion 11
 ***********************************************************************************
 *  IMPORTANT NOTICE FOR READER
 */

void split(const uint8_t* command, uint8_t* fname, uint8_t* args) {
    int i = 0, j = 0;

    // Extract filename
    while (command[i] != ' ' && command[i] != '\0' && j < MAX_FILENAME_LENGTH) {
        fname[j++] = command[i++];
    }
    fname[j] = '\0'; // Null-terminate filename

    // Skip space between fname and arg1
    while (command[i] == ' ' && command[i] != '\0') {
        i++;
    }

    // Extract args
    j = 0;
    while (command[i] != '\0' && j < MAX_ARGS_LEN) {
        args[j++] = command[i++];
    }
    while(j < MAX_ARGS_LEN) {
        args[j++] = '\0';
    }
}

/* close()
 *  Functionality: Closes the file descriptor specified by fd.
 *  Arguments:
 *              fd - the file descriptor to close.
 *  Return:
 *              -1: If fd is less than 2 or greater than the number of open files.
 *         return value of file_operations.close(): Result of calling the close function associated with the file descriptor.
 *  References: Discussion 11
 ***********************************************************************************
 *  IMPORTANT NOTICE FOR READER
 */
int32_t close(int32_t fd){
    if(fd < STDIO_OFFSET || fd > NUM_OPEN_FILES) { // checking if fd within range and open
        return -1;
    }
    pcb_t* pcb = (pcb_t*)(MB_8 - (KB_8*num_pcb)); // get current pointer to pcb
    /* Setting necessary struct parameters for closing */
    int32_t retVal = pcb->fd_table[fd].file_operations.close(fd);
    pcb->fd_table[fd].flags = FLAG_OFF;  
    pcb->fd_table[fd].file_operations.close = empty_close;
    pcb->fd_table[fd].file_operations.open = empty_open;
    pcb->fd_table[fd].file_operations.read = empty_read;
    pcb->fd_table[fd].file_operations.write = empty_write;
    pcb->fd_table[fd].file_pos = 0;
    pcb->fd_table[fd].inode = 0;
    return retVal;
}

/* read()
 *  Functionality: Reads data from the file descriptor specified by fd into the buffer buf.
 *  Arguments:
 *              fd - the file descriptor from which to read.
 *              buf - the buffer to store the read data.
 *              nbytes - the number of bytes to read.
 *  Return:
 *              -1: If buf is NULL, fd is less than 0, fd is 1, fd is greater than the number of open files, or if the file associated with fd is not open.
 *         return value of file_operations.read(): Result of calling the read function associated with the file descriptor.
 *  References: Discussion 11
 ***********************************************************************************
 *  IMPORTANT NOTICE FOR READER
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes){
    sti();
    if (buf == NULL || fd < 0 || fd == STDIO_OFFSET - 1 || fd > NUM_OPEN_FILES) { // validate params within range
       return -1;
    }
    pcb_t* pcb = (pcb_t*)(MB_8 - (KB_8*num_pcb)); // get current pointer to pcb
    if(pcb->fd_table[fd].flags == FLAG_OFF) { // check if fd not in use
        return -1;
    }
    return pcb->fd_table[fd].file_operations.read(fd,buf,nbytes); // return read from filesys
}

/* write()
 *  Functionality: Writes data from the buffer buf to the file descriptor specified by fd.
 *  Arguments:
 *              fd - the file descriptor to which to write.
 *              buf - the buffer containing the data to write.
 *              nbytes - the number of bytes to write.
 *  Return:
 *              -1: If buf is NULL, fd is less than or equal to 0, or fd is greater than the number of open files, or if the file associated with fd is not open.
 *         return value of file_operations.write(): Result of calling the write function associated with the file descriptor.
 *  References: Discussion 11
 ***********************************************************************************
 *  IMPORTANT NOTICE FOR READER
 */

int32_t write(int32_t fd, const void* buf, int32_t nbytes){
    if (buf == NULL || fd <= 0 || fd > NUM_OPEN_FILES){ // validate params within range
       return -1;
    }
    pcb_t* pcb = (pcb_t*)(MB_8 - (KB_8*num_pcb)); // get pointer to current pcb
    if(pcb->fd_table[fd].flags == FLAG_OFF) { // check if fd not in use
        return -1;
    }
    return pcb->fd_table[fd].file_operations.write(fd, buf, nbytes); // return write from filesys
}

/* open()
 *  Functionality: Opens the file specified by fname and assigns a file descriptor to it.
 *  Arguments:
 *              fname - the name of the file to open.
 *  Return:
 *              -1: If read_dentry_by_name fails to find the file specified by fname, or if there are no available file descriptors, or if the file is of an unsupported type.
 *          file descriptor: The file descriptor assigned to the opened file if successful.
 *  References: Discussion 11
 ***********************************************************************************
 *  IMPORTANT NOTICE FOR READER
 */

int32_t open(const uint8_t* fname){
    int i;
    directory_entry_t dentry;
    if(read_dentry_by_name(fname, &dentry) == -1){
        return -1;  
    }
    pcb_t* pcb = (pcb_t*)(MB_8 - (KB_8*num_pcb));
    /* looping thru file type possibilities */
    for(i = STDIO_OFFSET; i < NUM_OPEN_FILES; i++) {
        /* setting relevant struct params */
        if(pcb->fd_table[i].flags == FLAG_OFF) {
            pcb->fd_table[i].file_pos = 0;
            pcb->fd_table[i].inode = dentry.inode_number;
            pcb->fd_table[i].flags = FLAG_ON;
            if(dentry.file_type == 0){ // RTC device            
                pcb->fd_table[i].file_operations.read = RTC_read;
                pcb->fd_table[i].file_operations.write = RTC_write;
                pcb->fd_table[i].file_operations.open = RTC_open;
                pcb->fd_table[i].file_operations.close = RTC_close;
            }else if (dentry.file_type == 1){ // directory      
                pcb->fd_table[i].file_operations.read = read_directory;
                pcb->fd_table[i].file_operations.write = write_directory;
                pcb->fd_table[i].file_operations.open = open_directory;
                pcb->fd_table[i].file_operations.close = close_directory;
            }else if (dentry.file_type == 2){ // regular file       
                pcb->fd_table[i].file_operations.read = read_file;
                pcb->fd_table[i].file_operations.write = write_file;
                pcb->fd_table[i].file_operations.open = open_file;
                pcb->fd_table[i].file_operations.close = close_file;
            }
            return i;
        }
    }
  return -1;
}

/* get_pcb_count()
 *  Functionality: Returns the number of process control blocks (PCBs) currently in use.
 *  Arguments: None
 *  Return: The number of PCBs currently in use.
 *  References: None
 ***********************************************************************************
 *  IMPORTANT NOTICE FOR READER
 */
int32_t get_pcb_count() {
    return num_pcb;
}

/*  int32_t getargs(uint8_t* buf, int32_t nbytes)
 *  Functionality: Copies the args into the buf value
 *  Arguments: uint8_t* buf - pointer to the address of the buf into which args are copied
 *             int32_t nbytes - number of bytes to copy into buf from args
 *  Return: 0 if successful, -1 on error
 *  References: None
 ***********************************************************************************
 *  IMPORTANT NOTICE FOR READER
 */
int32_t getargs(uint8_t* buf, int32_t nbytes) {
    pcb_t* pcb = (pcb_t*)(MB_8 - (KB_8*num_pcb)); //get the current pcb
    if ((strlen((int8_t*)pcb->args)) == 0) { //checking if there are arguments returns -1 of none are found
        return -1;
    }
    strncpy((int8_t*) buf, (int8_t*) pcb->args, nbytes); //copying args to buf
    return 0;
}

/*  int32_t vidmap(uint8_t** screen_start)
 *  Functionality: Maps the text-mode video memory into user space at a pre-set virtual address
 *  Arguments: uint8_t** screen_start - pointer to a pointer that has the starting address of a screen buffer
 *  Return: 0 if successful, -1 on error
 *  References: Week 12 Discussion
 ***********************************************************************************
 *  IMPORTANT NOTICE FOR READER
 */
int32_t vidmap(uint8_t** screen_start) {

    /* Check if location is non-existent */
    if(screen_start == NULL)
    {
        return -1;
    }
    /* Check if location is invalid, from Appendix C */
    if((uint32_t)screen_start < VIDMAP_START || (uint32_t)screen_start > VIDMAP_END)
    {
        return -1;
    }

    /* Setting necessary flags for our VIDMEM portion */
    page_directory_entry_single[0].present = 1;
    page_directory_entry_single[0].read_write = 1;
    page_directory_entry_single[0].user_supervisor = 1;
    page_directory_entry_single[0].address_bits = VIDEO_MEMORY >> SHIFT_BYTES;

    /* Setting necessary flags for our page_directory */
    page_directory_single[VIDMAP].present = 1;
    page_directory_single[VIDMAP].read_write = 1;
    page_directory_single[VIDMAP].user_supervisor = 1;
    page_directory_single[VIDMAP].page_size = 0;
    page_directory_single[VIDMAP].address_bits = ((int)(page_directory_entry_single)) >> SHIFT_BYTES;

    /* Flush TLB */
    flush_tlb();

    /* Point pointer to screen_start to 132MB */
    *screen_start = (uint8_t*)(SCREENSTART);
    
    /* Return success */
    return 0;
}

/*
 *  int32_t set_handler(int32_t signum, void* handler_address)
 *  Functionality: Sets the handler function for the specified signal number
 *  Arguments:
 *      int32_t signum - the signal number
 *      void* handler_address - the address of the handler function
 *  Return:
 *      -1
 *  References:
 *      N/A
 ***********************************************************************************
 *  IMPORTANT NOTICE FOR READER
 */
int32_t set_handler(int32_t signum, void* handler_address) {
    return -1;
}

/*
 *  int32_t sigreturn(void)
 *  Functionality: Returns control to the point where the signal was received
 *  Arguments:
 *      None
 *  Return:
 *      -1
 *  References:
 *      N/A
 ***********************************************************************************
 *  IMPORTANT NOTICE FOR READER
 */
int32_t sigreturn(void) {
    return -1;
}
