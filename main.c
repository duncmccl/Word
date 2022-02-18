#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

// User defined enum list to allow for custom types
enum OBJTYP {INVALID, INT, UINT, LONG, ULONG, FLOAT, DOUBLE, HASHMAP, ARRAYLIST, STRING};

#include "universal.h"

// Define all destructors to be used in universal destructor
void destroy_hash_map(void * obj, void (*destructor)(void *));
void destroy_arraylist(void * obj, void (*destructor)(void *));
void destroy_string(void *);

void u_destroy(void * obj) {
	
	if (obj == NULL) return;
	
	switch( * ((enum OBJTYP *) obj) ) {
			
		case INT:
		case UINT:
		case LONG:
		case ULONG:
		case FLOAT:
		case DOUBLE:
			
			destroy_simple(obj);
			break;
			
		case HASHMAP:
			
			destroy_hash_map(obj, u_destroy);
			break;
			
		case ARRAYLIST:
			
			destroy_arraylist(obj, u_destroy);
			break;
			
		case STRING:
			
			destroy_string(obj);
			break;
			
		case INVALID:
		default:
			fprintf(stderr, "UNIVERSAL DESTROY ERR: Invalid Object Type!\n");
			break;
	}
	
}

// Include extra headers that implament universitality

#include "hashmap.h"
#include "arraylist.h"


// Universal String

typedef struct {
	
	enum OBJTYP objtyp;
	
	size_t length;
	char * bytes;
	
} string_t;

string_t * create_string(size_t length, const char * bytes) {
	string_t * rtn = (string_t *) malloc(sizeof(string_t));
	rtn->objtyp = STRING;
	rtn->length = length;
	rtn->bytes = (char *) malloc(length);
	memcpy(rtn->bytes, bytes, length);
	
	return rtn;
}

void destroy_string(void * str) {
	
	string_t * c_str = (string_t *) str;
	
	free(c_str->bytes);
	free(c_str);
}



/*

IDF(W) = Log((M+1) / k)

Log (A / B) = Log(A) - Log(B)

IDF(W) = Log(M+1) - Log(k)

M = total number of documents
K = number of documents containing word


When making a query:
Also query similar words and scale their relavancy factir by the similaity
This will help catch plurals, or adverbs. eg: station vs stations, swift vs switftly




Potential String similarity function:

float similarity(const char * str_A, const char * str_B) {
	
	similarity is proportional to number of consecutive identical characters
	similarity is inversly proportional to diffrence in lengths
	
	similarity = ( number of consecutive identical characters ) / max( len_A, len_B )
	
	eg:
	
	len_A = 5
	str_A = "penny"
	
	len_B = 7
	str_B = "pennies"
	
	similarity = 4 / 7 = 0.57
	
	
	
	len_A = 5
	str_A = "benny"
	
	len_B = 5
	str_B = "denny"
	
	similarity = 4 / 5 = 0.80
}




*/

// Helper functions

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



