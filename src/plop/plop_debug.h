

#ifndef plop_debug_h
#define plop_debug_h

#include "global.h"


#define ndebugs	10


extern int debugs [ndebugs];

extern char const *debug_strings [];

extern FILE *debug_file;

void err_msg_s  (char const *msg, char const *arg1);
void warn_msg_s (char const *msg, char const *arg1);
void err_msg_ss (char const *msg, char const *arg1, char const *arg2);
void err_msg_d  (char const *msg, int arg1);
void err_msg_dd (char const *msg, int arg1, int arg2);
void err_msg_ds (char const *msg, int arg1, char const *arg2);
void err_msg_g  (char const *msg, double arg1);
void err_msg_dg (char const *msg, int arg1, double arg2);
void err_msg_gd (char const *msg, double arg1, int arg2);

/* depending on gui or standalone will implement appropriately */
void err_msg (char const *s);
void warn_msg (char const *s);

#define debug_grid_gen		debugs [0]
#define debug_opt			debugs [1]
#define debug_basis_gen		debugs [2]
#define debug_triangle		debugs [3]
#define debug_interpolate	debugs [4]
#define debug_matrix		debugs [5]
#define debug_fem			debugs [6]
#define debug_plate_load   	debugs [7]
#define debug_displacement	debugs [8]


#endif
