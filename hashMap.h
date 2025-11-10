#ifndef CMAP_H
#define CMAP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define HT_INIT_SIZE 101 			// Prime number (Prime number are more effective with '%' op)

#ifdef DEBUG
size_t map_collision;				// collision counter (use gcc -DDEBUG ... to use it)
#endif

// Hash table's entry (bucket)
typedef struct Bucket {

	char *key;						// String representing the key
	char *val;						// String representing the value
	struct Bucket *next;			// Pointer to the next [key:value] pair or to NULL

} Bucket;

typedef size_t (*HashFun)(char *);	// Pointer to an hash function

// Hash table
typedef struct {
	
	Bucket **bucket;				// Array of linked list
	size_t table_size; 				// Size of the hash table (number of table entries)
	size_t n_keys;					// Number of loaded keys on the map (hash table)
	HashFun fun;					// Generic hash function

} HashTable;

/* Init the map.  										*/
/* Return 1 on succecss, 0  on failure.					*/
int map_init(HashTable *ht);

/* Destroy the map (free the allocated memory).			*/
/* Return 1 on succecss, 0  on failure.					*/
int map_destroy(HashTable *ht);

/* Allow the user to 'overload' the hash function.		*/
/* If the map is non-empty, all the table will  		*/
/* be rehased.											*/
void map_hashfun_overload(HashTable *ht, HashFun f);

/* Put (insert) a new key-value pair.					*/
/* If the key is already in the map, it is updated		*/
/* with the new value. 									*/
/* Return 1 on succecss, 0  on failure.					*/
int map_put(HashTable *ht, char* key, char *val);

/* Return a pointer to the value of a given key.		*/
/* Return NULL if the key is not loaded in the map		*/
/* You should always check if the returning value is	*/
/* NULL, in order to avoid segmentation faults.			*/
char **map_get(HashTable *ht, char* key);

/* Given a key, deletes the key-value pair from the map */
/* Return 1 on succecss, 0 if the key is not loaded.	*/
/* It is not mandatory to check the ret value			*/
int map_del(HashTable *ht, char* key);

/* A simple and effective hash function by Daniel J.	*/
/* Bernstein. Simple and with pretty good performance.	*/
size_t djb2(char *in);

/* Print a human readale ASCII representation of the map  */
void map_print(HashTable *ht);

#endif
