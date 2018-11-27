#include <stdio.h>
#ifdef __BORLANDC__
#include <alloc.h>
#endif
#include <stdlib.h>
#include <math.h>

#include "global.h"

#include "plate_global.h"
#include "plop_debug.h"

#ifndef __BORLANDC__
#define abs(x)	((x) > 0 ? (x) : -(x))
#endif

#define pow10_n_deg_free_m1		100		/* 10 ^ (deg_free - 1) */

#include "plate_global.cpp"

#ifdef gui_plop

extern void LockPlopData (int);
extern void UnlockPlopData (void);

#endif

#ifdef gui_plop
extern void DebugMsg (char const *);
#endif


#ifdef use_sparse
#include "sparsemat.h"
#else
double sk [max_eqn] [band_size];
#endif

#ifdef log_malloc
FILE *malloc_log_file = NULL;
#endif
void *malloc_or_die (
	int n,
	char const *why)
{	char *p;

#ifdef gui_plop
	char str [100];
#endif


	p = (char *) malloc (n);
	total_mem_alloced += n;
	if (p == (char *) NULL)
	{	fprintf (stderr, "malloc for %s failed; time to die\n", why);
		exit (1);
	}
#ifdef log_malloc
	if (malloc_log_file == NULL) {
		malloc_log_file = fopen ("malloc_log.txt", "wb");
	}
	fprintf (malloc_log_file, "malloc %s %d\n", why, n);
#endif

#ifdef gui_plop
	sprintf (str, "malloc %s %d\n", why, n);
	DebugMsg (str);
#endif
	return (p);
}



#ifdef standalone_plate

FILE *ifile;

void gdata (void)
{	int i;
	int j;
	char line [max_line_len];

	/* title */
	fgets (line, max_line_len, ifile);
	fgets (line, max_line_len, ifile);
	fgets (line, max_line_len, ifile);

	fgets (line, max_line_len, ifile);
	sscanf (line, " %d %d %d %*d", &plate_n_points, &plate_n_triangles, &plate_n_constraints);

	/* dummy material, exactly one line */
	fgets (line, max_line_len, ifile);
	fgets (line, max_line_len, ifile);
	sscanf (line, " %*d %lg %lg", &plate_modulus, &plate_poisson);

	fgets (line, max_line_len, ifile);

	for (i = 0; i < plate_n_points; i++)
	{	fgets (line, max_line_len, ifile);
		sscanf (line, " %*d %lg %lg", plate_point_x + i, plate_point_y + i);
	}

	fgets (line, max_line_len, ifile);
	for (i = 0; i < plate_n_triangles; i++)
	{	fgets (line, max_line_len, ifile);
		sscanf (line, " %*d %d %d %d %lg", plate_triangle_points [i], plate_triangle_points [i] + 1,
			plate_triangle_points [i] + 2, plate_thickness + i);
		for (j = 0; j < tri_pts; j++)			/* 1-based fortran */
			plate_triangle_points [i] [j]--;
	}

	fgets (line, max_line_len, ifile);
	for (i = 0; i < plate_n_constraints; i++)
	{	fgets (line, max_line_len, ifile);
		sscanf (line, " %d %d", plate_constraint_point + i, plate_constraint_code + i);
		plate_constraint_point [i]--;			/* 1-based fortran */
	}

	printf ("%d %d\n", plate_n_points, plate_n_triangles);
	for (i = 0; i < plate_n_points; i++)
	{	printf (" %20.12lg %20.12lg\n", plate_point_x [i], plate_point_y [i]);
	}

	for (i = 0; i < plate_n_triangles; i++)
	{	printf (" %d %d %d\n", plate_triangle_points [i] [0] + 1,
				plate_triangle_points [i] [1] + 1,
				plate_triangle_points [i] [2] + 1);	/* 1-based fortran */
	}
}

void load (void)
{	int i;
	int point;
	double r [tri_pts];
	char line [max_line_len];
	

	for (i = 0; i < plate_matrix_size; i++)
		plate_rhs [i] = 0.0;

	fgets (line, max_line_len, ifile);
	point = 0;
	while (point < plate_n_points)
	{	fgets (line, max_line_len, ifile);
		sscanf (line, " %d %lg %lg %lg", &point, r, r + 1, r + 2);
		point--;			/* 1-based fortran */
		for (i = 0; i < n_deg_free; i++)
		{	plate_rhs [mat_index (point, i)] += r [i];
		}
	}
}

