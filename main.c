#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#include "prime.h"
#include "hashmap.h"



/*

IDF(W) = Log((M+1) / k)

Log (A / B) = Log(A) - Log(B)

IDF(W) = Log(M+1) - Log(k)

M = total number of documents
K = number of documents containing word

*/

unsigned long simple_hash(const size_t len, const void * str) {
	
	char * cast_str = (char *) str;
	
	unsigned long rtn = 0;
	unsigned long p = 53;
	unsigned long pp = 1;
	unsigned long m = 1e9 + 9;
	
	for(size_t i = 0; i < len; i++) {
		rtn = (rtn + ((unsigned long)cast_str[i] * pp)) % m;
		pp = (pp * p) % m;
	}
	
	return rtn;
}




unsigned char is_letter(unsigned char c) {
	return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'));
}

// Reads in a word from file
// returns number of characters written to buffer, including null terminator.
size_t next_word(FILE * file_f, char * buffer, size_t limit) {
	
	unsigned char c = 0;
	
	// Ignore all garbage before word
	
	do {
		c = fgetc(file_f);
		if (c == 0xFF) return 0;
	} while (!is_letter(c) && c != 0xFF);
	
	
	size_t count = 0;
	while (is_letter(c) && !feof(file_f)) {
		buffer[count] = tolower(c);
		c = fgetc(file_f);
		count++;
	}
	
	
	// Write null terminator
	buffer[count] = '\0';
	count++;
	
	return count;
	
}



void tabulate(const char * file_name, hash_map_t * dictionary, hash_map_t * resolver) {
	
	size_t file_length = strlen(file_name) + 1;
	unsigned long file_hash = simple_hash(file_length, file_name);
	
	// Hash resolver for file name
	hash_node_t * file_hash_resolver_entry = search_hash_map(resolver, file_hash);
	if (!file_hash_resolver_entry) {
		string_t dummy;
		dummy.objtyp = STRING;
		dummy.length = file_length;
		dummy.bytes = (char *) malloc(file_length);
		memcpy(dummy.bytes, file_name, file_length);
		
		file_hash_resolver_entry = create_hash_node(file_hash, sizeof(string_t), &dummy);
		
		hash_node_t * reject = insert_hash_map(resolver, file_hash_resolver_entry);
		if (reject) destroy_hash_node(reject, u_destroy);
	}
	
	
	char dir[1024] = "./toread/";
	char * full_dir = strcat(dir, file_name);
	
	FILE * text_f = fopen(full_dir, "r");
	
	if (!text_f) {
		fprintf(stderr, "Failed to open file: %s\n", full_dir);
		return;
	}
	
	size_t buff_size = 2048;
	char buff[buff_size];
	size_t word_length = 0;
	
	
	do {
		
		word_length = next_word(text_f, buff, buff_size);
		
		if (word_length > 1) {
			
			unsigned long word_hash = simple_hash(word_length, buff);
			
			
			// Hash Resolver for word
			hash_node_t * word_hash_resolver_entry = search_hash_map(resolver, word_hash);
			if (!word_hash_resolver_entry) {
				
				string_t dummy;
				dummy.objtyp = STRING;
				dummy.length = word_length;
				dummy.bytes = (char *) malloc(word_length);
				memcpy(dummy.bytes, buff, word_length);
				
				word_hash_resolver_entry = create_hash_node(word_hash, sizeof(string_t), &dummy);
				
				hash_node_t * reject = insert_hash_map(resolver, word_hash_resolver_entry);
				if (reject) destroy_hash_node(reject, u_destroy);
				
			}
			
			
			
			// Does word exist in dictionary?
			
			hash_node_t * word_query = search_hash_map(dictionary, word_hash);
			
			if (word_query) {
				
				// Word does exist
				
				hash_map_t * dictionary_entry = (hash_map_t *) word_query->thing;
				
				
				
				hash_node_t * file_query = search_hash_map(dictionary_entry, file_hash);
				
				// Does file exist for word? 
				
				if (file_query) {
					
					// Word has appeared in file before
					
					counter_t * counter = (counter_t *) file_query->thing;
					
					counter->count++;
					
				} else {
					
					// Word has not appeared in file before
					
					// Create a file entry
					counter_t dummy;
					dummy.objtyp = COUNTER;
					dummy.count = 1;
					
					file_query = create_hash_node(file_hash, sizeof(counter_t), &dummy);
					
					hash_node_t * reject = insert_hash_map(dictionary_entry, file_query);
					if (reject) destroy_hash_node(reject, u_destroy);
				}
				
				
			} else {
				
				// Word does not exist
				
				// Must make a new word entry
				
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
				
				word_query = create_hash_node(word_hash, sizeof(hash_map_t), &dummy);
				
				hash_node_t * reject = insert_hash_map(dictionary, word_query);
				if (reject) destroy_hash_node(reject, u_destroy);
				
				
				
				hash_map_t * dictionary_entry = (hash_map_t *) word_query->thing;
				
				
				// Must make a new file entry
				
				counter_t dummy_2;
				dummy_2.objtyp = COUNTER;
				dummy_2.count = 1;
				
				hash_node_t * file_query = create_hash_node(file_hash, sizeof(counter_t), &dummy_2);
				
				reject = insert_hash_map(dictionary_entry, file_query);
				if (reject) destroy_hash_node(reject, u_destroy);
				
			}
			
		}
		
	} while (word_length > 0);
	
	fclose(text_f);
	
}







