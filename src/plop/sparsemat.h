#ifndef sparsematH
#define sparsematH

void alloc_sparse_pool (int);
void set_mat (int);
void clear_mat (void);
void add_to_el (int, int, double);
void zap_row (int);
void zap_col (int);
void solve_sparse_sys (double **, int);

#endif


 