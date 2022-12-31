/*
 * main.c
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
#include "cache_impl.h"

int num_cache_hits = 0;
int num_cache_misses = 0;

int num_bytes = 0;
int num_access_cycles = 0;

int global_timestamp = 0;

int retrieve_data(void* addr, char data_type) {
    int value_returned = -1; /* accessed data */

    /* Invoke check_cache_data_hit() */
    //Read values if you are checking cache for values first
    value_returned = check_cache_data_hit(addr, data_type);

    /* In case of the cache miss event, access the main memory by invoking access_memory() */
    //To access memory when a miss occurs in cache
    if (value_returned == -1) {
        value_returned = access_memory(addr, data_type);
    }
   
    return value_returned;
}

int main(void) {
    FILE* ifp = NULL, * ofp = NULL;
    unsigned long int access_addr; /* byte address (located at 1st column) in "access_input.txt" */
    char access_type; /* 'b'(byte), 'h'(halfword), or 'w'(word) (located at 2nd column) in "access_input.txt" */
    int accessed_data; /* This is the data that you want to retrieve first from cache, and then from memory */

    init_memory_content();
    init_cache_content();

    ifp = fopen("C://Users//user//Desktop//cache file//access_input2.txt", "r");
    ofp = fopen("C://Users//user//Desktop//cache file//access_output2_2.txt", "w");
    fputs("[Accessed Data] \n", ofp);

    if (ifp == NULL) {
        printf("Can't open input file\n");
        return -1;
    }

    if (ofp == NULL) {
        printf("Can't open output file\n");
        fclose(ifp);
        return -1;
    }

    /* Fill out here by invoking retrieve_data() */
    //read input file line by line and write it on output file
    while (1) {     //read & write loop until the end of input file
        // read
        fscanf(ifp, "%lu\t%c", &access_addr, &access_type); //read two data and put the first one
                                                            // in access_addr, and the second one in access_type
        if (feof(ifp)) break;   // escape from loop at the end of input file

        accessed_data = retrieve_data(&access_addr, access_type);   //put return data from retrieve_data
        global_timestamp++;          //increase global timestamp by 1                                                                      // function to accessed_data
        // write
        fprintf(ofp, "%lu\t%c\t%#x\n", access_addr, access_type, accessed_data);
        //write access_addr, aceess_type, and accessed_data in output file
    }

    double hit_ratio = num_cache_hits / (double)global_timestamp;           //calculate hit ratio
    double bandwidth = num_bytes / (double)num_access_cycles;               //calculate bandwidth

    fputs("----------------------------------------- \n", ofp);         //print to separate section
    if (DEFAULT_CACHE_ASSOC == 1)   //directed mapped cache case
        fputs("[Direct mapped cache performance] \n", ofp);            //print performance title
    else if (DEFAULT_CACHE_ASSOC == 4)  //fully associative cache case
        fputs("[Fully associative cache performance] \n", ofp);        //print performance title
    else {      //n-way set associative cache case
        fputs("[", ofp);
        fprintf(ofp, "%d", DEFAULT_CACHE_ASSOC);                   //print associativity
        fputs("-way set associative cache performance] \n", ofp);   //print performance title
    }
    fputs("Hit ratio = ", ofp);                          //print hit ratio
    fprintf(ofp, "%.2f", hit_ratio);               //print hit ratio
    fputs(" (", ofp);                                   //print (
    fprintf(ofp, "%d", num_cache_hits);           //print number of cache hits
    fputs("/", ofp);                                   //print /
    fprintf(ofp, "%d", global_timestamp);        //print number of accesses
    fputs(") \n", ofp);                               //print )

    fputs("Bandwidth = ", ofp);                         //print bandwidth
    fprintf(ofp, "%.2f", bandwidth);              //print bandwidth
    fputs(" (", ofp);                                 //print (
    fprintf(ofp, "%d", num_bytes);              //print num of bytes
    fputs("/", ofp);                                //print /
    fprintf(ofp, "%d", num_access_cycles);    //print num of clock cycles
    fputs(") \n", ofp);                           //print )


    fclose(ifp);
    fclose(ofp);

    print_cache_entries();
    return 0;
}