#else

void plate_setup (void)
{	int i;
	int irhs;

	plate_matrix_size = plate_n_points * n_deg_free;

	set_mat (plate_matrix_size);
	clear_mat ();

	for (irhs = 0; irhs < plate_n_basis_tests; irhs++)
	{	for (i = 0; i < plate_matrix_size; i++)
			plate_basis_rhs [irhs] [i] = 0.0;
	}

	/* no forces except z */

	for (irhs = 0; irhs < plate_n_basis_tests; irhs++)
	{	for (i = 0; i < plate_basis_n_forces [irhs]; i++)
		{	plate_basis_rhs [irhs] [mat_index (plate_basis_force_points [irhs] [i], 0)] +=
					plate_basis_forces [irhs] [i];
		}
	}
}


#endif

void tri_int (
	int i_tri,
	tri_int_corners *corners,
	double *x,
	double *y,
	double *tri_x_cg,
	double *tri_y_cg,
	double *xcg_p,
	double *ycg_p,
	double *area_p,
	double *mx_p,
	double *my_p,
	double *mxy_p)
{	int j;
	double area;
	double mx;
	double my;
	double mxy;
	double xcg;
	double ycg;
	double tri_x [tri_pts];
	double tri_y [tri_pts];

	xcg = 0;
	ycg = 0;
	for (j = 0; j < tri_pts; j++)
	{	tri_x [j] = x [corners [i_tri] [j]];
		tri_y [j] = y [corners [i_tri] [j]];
		xcg += tri_x [j];
		ycg += tri_y [j];
	}
	xcg /= tri_pts;
	ycg /= tri_pts;
	for (j = 0; j < tri_pts; j++)
	{	tri_x_cg [j] = tri_x [j] - xcg;
		tri_y_cg [j] = tri_y [j] - ycg;
	}
	area = 0;
	mx = 0;
	my = 0;
	mxy = 0;
	for (j = 0; j < tri_pts; j++)
	{	area += tri_x_cg [j] * (tri_y_cg [(j + 1) % tri_pts] - tri_y_cg [(j + tri_pts - 1) % tri_pts]);
		mx += tri_x_cg [j] * tri_x_cg [j];
		my += tri_y_cg [j] * tri_y_cg [j];
		mxy += tri_x_cg [j] * tri_y_cg [j];
	}
	area *= .5;
	mx *= area / 12.0;
	my *= area / 12.0;
	mxy *= area / 12.0;

#ifdef debug
	printf ("tri int %g %g %g %g %g %g\n", xcg, ycg, area, mx, my, mxy);
	printf ("x cg %g %g %g ycg %g %g %g\n", tri_x_cg [0], tri_x_cg [1], tri_x_cg [2], 
	tri_y_cg [0], tri_y_cg [1], tri_y_cg [2]);
#endif

	*xcg_p = xcg;
	*ycg_p = ycg;
	*area_p = area;
	*mx_p = mx;
	*my_p = my;
	*mxy_p = mxy;
	
}

void mat_inv (
	double a [tri_deg_free] [tri_deg_free],
    int n)
{	register int i, j, k;
	int pivotrow;
	double pivotval;
	double t;
    double b [tri_deg_free] [tri_deg_free];

	for (i = 0; i < tri_deg_free; i++)
	{	for (j = 0; j < tri_deg_free; j++)
			b [i] [j] = 0.0;
		b [i] [i] = 1.0;
	}
	for (i = 0; i < tri_deg_free; i++)
	{	pivotrow = i;
		pivotval = abs (a [i] [i]);
		for (j = i + 1; j < tri_deg_free; j++)
		{	if (abs (a [j] [i]) > pivotval)
			{	pivotval = abs (a [j] [i]);
				pivotrow = j;
			}
		}
		for (j = 0; j < tri_deg_free; j++)
		{	t = a [i] [j];
			a [i] [j] = a [pivotrow] [j];
			a [pivotrow] [j] = t;
			t = b [i] [j];
			b [i] [j] = b [pivotrow] [j];
			b [pivotrow] [j] = t;
		}
		pivotval = 1.0 / a [i] [i];
		for (j = 0; j < tri_deg_free; j++)
		{	a [i] [j] *= pivotval;
			b [i] [j] *= pivotval;
		}
		for (j = i + 1; j < tri_deg_free; j++)
		{	pivotval = a [j ] [i];
			a [j] [i] = 0.0;
			for (k = i + 1; k < tri_deg_free; k++)
			{	a [j] [k] -= pivotval * a [i] [k];
			}
			for (k = 0; k < tri_deg_free; k++)
			{	b [j] [k] -= pivotval * b [i] [k];
			}
		}
	}
	for (i = tri_deg_free - 1; i >= 0; i--)
	{	for (j = i - 1; j >= 0; j--)
		{	pivotval = a [j] [i];
			a [j] [i] = 0.0;
			for (k = 0; k < tri_deg_free; k++)
				b [j] [k] -= b [i] [k] * pivotval;
		}
	}
	for (i = 0; i < tri_deg_free; i++)
		for (j = 0; j < tri_deg_free; j++)
		 	a [i] [j] = b [i] [j];

}

