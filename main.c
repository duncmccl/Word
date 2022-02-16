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



char is_ascii(unsigned char c) {
	return (c >= 'A' && c < 'Z') || (c >= 'a' && c <= 'z');
}

// Reads in a word from file
// returns number of characters written to buffer, including null terminator.
size_t next_word(FILE * file_f, char * buffer, size_t limit) {
	
	unsigned char c = 0;
	
	// Ignore all garbage before word
	
	do {
		c = fgetc(file_f);
	} while (!is_ascii(c) && !feof(file_f));
	
	
	size_t count = 0;
	while (is_ascii(c) && !feof(file_f)) {
		buffer[count] = c;
		c = fgetc(file_f);
		count++;
	}
	
	
	// Write null terminator
	buffer[count] = '\0';
	count++;
	
	return count;
	
}






typedef struct {
	enum OBJTYP objtyp;
	size_t index, row, col;
} pos_t;


int main() {
	
	// Initilize global prime number generator
	init_prime();
	
	
	/*
	DIR * d;
	struct dirent * dir;
	d = opendir("./toread");
	if (d) {
		
		while ((dir = readdir(d)) != NULL) {
			
			if (!strcmp(dir->d_name, ".")) break;
			
			printf("%s\n", dir->d_name);
			
			
			// Open and read file into dictionary
			
			
			
		}
		closedir(d);
		
	} else {
		fprintf(stderr, "Failed to open toread dir!\n");
	}
	
	*/
	
	
	// Create Hashmap for dictionary
	hash_map_t * dictionary = create_hash_map(10);
	
	
	
	// Read file and populate hashmap
	FILE * text_f = fopen("text.txt", "r");
	
	if (!text_f) {
		fprintf(stderr, "Failed to open file!\n");
		return 1;
	}
	
	size_t buff_size = 2048;
	char buff[buff_size];
	size_t word_length = 0;
	size_t word_index = 0;
	
	word_length = next_word(text_f, buff, buff_size);
	
	
	
	while(!feof(text_f) && word_length > 1) {
		
		if (word_length > 1) {
			
			unsigned long word_hash = simple_hash(word_length, buff);

			size_t pos_len = sizeof(pos_t);
			pos_t pos = (pos_t) {INDEX, word_index};
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
				if (reject) destroy_hash_node(reject, u_destroy);

			}

			hash_map_t * word_list = (hash_map_t *) dictionary_entry->thing;

			hash_node_t * reject = insert_hash_map(word_list, pos_node);
			if (reject) destroy_hash_node(reject, u_destroy);

			word_length = next_word(text_f, buff, buff_size);
			word_index++;
		}
	}
	
	fclose(text_f);
	
	
	
	
	
	
	printf("Dictionary Stats:\n");
	printf("  Count: %ld\n", dictionary->count);
	printf("  Capacity: %ld\n", dictionary->capacity);
	printf("  Filled: %g\n", (double)dictionary->count / (double)dictionary->capacity);
	printf("  Collision Count: %ld\n", dictionary->collision_count);
	printf("  Collision Rate: %g\n", (double)dictionary->collision_count / (double)dictionary->count);
	printf("\n");
	
	
	unsigned long hash_queary;
	hash_node_t * queary;
	
	unsigned long miss_count = 0;
	
	FILE * queary_f = fopen("queary.txt", "r");
	
	if (!queary_f) {
		fprintf(stderr, "Failed to open queary.txt!\n");
		return 1;
	}
	
	
	word_length = next_word(queary_f, buff, buff_size);
	word_index = 0;
	
	while(!feof(queary_f) && word_length > 1) {
		
		if (word_length > 1) {
		
			hash_queary = simple_hash(word_length, buff);
			
			clock_t start = clock();
			
			queary = search_hash_map(dictionary, hash_queary);
			
			clock_t end = clock();
			
			printf("Queary: %s\n", buff);
			printf("  Hash: %ld\n", hash_queary);
			printf("  Miss: %ld\n", dictionary->miss_count);
			printf("  Clocks: %ld\n", end - start);


			miss_count += dictionary->miss_count;

			if (queary) {

				hash_map_t * word_list = queary->thing;

				printf("  Success!\n  Count: %ld\n", word_list->count);

				printf("  Indices\n");

				for(size_t j = 0; j < word_list->capacity; j++) {
					if (word_list->map[j]) {

						hash_node_t * pos_entry = word_list->map[j];
						pos_t * pos = (pos_t *) pos_entry->thing;

						printf("    %10ld\n", pos->index);
					}
				}

			} else {

				printf("  Fail!\n");

			}

			printf("\n");

			word_length = next_word(queary_f, buff, buff_size);
			word_index++;
			
		}
	}
	
	
	fclose(queary_f);
	
	
	printf("Average Miss rate: %g\n", (double)miss_count / (double)word_index);
	printf("Log2 of Dictionary Count: %g\n", log2(dictionary->count));
	
	
	/*
	printf("%5ld | ", 0l);
	for(size_t i = 0; i < dictionary->capacity; i++) {
		if (dictionary->map[i] == NULL) {
			printf("_");
		} else {
			printf("X");
		}
		if ((i + 1) % 64 == 0) printf("\n%5ld | ", i+1);
	}
	
	printf("\n");
	*/
	
	
	u_destroy(dictionary);
	
	
	
	clean_prime();
	
	return 0;
}




