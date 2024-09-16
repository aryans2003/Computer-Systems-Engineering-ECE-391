/* paging.c - Functions for various paging descriptors, descriptor tables,
 * and selectors
 * vim:ts=4 noexpandtab
 */

#include "paging.h"

/* Calling the initialize paging function defined in init_paging.S */
extern void init_paging(uint32_t directory);

/* void setup_paging()
 *  Functionality: Sets up paging, configures page directory entries
 *  Arguments: None
 *  Return: None
 *  References: OSDev
 ***********************************************************************************
 *      
 */
void setup_paging() {
    uint32_t i; // init var for loop

    /* Info taken from table 4-5: PDE that references a page table */
    /* The way we set this is by accessing the single PDE defined in paging.h 
       we access the specific bit position */
    /* Table Index for first PDE */
    page_directory_single[TABLE_INDEX].present         = 1; // must be 1 to reference a page table
    page_directory_single[TABLE_INDEX].read_write      = 1; // writes allowed to 4MB region
    page_directory_single[TABLE_INDEX].address_bits    = ((uint32_t)page_directory_entry_single) >> SHIFT_BYTES; // ptindex under OSDev Manipulation to get MSB
    /* Kernel Index for second PDE */
    page_directory_single[KERNEL_INDEX].present         = 1; // must be 1 to reference a page table
    page_directory_single[KERNEL_INDEX].read_write      = 1; // writes may be allowed to 4MB page 
    page_directory_single[KERNEL_INDEX].page_size       = 1; // must be 1  
    page_directory_single[KERNEL_INDEX].address_bits    = KERNEL_MEMORY >> SHIFT_BYTES; // ptindex under OSDev Manipulation to get MSB
    /* User Index for third PDE */
    page_directory_single[USERMEM_INDEX].present         = 1; // must be 1 to reference a page table
    page_directory_single[USERMEM_INDEX].read_write      = 1; // writes may be allowed to 4MB page 
    page_directory_single[USERMEM_INDEX].user_supervisor = 1; // user can access this memory
    page_directory_single[USERMEM_INDEX].page_size       = 1; // must be 1  
    page_directory_single[USERMEM_INDEX].address_bits    = USER_MEMORY >> SHIFT_BYTES; // ptindex under OSDev Manipulation to get MSB

    /* Initializing PTE */
    for(i = 0; i < ENTRIES; i++){
        // Initializing remaining PDEs
        if(page_directory_single[i].present == 0) {
            page_directory_single[i].read_write   = 1;  //writes allowed 
            page_directory_single[i].page_size    = 1;  //page size must be a single 4MB page
        }
        /*Initializing table entry*/
        if((i  == VIDEO_MEMORY >> SHIFT_BYTES) || (i  == (VIDEO_MEMORY + KB_4*1) >> SHIFT_BYTES) || (i  == (VIDEO_MEMORY + KB_4*2) >> SHIFT_BYTES) || (i  == (VIDEO_MEMORY + KB_4*3) >> SHIFT_BYTES)){
            page_directory_entry_single[i].present = 1; // must be 1
        }
        page_directory_entry_single[i].read_write           = 1; // writes allowed
        page_directory_entry_single[i].address_bits         = i; // set address bits to corresponding index
    }
    init_paging((uint32_t)page_directory_single); // make call to the ASM function wih corresponding PDE
}
