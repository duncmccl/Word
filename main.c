#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "prime.h"
#include "hashmap.h"





unsigned long simple_hash(const size_t len, const void * str) {
	
	char * cast_str = (char *) str;
	
	unsigned long rtn = 0;
	unsigned long p = 53;
	unsigned long pp = 1;
	unsigned long m = 1e9 + 9;
	
	for(size_t i = 0; i < len; i++) {
		rtn = (rtn + (cast_str[i] * pp)) % m;
		pp = (pp * p) % m;
	}
	
	return rtn;
}


void clean_string(char * str) {
	
	
	char * head = str;
	
	while (*head != '\0') {
		
		if (*head >= 'A' && *head <= 'Z') {
			
			*head = tolower(*head);
			
		} else if (*head >= 'a' && *head <= 'z') {
			
			// Do nothing
			
		} else {
			
			*head = ' ';
			
		}
		
		head++;
	}
	
	
	
	// Remove any leading or repeating spaces
	
	head = str;
	char * tail = head;
	char space = 1;
	
	while (*head != '\0') {
		
		if (*head == ' ' && space) {
			
			head++;
			
		} else if (*head == ' ' && !space) {
			
			*tail = *head;
			head++;
			tail++;
			space = 1;
			
		} else {
			
			*tail = *head;
			head++;
			tail++;
			space = 0;
			
		}
	}
	
	*tail = *head;
}


typedef struct {
	enum OBJTYP objtyp;
	size_t row, col;
} pos_t;


int main() {
	
	// Initilize global prime number generator
	init_prime();
	
	// Create Hashmap for dictionary
	hash_map_t * dictionary = create_hash_map(10);
	
	// Read file and populate hashmap
	FILE * text_f = fopen("text.txt", "r");
	
	size_t buff_size = 2048;
	char buff[buff_size];
	
	size_t row = 0;
	
	
	
	while(fgets(buff, buff_size, text_f)) {
		
		clean_string(buff);
		
		if (strlen(buff) > 0) {
			
			size_t col = 0;
			
			char * word = strtok(buff, " ");
			
			while(word) {
				
				size_t word_len = strlen(word) + 1;
				unsigned long word_hash = simple_hash(word_len, word);
				
				
				
				size_t pos_len = sizeof(pos_t);
				pos_t pos = (pos_t) {SIMPLE, row, col};
				unsigned long pos_hash = simple_hash(pos_len, &pos);
				hash_node_t * pos_node = create_hash_node(pos_hash, pos_len, &pos);
				
				
				hash_node_t * dictionary_entry = search_hash_map(dictionary, word_hash);
				
				
				if (dictionary_entry == NULL) {
					hash_map_t dummy;
					dummy.objtyp = HASHMAP;
					dummy.capacity = next_prime(10);
					dummy.count = 0;
					dummy.map = (hash_node_t **) malloc(sizeof(hash_node_t *) * dummy.capacity);
					for(size_t i = 0; i < dummy.capacity; i++) {
						dummy.map[i] = NULL;
					}
					dummy.collision_count = 0;
					dummy.miss_count = 0;
					
					dictionary_entry = create_hash_node(word_hash, sizeof(hash_map_t), &dummy);
					hash_node_t * reject = insert_hash_map(dictionary, dictionary_entry);
					if (reject) destroy_hash_node(reject, beelzebub);
					
				}
				
				hash_map_t * word_list = (hash_map_t *) dictionary_entry->thing;
				
				hash_node_t * reject = insert_hash_map(
					word_list, 
					pos_node);
				if (reject) destroy_hash_node(reject, beelzebub);
				
				
				word = strtok(word + word_len, " ");
				col++;
			}
			
		}
		
		
		row++;
	}
	
	fclose(text_f);
	
	
	
	
	
	
	printf("Dictionary Stats:\n");
	printf("  Count: %ld\n", dictionary->count);
	printf("  Capacity: %ld\n", dictionary->capacity);
	printf("\n");
	
	
	char * str_queary;
	size_t len_queary;
	unsigned long hash_queary;
	hash_node_t * queary;
	
	size_t queary_count = 20;
	char queary_list[20][256] = {
		"sherlock",
		"watson",
		"lock",
		"door",
		"apple",
		"aware",
		"document",
		"sour",
		"grudge",
		"flex",
		"galaxy",
		"system",
		"injection",
		"frighten",
		"medium",
		"increase",
		"salon",
		"convert",
		"color",
		"finance"
	};
	
	
	
	for(size_t i = 0; i < queary_count; i++) {
		
		str_queary = queary_list[i];
		len_queary = strlen(str_queary);
		
		hash_queary = simple_hash(len_queary, str_queary);
		
		queary = search_hash_map(dictionary, hash_queary);
		
		printf("Queary: %s\n", str_queary);
		printf("  Hash: %ld\n", hash_queary);
		
		if (queary) {
			
			hash_map_t * word_list = queary->thing;
			
			printf("  Success!\n  Misses: %ld\n  Count: %ld\n", dictionary->miss_count, word_list->count);
			
			printf("      row,   col\n");
			
			for(size_t j = 0; j < word_list->capacity; j++) {
				if (word_list->map[j]) {
					
					hash_node_t * pos_entry = word_list->map[j];
					pos_t * pos = (pos_t *) pos_entry->thing;
					
					printf("  [ %5ld, %5ld ]\n", pos->row, pos->col);
				}
			}
			
		} else {
			
			printf("  Fail!\n  Misses: %ld\n", dictionary->miss_count);
			
		}
		
		printf("\n");
	}
	
	
	
	destroy_hash_map(dictionary, beelzebub);
	
	clean_prime();
	
	return 0;
}




