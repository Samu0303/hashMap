#include "hashMap.h"

#ifdef DEBUG
extern size_t map_collision;
#endif

// Init the map
int map_init(HashTable *ht) {
	
	ht->bucket = calloc(sizeof(*ht->bucket),HT_INIT_SIZE);
	if (!ht->bucket) {
		perror("map_init() failure");
		return 0;
	}	

	ht->table_size = HT_INIT_SIZE;
	ht->n_keys = 0;
	ht->fun = djb2;

	#ifdef DEBUG
	map_collision = 0;
	#endif

	return 1;
}	

// Free a Bucket *list
void bucket_list_free(Bucket *list) {
	
	if (!list) return;

	while (list) {
		
		Bucket *tmp = list;
		list = list->next;
		free(tmp->key);
		free(tmp->val);
		free(tmp);
	}	
}	

// Free all the table entries
int map_destroy(HashTable *ht) {
	
	if (!ht) return 0;

	for (size_t i = 0; i < ht->table_size; i++) {

		Bucket *curr = ht->bucket[i];
		
		if (!curr) continue;

		bucket_list_free(curr);
	}	

	free(ht->bucket);

	return 1;
}	

// Check wether n is prime
static int is_prime(size_t n) {

	// Since A = sqrt(A) * sqrt(A) ==> there is no factor n
	// suck that n > sqrt(A)
	for (size_t i = 3; i*i <= n; i += 2) {
		
		// Unique divior for a prime number is 1
		if (n % i == 0) return 0;
	}	

	return 1;
}	

// Find and return the next prime number greater than n
static size_t next_prime(size_t n) {
	
	if (n % 2 == 0) n++;

	while (!is_prime(n)) n += 2;

	return n;

}	

// Perform the rehash of the entire table.
// As the table grow we need to realloc a new 
// Bucket**, so the key stored in the old array
// needs to be rehashed and reindexed in the new one
static int rehash(HashTable *ht, int high_lf) {

	// Save the old size
	size_t old_table_size = ht->table_size;

	// Necessary if we are rehashing the table after
	// a map_hashfun_overload() call
	// The size is doubled and rounded to the next prime number
	if (high_lf) {

		ht->table_size *= 2;
		ht->table_size = next_prime(ht->table_size);
	}	
	
	// Save the old array
	Bucket **old_bucket = ht->bucket;

	// Allocate a new bigger array
	ht->bucket = calloc(sizeof(*ht->bucket),ht->table_size);
	if (!ht->bucket) {
		perror("rehash() failure");
		return 0;
	}	

	// Reset n_keys and collisions
	// (map_put increments n_keys for every new key added)
	ht->n_keys = 0;
	#ifdef DEBUG
	map_collision = 0;
	#endif

	// Iterate on the old array and re-put every key
	// in the new array
	for (size_t i = 0; i < old_table_size; i++) {
		
		Bucket *curr = old_bucket[i];
		if (!curr) continue;

		while (curr) {
			map_put(ht,curr->key,curr->val);
			curr = curr->next;
		}	
		// Free the i-entry's list
		bucket_list_free(old_bucket[i]);
	}	
	
	// Free the entire old bucket array
	free(old_bucket);

	return 1;
}	

// Self explanatory
void map_hashfun_overload(HashTable *ht, HashFun f) {

	ht->fun = f;
	if (ht->n_keys)
		rehash(ht,0);
}	

// Append (push?) a new Bucket node with his [key:value] pair in the list
static int bucket_list_append(Bucket** list, char* key, char *val) {
	
	Bucket *ret = malloc(sizeof(*ret));
	if (!ret) {
		goto err;
	}	

	size_t key_len = strlen(key);
	size_t val_len = strlen(key);

	ret->key = malloc(sizeof(*ret->key)*(key_len+1));
	if (!ret->key) {
		free(ret);
		goto err;
	}	
	ret->val = malloc(sizeof(*ret->val)*(val_len+1));
	if (!ret->val) {
		free(ret->key);
		free(ret);
		goto err;
	}	

	strcpy(ret->key,key);
	strcpy(ret->val,val);
	ret->next = NULL;

	if (!*list) {
		*list = ret;
		return 1;
	}

	Bucket *tmp = *list;

	while(tmp->next) {
		
		tmp = tmp->next;
	}

	#ifdef DEBUG
	map_collision++;
	#endif

	tmp->next = ret;

	return 1;

	err:
	perror("map_init()->Bucket_list_append() failure");
	return 0;
}	