void tabulate(const char * file_name, hash_map_t * dictionary, hash_map_t * stats, hash_map_t * resolver) {
	
	size_t filename_length = strlen(file_name) + 1;
	unsigned long filename_hash = simple_hash(filename_length, file_name);
	
	// Hash resolver for file name
	hash_node_t * file_hash_resolver_entry = search_hash_map(resolver, filename_hash);
	if (!file_hash_resolver_entry) {
		
		file_hash_resolver_entry = create_hash_node(filename_hash, create_string(filename_length, file_name));
		
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
	size_t word_count = 0;
	hash_node_t * reject;
	
	do {
		
		word_length = next_word(text_f, buff, buff_size);
		
		if (word_length > 1) {
			
			unsigned long word_hash = simple_hash(word_length, buff);
			
			
			
			// Does word exist in dictionary?
			
			hash_node_t * word_query = search_hash_map(dictionary, word_hash);
			
			if (word_query) {
				
				// Word does exist
				
				hash_map_t * dictionary_entry = (hash_map_t *) word_query->thing;
				
				
				
				// Does File exist for word? 
				
				hash_node_t * file_query = search_hash_map(dictionary_entry, filename_hash);
				
				if (file_query) {
					
					// File exists for word already
					
					arraylist_t * index_list = (arraylist_t *) file_query->thing;
					
					push_back_arraylist(index_list, create_simple(ULONG, &word_count));
					
				} else {
					
					// File does not exist for word
					
					// Create a new arraylist for indices
					arraylist_t * index_list = create_arraylist(sizeof(u_simple), 16);
					
					file_query = create_hash_node(filename_hash, index_list);
					
					reject = insert_hash_map(dictionary_entry, file_query);
					if (reject) u_destroy(reject);
					
					// Insert index into new arraylist
					
					push_back_arraylist(index_list, create_simple(ULONG, &word_count));
					
				}
				
				
			} else {
				
				// Word does not exist
				
				// New Hash Resolver for word
				hash_node_t * word_hash_resolver_entry = create_hash_node(word_hash, create_string(word_length, buff));
				
				reject = insert_hash_map(resolver, word_hash_resolver_entry);
				if (reject) u_destroy(reject);
				
				
				
				// Must make a new dictionary entry for word
				
				hash_map_t * dictionary_entry = create_hash_map(10);
				
				
				word_query = create_hash_node(word_hash, dictionary_entry);
				
				reject = insert_hash_map(dictionary, word_query);
				if (reject) u_destroy(reject);
				
				
				
				// Must make a new arraylist of indices
				
				arraylist_t * index_list = create_arraylist(sizeof(u_simple), 16);
				
				hash_node_t * file_query = create_hash_node(filename_hash, index_list);
				
				reject = insert_hash_map(dictionary_entry, file_query);
				if (reject) u_destroy(reject);
				
				
				
				// insert index into arraylist
				
				push_back_arraylist(index_list, create_simple(ULONG, &word_count));
			}
			
			word_count++;
			
		}
		
	} while (word_length > 0);
	
	fclose(text_f);
	
}







int main() {
	
	// Initilize global prime number generator
	init_prime();
	
	// Create Hashmap for dictionary
	// HashMap[ Word Hash : HashMap[ Filename Hash : ArrayList[ Word Index ] ] ]
	hash_map_t * dictionary = create_hash_map(10);
	
	// Create Hashmap for file statistics
	// HashMap[ Filename Hash : File Statistics ]
	hash_map_t * stats = create_hash_map(10);
	
	// Create Hashmap for hash resolver
	// HashMap[ Word or Filename Hash : String ]
	hash_map_t * resolver = create_hash_map(10);
	
	
	
	// Tabulate all words in all files in dir 'toread'
	DIR * d;
	struct dirent * dir;
	d = opendir("./toread");
	if (d) {
		
		while ((dir = readdir(d)) != NULL) {
			
			if (!strcmp(dir->d_name, ".")) break;
			
			tabulate(dir->d_name, dictionary, stats, resolver);
			
		}
		closedir(d);
		
	} else {
		fprintf(stderr, "Failed to open toread dir!\n");
	}
	
	
	
	
	
	// Print entire dictionary!
	
	// For every word
	for(size_t i = 0; i < dictionary->capacity; i++) {
		// If there node present
		if (dictionary->map[i]) {
			
			// Get hash, resolve it into Word and print
			unsigned long word_hash = dictionary->map[i]->thing_hash;
			hash_node_t * word_result = search_hash_map(resolver, word_hash);
			if (word_result) {
				
				string_t * word_str = (string_t *) word_result->thing;
				
				printf("WORD: %s\n", word_str->bytes);
				
			} else {
				
				printf("WORD: ERR\n");
				
			}
			printf("HASH: %ld\n|\n", word_hash);
			
			
			
			// Hashmap of every file the word occurs in
			hash_map_t * dictionary_entry = (hash_map_t *) dictionary->map[i]->thing;
			
			// For every file
			for(size_t j = 0; j < dictionary_entry->capacity; j++) {
				// If there is a node present
				if (dictionary_entry->map[j]) {
					
					// Get hash, resolve it into file name and print
					unsigned long file_hash = dictionary_entry->map[j]->thing_hash;
					hash_node_t * file_result = search_hash_map(resolver, file_hash);
					if (file_result) {
						
						string_t * file_str = (string_t *) file_result->thing;
						
						printf("| FILE: %s\n", file_str->bytes);
						
					} else {
						
						printf("| FILE: ERR\n");
						
					}
					printf("| HASH: %ld\n", file_hash);
					
					
					// Arraylist of indices of all instances of the word
					arraylist_t * indices = (arraylist_t *) dictionary_entry->map[j]->thing;
					
					printf("| COUNT: %ld\n", indices->count);
					
					
					for(size_t k = 0; k < indices->count; k++) {
						
						u_simple * dummy = indices->data[k];
						
						printf("|  %5ld : %10ld\n", k, dummy->data.ul);
						
					}
					
					
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
	u_destroy(stats);
	u_destroy(resolver);
	
	
	clean_prime();
	
	return 0;
}




