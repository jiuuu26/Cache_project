/*
 * cache.c
 *
 * 20493-01 Computer Architecture
 * Term Project on Implementation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 16, 2022
 *
 */

#define _CRT_SECURE_NO_WARINGS
#include <stdio.h>
#include <string.h>
#include "cache_impl.h"

extern int num_cache_hits;
extern int num_cache_misses;

extern int num_bytes;
extern int num_access_cycles;

extern int global_timestamp;

cache_entry_t cache_array[CACHE_SET_SIZE][DEFAULT_CACHE_ASSOC];
int memory_array[DEFAULT_MEMORY_SIZE_WORD];
int set = 0;

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_memory_content() {
    unsigned char sample_upward[16] = { 0x001, 0x012, 0x023, 0x034, 0x045, 0x056, 0x067, 0x078, 0x089, 0x09a, 0x0ab,
                                        0x0bc, 0x0cd, 0x0de, 0x0ef };
    unsigned char sample_downward[16] = { 0x0fe, 0x0ed, 0x0dc, 0x0cb, 0x0ba, 0x0a9, 0x098, 0x087, 0x076, 0x065, 0x054,
                                          0x043, 0x032, 0x021, 0x010 };
    int index, i = 0, j = 1, gap = 1;

    for (index = 0; index < DEFAULT_MEMORY_SIZE_WORD; index++) {
        memory_array[index] =
            (sample_upward[i] << 24) | (sample_upward[j] << 16) | (sample_downward[i] << 8) | (sample_downward[j]);
        if (++i >= 16)
            i = 0;
        if (++j >= 16)
            j = 0;

        if (i == 0 && j == i + gap)
            j = i + (++gap);

        printf("mem[%d] = %#x\n", index, memory_array[index]);
    }
}

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_cache_content() {
    int i, j;

    for (i = 0; i < CACHE_SET_SIZE; i++) {
        for (j = 0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t* pEntry = &cache_array[i][j];
            pEntry->valid = 0;
            pEntry->tag = -1;
            pEntry->timestamp = 0;
        }
    }
}

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
/* This function is a utility function to print all the cache entries. It will be useful for your debugging */
void print_cache_entries() {
    int i, j, k;

    for (i = 0; i < CACHE_SET_SIZE; i++) {
        printf("[Set %d] ", i);
        for (j = 0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t* pEntry = &cache_array[i][j];
            printf("V: %d Tag: %#x Time: %d Data: ", pEntry->valid, pEntry->tag, pEntry->timestamp);
            for (k = 0; k < DEFAULT_CACHE_BLOCK_SIZE_BYTE; k++) {
                printf("%#x(%d) ", pEntry->data[k], k);
            }
            printf("\t");
        }
        printf("\n");
    }
}

