#ifndef __HASH_MAP_H
#define __HASH_MAP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "prime.h"

#include "universal.h"

typedef struct {
	
	unsigned long thing_hash;
	void * thing;
	
} hash_node_t;

typedef struct {
	
	enum OBJTYP objtyp;
	
	size_t capacity;
	size_t count;
	
	hash_node_t ** map;
	
	// Debug statistics
	size_t collision_count;
	size_t miss_count;
	
} hash_map_t;



// Creates a hashnode
hash_node_t * create_hash_node(unsigned long thing_hash, void * thing);

// Destroys a hashnode
void destroy_hash_node(hash_node_t * hn, void (*destructor)(void *));

// Creates a hashmap
hash_map_t * create_hash_map(size_t initial_size);

// Destroys a hashmap
void destroy_hash_map(void * hm, void(*destructor)(void *));

// Increases capacity of hashmap
void expand_map(hash_map_t * hm);

// Addes hashnode to hashmap
hash_node_t * insert_hash_map(hash_map_t * hm, hash_node_t * hn);

// returns hashnode with the given hash or NULL if no node has hash
hash_node_t * search_hash_map(hash_map_t * hm, unsigned long hash);





hash_node_t * create_hash_node(unsigned long thing_hash, void * thing) {
	
	hash_node_t * rtn = (hash_node_t *) malloc(sizeof(hash_node_t));
	
	rtn->thing_hash = thing_hash;
	
	rtn->thing = thing;
	
	return rtn;
}

void destroy_hash_node(hash_node_t * hn, void (*destructor)(void *)) {
	
	if (destructor) destructor(hn->thing);
	free(hn);
	
}


hash_map_t * create_hash_map(size_t initial_size) {
	
	hash_map_t * rtn = (hash_map_t *) malloc(sizeof(hash_map_t));
	rtn->objtyp = HASHMAP;
	rtn->capacity = next_prime(initial_size);
	rtn->count = 0;
	rtn->map = (hash_node_t **) malloc(sizeof(hash_node_t *) * rtn->capacity);
	for(size_t i = 0; i < rtn->capacity; i++) {
		rtn->map[i] = NULL;
	}
	rtn->collision_count = 0;
	rtn->miss_count = 0;
	return rtn;
}

void destroy_hash_map(void * hm, void (*destructor)(void *)) {
	
	hash_map_t * c_hm = (hash_map_t *) hm;
	
	for(size_t i = 0; i < c_hm->capacity; i++) {
		if (c_hm->map[i] != NULL) destroy_hash_node(c_hm->map[i], destructor);
	}
	
	free(c_hm->map);
	free(c_hm);
	
}


void expand_map(hash_map_t * hm) {
	
	size_t old_capacity = hm->capacity;
	hash_node_t ** old_map = hm->map;
	
	hm->capacity = next_prime(old_capacity * 2);
	hm->map = (hash_node_t **) malloc(sizeof(hash_node_t *) * hm->capacity);
	for(size_t i = 0; i < hm->capacity; i++) {
		hm->map[i] = NULL;
	}
	hm->collision_count = 0;
	
	for(size_t i = 0; i < old_capacity; i++) {
		if (old_map[i] != NULL) {
			
			hash_node_t * hn = old_map[i];
			
			hm->miss_count = 0;
			
			size_t j = hn->thing_hash % hm->capacity;
			
			while (hm->map[j] != NULL) {
				hm->collision_count++;
				hm->miss_count++;
				j = (j + 3) % hm->capacity;
				
			}
			
			hm->map[j] = hn;
			
		}
	}
	
	free(old_map);
}

hash_node_t * insert_hash_map(hash_map_t * hm, hash_node_t * hn) {
	
	if (hm->count * 4 >= hm->capacity * 3) expand_map(hm);
	
	hm->miss_count = 0;
	
	size_t i = hn->thing_hash % hm->capacity;
	
	while(hm->map[i] != NULL && hm->map[i]->thing_hash != hn->thing_hash) {
		hm->collision_count++;
		hm->miss_count++;
		i = (i + 3) % hm->capacity;
	}
	
	hash_node_t * rtn = hm->map[i];
	if (rtn == NULL) hm->count++;
	hm->map[i] = hn;
	return rtn;
	
}


hash_node_t * search_hash_map(hash_map_t * hm, unsigned long hash) {
	
	if (hm->count == 0) return NULL;
	
	hm->miss_count = 0;
	
	size_t i = hash % hm->capacity;
	
	while (hm->map[i] != NULL && hm->map[i]->thing_hash != hash) {
		hm->miss_count++;
		i = (i + 3) % hm->capacity;
	}
	
	return hm->map[i];
	
}







#endif