// this code screws up on occassion for unknown reason

void taki_mat_inv (
	double a [tri_deg_free] [tri_deg_free],
	int n)
{	int i;
	int j;
	int k;
	int index [tri_deg_free];
	int indexk;
	int k1;
	double piv;
	double w;
	int pivr;

#ifdef debug
	printf ("ainv input\n");
	for (i = 0; i < 9; i++)
	{	for (j = 0; j < 9; j++)
		{	printf (" %10g", a [i] [j]);
		}
		printf ("\n");
	}
#endif

	for (k = 0; k < n; k++)
	{	piv = 0.0;
		pivr = 0;
		for (i = k; i < n; i++)
		{	w = abs (a [i] [k]);
			if (w > piv)
			{	piv = w;
				pivr = i;
			}
		}
		piv = a [pivr] [k];
		index [k] = pivr;
		if (pivr != k)
		{	for (j = 0; j < n; j++)
			{	w = a [pivr] [j];
				a [pivr] [j] = a [k] [j];
				a [k] [j] = w;
			}
		}
		a [k] [k] = 1.0;
		for (j = 0; j < n; j++)
		{	a [k] [j] /= piv;
		}
		for (i = 0; i < n; i++)
		{	if (i != k)
			{	w = a [i] [k];
				a [i] [k] = 0.0;
				for (j = 0; j < n; j++)
				{	a [i] [j] -= a [k] [j] * w;
				}
			}
		}

#ifdef debug_ainv_middle
		printf ("ainv middle\n");
		for (i = 0; i < 9; i++)
		{	for (j = 0; j < 9; j++)
			{	printf (" %10g", a [i] [j]);
			}
			printf ("\n");
		}
#endif

	}
	for (k = 0; k < n - 1; k++)
	{	k1 = n - k - 2;
		indexk = index [k1];
		if (indexk != k1)
		{	for (i = 0; i < n; i++)
			{	w = a [i] [indexk];
				a [i] [indexk] = a [i] [k1];
				a [i] [k1] = w;
			}
		}
	}
#ifdef debug
	printf ("ainv output\n");
	for (i = 0; i < 9; i++)
	{	for (j = 0; j < 9; j++)
		{	printf (" %10g", a [i] [j]);
		}
		printf ("\n");
	}
#endif
}