// Put (add) a new [key:value] pair in the table
int map_put(HashTable *ht, char* key, char *val) {

	// Try to retrieve the key from the map
	char **check = map_get(ht,key);

	// If the key is loaded in the map, the value is simply updated
	if (check) {
		if (strcmp(*check,val)) {

			free(*check);
			*check = calloc(sizeof(**check), (strlen(val) + 1));
			strcpy(*check,val);
		}	
		return 1;
	}	
	// If the key is a new key, the new [key:value] pair is added
	else {

		double load_factor = (double)ht->n_keys / (double)ht->table_size;
	
		// Best performace in terms of collision and memory with lf ~= 0.75
		if (load_factor >= 0.75)
			rehash(ht,1);
	
		// Compute the index (hash) and adjust it to the size of the table
		size_t index = ht->fun(key) % ht->table_size;
	
		// Append the new [key:value] pair to the list in index position
		int ret = bucket_list_append(&ht->bucket[index],key,val);
	
		if(!ret) {
			fprintf(stderr, "map_put() failure\n");
			return 0;
		}	
	
		// A new key has been added
		ht->n_keys++;
	}	
	return 1;
}	

// Given a key, return a pointer to the associated value
char **map_get(HashTable *ht, char* key) {
	
	// Compute the index (hash) and adjust it to the size of the table
	size_t index = ht->fun(key) % ht->table_size;

	Bucket *curr = ht->bucket[index];

	// If the list is empty, the key is not loaded
	if (!curr)
		goto err;

	// Scroll the list until curr reaches NULL or the key is found
	while (curr && strcmp(curr->key,key))
		curr = curr->next;
	
	// If curr reaches NULL, the key is not loaded
	if (!curr)
		goto err;

	// Return the address of the value associated to the key
	return &curr->val;

	err:
	return NULL;
}	

// Remove a Bucket node frome the list
static int bucket_list_remove(Bucket **list, char* key) {
	
	if (!*list) return 0;

	Bucket* prev = *list;
	Bucket* curr = *list;

	// Scroll the list until curr is NULL or the key is found
	while (curr && strcmp(curr->key,key)) {
		
		prev = curr;
		curr = curr->next;
	}	

	// If curr reaches NULL, the key is not loaded
	if (!curr) return 0;

	// If curr is the head of the list, update the head
	if (curr == *list)
		*list = curr->next;
	// Else patch the nodes
	else
		prev->next = curr->next;

	// Free the memory
	free(curr->key);
	free(curr);

	return 1;
}	

// Given a key, delete a [key:value] pair from the table
int map_del(HashTable *ht, char* key) {
	
	// Compute the index (hash) and adjust it to the size of the table
	size_t index = ht->fun(key) % ht->table_size;

	// if the list is empty, the key is not loaded
	if (!ht->bucket[index])
		goto err;

	// If the key is not loaded in the map returns 0
	if (!bucket_list_remove(&ht->bucket[index], key)) {
		goto err;
	}	
	
	return 1;

	err:
	fprintf(stderr, "[map_del() failure] the key '%s' isn't loaded in the map\n", key);
	return 0;
}	

// Simple Hash function
size_t djb2(char *in) {
	
	size_t hash = 5381;
    int c;

    while ((c = *in++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}	

// Human readale ASCII representation of the map 
void map_print(HashTable *ht) {

	puts("+----------+--------------------+------------------------------------+");
	puts("+   hash   +        key         +                  value             +");
	puts("+----------+--------------------+------------------------------------+");

	size_t counter = 0;

	for (size_t i = 0; i < ht->table_size; i++) {
		
		if (!ht->bucket[i]) continue;

		Bucket *tmp = ht->bucket[i];	

		while (tmp) {

			printf("|%9lu |%*s |%*s|\n", i, 19, tmp->key, 36, tmp->val);
			puts("+----------+--------------------+------------------------------------+");
			counter++;
			tmp = tmp->next;
		}	
	}	

	printf("| n_keys: %21lu | table_size: %22lu |\n", ht->n_keys, ht->table_size);
	puts("+----------+--------------------+------------------------------------+");

	#ifdef DEBUG
	double eff = (double)map_collision/(double)ht->n_keys;
	printf("| Collisions: %17lu | Collision / n_keys = %.11lf |\n", map_collision,  eff); 
	puts("+----------+--------------------+------------------------------------+");
	#endif

}	
