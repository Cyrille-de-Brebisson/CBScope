#include <stdio.h>

#ifdef __BORLANDC__
#include <alloc.h>
#endif

#include <stdlib.h>

#define sparse_pool_max_chunks	10000
#define sparse_pool_chunk_size	16384
#include "global.h"

#ifdef gui_plop
#include "SparseStat.h"

extern void UpdateSparseStat (int);

extern void DebugMsg (char const *);
#endif

#include "plop_debug.h"

extern int please_stop_plop;

extern void terminate_plop (void);

void *malloc_or_die (int, char const *);

typedef struct sparse_mat_el_rec *sparse_ptr;

// #define obsolete

#ifdef obsolete
typedef struct sparse_mat_el_rec {
	int i, j;
	double val;
	sparse_ptr next_j;
	sparse_ptr prev_j;
	sparse_ptr i_down;	/* next row down with this j, or -1 */
	sparse_ptr i_up;	/* next row up with this j, or -1 */
	} sparse_mat_el;

#endif

#ifndef obsolete
typedef struct sparse_mat_el_rec {
	int j;
	sparse_ptr next_j;
	double val;
	} sparse_mat_el;
#endif


sparse_ptr free_mat_el_chunks [sparse_pool_max_chunks];


int next_avail_in_chunk;
int current_sparse_pool_chunk;

sparse_ptr free_mat_el_list;

int n_matrix_ops;

sparse_ptr *sparse_matrix_rows;
int sparse_matrix_size;


void clear_mat (void)
{	int i;

	for (i = 0; i < sparse_matrix_size; i++)
	{	sparse_matrix_rows [i] = (sparse_ptr) NULL;
	}
    current_sparse_pool_chunk = 0;
	next_avail_in_chunk = 0;
	free_mat_el_list = (sparse_ptr) NULL;
}

void alloc_sparse_pool (
	int n_mat)
{

	// in gui_plop will call repeatedly
	if (sparse_matrix_rows == NULL)
        sparse_matrix_rows = (sparse_ptr *) malloc_or_die (n_mat * sizeof (int), "sparse_matrix_rows");
}

/* allows us to do a single alloc and many solves of various sizes */

void set_mat (
	int n_mat)
{	sparse_matrix_size = n_mat;
}

sparse_ptr alloc_sparse_el (void)
{	sparse_ptr p;

	if (free_mat_el_list == (sparse_ptr) NULL)
	{	if (free_mat_el_chunks [current_sparse_pool_chunk] == (sparse_ptr) NULL)
    	{	free_mat_el_chunks [current_sparse_pool_chunk] = (sparse_ptr)
				malloc_or_die (sparse_pool_chunk_size * sizeof (sparse_mat_el), "sparse_matrix_space");
        }
		free_mat_el_list = free_mat_el_chunks [current_sparse_pool_chunk] + next_avail_in_chunk;
		free_mat_el_list->next_j = (sparse_ptr) NULL;
		next_avail_in_chunk++;
		if (next_avail_in_chunk == sparse_pool_chunk_size)
		{	next_avail_in_chunk = 0;
			current_sparse_pool_chunk++;
			if (current_sparse_pool_chunk == sparse_pool_max_chunks)
			{	fprintf (stderr, "ran out of space for sparse_pool_chunks\n");
				exit (1);
			}
		}
	}
	p = free_mat_el_list;
	free_mat_el_list = p->next_j;
	return (p);
}


void zap_row (
	int i)
{
	sparse_matrix_rows [i] = NULL;
}



void zap_col (
	int j)
{	int irow;
	sparse_ptr p;
	sparse_ptr pp;

	for (irow = 0; irow < sparse_matrix_size; irow++) {
		for (pp = NULL, p = sparse_matrix_rows [irow]; p != NULL && p->j < j; pp = p, p = p->next_j)
			;
		if (p != NULL && p->j == j) {
			if (pp != NULL) {
				pp->next_j = p->next_j;
			} else {
				sparse_matrix_rows [irow] = p->next_j;
			}
		}
	}
}

double find_el (
	int i,
   int j)
{	register sparse_ptr p;

	for (p = sparse_matrix_rows [i]; p != NULL && p->j != j; p = p->next_j)
		;
	if (p != NULL)
		return (p->val);
	else
		return (0.0);
}

void add_to_el (
	int i,
	int j,
	double value)
{	register sparse_ptr p;
	register sparse_ptr pp;
	register sparse_ptr pnew;

#ifdef debug
	printf ("%d %d %g\n", i, j, value);
#endif
	pp = NULL;
	for (p = sparse_matrix_rows [i]; p != NULL && p->j < j; p = p->next_j)
		pp = p;
	if (p != NULL && p->j == j)
		p->val += value;
	else {
		pnew = alloc_sparse_el ();
		pnew->j = j;
		pnew->val = value;
		pnew->next_j = p;
		if (pp == NULL) {
			sparse_matrix_rows [i] = pnew;
		} else {
			pp->next_j = pnew;
		}
	}
}