void tri_setup (
	double *tri_x_cg,
    double *tri_y_cg,
	double ainv [tri_deg_free] [tri_deg_free],
    int n)
{   int i;
    int j;

	ainv [0] [0] = 1.0;
	ainv [0] [1] = tri_x_cg [0];
	ainv [0] [2] = tri_y_cg [0];
	ainv [0] [3] = 0.0;
	ainv [0] [4] = -0.5 * tri_x_cg [0] * tri_x_cg [0];
	ainv [0] [5] = -0.5 * tri_x_cg [0] * tri_y_cg [0];
	ainv [0] [6] = 0.0;
	ainv [0] [7] = -0.5 * tri_x_cg [0] * tri_y_cg [0];
	ainv [0] [8] = -0.5 * tri_y_cg [0] * tri_y_cg [0];

	ainv [1] [0] = 1.0;
	ainv [1] [1] = tri_x_cg [1];
	ainv [1] [2] = tri_y_cg [1];
	ainv [1] [3] = 0.0;
	ainv [1] [4] = -0.5 * tri_x_cg [1] * tri_x_cg [1];
	ainv [1] [5] = -0.5 * tri_x_cg [1] * tri_y_cg [1];
	ainv [1] [6] = 0.0;
	ainv [1] [7] = -0.5 * tri_x_cg [1] * tri_y_cg [1];
	ainv [1] [8] = -0.5 * tri_y_cg [1] * tri_y_cg [1];

	ainv [2] [0] = 1.0;
	ainv [2] [1] = tri_x_cg [2];
	ainv [2] [2] = tri_y_cg [2];
	ainv [2] [3] = 0.0;
	ainv [2] [4] = -0.5 * tri_x_cg [2] * tri_x_cg [2];
	ainv [2] [5] = -0.5 * tri_x_cg [2] * tri_y_cg [2];
	ainv [2] [6] = 0.0;
	ainv [2] [7] = -0.5 * tri_x_cg [2] * tri_y_cg [2];
	ainv [2] [8] = -0.5 * tri_y_cg [2] * tri_y_cg [2];

	ainv [3] [0] = 0.0;
	ainv [3] [1] = 0.0;
	ainv [3] [2] = 0.0;
	ainv [3] [3] = 1.0;
	ainv [3] [4] = tri_x_cg [0];
	ainv [3] [5] = tri_y_cg [0];
	ainv [3] [6] = 0.0;
	ainv [3] [7] = 0.0;
	ainv [3] [8] = 0.0;

	ainv [4] [0] = 0.0;
	ainv [4] [1] = 0.0;
	ainv [4] [2] = 0.0;
	ainv [4] [3] = 1.0;
	ainv [4] [4] = tri_x_cg [1];
	ainv [4] [5] = tri_y_cg [1];
	ainv [4] [6] = 0.0;
	ainv [4] [7] = 0.0;
	ainv [4] [8] = 0.0;

	ainv [5] [0] = 0.0;
	ainv [5] [1] = 0.0;
	ainv [5] [2] = 0.0;
	ainv [5] [3] = 1.0;
	ainv [5] [4] = tri_x_cg [2];
	ainv [5] [5] = tri_y_cg [2];
	ainv [5] [6] = 0.0;
	ainv [5] [7] = 0.0;
	ainv [5] [8] = 0.0;

	ainv [6] [0] = 0.0;
	ainv [6] [1] = 0.0;
	ainv [6] [2] = 0.0;
	ainv [6] [3] = 0.0;
	ainv [6] [4] = 0.0;
	ainv [6] [5] = 0.0;
	ainv [6] [6] = 1.0;
	ainv [6] [7] = tri_x_cg [0];
	ainv [6] [8] = tri_y_cg [0];

	ainv [7] [0] = 0.0;
	ainv [7] [1] = 0.0;
	ainv [7] [2] = 0.0;
	ainv [7] [3] = 0.0;
	ainv [7] [4] = 0.0;
	ainv [7] [5] = 0.0;
	ainv [7] [6] = 1.0;
	ainv [7] [7] = tri_x_cg [1];
	ainv [7] [8] = tri_y_cg [1];

	ainv [8] [0] = 0.0;
	ainv [8] [1] = 0.0;
	ainv [8] [2] = 0.0;
	ainv [8] [3] = 0.0;
	ainv [8] [4] = 0.0;
	ainv [8] [5] = 0.0;
	ainv [8] [6] = 1.0;
	ainv [8] [7] = tri_x_cg [2];
	ainv [8] [8] = tri_y_cg [2];
    if (debug_plate_load)
    {	for (i = 0; i < tri_deg_free; i++)
        {	fprintf (debug_file, "ainv_load %d %d:", n, i);
            for (j = 0; j < tri_deg_free; j++)
                fprintf (debug_file, " %g", ainv [i] [j]);
            fprintf (debug_file, "\n");
        }
    }


	mat_inv (ainv, tri_deg_free);

    if (debug_plate_load)
    {	for (i = 0; i < tri_deg_free; i++)
        {	fprintf (debug_file, "ainv_inv %d %d:", n, i);
            for (j = 0; j < tri_deg_free; j++)
                fprintf (debug_file, " %g", ainv [i] [j]);
            fprintf (debug_file, "\n");
        }
    }

}

void int_triangles (void)
{	int i;

	for (i = 0; i < plate_n_triangles; i++)
	{	tri_int (i, plate_triangle_points, plate_point_x, plate_point_y,
			plate_tri_x_cg [i], plate_tri_y_cg [i], plate_xcg + i, plate_ycg + i,
			plate_area + i, plate_mx + i, plate_my + i, plate_mxy + i);
	}
}

