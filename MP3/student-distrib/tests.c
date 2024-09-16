#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "rtc.h"
#include "terminal.h"
#include "filesys.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// division error test
int div_zero_error_test() {
	TEST_HEADER;

	int a = 0;
	int b;
	b = 1/a;
	return FAIL;
}

// system call handler test
int system_call_handler_test() {
	TEST_HEADER;

	__asm__("int $0x80");
	return FAIL;
}

// invalid opcode test
int invalid_opcode_test() {
	TEST_HEADER;
	
	__asm__("ud2");
	return FAIL;
}

/* Before video memory address test
 * 
 * Checks that dereferencing adress before the video memory starts doesn't work
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: Kernel freeze
 * Coverage: 0x00000 - 0xB7FFF
 */
int before_vid_test(){
	TEST_HEADER;

	unsigned int BEFORE_VID_ADDR = 0xB7FFF;
	char test;
	char *ptr = (char*)BEFORE_VID_ADDR;
	test = *ptr;

	return FAIL;	/*If kernel didn't freeze, FAIL returned*/
}

/* Starting video memory address test
 * 
 * Checks that dereferencing adress before at video memory starts works
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: 0xB8000 - 0xB8FFF
 */
int start_of_vid_test(){
	TEST_HEADER;

	unsigned int VIDEO_MEM_ADDR = 0xB8000;
	char test;
	char *ptr = (char*)VIDEO_MEM_ADDR;
	test = *ptr;

	return PASS;
}

/* End of video memory address test
 * 
 * Checks that dereferencing adress at end of video memory starts works
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: 0xB8FFF
 */
int end_of_vid(){
	TEST_HEADER;

	unsigned int END_OF_VID_ADDR = 0xB8FFF;
	char test;
	char *ptr = (char*)END_OF_VID_ADDR;
	test = *ptr;

	return PASS;
}

/* After video memory address test
 * 
 * Checks that dereferencing adress after video memory starts doesnt work
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: Kernel freeze
 * Coverage: 0xB9000 - 0x3FFFFF
 */
int after_vid_test()
{
	TEST_HEADER;

	unsigned int AFTER_VID_ADDR = 0xB9000;
	char test;
	char *ptr = (char*)AFTER_VID_ADDR;
	test = *ptr;

	return FAIL;	/*If kernel didn't freeze, FAIL returned*/
}

/* Before kernel address dereferencing test
 * 
 * Checks that dereferencing adress before kernel memory starts doesnt work
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: Kernel freeze
 * Coverage: 0x3FFFFF
 */
int before_kernel_page_test(){
	TEST_HEADER;

	unsigned int BEFORE_KERNEL_ADDR = 0x3FFFFF;
	char test;
	char *ptr = (char*)BEFORE_KERNEL_ADDR;
	test = *ptr;
	return FAIL;	/*If kernel didn't freeze, FAIL returned*/
}

/* Start of kernel address dereferencing test
 * 
 * Checks that dereferencing adress before kernel memory starts works
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: 0x400000
 */
int start_of_kernel_test(){
	TEST_HEADER;

	unsigned int KERNEL_ADDR = 0x400000;
	char test;
	char *ptr = (char*)KERNEL_ADDR;
	test = *ptr;

	return PASS;
}

/* Bottom of kernel address dereferencing test
 * 
 * Checks that dereferencing adress at bottom of kernel memory starts works
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: 0x7FFFFC
 */
int bottom_kernel_page_test(){
	TEST_HEADER;

	unsigned int BOTTOM_OF_KERNEL = 0x7FFFFF;

	char test;
	char *ptr = (char*)BOTTOM_OF_KERNEL;
	test = *ptr;
	return PASS;
}

/* After kernel address dereferencing test
 * 
 * Checks that dereferencing adress after kernel memory starts doesnt work
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: Kernel freeze
 * Coverage: 0x800000
 */
int after_kernel(){
	TEST_HEADER;

	unsigned int OUT_OF_RANGE_ADDR = 0x800000;

	char test;
	char *ptr = (char*)OUT_OF_RANGE_ADDR;
	test = *ptr;
	return FAIL;		/*If kernel didn't freeze, FAIL returned*/
}

/* Null dereferencing test
 * 
 * Checks that dereferencing null doesnt work
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: Kernel freeze
 * Coverage: 0x800000
 */
int null_paging_test(){
	TEST_HEADER;

	char test;
	char *ptr = NULL;
	test = *ptr;
	return FAIL;		/*If kernel didn't freeze, FAIL returned*/
}

/* Checkpoint 2 tests */

/* Terminal read and write test
 * 
 * Checks that the buffer reads an input from the keyboard buffer and outputs that into 
 * Inputs: None
 * Outputs: PASS
 * Side Effects: c gets printed out 127 times and we'll see that it prints out c the same number of times whether nbytes is 
 * 				 300 or 400
 * Coverage: 0x800000
 */
int test_terminal_write() {
	int nbytes = 12;
	char buf[nbytes];
	keyboard_buffer[0] = 'H';
	keyboard_buffer[1] = 'e';
	keyboard_buffer[2] = 'l';
	keyboard_buffer[3] = 'l';
	keyboard_buffer[4] = 'o';
	keyboard_buffer[5] = ' ';
	keyboard_buffer[6] = 'W';
	keyboard_buffer[7] = 'o';
	keyboard_buffer[8] = 'r';
	keyboard_buffer[9] = 'l';
	keyboard_buffer[10] = 'd';
	keyboard_buffer[11] = '\n';
	terminal_read(0, buf, nbytes);
	terminal_write(0, buf, nbytes);
	return PASS;
}