void print_vector (
	double *v,
	int n,
	char *str)
{	int i;

	printf (str);
	for (i = 0; i < n; i++)
		printf (" %e", v [i]);
	printf ("\n");
}

void print_sparse_mat (
	char const *str,
	int nmax)
{	int i, j;
	sparse_ptr p;

	fprintf (debug_file, "%s\n", str);
	for (i = 0; i < sparse_matrix_size && i < nmax; i++)
	{	for (j = 0, p = sparse_matrix_rows [i]; j < sparse_matrix_size && j < nmax; j++)
		{	if (p != NULL && p->j == j)
			{	fprintf (debug_file, "%d %d %lg\n", i, p->j, p->val);
				p = p->next_j;
			}
		}
	}
	fflush (debug_file);
}

void print_rhs (
	double **results,
	int nresults)
{	int i, j;

	fprintf (debug_file, "rhs\n");
	for (i = 0; i < sparse_matrix_size; i++)
	{	fprintf (debug_file, "%d:", i);
    	for (j = 0; j < nresults; j++)
		{	fprintf (debug_file, " %g", results [j] [i]);
		}
        fprintf (debug_file, "\n");
	}
	fflush (debug_file);
}

void print_fills (
	double pitch,
	FILE *f)
{	int i;
	int cnt;
	sparse_ptr p;

	if (f == (FILE *) NULL)
	{	fprintf (stderr, "bad file to fills\n");
		return;
	}
	cnt = 0;
	fprintf (f, ".SK 0\n");
	fprintf (f, ".PS\n");
	fprintf (f, "box ht %lg wid %lg at %lg,%lg\n",
		sparse_matrix_size * pitch,
		sparse_matrix_size * pitch,
		sparse_matrix_size * pitch * .5,
		sparse_matrix_size * pitch * .5);
	for (i = 0; i < sparse_matrix_size; i++)
	{	for (p = sparse_matrix_rows [i]; p != NULL; p = p->next_j)
		{	fprintf (f, "\"\\(bu\" at %e,%e\n", (p->j + .5) * pitch, (sparse_matrix_size - i - .5) * pitch);
			cnt++;
		}
	}
	fprintf (f, "\"%d entries\" at .5,-.5\n", cnt);
	fprintf (f, ".PE\n");
}

void print_arrays (
	double size,
	char *s)
{	FILE *matrix_fills_file;
	double pitch;

	pitch = size / sparse_matrix_size;
	if ((matrix_fills_file = fopen (s, "w")) == NULL)
	{	fprintf (stderr, "can't open %s\n", s);
		return;
	}
	print_fills (pitch, matrix_fills_file);
}

#ifdef debug_detailed
woof ()
{
	printf ("woof\n");
}


void print_row (
	int i)
{	sparse_ptr p; 
	int n;

	n = 0;
	for (p = sparse_matrix_rows [i]; p != NULL; p = p->next_j)
	{	printf (" [%d,%d,%12g]", p->i, p->j, p->val);
		n++;
		if (n % 5 == 0)
			printf ("\n");
	}
	if (n % 5 != 0)
		printf ("\n");
}

void print_col (
	sparse_ptr p)
{	int n;

	for (;p != NULL; p = p->i_down)
	{	printf (" [%d,%d,%12g]", p->i, p->j, p->val);
		n++;
		if (n % 5 == 0)
			printf ("\n");
	}
	if (n % 5 != 0)
		printf ("\n");
}
	
#endif



//#define solve_by_col
#ifdef solve_by_col