void pload (void)
{	int i;
	int j;
	int k;
	int irhs;
	double pressure;
	double ptotal;
	double area;
	double mx;
	double my;
	double mxy;
	double ainv [tri_deg_free] [tri_deg_free];
	double xm [tri_pts * n_deg_free];
	double fp [tri_pts * n_deg_free];
#ifdef standalone_plate
	char line [max_line_len];
#endif

	ptotal = 0;
#ifdef standalone_plate
	fgets (line, max_line_len, ifile);
#endif

	int_triangles ();

	for (i = 0; i < plate_n_triangles; i++)
	{
#ifdef standalone_plate
		fgets (line, max_line_len, ifile);
		sscanf (line, " %*d %lg", &pressure);
#else
		pressure = plate_tri_pressure [i];
#endif
		area = plate_area [i];
		mx = plate_mx [i];
		my = plate_my [i];
		mxy = plate_mxy [i];
		ptotal += pressure * area;

#ifdef debug
		printf ("pload %d\n", i);
#endif

        if (debug_plate_load)
            fprintf (debug_file, "pload setup %d\n", i);


		tri_setup (plate_tri_x_cg [i], plate_tri_y_cg [i], ainv, i);

		xm [0] = area * pressure;
		xm [1] = 0.0;
		xm [2] = 0.0;
		xm [3] = 0.0;
		xm [4] = -0.5 * mx * pressure;
		xm [5] = -0.5 * mxy * pressure;
		xm [6] = 0.0;
		xm [7] = -0.5 * mxy * pressure;
		xm [8] = -0.5 * my * pressure;
		
		for (j = 0; j < tri_deg_free; j++)
		{	fp [j] = 0.0;
			for (k = 0; k < tri_deg_free; k++)
				fp [j] += ainv [k] [j] * xm [k];
		}
#ifdef debug
		for (k = 0; k < 9; k++)
			printf (" %g", fp [k]);
		printf (" rhs %g\n", plate_basis_rhs [0] [3]);
#endif
		for (irhs = 0; irhs < plate_n_basis_tests; irhs++)
		{	plate_basis_rhs [irhs] [mat_index (plate_triangle_points [i] [0], 0)] += fp [0];
			plate_basis_rhs [irhs] [mat_index (plate_triangle_points [i] [0], 1)] += fp [3];
			plate_basis_rhs [irhs] [mat_index (plate_triangle_points [i] [0], 2)] += fp [6];
	
			plate_basis_rhs [irhs] [mat_index (plate_triangle_points [i] [1], 0)] += fp [1];
			plate_basis_rhs [irhs] [mat_index (plate_triangle_points [i] [1], 1)] += fp [4];
			plate_basis_rhs [irhs] [mat_index (plate_triangle_points [i] [1], 2)] += fp [7];
	
			plate_basis_rhs [irhs] [mat_index (plate_triangle_points [i] [2], 0)] += fp [2];
			plate_basis_rhs [irhs] [mat_index (plate_triangle_points [i] [2], 1)] += fp [5];
			plate_basis_rhs [irhs] [mat_index (plate_triangle_points [i] [2], 2)] += fp [8];
		}
	}
}