int check_cache_data_hit(void* addr, char type) {
    /*(1)*/

    int return_data = -1;
    int byte_addr = *(unsigned long int*) addr;
    int block_addr;
    block_addr = byte_addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE;
   
    //Initialize tag and set
    int tag = 0;
    int set = 0;

    //change block_addr to binary for tag and set's calculation
    int binary[6];
    int i, j;
    int ex = 1;

    //save block_addr to binary array in this binary[6] binary[5] binary[4] binary[3] binary[2] binary[1] binary[0] order
    for (i = 0; i < 6; i++) {   
        binary[i] = block_addr % 2;
        block_addr /= 2;
    }

    //caculation of set value according to CACHE_SET_SIZE
    for (i = 0; i < CACHE_SET_SIZE / 2; i++) {
        for (j = 0; j < i; j++) {
            ex *= 2;
        }
        set += (binary[i] * ex);
    }

    //caculation of tag value according to CACHE_SET_SIZE
    if (CACHE_SET_SIZE == 4) {           //tag calculation when direct mapped
        for (int i = CACHE_SET_SIZE / 2; i < 6; i++) {
            ex = 1;
            //Calculate the power value of 2 for tag calculation
            for (int j = 0; j < (i - CACHE_SET_SIZE / 2); j++) {
                ex *= 2;
            }
            tag += (binary[i] * ex);
        }
    }
    else {                               //tag calculation when 2-way or fully 
        for (int i = CACHE_SET_SIZE / 2; i <6; i++) {
            ex = 1;
            //Calculate the power value of 2 for tag calculation
            for (int j = 0; j < i - CACHE_SET_SIZE + 1; j++) {
                ex *= 2;
            }
            tag += (binary[i] * ex);
        }
    }
    
    /*(2)*/

    int word_addr = byte_addr / WORD_SIZE_BYTE;
    int word_off = byte_addr % WORD_SIZE_BYTE;
    //Get the return_data value if the data that fits the byte addr from the input file is in cache
    for (i = 0; i < CACHE_SET_SIZE; i++) {
        if (return_data != -1 || i > set) break;  //If the value of return_data has changed or is out of set value, break
        for (j = 0; j < DEFAULT_CACHE_ASSOC; j++) {
            if (return_data != -1) break;
            cache_entry_t* pEntry2 = &cache_array[i][j];
            if (set == i) {     //hit condition 1: set value must be the same
                if (pEntry2->tag == tag && pEntry2->valid == 1) {   //hit condition 2: set, tag match and hit if valid is 1
                    //Express the appropriate value according to char type
                    //Since char type is a hit if it is one of b, h, and w, increase num_bytes, num_cache_hits in each case, and add CACHE_ACCESS_CYCLE to num_access_cycles
                    //Save 1byte because 'b'
                    if (type == 'b') {
                        num_bytes++;
                        num_cache_hits++;
                        num_access_cycles += CACHE_ACCESS_CYCLE;
                        pEntry2->timestamp=global_timestamp;
                        if (word_addr % 2 == 0) {   //If word_addr is even, read from the beginning of the previous memory value of that block
                            return_data = ((pEntry2->data[word_off]));   //because it has to read the word_off first
                        }
                        else {                  //If word_addr is odd, read starting with the value of the back memory of that block
                            return_data = ((pEntry2->data[word_off + 4]));  // Must read from word_off+4 since it starts with a back memory value
                        }

                    }
                    // save 2bytes because 'h'
                    else if (type == 'h') {
                        num_bytes += 2;
                        num_cache_hits++;
                        num_access_cycles += CACHE_ACCESS_CYCLE;
                        pEntry2->timestamp = global_timestamp;
                        if (word_addr % 2 == 0) {    //If word_addr is even, read from the beginning of the previous memory value of that block
                            return_data =
                                (pEntry2->data[word_off + 1] << 8) | ((unsigned char)(pEntry2->data[word_off]));    //because it has to read the word_off first
                        }
                        else {       //If word_addr is odd, read starting with the value of the back memory of that block
                            return_data = (pEntry2->data[word_off + 5] << 8) |
                                ((unsigned char)(pEntry2->data[word_off + 4]));     // Must read from word_off+4 since it starts with a back memory value
                        }
                    }
                    //save 4bytes because 'w'
                    else if (type == 'w') {
                        num_bytes += 4;
                        num_cache_hits++;
                        num_access_cycles += CACHE_ACCESS_CYCLE;
                        pEntry2->timestamp = global_timestamp;
                        if (word_addr % 2 == 0) {   //If word_addr is even, read from the beginning of the previous memory value of that block
                            return_data = ((unsigned char)pEntry2->data[word_off + 3] << 24) | ((unsigned char)pEntry2->data[word_off + 2] << 16) | ((unsigned char)pEntry2->data[word_off + 1] << 8) | ((unsigned char)pEntry2->data[word_off]);
                        }
                        else {     //We do not take into account the case of exceeding one block, so if you start with the back memory value of the block, read the data[4] value first
                            return_data = ((unsigned char)pEntry2->data[7] << 24) | (unsigned char)(pEntry2->data[6] << 16) |
                                ((unsigned char)pEntry2->data[5] << 8) |
                                ((unsigned char)(pEntry2->data[4]));
                        }
                    }
                    else {      //If char type is not in b, h, or w, it is a miss, so save -1 in return_data
                        return_data = -1;
                        break;
                    }
                }   
                else {          //Miss if it does not meet hit's conditions
                    return_data = -1;
                    if (DEFAULT_CACHE_ASSOC<4) break;
                    
                }
            }
            else break;

        }
    }
    //If cache is not hit, return_data=-1 increases the num_cache_misses value, and adds MEMORY_ACCESS_CYCLE + CACHE_ACCESS_CYCLE to numa_access_cycles
    if (return_data == -1) {
        num_cache_misses++;
        num_access_cycles += (MEMORY_ACCESS_CYCLE + CACHE_ACCESS_CYCLE);
        //pEntry2->timestamp++;
    }

    /* Return the data */

    return return_data;
}


int find_entry_index_in_set(int cache_index) {
    int entry_index = -1;
    int LRUtime;
    int i = 0, j = 0;
    /*(1)*/
    if (CACHE_SET_SIZE == 4) {  //direct has cache set size of 4, direct has entry of 0
        entry_index = 0;
    }
    else {
        for (i = 0; i < DEFAULT_CACHE_ASSOC; i++) {     //2way has 2 entries, and fully has 4 entries
            cache_entry_t* pEntry = &cache_array[cache_index][i];
            //If the cache_array has a valid value of 0, it means that the data is empty. Return an empty entry_index for that cache
            if (pEntry->valid == 0) {
                entry_index = i;
                break;
            }
        }
        /*(2)*/
        if (entry_index == -1) {                        //2way has 2 entries, and fully has 4 entries
            for (i = 0; i < DEFAULT_CACHE_ASSOC; i++) {                 //loop until DEFAULT_CACHE_ASSOC
                cache_entry_t* pEntry = &cache_array[cache_index][i];   //declare pointer pointing cache_index
                LRUtime = 100;                                             //initialize LRUtime with 100
                for (int k = 0; k < CACHE_SET_SIZE; k++) {              //loop until CACHE_SET_SIZE
                    if (pEntry->timestamp < LRUtime) {                  //if timestamp < LRUtime
                        LRUtime = pEntry->timestamp;                    //change LRUtime with timestamp
                        entry_index = LRUtime;                           //change entry_index to LRUtime
                    }
                }
            }
        }
    }
    /* Check if there exists any empty cache space by checking 'valid' */
    /* Otherwise, search over all entries to find the least recently used entry by checking 'timestamp' */
    return entry_index;
}