int main() {
	
	// Initilize global prime number generator
	init_prime();
	
	// Create Hashmap for dictionary
	hash_map_t * dictionary = create_hash_map(10);
	
	// Create Hashmap for hash resolver
	hash_map_t * resolver = create_hash_map(10);
	
	DIR * d;
	struct dirent * dir;
	d = opendir("./toread");
	if (d) {
		
		while ((dir = readdir(d)) != NULL) {
			
			if (!strcmp(dir->d_name, ".")) break;
			
			tabulate(dir->d_name, dictionary, resolver);
			
		}
		closedir(d);
		
	} else {
		fprintf(stderr, "Failed to open toread dir!\n");
	}
	
	
	
	// Print entire dictionary!
	// For every word
	for(size_t i = 0; i < dictionary->capacity; i++) {
		if (dictionary->map[i]) {
			
			unsigned long word_hash = dictionary->map[i]->thing_hash;
			
			hash_node_t * word_result = search_hash_map(resolver, word_hash);
			
			if (word_result) {
				
				string_t * word_str = (string_t *) word_result->thing;
				
				printf("WORD: %s\n", word_str->bytes);
				
			} else {
				
				printf("WORD: ERR\n");
				
			}
			printf("HASH: %ld\n|\n", word_hash);
			
			hash_map_t * dictionary_entry = (hash_map_t *) dictionary->map[i]->thing;
			
			for(size_t j = 0; j < dictionary_entry->capacity; j++) {
				if (dictionary_entry->map[j]) {
					
					unsigned long file_hash = dictionary_entry->map[j]->thing_hash;
					
					hash_node_t * file_result = search_hash_map(resolver, file_hash);
					
					if (file_result) {
						
						string_t * file_str = (string_t *) file_result->thing;
						
						printf("| FILE: %s\n", file_str->bytes);
						
					} else {
						
						printf("| FILE: ERR\n");
						
					}
					printf("| HASH: %ld\n", file_hash);
					
					
					counter_t * count = (counter_t *) dictionary_entry->map[j]->thing;
					
					printf("| COUNT: %ld\n", count->count);
					
					
					printf("|\n");
				}
			}
			
			printf("\n");
		}
	}
	
	printf("Dictionary stats:\n");
	printf("  Count: %ld\n", dictionary->count);
	printf("  Capacity: %ld\n", dictionary->capacity);
	printf("  Fill Percent: %g\n", (float)dictionary->count / (float)dictionary->capacity);
	printf("  Collisions: %ld\n", dictionary->collision_count);
	printf("  Collision Percent: %g\n", (float)dictionary->collision_count / (float)dictionary->count);
	printf("\n");
	printf("Resolver stats:\n");
	printf("  Count: %ld\n", resolver->count);
	printf("  Capacity: %ld\n", resolver->capacity);
	printf("  Fill Percent: %g\n", (float)resolver->count / (float)resolver->capacity);
	printf("  Collisions: %ld\n", resolver->collision_count);
	printf("  Collision Percent: %g\n", (float)resolver->collision_count / (float)resolver->count);
	printf("\n");
	
	u_destroy(dictionary);
	u_destroy(resolver);
	
	
	clean_prime();
	
	return 0;
}