void formk (void)
{	int i;
	int j;
	int jj;
	int k;
	int kk;
	int l;
	int n;
	int irhs;
	int nrowb;
	int ncolb;
#ifndef  use_sparse
	int nr;
   int ncol;
#endif

	int m;
	int nx;
	int icon;

	double tri_x [tri_pts];
	double tri_y [tri_pts];
	double mx;
	double my;
	double area;
	double thick;
	double d;
	double g;
	double ainv [tri_deg_free] [tri_deg_free];
	double bdb [tri_deg_free] [tri_deg_free];
	double xk1 [tri_deg_free] [tri_deg_free];
	double xk [tri_deg_free] [tri_deg_free];
	double dummy [tri_deg_free] [tri_deg_free];
	int ixx [tri_deg_free];

#ifndef use_sparse
	for (i = 0; i < plate_matrix_size; i++)
	{	for (j = 0; j < band_size; j++)
		{	sk [i] [j] = 0.0;
		}
	}
#endif

	g = .5 * plate_modulus / (1 + plate_poisson);
	for (n = 0; n < plate_n_triangles; n++)
	{
#ifdef debug
		printf ("formk %d\n", n);
#endif
		for (i = 0; i < tri_pts; i++)
		{	tri_x [i] = plate_point_x [plate_triangle_points [n] [i]];
			tri_y [i] = plate_point_y [plate_triangle_points [n] [i]];
		}

		thick = plate_thickness [n];
		d = plate_modulus * thick * thick * thick / 12.0 / (1.0 - plate_poisson * plate_poisson);
		area = plate_area [n];
		mx = plate_mx [n];
		my = plate_my [n];

        if (debug_plate_load)
            fprintf (debug_file, "formk setup %d\n", n);

		tri_setup (plate_tri_x_cg [n], plate_tri_y_cg [n], ainv, i);


		for (i = 0; i < tri_deg_free; i++)
		{	for (j = 0; j < tri_deg_free; j++)
			{	bdb [i] [j] = 0.0;
				xk1 [i] [j] = 0.0;
				xk [i] [j] = 0.0;
				dummy [i] [j] = 0.0;
			}
		}

		bdb [1] [1] = plate_shear * thick * g * area;
		bdb [1] [3] = bdb [1] [1];
		bdb [2] [2] = bdb [1] [1];
		bdb [2] [6] = bdb [1] [1];
		bdb [3] [1] = bdb [1] [1];
		bdb [3] [3] = bdb [1] [1];
		bdb [4] [4] = d * area;
		bdb [4] [8] = plate_poisson * d * area;
		bdb [5] [5] = 0.5 * (1.0 - plate_poisson) * d * area + 0.25 * plate_shear * thick * g * (mx + my);
		bdb [5] [7] = 0.5 * (1.0 - plate_poisson) * d * area - 0.25 * plate_shear * thick * g * (mx + my);
		bdb [6] [2] = bdb [1] [1];
		bdb [6] [6] = bdb [1] [1];
		bdb [7] [5] = 0.5 * (1.0 - plate_poisson) * d * area - 0.25 * plate_shear * thick * g * (mx + my);
		bdb [7] [7] = 0.5 * (1.0 - plate_poisson) * d * area + 0.25 * plate_shear * thick * g * (mx + my);
		bdb [8] [4] = plate_poisson * d * area;
		bdb [8] [8] = d * area;

		for (i = 0; i < tri_deg_free; i++)
		{	for (j = 0; j < tri_deg_free; j++)
			{	for (k = 0; k < tri_deg_free; k++)
				{	dummy [i] [j] += bdb [i] [k] * ainv [k] [j];
				}
			}
		}

		for (i = 0; i < tri_deg_free; i++)
		{	for (j = 0; j < tri_deg_free; j++)
			{	for (k = 0; k < tri_deg_free; k++)
				{	xk1 [i] [j] += ainv [k] [i] * dummy [k] [j];
				}
			}
		}
		ixx [0] = 0;
		ixx [1] = 3;
		ixx [2] = 6;
		ixx [3] = 1;
		ixx [4] = 4;
		ixx [5] = 7;
		ixx [6] = 2;
		ixx [7] = 5;
		ixx [8] = 8;

		for (i = 0; i < tri_deg_free; i++)
		{	for (j = 0; j < tri_deg_free; j++)
			{	xk [i] [j] = xk1 [ixx [i]] [ixx [j]];
			}
		}

		for (jj = 0; jj < tri_pts; jj++)
		{	for (j = 0; j < n_deg_free; j++)
			{	nrowb = mat_index (plate_triangle_points [n] [jj], j);
				i = mat_index (jj, j);
				for (kk = 0; kk < tri_pts; kk++)
				{	for (k = 0; k < n_deg_free; k++)
					{	ncolb = mat_index (plate_triangle_points [n] [kk], k);
						l = mat_index (kk, k); 
#ifdef use_sparse
						add_to_el (nrowb, ncolb, xk [i] [l]);
#else
						ncol = ncolb - nrowb;
						if (ncol >= 0)
						{	if (ncol >= band_size)
							{	fprintf (stderr, "out of band\n");
								exit (1);
							}
							sk [nrowb] [ncol] += xk [i] [l];
#ifdef debug
							printf ("%d %d %d %d %g %g\n", nrowb, ncol, i, l, sk [nrowb] [ncol], xk [i] [l]);
#endif
						}
#endif
					}
				}
			}
		}
	}

	for (n = 0; n < plate_n_constraints; n++)
	{	nx = pow10_n_deg_free_m1;
		k = plate_constraint_code [n];
		for (m = 0; m < n_deg_free; m++)
		{	nrowb = mat_index (plate_constraint_point [n], m);
			icon = k / nx;
			if (icon > 0)
			{	
#ifdef use_sparse
				zap_row (nrowb);
				zap_col (nrowb);
				add_to_el (nrowb, nrowb, 1.0);
				for (irhs = 0; irhs < plate_n_basis_tests; irhs++)
					plate_basis_rhs [irhs] [nrowb] = 0.0;
#else
				sk [nrowb] [0] = 1.0;
				plate_rhs [nrowb] = 0.0;
				for (j = 1; j < band_size; j++)
				{	sk [nrowb] [j] = 0.0;
					nr = nrowb - j;
					if (nr >= 0)
						sk [nr] [j] = 0.0;
				}
#endif
			}
			k -= icon * nx;
			nx /= 10;
		}
	}
}

