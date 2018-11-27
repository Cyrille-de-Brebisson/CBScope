#ifndef parm_typesH
#define parm_typesH
#include "global.h"
typedef struct {
	char const *name;
	int max_num;		/* if != 1, then vector */
	int *num_found;
	int type_flag;
	int *iptr;
	double *dptr;
	int affects_basis;		/* 1 if affects mass or stiffness of a mesh */
    int *bound_to_var;		/* >= 0 if bound to some var; lets alloc this to save some PITA */
	} parm_descr;


    
#define opnd_const		0
#define opnd_var		1

typedef struct {
	int opnd_type;
    double opnd_value;
    int opnd_which_var;
    } opnd;

#define var_unbound		0
#define var_number		1
#define var_sum			2
#define var_diff		3
#define var_mul			4
#define var_div			5

typedef struct {
	char name [wordlen];
    double value;
    int affects_basis;
    int var_def;
    opnd op1;
    opnd op2;
    } var_def;

#endif


