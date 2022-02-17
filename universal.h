#ifndef __UNIVERSAL_H
#define __UNIVERSAL_H

// Universal identifier
enum OBJTYP {INVALID, INT, UINT, LONG, ULONG, FLOAT, DOUBLE, HASHMAP, ARRAYLIST, STRING};

typedef struct {
	enum OBJTYP objtyp;
	
	union {
		
		int i;
		unsigned int ui;
		
		long l;
		unsigned long ul;
		
		float f;
		double d;
		
	} data;
	
} u_simple;

u_simple * create_simple(enum OBJTYP objtyp, void * data) {
	
	u_simple * rtn = malloc(sizeof(u_simple));
	rtn->objtyp = objtyp;
	
	switch (objtyp) {
		case INT:
			rtn->data.i = *((int *)data);
			break;
				
		case UINT:
			rtn->data.ui = *((unsigned int *)data);
			break;
			
		case LONG:
			rtn->data.l = *((long *)data);
			break;
			
		case ULONG:
			rtn->data.ul = *((unsigned long *)data);
			break;
			
		case FLOAT:
			rtn->data.f = *((float *)data);
			break;
			
		case DOUBLE:
			rtn->data.d = *((double *)data);
			break;
			
		default:
			break;
	}
	
	return rtn;
}

// Universal destroyer
void u_destroy(void *);






#endif