#ifdef use_sparse
void solve (void)
{
	set_mat (plate_matrix_size);

	solve_sparse_sys (plate_basis_rhs, plate_n_basis_tests);
}

	
#else
void solve (void)
{	int n;
	int j;
	int k;
	int i;
	int l;
	double c;

#ifdef debug
	for (i = 0; i < plate_matrix_size; i++)
	{	for (j = 0; j < band_size; j++)
		{	printf ("%d %d %g\n", i, j, sk [i] [j]);
		}
	}
	for (i = 0; i < plate_matrix_size; i++)
	{	printf ("%d %g\n", i, plate_rhs [i]);
	}
#endif
	for (n = 0; n < plate_matrix_size; n++)
	{	i = n;
		for (l = 1; l < band_size; l++)
		{	i = i + 1;
			if (sk [n] [l] != 0)
			{	c = sk [n] [l] / sk [n] [0];
				j = 0;
				for (k = l; k < band_size; k++)
				{	sk [i] [j] -= c * sk [n] [k];
					j++;
				}
				sk [n] [l] = c;
				plate_rhs [i] -= c * plate_rhs [n];
			}
		}
		plate_rhs [n] /= sk [n] [0];
	}

	for (n = plate_matrix_size - 2; n >= 0; n--)
	{	l = n;
		for (k = 1; k < band_size; k++)
		{	l++;
			plate_rhs [n] -= sk [n] [k] * plate_rhs [l];
		}
	}

}
#endif

#ifdef standalone_plate

void out (void)
{	int i;

	double xcg;
	double ycg;
	double area;
	double z_center;

	for (i = 0; i < plate_n_points; i++)
	{	printf ("%20.12g\n", plate_rhs [mat_index (i, 0)]);
	}

	for (i = 0; i < plate_n_triangles; i++)
	{	xcg = plate_xcg [i];
		ycg = plate_ycg [i];
		area = plate_area [i];
		z_center = (
			plate_rhs [mat_index (plate_triangle_points [i] [0], 0)] +
			plate_rhs [mat_index (plate_triangle_points [i] [1], 0)] +
			plate_rhs [mat_index (plate_triangle_points [i] [2], 0)])
			/ 3.0;
		printf ("%20.12g %20.12g %20.12g %20.12g\n", xcg, ycg, area, z_center);
	}
}

#else

void out (void)
{	int i;


	if (plate_n_basis_tests == 1)
	{	for (i = 0; i < plate_n_points; i++)
			plate_z_displacement [i] = plate_rhs [mat_index (i, 0)];
	}
}
#endif

void plate_init (void)
{
#ifdef use_sparse
	alloc_sparse_pool (max_eqn);
#endif
	plate_shear = 5.0 / 6.0;

/* plop sets it if not standalone_plate */
#ifdef standalone_plate
	plate_n_basis_tests = 1;
#endif
	plate_basis_rhs [0] = plate_rhs;
	plate_basis_forces [0] = plate_forces;
	plate_basis_n_forces = &plate_n_forces;
	plate_basis_force_points [0] = plate_force_points;
}

#ifdef standalone_plate

int main (
	int argc,
	char *argv[])
{

	alloc_plate_globals ();
	plate_init ();
	if (argc == 0)
		ifile = fopen ("INPUT.DAT", "r");
	else
		ifile = fopen (argv [1], "r");
	if (ifile == (FILE *) NULL)
	{	fprintf (stderr, "can't open input file\n");
		exit (1);
	}

	gdata ();

	plate_matrix_size = plate_n_points * n_deg_free;

#ifdef use_sparse
	set_mat (plate_matrix_size);
	clear_mat ();
#endif

	load ();

	pload ();

	formk ();

	solve ();

	out ();

	return 0;
}
#endif