/* Terminal overflow test
 * 
 * Checks that the buffer gets cutoff after 127 characters(excluding \n)
 * Inputs: None
 * Outputs: PASS
 * Side Effects: c gets printed out 127 times and we'll see that it prints out c the same number of times whether nbytes is 
 * 				 300 or 400
 * Coverage: 0x800000
 */
int test_terminal_overflow() {
	int i;
	int nbytes = 300;
	char buf1[nbytes];
	for(i = 0; i < nbytes; i++) {
		buf1[i] = 'c';
	}
	terminal_write(0, buf1, nbytes);
	nbytes = 400;
	char buf2[nbytes];
	for(i = 0; i < nbytes; i++) {
		buf2[i] = 'c';
	}
	terminal_write(0, buf2, nbytes);
	return PASS;
}

int rtc_freq_test() {
	int i = 2;
    int j;
    int check = 0;

    check = RTC_open(0);
	clear();
	while(i <= 1024) {
		int check_ret = RTC_write(0, &i, 4);		//4 represents the number of bytes in an integer datatype
		check += check_ret;
		printf("Frequency test: %d", i);
		printf(" Hz \n");
		for(j = 0; j < i*5; j++) {
			check_ret = RTC_read(0, 0, 0);
			putc('1');
		}
		putc('\n');
		i = i*2;
	}
    if(check == 0) {
        return PASS;
    } 
	return FAIL;
}

int read_dentry_idx_test() {
	directory_entry_t dentry;
	int i;
	int j;
	for (i = 0; i < 17; i++) { // 17 files should be displayed
		if (read_dentry_by_index(i, &dentry) == 0) {
			printf("file_name: ");
			for (j = 0; j < MAX_FILENAME_LENGTH; j++) {
				putc(dentry.filename[j]);
			}
			printf(", file_type: %u, file_size: %d", dentry.file_type, (inode_block + dentry.inode_number)->length);
			printf("\n");
		} else {
			return FAIL;
		}
	}
	return PASS;
}

int read_dentry_name_test() {
	directory_entry_t dentry;
	char* filename = "frame0.txt";
	if (read_dentry_by_name((uint8_t*)filename, &dentry) == 0) {
		return PASS;
	} else {
		return FAIL;
	}
}

int read_small_file_test() {
	char* filename = "frame0.txt";
	uint8_t buf[187]; 
	int32_t size = 187;
	if (read_file((int32_t)filename, buf, size) != -1) {
		printf("file_name: frame0.txt\n");
		return PASS;
	} else {
		return FAIL;
	}
}

int read_large_file_test() {
	char* filename = "verylargetextwithverylongname.tx";
	uint8_t buf[187]; 
	int32_t size = 187;
	if (read_file((int32_t)filename, buf, size) != -1) {
		return PASS;
	} else {
		return FAIL;
	}
}

int read_exec_file_test() {
	char* filename = "grep";
	uint8_t buf[187]; 
	int32_t size = 187;
	if (read_file((int32_t)filename, buf, size) != -1) {
		return PASS;
	} else {
		return FAIL;
	}
}

int read_dir_test() {
	char buffer[33];
	int i;
	int32_t size = 32;
	memset(buffer, 0, 33);
	for (i = 0; i < 15; i++) {
		read_directory(0, buffer, size);
		printf(buffer);
		printf("\n");
		memset(buffer, 0, 33);
	}
	return PASS;
}


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	// TEST_OUTPUT("zero div test", div_zero_error_test());
	// TEST_OUTPUT("system call handler test", system_call_handler_test());
	// TEST_OUTPUT("invalid opcode test", invalid_opcode_test());
	// TEST_OUTPUT("before vid", before_vid_test());
	// TEST_OUTPUT("start_of_vid_test", start_of_vid_test());
	// TEST_OUTPUT("end of vid", end_of_vid());
	// TEST_OUTPUT("after_vid_test", after_vid_test());
	// TEST_OUTPUT("before_kernel_page_test", before_kernel_page_test());
	// TEST_OUTPUT("start_of_kernel_test", start_of_kernel_test());
	// TEST_OUTPUT("bottom_kernel_page_test", bottom_kernel_page_test());
	// TEST_OUTPUT("out_of_range_paging_test", after_kernel());
	// TEST_OUTPUT("null_paging_test", null_paging_test());
	// TEST_OUTPUT("terminal read and write test", test_terminal_write());
	// TEST_OUTPUT("terminal overflow test", test_terminal_overflow());
	// TEST_OUTPUT("RTC frequency test", rtc_freq_test());
	// TEST_OUTPUT("read_dentry_name_test", read_dentry_name_test());
	//  TEST_OUTPUT("read_dentry_idx_test", read_dentry_idx_test()); // TEST 1 for CP2
	// TEST_OUTPUT("read directory", read_dir_test());
	// TEST_OUTPUT("read small file", read_small_file_test()); // TEST 2 for CP2
	// TEST_OUTPUT("read large file", read_large_file_test());
	// TEST_OUTPUT("read exec file", read_exec_file_test());
}
