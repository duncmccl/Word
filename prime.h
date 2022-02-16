#ifndef __PRIME_H
#define __PRIME_H

#include <stdlib.h>
#include <string.h>

static size_t PRIME_GEN_CAPACITY = 0;
static size_t PRIME_GEN_COUNT = 0;
unsigned long * PRIME_GEN_LIST;


// Code shamelessly taken from rosettacode.org
unsigned long isqrt(unsigned long x) {
    unsigned long q = 1, r = 0;
    while (q <= x) {
        q <<= 2;
    }
    while (q > 1) {
        unsigned long t;
        q >>= 2;
        t = x - r - q;
        r >>= 1;
        if (t >= 0) {
            x = t;
            r += q;
        }
    }
    return r;
}



// Call this function at begining of program.
void init_prime();



// Call this function at end of program.
void clean_prime();


// returns 1 if seed is prime, 0 if seed is not prime
char is_prime(unsigned long seed);

// Returns prime number that is greater or equal to seed
unsigned long next_prime(unsigned long seed);




// Helper functions
size_t find_prime(unsigned long seed, size_t start, size_t end);
void grow_primes();





void init_prime() {
	PRIME_GEN_CAPACITY = 3;
	PRIME_GEN_COUNT = 3;
	
	PRIME_GEN_LIST = (unsigned long *) malloc(sizeof(unsigned long) * 3);
	PRIME_GEN_LIST[0] = 2;
	PRIME_GEN_LIST[1] = 3;
	PRIME_GEN_LIST[2] = 5;
}

void clean_prime() {
	PRIME_GEN_CAPACITY = 0;
	PRIME_GEN_COUNT = 0;
	free(PRIME_GEN_LIST);
	PRIME_GEN_LIST = NULL;
}


size_t find_prime(unsigned long seed, size_t start, size_t end) {
	size_t mid = start + ((end - start) >> 1);
	
	if (start == mid) {
		if (PRIME_GEN_LIST[start] < seed) {
			return start + 1;
		} else {
			return start;
		}
	}
	
	if (PRIME_GEN_LIST[mid] < seed) {
		return find_prime(seed, mid, end);
	} else {
		return find_prime(seed, start, mid);
	}
}

char is_prime(unsigned long seed) {
	
	unsigned long sqrtseed = isqrt(seed);
	
	if (sqrtseed > PRIME_GEN_LIST[PRIME_GEN_COUNT - 1]) {
		// Need more prime numbers!
		next_prime(sqrtseed);
	}
	
	size_t i = 0;
	
	while (PRIME_GEN_LIST[i] < sqrtseed) {
		
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
		
		PRIME_GEN_CAPACITY *= 2;
		
		free(PRIME_GEN_LIST);
		
		PRIME_GEN_LIST = new_prime_list;
	}
	
	unsigned long new_prime = PRIME_GEN_LIST[PRIME_GEN_COUNT - 1] + 2;
	
	while(!is_prime(new_prime)) {
		new_prime += 2;
	}
	
	PRIME_GEN_LIST[PRIME_GEN_COUNT] = new_prime;
	PRIME_GEN_COUNT++;
}



unsigned long next_prime(unsigned long seed) {
	
	unsigned long i = find_prime(seed, 0, PRIME_GEN_COUNT - 1);
	
	while (i >= PRIME_GEN_COUNT - 1 && PRIME_GEN_LIST[PRIME_GEN_COUNT - 1] < seed) {
		grow_primes();
		i = find_prime(seed, 0, PRIME_GEN_COUNT - 1);	
	}
	return PRIME_GEN_LIST[i];
	
}


#endif