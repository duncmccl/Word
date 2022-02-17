#ifndef __ARRAYLIST_H
#define __ARRAYLIST_H

#include <string.h>

#include "universal.h"

typedef struct {
	
	enum OBJTYP objtyp;
	
	size_t member_size;
	size_t count;
	size_t capacity;
	
	void ** data;
	
} arraylist_t;

arraylist_t * create_arraylist(size_t member_size, size_t initial_capacity);

void destroy_arraylist(void * arr, void (*destructor)(void *));

void expand_arraylist(arraylist_t * arr);
void push_front_arraylist(arraylist_t * arr, void * obj);
void * pop_back_arraylist(arraylist_t * arr);





arraylist_t * create_arraylist(size_t member_size, size_t initial_capacity) {
	
	arraylist_t * rtn = (arraylist_t *) malloc(sizeof(arraylist_t));
	
	rtn->objtyp = ARRAYLIST;
	rtn->member_size = member_size;
	rtn->count = 0;
	rtn->capacity = (initial_capacity >= 16 ? initial_capacity : 16);
	rtn->data = (void **) malloc(sizeof(void *) * rtn->capacity);
	for(size_t i = 0; i < rtn->capacity; i++) {
		rtn->data[i] = NULL;
	}
	
	return rtn;
	
}

void destroy_arraylist(void * arr, void (*destructor)(void *)) {
	
	arraylist_t * c_arr = (arraylist_t *) arr;
	
	for(size_t i = 0; i < c_arr->count; i++) {
		destructor(c_arr->data[i]);
	}
	
	free(c_arr->data);
	free(c_arr);
	
}

void expand_arraylist(arraylist_t * arr) {
	
	void ** old_data = arr->data;
	size_t old_count = arr->count;
	
	arr->capacity *= 2;
	arr->data = (void **) malloc(sizeof(void *) * arr->capacity);
	
	memcpy(arr->data, old_data, sizeof(void *) * old_count);
	
	free(old_data);
	
}

void push_back_arraylist(arraylist_t * arr, void * obj) {
	
	if (arr->count == arr->capacity) {
		expand_arraylist(arr);
	}
	
	arr->data[arr->count] = obj;
	
	arr->count++;
}

void * pop_back_arraylist(arraylist_t * arr) {
	
	if (arr->count <= 0) {
		
		return NULL;
		
	} else {
		
		arr->count--;
		void * rtn = arr->data[arr->count];
		arr->data[arr->count] = NULL;
		return rtn;
	}
}



#endif