void solve_sparse_sys (
	double **result,
	int n_results)
{	register sparse_ptr p, pp, pnew;
	sparse_ptr pother;
	register int i, j;
	int iother;
	register int k;
	register sparse_ptr piv_p;
	double pivotval;
#ifdef gui_plop
	int rows_per_step;
    char str [100];
#endif

	if (debug_matrix)
	{	print_sparse_mat ("matrix", sparse_matrix_size);
    	print_rhs (result, n_results);
    }
#ifdef gui_plop
    sprintf (str, "matrix size: %d\n", sparse_matrix_size);
	DebugMsg (str);
#endif

#ifdef gui_plop
	rows_per_step = sparse_matrix_size / SparseStateSteps;
#endif

	for (i = 0; i < sparse_matrix_size && !please_stop_plop; i++)
	{	p = sparse_matrix_rows [i];
#ifdef debug_detailed
		print_sparse_mat ("first 6", 6);
		check_matrix (i);
#endif
#ifdef gui_plop
		if (i % rows_per_step == 0)
			UpdateSparseStat (i / rows_per_step);
#endif
		pivotval = 1.0 / p->val;
		p->val = 1.0;
		for (k = 0; k < n_results; k++)
			result [k] [i] *= pivotval;
		for (p = p->next_j; p != NULL; p = p->next_j)
		{	p->val *= pivotval;
			n_matrix_ops++;
		}
		for (j = i + 1; j < sparse_matrix_size; j++) {
			if (sparse_matrix_rows [j]->j == i) {
				p = sparse_matrix_rows [j];
				pivotval = p->val;
				pp = NULL;
				sparse_matrix_rows [j] = p->next_j;
				p->next_j = free_mat_el_list;
				free_mat_el_list = p;
				p = sparse_matrix_rows [j];
				for (pother = sparse_matrix_rows [i]->next_j; pother != NULL; pother = pother->next_j) {
					while (p != NULL && p->j < pother->j) {
						pp = p;
						p = p->next_j;
					}
					if (p == NULL || p->j > pother->j) {
						pnew = alloc_sparse_el ();
						pnew->val = 0;
						pnew->j = pother->j;
						pnew->next_j = p;
						if (pp == NULL) {
							sparse_matrix_rows [i] = pnew;
						} else {
							pp->next_j = pnew;
						}
						p = pnew;
					}
					p->val -= pother->val * pivotval;
					n_matrix_ops++;
				}
				for (k = 0; k < n_results; k++) {
					result [k] [j] -= pivotval * result [k] [i];
				}
			}
		}
	}
	if (please_stop_plop)
		terminate_plop ();
#ifdef gui_plop
	UpdateSparseStat (SparseStateSteps);
	sprintf (str, "matrix ops: %d\n", n_matrix_ops);
	DebugMsg (str);

#endif

	/* back substitution */
	for (i = sparse_matrix_size - 1; (i >= 0) & !please_stop_plop; i--)
	{	for (p = sparse_matrix_rows [i]->next_j; p != NULL; p = p->next_j)
		{	for (k = 0; k < n_results; k++)
			{	result [k] [i] -= p->val * result [k] [p->j];
				n_matrix_ops++;
			}
		}
	}
	if (debug_matrix)
    {	print_rhs (result, n_results);
    }
	if (please_stop_plop)
		terminate_plop ();
}

#else

void solve_sparse_sys (
	double **result,
	int n_results)
{	register sparse_ptr p, pp, pnew;
	sparse_ptr pother;
	register int i, j;
	int iother;
	register int k;
	register sparse_ptr piv_p;
	double pivotval;
#ifdef gui_plop
	int rows_per_step;
    char str [100];
#endif

	if (debug_matrix)
	{	print_sparse_mat ("matrix", sparse_matrix_size);
    	print_rhs (result, n_results);
    }
#ifdef gui_plop
    sprintf (str, "matrix size: %d\n", sparse_matrix_size);
	DebugMsg (str);
#endif

#ifdef gui_plop
	rows_per_step = sparse_matrix_size / SparseStateSteps;
#endif

	for (i = 0; i < sparse_matrix_size && !please_stop_plop; i++)
	{	p = sparse_matrix_rows [i];
#ifdef debug_detailed
		print_sparse_mat ("first 6", 6);
		check_matrix (i);
#endif
#ifdef gui_plop
		if (i % rows_per_step == 0)
			UpdateSparseStat (i / rows_per_step);
#endif
		while (sparse_matrix_rows [i]->j < i) {
			p = sparse_matrix_rows [i];
			iother = p->j;
			pivotval = p->val;
			pp = NULL;
			sparse_matrix_rows [i] = p->next_j;
			p->next_j = free_mat_el_list;
			free_mat_el_list = p;
			p = sparse_matrix_rows [i];
			for (pother = sparse_matrix_rows [iother]->next_j; pother != NULL; pother = pother->next_j) {
				while (p != NULL && p->j < pother->j) {
					pp = p;
					p = p->next_j;
				}
				if (p == NULL || p->j > pother->j) {
					pnew = alloc_sparse_el ();
					pnew->val = 0;
					pnew->j = pother->j;
					pnew->next_j = p;
					if (pp == NULL) {
						sparse_matrix_rows [i] = pnew;
					} else {
						pp->next_j = pnew;
					}
					p = pnew;
				}
				p->val -= pother->val * pivotval;
				n_matrix_ops++;
			}
			for (k = 0; k < n_results; k++) {
				result [k] [i] -= pivotval * result [k] [iother];
			}
//			sparse_matrix_rows [i] = sparse_matrix_rows [i]->next_j;
		}
		p = sparse_matrix_rows [i];
		pivotval = 1.0 / p->val;
		for (k = 0; k < n_results; k++)
			result [k] [i] *= pivotval;
		for (; p != NULL; p = p->next_j)
		{	p->val *= pivotval;
			n_matrix_ops++;
		}
	}
	if (please_stop_plop)
		terminate_plop ();
#ifdef gui_plop
    UpdateSparseStat (SparseStateSteps);
	sprintf (str, "matrix ops: %d\n", n_matrix_ops);
	DebugMsg (str);

#endif

	/* back substitution */
	for (i = sparse_matrix_size - 1; (i >= 0) & !please_stop_plop; i--)
	{	for (p = sparse_matrix_rows [i]->next_j; p != NULL; p = p->next_j)
		{	for (k = 0; k < n_results; k++)
			{	result [k] [i] -= p->val * result [k] [p->j];
				n_matrix_ops++;
			}
		}
	}
	if (debug_matrix)
    {	print_rhs (result, n_results);
    }
	if (please_stop_plop)
		terminate_plop ();
}

#endif


