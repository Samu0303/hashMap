#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashMap.h"

extern size_t map_collision;

int main() {

	// Init
    HashTable ht;
    if (!map_init(&ht)) {
        fprintf(stderr, "Failed to initialize hash table\n");
        exit(EXIT_FAILURE);
    }

	char s_key[100];
	char s_val[100];

	// Put (add) some key
	for (size_t i = 0; i < 800000; i++) {
		
		sprintf(s_key,"%lu_key", i);
		sprintf(s_val,"%lu_val", i);

		if (!map_put(&ht,s_key,s_val)) {
			perror("failure");
			exit(EXIT_FAILURE);
		}	

		// Trying to put the same key multiple times
		if (!map_put(&ht,"readded_key", "readded_val")) {
			perror("failure");
			exit(EXIT_FAILURE);
		}	
	}	
	
	// Delete some key
	for (size_t i = 1501; i < 2001; i++) {
		
		sprintf(s_key,"%lu_key", i);
		if(!map_del(&ht,s_key))
			puts("Key not loaded!");
	}	

	// Print the table
	map_print(&ht);

	// Free the memory
    map_destroy(&ht);

    return 0;
}
