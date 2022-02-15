#ifndef __PRIME_H
#define __PRIME_H

#include <stdlib.h>
#include <string.h>

static size_t PRIME_GEN_CAPACITY = 0;
static size_t PRIME_GEN_COUNT = 0;
unsigned long * PRIME_GEN_LIST;
unsigned long * PRIME_SQUARE_LIST;




// Call this function at begining of program.
void init_prime();



// Call this function at end of program.
void clean_prime();



// returns index of prime number that is greater or equal to seed
// If there are not enough prime numbers to determine if seed is
// prime or not, function returns the capacity of the prime gen list
unsigned long find_prime(unsigned long seed);



// returns 1 if seed is prime, 0 if seed is not prime
char is_prime(unsigned long seed);



// Function to calculate the next prime number to be added to the list
// Will also increase size of the list to accomidate new prime numbers
void grow_primes();



// Returns prime number that is greater or equal to seed
unsigned long next_prime(unsigned long seed);






void init_prime() {
	PRIME_GEN_CAPACITY = 3;
	PRIME_GEN_COUNT = 3;
	
	PRIME_GEN_LIST = (unsigned long *) malloc(sizeof(unsigned long) * 3);
	PRIME_GEN_LIST[0] = 2;
	PRIME_GEN_LIST[1] = 3;
	PRIME_GEN_LIST[2] = 5;
	
	PRIME_SQUARE_LIST = (unsigned long *) malloc(sizeof(unsigned long) * 3);
	PRIME_SQUARE_LIST[0] = 4;
	PRIME_SQUARE_LIST[1] = 9;
	PRIME_SQUARE_LIST[2] = 25;
}

void clean_prime() {
	PRIME_GEN_CAPACITY = 0;
	PRIME_GEN_COUNT = 0;
	free(PRIME_GEN_LIST);
	free(PRIME_SQUARE_LIST);
	PRIME_GEN_LIST = NULL;
}

unsigned long find_prime(unsigned long seed) {
	unsigned long i = 0;
	while(i < PRIME_GEN_COUNT && PRIME_GEN_LIST[i] < seed) i++;
	return i;
}

char is_prime(unsigned long seed) {
	
	if (seed > PRIME_SQUARE_LIST[PRIME_GEN_COUNT - 1]) {
		// Need more prime numbers!
		next_prime(seed / 2);
	}
	
	size_t i = 0;
	
	while ((i < PRIME_GEN_COUNT) && (PRIME_SQUARE_LIST[i] < seed)) {
		
		if ((seed % PRIME_GEN_LIST[i]) == 0) return 0;
		
		i++;
	}
	
	return 1;
}

void grow_primes() {
	
	// If lists need to be expanded
	if (PRIME_GEN_COUNT == PRIME_GEN_CAPACITY) {
		
		unsigned long * new_prime_list = (unsigned long *) malloc(sizeof(unsigned long) * PRIME_GEN_CAPACITY * 2);
		memcpy(new_prime_list, PRIME_GEN_LIST, sizeof(unsigned long) * PRIME_GEN_COUNT);
		
		unsigned long * new_square_list = (unsigned long *) malloc(sizeof(unsigned long) * PRIME_GEN_CAPACITY * 2);
		memcpy(new_square_list, PRIME_SQUARE_LIST, sizeof(unsigned long) * PRIME_GEN_COUNT);
		
		PRIME_GEN_CAPACITY *= 2;
		
		free(PRIME_GEN_LIST);
		free(PRIME_SQUARE_LIST);
		
		PRIME_GEN_LIST = new_prime_list;
		PRIME_SQUARE_LIST = new_square_list;
	}
	
	unsigned long new_prime = PRIME_GEN_LIST[PRIME_GEN_COUNT - 1] + 2;
	
	while(!is_prime(new_prime)) {
		new_prime += 2;
	}
	
	PRIME_GEN_LIST[PRIME_GEN_COUNT] = new_prime;
	PRIME_SQUARE_LIST[PRIME_GEN_COUNT] = new_prime * new_prime;
	
	PRIME_GEN_COUNT++;
}



unsigned long next_prime(unsigned long seed) {
	
	unsigned long i = find_prime(seed);
	
	while (i == PRIME_GEN_COUNT) {
		grow_primes();
		i = find_prime(seed);	
	}
	return PRIME_GEN_LIST[i];
	
}


#endif