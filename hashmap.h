#ifndef __HASH_MAP_H
#define __HASH_MAP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "prime.h"

// Universal identifier
enum OBJTYP {HASHMAP, STRING, COUNTER};

// Universal destroyer
void u_destroy(void *);

// Universal writer
size_t u_write(void * src, FILE * dst);

// Universal reader
size_t u_read(FILE * src, void ** dst);


typedef struct {
	
	unsigned long thing_hash;
	size_t thing_size;
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


typedef struct {
	
	enum OBJTYP objtyp;
	
	size_t count;
	
} counter_t;

typedef struct {
	
	enum OBJTYP objtyp;
	
	size_t length;
	char * bytes;
	
} string_t;

string_t * create_string(size_t length, char * bytes) {
	string_t * rtn = (string_t *) malloc(sizeof(string_t));
	rtn->objtyp = STRING;
	rtn->length = length;
	rtn->bytes = (char *) malloc(length);
	memcpy(rtn->bytes, bytes, length);
	
	return rtn;
}

void destroy_string(string_t * str) {
	
	free(str->bytes);
	free(str);
}


// Creates a hashnode
hash_node_t * create_hash_node(unsigned long thing_hash, size_t thing_size, void * thing);

// Destroys a hashnode
void destroy_hash_node(hash_node_t * hn, void (*destructor)(void *));

// Creates a hashmap
hash_map_t * create_hash_map(size_t initial_size);

// Destroys a hashmap
void destroy_hash_map(hash_map_t * hm, void(*destructor)(void *));

// Increases capacity of hashmap
void expand_map(hash_map_t * hm);

// Addes hashnode to hashmap
hash_node_t * insert_hash_map(hash_map_t * hm, hash_node_t * hn);

// returns hashnode with the given hash or NULL if no node has hash
hash_node_t * search_hash_map(hash_map_t * hm, unsigned long hash);

size_t read_hash_map(FILE * src, hash_map_t ** dst);
size_t write_hash_map(hash_map_t * src, FILE * dst);



hash_node_t * create_hash_node(unsigned long thing_hash, size_t thing_size, void * thing) {
	
	hash_node_t * rtn = (hash_node_t *) malloc(sizeof(hash_node_t));
	
	rtn->thing_hash = thing_hash;
	
	rtn->thing_size = thing_size;
	
	rtn->thing = (void *) malloc(thing_size);
	
	memcpy(rtn->thing, thing, thing_size);
	
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

void destroy_hash_map(hash_map_t * hm, void (*destructor)(void *)) {
	
	for(size_t i = 0; i < hm->capacity; i++) {
		if (hm->map[i] != NULL) destroy_hash_node(hm->map[i], destructor);
	}
	
	free(hm->map);
	free(hm);
	
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


size_t read_hash_map(FILE * src, hash_map_t ** dst) {
	return 0;
}

size_t write_hash_map(hash_map_t * src, FILE * dst) {
	return 0;
}




void u_destroy(void * obj) {
	
	if (obj == NULL) return;
	
	switch( * ((enum OBJTYP *) obj) ) {
			
		case HASHMAP:
			
			destroy_hash_map((hash_map_t *)obj, u_destroy);
			
			break;
			
		case STRING:
			
			destroy_string((string_t *)obj);
			
			break;
			
		case COUNTER:
			
			free(obj);
			
			break;
			
			
		default:
			
			break;
	}
	
}





size_t u_write(void * src, FILE * dst) {
	
	if (src == NULL) return 0;
	
	switch( * ((enum OBJTYP *) src) ) {
			
		case HASHMAP:
			
			return write_hash_map((hash_map_t *) src, dst);
			
		case STRING:
			
			return 0;
			
		case COUNTER:
			
			return 0;
			
		default:
			
			return 0;
			
	}
	
}




size_t u_read(FILE * src, void ** dst) {
	
	enum OBJTYP objtyp;
	
	int n = fread(&objtyp, sizeof(enum OBJTYP), 1, src);
	if (n <= 0) return n;
	
	fseek(src, -sizeof(enum OBJTYP), ftell(src));
	
	switch (objtyp) {
			
		case HASHMAP:
			
			return read_hash_map(src, (hash_map_t **) dst);
			
		case STRING:
			
			return 0;
			
		case COUNTER:
			
			return 0;
			
		default:
			
			return 0;
	}
	
}


#endif