int access_memory(void* addr, char type) {
    /* Fetch the data from the main memory and copy them to the cache */
    /* void *addr: addr is byte address, whereas your main memory address is word address due to 'int memory_array[]' */

    /* You need to invoke find_entry_index_in_set() for copying to the cache */

    /*(1)*/
    int byte_addr = *(unsigned long int*) addr;
    int memory_index = (byte_addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE) * DEFAULT_CACHE_BLOCK_SIZE_BYTE / WORD_SIZE_BYTE;
    int set = 0;
    int tag = 0;
    int return_data;
    int binary[6];
    int ex = 1;
    int block_addr = byte_addr / DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    // Calculate binary, set, hit just like the check_cache_data_hit function
    // Binary calculation
    for (int i = 0; i < 6; i++) {   
        binary[i] = block_addr % 2;
        block_addr /= 2;
    }
    // set caculation
    for (int i = 0; i < CACHE_SET_SIZE / 2; i++) {
        for (int j = 0; j < i; j++) {
            ex *= 2;
        }
        set += (binary[i] * ex);
    }
    // tag calculation
    if (CACHE_SET_SIZE == 4) {
        for (int i = CACHE_SET_SIZE / 2; i < 6; i++) {
            ex = 1;
            for (int j = 0; j < (i - CACHE_SET_SIZE / 2); j++) {
                ex *= 2;
            }
            tag += (binary[i] * ex);
        }
    }
    else {
        for (int i = CACHE_SET_SIZE / 2; i < 6; i++) {
            ex = 1;
            for (int j = 0; j < i - CACHE_SET_SIZE + 1; j++) {
                ex *= 2;
            }
            tag += (binary[i] * ex);
        }
    }
    /*(2)*/
    //Get entry_index correspoding to set value
    int entry_index = find_entry_index_in_set(set);
    cache_entry_t* pEntry = &cache_array[set][entry_index];
    pEntry->timestamp = global_timestamp;
    //Set valid=1 because the corresponding cache_array will contain a data value
    pEntry->valid = 1;
    pEntry->tag = tag;
    //Init memory content function saves values by combining them with << operators, so use a >> similar principle to save them separately
    pEntry->data[0] = memory_array[memory_index];
    pEntry->data[1] = memory_array[memory_index] >> 8;
    pEntry->data[2] = memory_array[memory_index] >> 16;
    pEntry->data[3] = memory_array[memory_index] >> 24;
    //0~3 The part where the mem index is even in one block. For example, the mem[16] part
    pEntry->data[4] = memory_array[memory_index + 1];
    pEntry->data[5] = memory_array[memory_index + 1] >> 8;
    pEntry->data[6] = memory_array[memory_index + 1] >> 16;
    pEntry->data[7] = memory_array[memory_index + 1] >> 24;
    //4~7 The part where the mem index is odd in one block. For example, the mem[17] part

    /* Return the accessed data with a suitable type */

    /*(3)*/
    int word_addr = byte_addr / WORD_SIZE_BYTE;
    int word_off = byte_addr % WORD_SIZE_BYTE;

    //same principle as check_cache_data_hit, store return_data value according to char type
    if (type == 'b') {
        num_bytes++;
        if (word_addr % 2 == 0) {
            return_data = ((pEntry->data[word_off]));
        }
        else {
            return_data = ((pEntry->data[(word_off + 4)]));
        }

    }
    else if (type == 'h') {
        num_bytes += 2;
        if (word_addr % 2 == 0) {
            return_data = (pEntry->data[(word_off + 1)] << 8) | ((unsigned char)(pEntry->data[word_off]));
        }
        else {
            return_data = (pEntry->data[(word_off + 5)] << 8) | ((unsigned char)(pEntry->data[(word_off + 4)]));
        }
    }
    else if (type == 'w') {
        num_bytes += 4;
        if (word_addr % 2 == 0) {
            return_data = ((unsigned char)pEntry->data[word_off + 3] << 24) | ((unsigned char)pEntry->data[word_off + 2] << 16) | ((unsigned char)pEntry->data[word_off + 1] << 8) | ((unsigned char)pEntry->data[word_off]);
        }
        else {
            return_data = ((unsigned char)pEntry->data[7] << 24) | ((unsigned char)pEntry->data[6] << 16) | ((unsigned char)pEntry->data[5] << 8) | ((unsigned char)(pEntry->data[4]));
        }
    }
    return return_data;

}


