#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "plot.h"


#include "plate_global.h"

#include "plop_parms.h"
#include "grid_gen.h"
#include "plop_parser.h"

#ifdef use_gd_graphics
#include "gd.h"
#endif

#ifndef __BORLANDC__
#define max(x,y) ((x)>(y)? (x) : (y))
#define min(x,y) ((x)<(y)? (x) : (y))
#endif

#ifdef use_gd_graphics
#include "gd.h"
#endif

#ifdef gui_plop
extern void LockPlopData (int);
extern void UnlockPlopData (void);

extern void DebugMsg (char const *);
#endif

int color_max_inten = default_max_inten;

typedef struct {
	double x, y;
	} point;

int *mirrored_point;	/* [max_points] */

double *tri_slope_x;	/* [max_triangles] */
double *tri_slope_y;	/* [max_triangles] */
double *plate_plot_tri_slope_x;	/* [max_triangles] */
double *plate_plot_tri_slope_y;	/* [max_triangles] */
double *z88_plot_tri_slope_x;	/* [max_triangles] */
double *z88_plot_tri_slope_y;	/* [max_triangles] */

plop_cmap color_cmap;
plop_cmap contour_cmap;
plop_cmap bw_cmap;

int color_white_index;
int color_black_index;

#ifdef use_gd_graphics
gdImagePtr gd_image;
int gd_color_table [max_colors];
int gd_color_black;
int gd_color_white;

#endif

unsigned char picture [graph_xy_max] [graph_xy_max];

int mirror_x = 0;

extern void alloc_plate_globals (void);

extern void *malloc_or_die (int, char const *);

void init_global_graphics (void)
{	pic_size = default_pic_size;
	n_colors = default_n_colors;
	n_contours = default_n_contours;
	pic_z_range = 0;
    contour_space = 0;
}

void init_graphics (void)
{	int i;

	mirrored_point = (int *) malloc_or_die (max_points * sizeof (int), "mirrored_point");

	tri_slope_x = (double *) malloc_or_die (max_triangles * sizeof (double), "tri_slope_x");
	tri_slope_y = (double *) malloc_or_die (max_triangles * sizeof (double), "tri_slope_y");
	plate_plot_tri_slope_x = (double *) malloc_or_die (max_triangles * sizeof (double), "plate_plot_tri_slope_x");
	plate_plot_tri_slope_y = (double *) malloc_or_die (max_triangles * sizeof (double), "plate_plot_tri_slope_y");
	z88_plot_tri_slope_x = (double *) malloc_or_die (max_triangles * sizeof (double), "plate_plot_tri_slope_x");
	z88_plot_tri_slope_y = (double *) malloc_or_die (max_triangles * sizeof (double), "plate_plot_tri_slope_y");


    for (i = 0; i < n_contour_colors; i++)
    {	contour_cmap.color_r [i] = color_max_inten * i / (n_contour_colors - 1);
		contour_cmap.color_g [i] = color_max_inten * i / (n_contour_colors - 1);
        contour_cmap.color_b [i] = color_max_inten * i / (n_contour_colors - 1);
    }
    contour_cmap.n_colors = i;

 	for (i = 0; i < color_map_size; i++)
    {	if (i < color_map_size / 2)
        {	color_cmap.color_r [i] = 0;
            color_cmap.color_g [i] = (i * color_max_inten) / (color_map_size / 2);
            color_cmap.color_b [i] = color_max_inten - (i * color_max_inten) / (color_map_size / 2);
        }
        else
        {	color_cmap.color_r [i] = (i - color_map_size / 2) * color_max_inten / (color_map_size / 2);
            color_cmap.color_g [i] = color_max_inten - ((i - n_colors / 2) * color_max_inten) / (n_colors / 2);
            color_cmap.color_b [i] = 0;
        }
    }
 	color_cmap.color_r [i] = 0;
    color_cmap.color_g [i] = 0;
    color_cmap.color_b [i] = 0;
    color_black_index = i++;

 	color_cmap.color_r [i] = color_max_inten;
    color_cmap.color_g [i] = color_max_inten;
    color_cmap.color_b [i] = color_max_inten;
    color_white_index = i++;
    color_cmap.n_colors = i;

    bw_cmap.color_r [0] = 0;
    bw_cmap.color_g [0] = 0;
    bw_cmap.color_b [0] = 0;

 	bw_cmap.color_r [1] = color_max_inten;
    bw_cmap.color_g [1] = color_max_inten;
    bw_cmap.color_b [1] = color_max_inten;

    bw_cmap.n_colors = 2;

}

#ifdef standalone
void read_surface (
	char *fname)
{	int i;
	FILE *f;
	double dummy_double;

	if ((f = fopen (fname, "r")) == (FILE *) NULL)
	{	fprintf (stderr, "can't open %s\n", fname);
		exit (1);
	}
	fscanf (f, " %d %d", &plate_n_points, &plate_n_triangles);
	for (i = 0; i < plate_n_points; i++)
		fscanf (f, " %lg %lg", &plate_point_x [i], &plate_point_y [i]);
	for (i = 0; i < plate_n_triangles; i++)
	{	fscanf (f, " %d %d %d",
			&plate_triangle_points [i] [0],
			&plate_triangle_points [i] [1],
			&plate_triangle_points [i] [2]);
		plate_triangle_points [i] [0]--;			/* 1-based fortran results */
		plate_triangle_points [i] [1]--;
		plate_triangle_points [i] [2]--;
	}
	for (i = 0; i < plate_n_points; i++)
		fscanf (f, " %lg", &plate_z_displacement [i]);
	for (i = 0; i < plate_n_triangles; i++)
	{	fscanf (f, " %lg %lg %lg %lg",
			&plate_xcg [i],
			&plate_ycg [i],
			&plate_area [i],
			&dummy_double);
	}
	fclose (f);
}

#endif
/* reflect the shape in x, only replicating points with x != 0 */

void reflect_x (void)
{	int i;
	int j;
	int pt_next;
	int tri_next;

	pt_next = plate_n_points;
	for (i = 0; i < plate_n_points; i++)
	{	if (plate_point_x [i] != 0)
		{	plate_point_x [pt_next] = - plate_point_x [i];
			plate_point_y [pt_next] = plate_point_y [i];
			plate_z_displacement [pt_next] = plate_z_displacement [i];
			mirrored_point [i] = pt_next;
			pt_next++;
		}
		else
			mirrored_point [i] = i;
	}
	plate_n_points = pt_next;
	for (i = 0; i < plate_n_triangles; i++)
	{	tri_next = i + plate_n_triangles;
		for (j = 0; j < tri_pts; j++)
		{	plate_triangle_points [tri_next] [j] = mirrored_point [plate_triangle_points [i] [j]];
		}
		plate_xcg [tri_next] = - plate_xcg [i];
		plate_ycg [tri_next] = plate_ycg [i];
		plate_area [tri_next] = plate_area [i];
	}
	plate_n_triangles += plate_n_triangles;
}
	

/* find coefficients that do linear interpolatin of z plate_z_displacement in a triangle. */

void find_xy_interp_coeff (
	int *pts,
	double *x,
	double *y,
	double *z,
	double *slope_x,
	double *slope_y)
{	double m00;
	double m01;
	double m10;
	double m11;
	double r0;
	double r1;
	int p0, p1, p2;

	p0 = pts [0];
	p1 = pts [1];
	p2 = pts [2];

	/* set up 2*2 system of eqns adopting point[0] as reference */

	m00 = x [p1] - x [p0];
	m01 = y [p1] - y [p0];
	m10 = x [p2] - x [p0];
	m11 = y [p2] - y [p0];
	r0 = z [pts [1]] - z [pts [0]];
	r1 = z [pts [2]] - z [pts [0]];

	*slope_x = (m11 * r0 - m01 * r1) / (m00 * m11 - m10 * m01);
	*slope_y = (m00 * r1 - m10 * r0) / (m00 * m11 - m10 * m01);
}


/* find coefficients that do linear interpolatin of z plate_z_displacement in a triangle. */

void find_plate_xy_interp_coeff (
	int t)
{
	find_xy_interp_coeff (plate_triangle_points [t],
		plate_point_x, plate_point_y,  plate_z_displacement,
		&(tri_slope_x [t]), & tri_slope_y [t]);
}

void fcorner (
	int t,
	int i,
	int j,
	double wts [tri_pts],
	double *x,
	double *y)
{	int k;
    int ipoint;

	wts [0] = (double) (tri_fract - i) * tri_fract_inv;
	wts [1] = (double) i * (tri_fract - j) * tri_fract_sq_inv;
	wts [2] = (double) i * j * tri_fract_sq_inv;
	*x = 0;
	*y = 0;
	for (k = 0; k < tri_pts; k++)
	{	ipoint = plate_triangle_points [t] [k];
        *x += wts [k] * plate_point_x [ipoint];
		*y += wts [k] * plate_point_y [ipoint];
	}
}

/*
 * continuity of error function is essential for gradient based optimization.
 * we ensure this by weighting triangles that straddle the obstruction radius.
 *
 * If the tri straddles the obstruction radius, set *strad to 1 and *strad_wt
 * to an estimate of the fraction outside the obstruction, else *strad = 0.
 */

void calc_straddle (
	double *xp,
	double *yp,
	int *strad,
	double *strad_wt)
{	int i;
	int n_inside;
	double r_inner;	/* used as both r and r^2, calc r^2 but only sqrt if necessary */
	double r_outer;
	double r_pt;
	double obstruction_radius_squared;

	r_inner = 1e10;
	r_outer = 0;
	n_inside = 0;
	obstruction_radius_squared = obstruction_radius * obstruction_radius;
	for (i = 0; i < tri_pts; i++)
	{	r_pt = xp [i] * xp [i] + yp [i] * yp [i];
		if (r_pt <= obstruction_radius_squared)
		{	n_inside++;
		}
		if (r_pt < r_inner)
			r_inner = r_pt;
		if (r_pt > r_outer)
			r_outer = r_pt;
	}

	if (n_inside == 0)
	{	*strad = 0;
		*strad_wt = 1.0;
	}
	else if (n_inside == tri_pts)
	{	*strad = 0;
		*strad_wt = 0.0;
	}
	else
	{	*strad = 1;

		/* The sub-triangle may not be oriented with tip problem along a radius,
		 * (even though mesh triangles are)
		 * and exact calculation of overlap area is a problem so we use a linear spline
		 * to approximate the area. This is geometrically bogus, but since triangles
		 * alternate with respect to orientation, in fact it is a pretty good approximation
		 * over the set of trangles that straddle the obstruction_radius.
		 */
		
		r_inner = sqrt (r_inner);
		r_outer = sqrt (r_outer);
		*strad_wt = (r_outer - obstruction_radius) / (r_outer - r_inner);
		if (*strad_wt < 0 || *strad_wt > 1)
			printf ("woof\n");
	}
}

/*
 * fracture triangle t, place weights of corners in c_wts, center of gravity
 * in xcg, ycg
 *
 * c_wts [i] [j] [k] gives weight of fracture i, corner j of triangle t corner k.
 *
 */

void fracture_tri (
	int t,
	double c_wts [] [tri_pts] [tri_pts],
	double *xcg,
	double *ycg,
	int *straddles,
	double *straddle_weight)
{	int i;
	int j;
	int tf;		/* which fractured triangle */
	double cx [tri_pts];
	double cy [tri_pts];

#if (tri_fract == 1)
    int ipoint;

    ipoint = plate_triangle_points [t] [0];
    cx [0] = plate_point_x [ipoint];
	cy [0] = plate_point_y [ipoint];
    c_wts [0] [0] [0] = 1.0;
    c_wts [0] [0] [1] = 0.0;
    c_wts [0] [0] [2] = 0.0;
    ipoint = plate_triangle_points [t] [1];
    cx [1] = plate_point_x [ipoint];
	cy [1] = plate_point_y [ipoint];
    c_wts [0] [1] [0] = 0.0;
    c_wts [0] [1] [1] = 1.0;
    c_wts [0] [1] [2] = 0.0;
	ipoint = plate_triangle_points [t] [2];
    cx [2] = plate_point_x [ipoint];
	cy [2] = plate_point_y [ipoint];
    c_wts [0] [2] [0] = 0.0;
    c_wts [0] [2] [1] = 0.0;
    c_wts [0] [2] [2] = 1.0;
    xcg [0] = (cx [0] + cx [1] + cx [2]) * 0.3333333333333333333333;
    ycg [0] = (cy [0] + cy [1] + cy [2]) * 0.3333333333333333333333;
    if (obstruction_radius > 0)
	{   calc_straddle (cx, cy, straddles, straddle_weight);
    }
    else
    {   *straddles = 0;
        *straddle_weight = 1.0;
    }


#else
	tf = 0;
	for (i = 0; i < tri_fract; i++)
	{	for (j = 0; j <= i; j++)
		{
        	fcorner (t, i, j, &(c_wts [tf] [0] [0]), cx + 0, cy + 0);
			fcorner (t, i + 1, j, &(c_wts [tf] [1] [0]), cx + 1, cy + 1);
			fcorner (t, i + 1, j + 1, &(c_wts [tf] [2] [0]), cx + 2, cy + 2);
			xcg [tf] = (cx [0] + cx [1] + cx [2]) / 3.0;
			ycg [tf] = (cy [0] + cy [1] + cy [2]) / 3.0;
			calc_straddle (cx, cy, straddles + tf, straddle_weight + tf);
			tf++;
			if (j < i)
			{	fcorner (t, i, j, &(c_wts [tf] [0] [0]), cx + 0, cy + 0);
				fcorner (t, i + 1, j + 1, &(c_wts [tf] [1] [0]), cx + 1, cy + 1);
				fcorner (t, i, j + 1, &(c_wts [tf] [2] [0]), cx + 2, cy + 2);
				xcg [tf] = (cx [0] + cx [1] + cx [2]) / 3.0;
				ycg [tf] = (cy [0] + cy [1] + cy [2]) / 3.0;
				calc_straddle (cx, cy, straddles + tf, straddle_weight + tf);
				tf++;
			}
		}
	}
#endif
}

void calc_rms_new (
	int n_points,
	int n_tri,
	tri_int_corners *corners,
	double *x,
	double *y,
	double *z,
	double *err_p_v,
	double *err_vis_p_v,
	double *err_rms,
	double *err_vis_rms)
{   int itri;
	int icorner;
	int ipt;
	int i;
	int j;
	double fw;
	double w;
	double w_sq;
	double a;
	double vis_w_sq;
	double vis_w;
	double vis_a;
	double total_err2;
	double total_weight;
	double z_min;
	double z_max;
	double vis_z_min;
	double vis_z_max;
	double tri_x_cg [tri_pts], tri_y_cg [tri_pts];
	double xcg, ycg, area, mx, my, mxy;
	double tx [tri_pts], ty [tri_pts];
	double straddle;
	int istrad;
	double obstruction_radius_squared;


	w_sq = 0;
	w = 0;
	vis_w_sq = 0;
	vis_w = 0;

	a = 0;
	vis_a = 0;

	z_min = 1e10;
	z_max = -1e10;
	vis_z_min = 1e10;
	vis_z_max = -1e10;
	obstruction_radius_squared = obstruction_radius * obstruction_radius;

	for (i = 0; i < n_points; i++)
	{
		z_min = min (z_min, z [i]);
		z_max = max (z_max, z [i]);
		if (x [i] * x [i] + y [i] * y [i] >=
			obstruction_radius_squared)
		{	vis_z_min = min (vis_z_min, z [i]);
			vis_z_max = max (vis_z_max, z [i]);
		}
	}
	*err_p_v = z_max - z_min;
	*err_vis_p_v = vis_z_max - vis_z_min;

	for (itri = 0; itri < n_tri; itri++) {
		tri_int (itri, corners, x, y, tri_x_cg, tri_y_cg, &xcg, &ycg, &area, &mx, &my, &mxy);
		for (icorner = 0; icorner < tri_pts; icorner++) {
			ipt = corners [itri] [icorner];
			tx [icorner] = x [ipt];
			ty [icorner] = y [ipt];
		}
		calc_straddle (tx, ty, &istrad, &straddle);

		fw = area * straddle;

		for (icorner = 0; icorner < tri_pts; icorner++) {
			ipt = corners [itri] [icorner];
			w_sq += area * z [ipt] * z [ipt];
			w += area * z [ipt];
			a += area;
			vis_w_sq += fw * z [ipt] * z [ipt];
			vis_w += fw * z [ipt];
			vis_a += fw;
		}
	}
	*err_rms = sqrt ((w_sq - w * w / a) / a);
	*err_vis_rms = sqrt ((vis_w_sq - vis_w * vis_w / vis_a) / vis_a);
}




void calc_rms_old (void)
{	int i;
	int j;

	double w_sq;
	double w;
	double vis_w_sq;
	double vis_w;
	double vis_a;
	double a;
	double z_min;
	double z_max;
	double vis_z_min;
	double vis_z_max;
	double f_wts [tri_fract_sq] [tri_pts] [tri_pts];
	double f_xcg [tri_fract_sq];
	double f_ycg [tri_fract_sq];
	int t_straddle [tri_fract_sq];
	double t_straddle_wt [tri_fract_sq];
	double u;
	double v;
	double z;
	double obstruction_radius_squared;

	w_sq = 0;
	w = 0;
	vis_w_sq = 0;
	vis_w = 0;

	a = 0;
	vis_a = 0;

	z_min = 1e10;
	z_max = -1e10;
	vis_z_min = 1e10;
	vis_z_max = -1e10;

	obstruction_radius_squared = obstruction_radius * obstruction_radius;

	for (i = 0; i < plate_n_points; i++)
	{
		z_min = min (z_min, plate_z_displacement [i]);
		z_max = max (z_max, plate_z_displacement [i]);
		if (plate_point_x [i] * plate_point_x [i] + plate_point_y [i] * plate_point_y [i] >=
			obstruction_radius_squared)
		{	vis_z_min = min (vis_z_min, plate_z_displacement [i]);
			vis_z_max = max (vis_z_max, plate_z_displacement [i]);
		}
	}
	plate_err_p_v = z_max - z_min;
	plate_err_vis_p_v = vis_z_max - vis_z_min;

	for (i = 0; i < plate_n_triangles; i++)
	{	fracture_tri (i, f_wts, f_xcg, f_ycg, t_straddle, t_straddle_wt);
		for (j = 0; j < tri_fract_sq; j++)
		{	u = f_xcg [j] - plate_point_x [plate_triangle_points [i] [0]];
			v = f_ycg [j] - plate_point_y [plate_triangle_points [i] [0]];
			z = u * tri_slope_x [i] + v * tri_slope_y [i] + plate_z_displacement [plate_triangle_points [i] [0]];
			w_sq += plate_area [i] * z * z;
			w += plate_area [i] * z;
			a += plate_area [i];
			vis_w_sq += plate_area [i] * z * z * t_straddle_wt [j];
			vis_w += plate_area [i] * z * t_straddle_wt [j];
			vis_a += plate_area [i] * t_straddle_wt [j];
		}
	}
	plate_err_rms = sqrt ((w_sq - w * w / a) / a);
	plate_err_vis_rms = sqrt ((vis_w_sq - vis_w * vis_w / vis_a) / vis_a);
}

void calc_rms (void)
{
	calc_rms_new (plate_n_points, plate_n_triangles, plate_triangle_points,
	plate_point_x,  plate_point_y, plate_z_displacement,
	&plate_err_p_v, &plate_err_vis_p_v, &plate_err_rms, &plate_err_vis_rms);

//	calc_rms_old ();
}

void calc_z88_rms (void)
{
	calc_rms_new (z88_plot_n_points, z88_plot_n_triangles, z88_plot_triangle_points,
	z88_plot_point_x,  z88_plot_point_y, z88_plot_z_displacement,
	&z88_plot_err_p_v, &z88_plot_err_vis_p_v, &z88_plot_err_rms, &z88_plot_err_vis_rms);

//	calc_rms_old ();
}

double powint (double x, int n)
{   int i;
    double r;

    r = 1;
    for (i = 0; i < n; i++)
        r *= x;
    return r;
}


double zernike (double x, double y, int ifn)
{   int n;
    int m;
    double theta;
    double r;
    double rfn;
    int s;
    int n_m_m2;
    int n_p_m2;

    m = 0;
    while (ifn > 2 * m + 1)
    {   ifn -= 2 * m + 1;
        m++;
    }
    n = m;
    n += (ifn - 1) / 2;
    m -= (ifn - 1) / 2;

    r = sqrt (x * x + y * y) / radius;
    if (r < 1e-6)
		theta = 0;
    else
        theta = atan2 (y, x);

    rfn = 0;
    n_m_m2 = (n - m) / 2;
    n_p_m2 = (n + m) / 2;
    for (s = 0; s <= n_m_m2; s++)
    {   rfn += (s & 1 ? -1 : 1) * fact_table [n - s] / (fact_table [s] * fact_table [n_p_m2 - s] *
                fact_table [n_m_m2 - s]) * powint (r, n - 2 * s);
    }
    if (m == 0)
        rfn *= sqrt ((double) n + 1);
    else if (ifn % 2 == 0)
        rfn *= sqrt ((double) 2.0 * (n + 1)) * cos (m * theta);
    else
        rfn *= sqrt ((double) 2.0 * (n + 1)) * sin (m * theta);

    return (rfn);
}


void load_refocus_functions (
	e_refocus_functions ref_type,
	int n_functions,
	int n_points,
	double *x,
	double *y,
	double *f_vals [max_refocus_distortion_order])
{	int ipoint;
	int ifn;
	double xi, yi;

	for (ifn = 0; ifn < n_functions; ifn++) {
		if (f_vals [ifn] == NULL) {
			f_vals [ifn] = (double *) malloc_or_die (max_points * sizeof (double), "tmp_function_vals");
		}
	}
	for (ipoint = 0; ipoint < n_points; ipoint++) {
		xi = x [ipoint];
		yi = y [ipoint];
		switch (ref_type) {
			case e_refocus_parab:
				f_vals [0] [ipoint] = 1;
				f_vals [1] [ipoint] = xi * xi + yi * yi;
				break;

			case e_refocus_xy_parab:
				f_vals  [0] [ipoint]= 1;
				f_vals  [1] [ipoint]= xi;
				f_vals  [2] [ipoint]= yi;
				f_vals  [3] [ipoint]= xi * xi + yi * yi;
				break;

			case e_refocus_zernike:
				for (ifn = 0; ifn < n_functions; ifn++) {
					f_vals [ifn] [ipoint] = zernike (xi, yi, ifn + 1);
				}
				break;
		}
	}
}

void solve_matrix (
	double a [max_refocus_distortion_order] [max_refocus_distortion_order],
	double rhs [max_refocus_distortion_order],
	int n)
{	register int i, j, k;
	double pivotval;
	int ipiv;
	double pbig;
	double t;

	for (i = 0; i < n; i++)
	{   pbig = a [i] [i];
		ipiv = i;
		for (j = i + 1; j < n; j++) {
			if (fabs (a [j] [i]) > pbig) {
				ipiv = j;
				pbig = a [j] [i];
			}
		}
		for (j = i; j < n; j++) {
			t = a [i] [j];
			a [i] [j] = a [ipiv] [j];
			a [ipiv] [j] = t;
		}
		t = rhs [i];
		rhs [i] = rhs [ipiv];
		rhs [ipiv] = t;

		pivotval = 1.0 / a [i] [i];
		for (j = 0; j < n; j++)
		{	a [i] [j] *= pivotval;
		}
		rhs [i] *= pivotval;
		for (j = i + 1; j < n; j++)
		{	pivotval = a [j ] [i];
			a [j] [i] = 0.0;
			for (k = i + 1; k < n; k++)
			{	a [j] [k] -= pivotval * a [i] [k];
			}
			rhs [j] -= pivotval * rhs [i];
		}
	}
	for (i = n - 1; i >= 0; i--)
	{	for (j = i - 1; j >= 0; j--)
		{	pivotval = a [j] [i];
			a [j] [i] = 0.0;
			rhs [j] -= pivotval * rhs [i];
		}
	}
}


void refocus_points (
	e_refocus_functions ref_type,
	int n_functions,
	bool refocus_values,
	int n_points,
	int n_tri,
	tri_int_corners *corners,
	double *x,
	double *y,
	double *z,
	double *coeff)
{   int itri;
	int icorner;
	int ipt;
	double f_mat [max_refocus_distortion_order] [max_refocus_distortion_order];
	double f_rhs [max_refocus_distortion_order];
	int i;
	int j;
	double fw;
	int fi, fj;
	double tri_x_cg [tri_pts], tri_y_cg [tri_pts];
	double xcg, ycg, area, mx, my, mxy;
	double tx [tri_pts], ty [tri_pts];
	double straddle;
	int istrad;

	for (i = 0; i < n_functions; i++)
	{   f_rhs [i] = 0;
		for (j = 0; j < n_functions; j++)
			f_mat [i] [j] = 0;
	}
	load_refocus_functions (ref_type, n_functions, n_points, x, y, tmp_function_vals);
	for (itri = 0; itri < n_tri; itri++) {
		tri_int (itri, corners, x, y, tri_x_cg, tri_y_cg, &xcg, &ycg, &area, &mx, &my, &mxy);
		for (icorner = 0; icorner < tri_pts; icorner++) {
			ipt = corners [itri] [icorner];
			tx [icorner] = x [ipt];
			ty [icorner] = y [ipt];
		}
		calc_straddle (tx, ty, &istrad, &straddle);

		fw = area * straddle;

		for (icorner = 0; icorner < tri_pts; icorner++) {
			ipt = corners [itri] [icorner];
			for (fi = 0; fi < n_functions; fi++)
			{   f_rhs [fi] += fw * z [ipt] * tmp_function_vals [fi] [ipt];
				for (fj = 0; fj < n_functions; fj++) {
					f_mat [fi] [fj] += fw * tmp_function_vals [fi] [ipt] * tmp_function_vals [fj] [ipt];
				}
			}
		}
	}


	solve_matrix (f_mat, f_rhs, n_functions);
	for (i = 0; i < n_functions; i++) {
		coeff [i] = f_rhs [i];
	}
	if (refocus_values) {
		for (ipt = 0; ipt < n_points; ipt++) {
			for (i = 0; i < n_functions; i++) {
				z [ipt] -= f_rhs [i] * tmp_function_vals [i] [ipt];
			}
		}
	}
}

void calculate_zernike (void)
{
		refocus_points (e_refocus_zernike, zernike_order, false, plate_n_points, plate_n_triangles,
			plate_triangle_points, plate_point_x, plate_point_y, plate_z_displacement, zernike_coeff);
}

void refocus_mirror (void)
{	double coeff [4];

	if (refocus_xyr_flag) {
		refocus_points (e_refocus_xy_parab, 4, true, plate_n_points, plate_n_triangles,
			plate_triangle_points, plate_point_x, plate_point_y, plate_z_displacement, coeff);

	} else {
		refocus_points (e_refocus_parab, 2, true, plate_n_points, plate_n_triangles,
			plate_triangle_points, plate_point_x, plate_point_y, plate_z_displacement, coeff);
	}
}


void refocus_z88_mirror (void)
{	double coeff [4];

	if (refocus_xyr_flag) {
		refocus_points (e_refocus_xy_parab, 4, true, z88_plot_n_points, z88_plot_n_triangles,
			z88_plot_triangle_points, z88_plot_point_x, z88_plot_point_y, z88_plot_z_displacement, coeff);

	} else {
		refocus_points (e_refocus_parab, 2, true, z88_plot_n_points, z88_plot_n_triangles,
			z88_plot_triangle_points, z88_plot_point_x, z88_plot_point_y, z88_plot_z_displacement, coeff);
	}
}

void interpolate_tris (void)
{	int i;

	for (i = 0; i < plate_n_triangles; i++)
		find_plate_xy_interp_coeff (i);
}

void copy_plot_data (void)
{   int i;
    int j;
#ifdef gui_plop
	char str [100];

    LockPlopData (9);
    sprintf (str, "start copy_plop %g %g\n", plate_plot_z_displacement [0], plate_z_displacement [0]);
    DebugMsg (str);
#endif

    plate_plot_n_points = plate_n_points;
    plate_plot_n_triangles = plate_n_triangles;
    for (i = 0; i < plate_n_points; i++)
    {   plate_plot_point_x [i] = plate_point_x [i];
        plate_plot_point_y [i] = plate_point_y [i];
        plate_plot_z_displacement [i] = plate_z_displacement [i];
    }
	for (i = 0; i < plate_n_triangles; i++)
    {   for (j = 0; j < tri_pts; j++)
        {   plate_plot_triangle_points [i] [j] = plate_triangle_points [i] [j];
        }
        plate_plot_tri_slope_x [i] = tri_slope_x [i];
        plate_plot_tri_slope_y [i] = tri_slope_y [i];
    }
#ifdef gui_plop
    UnlockPlopData ();
    DebugMsg ("end copy_plop\n");
#endif
}

void load_z88_deflection_data (int n_mesh_depth)
{   int i;
	int j;
	FILE *def_file;
	int inode;
	double dx, dy, dz;
	double x, y;
#ifdef gui_plop
	char str [100];

	LockPlopData (9);
	sprintf (str, "start load_z88 %g %g\n", plate_plot_z_displacement [0], plate_z_displacement [0]);
	DebugMsg (str);
#endif

	def_file = fopen (z88_deflection_file_name, "rb");
	fgets (str, 100, def_file);
	fgets (str, 100, def_file);
	fgets (str, 100, def_file);
	fgets (str, 100, def_file);
	fgets (str, 100, def_file);

	z88_plot_n_points = total_mesh_points;
	z88_plot_n_triangles = n_triangles;
	for (i = 0; i < total_mesh_points; i++)
	{	polar_to_euc (point_radius [i], point_angle [i], &x, &y);
		z88_plot_point_x [i] = x;
		z88_plot_point_y [i] = y;
	}
	for (i = 0; i < plate_n_triangles; i++)
	{   for (j = 0; j < tri_pts; j++)
		{   z88_plot_triangle_points [i] [j] = triangle_points [i] [j];
		}
	}
	while (fscanf (def_file, " %d %lg %lg %lg", &inode, &dx, &dy, &dz) == 4) {
		inode -= total_mesh_points * (n_mesh_depth - 1) + 1;
		if (inode >= 0 && inode < total_mesh_points) {
			z88_plot_z_displacement [inode] = -dz;
		}
	}
	fclose (def_file);

	if (refocus_flag) {
		refocus_z88_mirror ();
	}
	for (i = 0; i < plate_n_triangles; i++) {
		find_xy_interp_coeff (z88_plot_triangle_points [i],
			z88_plot_point_x, z88_plot_point_y, z88_plot_z_displacement,
			&(z88_plot_tri_slope_x [i]),&(z88_plot_tri_slope_y [i]));
	}

	calc_z88_rms ();

#ifdef gui_plop
	UnlockPlopData ();
	DebugMsg ("end copy_plop\n");
#endif
}

void calculate_plot (int plot_cont,
	int plot_n_points,
	int plot_n_triangles,
	double *plot_z_displacement,
	double *plot_point_x,
	double *plot_point_y,
	tri_int_corners *plot_triangle_points,
	double *plot_tri_slope_x,
	double *plot_tri_slope_y,
	double *plot_z_min,
	double *plot_z_max,
	double z_range,
	unsigned char pic [] [graph_xy_max],
	int pic_size)
{	int i;
	int j;

	double xpoint_min;
	double xpoint_max;
	double ypoint_min;
	double ypoint_max;
	double z_min;
	double z_max;
	double z_mid;
	double pic_x_min;
	double pic_y_min;
	double pixel_delta;
	double pixel_delta_sub;
	double tx_min;
	double tx_max;
	double ty_min;
	double ty_max;
	double pic_span;
	int pixel_xmin;
	int pixel_xmax;
	int pixel_ymin;
	int pixel_ymax;
	int xp;
	int yp;
	int xp_sub;
	int yp_sub;
	double u;
	double v;
	double xpt;
	double ypt;
	double z;
	double z_delta;
	int pixel;

	int line_spans_y [tri_pts];
	double line_intersect_x [tri_pts];
	int p0;
	int p1;
	int left_count;

	double contour_size;
	double contour_thresh;

    int cmap_trunc;

#ifdef gui_plop
    char str [100];

	sprintf (str, "start calc_plot %g\n", plot_z_displacement [0]);
    DebugMsg (str);
#endif
    cmap_trunc = (color_map_size - 1) / (n_colors - 1);

	if (plot_cont)
	{   for (i = 0; i < pic_size; i++)
        {	for (j = 0; j < pic_size; j++)
            {	pic [i] [j] = n_contour_colors - 1;
            }
        }
    }
    else
    {   for (i = 0; i < pic_size; i++)
        {	for (j = 0; j < pic_size; j++)
            {	pic [i] [j] = color_white_index;
            }
        }
    }

	xpoint_min = 1e10;
	xpoint_max = -1e10;
	ypoint_min = 1e10;
	ypoint_max = -1e10;
	z_min = 1e10;
	z_max = -1e10;


	/* find out the bounds of the picture in x, y, and z */

	for (i = 0; i < plot_n_points; i++)
	{	xpoint_min = min (xpoint_min, plot_point_x [i]);
		xpoint_max = max (xpoint_max, plot_point_x [i]);
		ypoint_min = min (ypoint_min, plot_point_y [i]);
		ypoint_max = max (ypoint_max, plot_point_y [i]);
		z_min = min (z_min, plot_z_displacement [i]);
		z_max = max (z_max, plot_z_displacement [i]);
	}
	z_mid = (z_min + z_max) * 0.5;
	*plot_z_min = z_min;
	*plot_z_max = z_max;
	pic_span = max (xpoint_max - xpoint_min, ypoint_max - ypoint_min) * 1.1;
	pic_x_min = (xpoint_max + xpoint_min - pic_span) * .5;
	pic_y_min = (ypoint_max + ypoint_min - pic_span) * .5;
	pixel_delta = pic_span / pic_size;
#ifdef gui_plop

    sprintf (str, "calc_plot z range %g %g\n", z_min, z_max);
    DebugMsg (str);
#endif

	if (z_range != 0)
	{   z_delta = (color_map_size - 1) / z_range;
		*plot_z_max = z_mid + z_range * 0.5;
		*plot_z_min = z_mid - z_range * 0.5;
    }
	else
		z_delta = (color_map_size - 1) / (z_max - z_min);

	if (plot_cont)
	{	if (contour_space != 0)
			contour_size = contour_space;
		else
			contour_size = (z_max - z_min) / n_contours;
		contour_thresh = contour_thresh_ratio * contour_size;
	}
	/* now scan each triangle and draw it */

	for (i = 0; i < plot_n_triangles; i++)
	{	/* find a bounding box of the triangle */
		tx_min = 1e10;
		tx_max = -1e10;
		ty_min = 1e10;
		ty_max = -1e10;
		for (j = 0; j < tri_pts; j++)
		{	tx_min = min (tx_min, plot_point_x [plot_triangle_points [i] [j]]);
			tx_max = max (tx_max, plot_point_x [plot_triangle_points [i] [j]]);
			ty_min = min (ty_min, plot_point_y [plot_triangle_points [i] [j]]);
			ty_max = max (ty_max, plot_point_y [plot_triangle_points [i] [j]]);
		}
		pixel_xmin = floor ((tx_min - pic_x_min) / pixel_delta);
		pixel_xmax = ceil ((tx_max - pic_x_min) / pixel_delta) + 1;
		pixel_ymin = floor ((ty_min - pic_y_min) / pixel_delta);
		pixel_ymax = ceil ((ty_max - pic_y_min) / pixel_delta) + 1;


		if (plot_cont)
		{	pixel_delta_sub = pixel_delta / n_contour_sub_sample;
			for (yp = pixel_ymin; yp < pixel_ymax; yp++)
			{	for (xp = pixel_xmin; xp < pixel_xmax; xp++)
				{	// note later code smashes xpt and ypt so caclualte here
                	ypt = pic_y_min + pixel_delta * yp;
					pixel = 0;
					/*
					 * calculate x-position of intersection of each edge for this y value,
					 * to use in determining if inside the triangle.
					 */

					xpt = pic_x_min + pixel_delta * xp;
					for (j = 0; j < tri_pts; j++)
					{	p0 = plot_triangle_points [i] [j];
						p1 = plot_triangle_points [i] [(j + 1) % tri_pts];
						line_spans_y [j] = 1;
		
						/*
						 * important to use < and >= below so a pair of lines ending in this
						 * y is only counted once
						 */

						if (max (plot_point_y [p0], plot_point_y [p1]) < ypt)
							line_spans_y [j] = 0;
						if (min (plot_point_y [p0], plot_point_y [p1]) >= ypt)
								line_spans_y [j] = 0;
						if (line_spans_y [j])
							line_intersect_x [j] = plot_point_x [p0] + (ypt - plot_point_y [p0]) *
								(plot_point_x [p1] - plot_point_x [p0]) / (plot_point_y [p1] - plot_point_y [p0]);
					}
					left_count = 0;
					for (j = 0; j < tri_pts; j++)
					{	if (line_spans_y [j] && line_intersect_x [j] < xpt)
							left_count++;
					}
					/* test the pixel, interpolate for it */
					if (left_count == 1)
					{	for (yp_sub = 0; yp_sub < n_contour_sub_sample; yp_sub++)
						{	ypt = pic_y_min + pixel_delta * yp + yp_sub * pixel_delta_sub;
							v = ypt - plot_point_y [plot_triangle_points [i] [0]];
							{	for (xp_sub = 0; xp_sub < n_contour_sub_sample; xp_sub++)
								{	xpt = pic_x_min + pixel_delta * xp + pixel_delta_sub * xp_sub;
									u = xpt - plot_point_x [plot_triangle_points [i] [0]];
									z = u * plot_tri_slope_x [i] + v * plot_tri_slope_y [i] + plot_z_displacement [plot_triangle_points [i] [0]];
									z = fmod (z - z_min, contour_size);
									if (z < contour_thresh)
										pixel++;
								}
							}
						}
						if (pixel > 0)
						{
							pixel = n_contour_colors - 1;
                        }
                        pic [pic_size - yp - 1] [xp] = pixel;
                        // test for clearing
						// pic [pic_size - yp - 1] [xp] = 0;
					}
				}
			}
		}
		else
		{	for (yp = pixel_ymin; yp < pixel_ymax; yp++)
			{	ypt = pic_y_min + pixel_delta * yp;
				v = ypt - plot_point_y [plot_triangle_points [i] [0]];

				/*
				 * calculate x-position of intersection of each edge for this y value,
				 * to use in determining if inside the triangle.
				 */

				for (j = 0; j < tri_pts; j++)
				{	p0 = plot_triangle_points [i] [j];
					p1 = plot_triangle_points [i] [(j + 1) % tri_pts];
					line_spans_y [j] = 1;

					/*
					 * important to use < and >= below so a pair of lines ending in this
					 * y is only counted once
					 */
                    /*
					 * Completely wrong answer if the test for line_spans_y is not made when
                     * running under windows. Fucks up for horizontal line, which causes divide by 0.
					 * This is bogus, since it should cause an Infinity which is never used because
					 * of later test of line_spans_y. So we include the test before calculating
					 * line_intersect_x.
					 */
					if (max (plot_point_y [p0], plot_point_y [p1]) < ypt)
						line_spans_y [j] = 0;
					if (min (plot_point_y [p0], plot_point_y [p1]) >= ypt)
						line_spans_y [j] = 0;
					if (line_spans_y [j])
                    	line_intersect_x [j] = plot_point_x [p0] + (ypt - plot_point_y [p0]) *
							(plot_point_x [p1] - plot_point_x [p0]) / (plot_point_y [p1] - plot_point_y [p0]);
				}
				for (xp = pixel_xmin; xp < pixel_xmax; xp++)
				{	xpt = pic_x_min + pixel_delta * xp;
					u = xpt - plot_point_x [plot_triangle_points [i] [0]];
					z = u * plot_tri_slope_x [i] + v * plot_tri_slope_y [i] + plot_z_displacement [plot_triangle_points [i] [0]];
					left_count = 0;
					for (j = 0; j < tri_pts; j++)
					{	if (line_spans_y [j] && line_intersect_x [j] < xpt)
							left_count++;
					}
					if (left_count == 1)
					{	pixel = (color_map_size / 2) + (z - z_mid) * z_delta;
						pixel = pixel / cmap_trunc;
						pixel = pixel * cmap_trunc;
						if (pixel < 0)
							pixel = 0;
						else if (pixel >= color_map_size - 1)
							pixel = color_map_size - 1;
						pic [pic_size - yp - 1] [xp] = pixel;
						// test for full fill
						// pic [pic_size - yp - 1] [xp] = color_black_index;
					}
				}
			}
		}
	}
#ifdef gui_plop
	sprintf (str, "end calc_plot %d\n", pic [pic_size / 2] [pic_size / 2]);
	DebugMsg (str);
#endif
}

void calculate_plop_plot (int plot_cont, double *pic_z_min, double *pic_z_max,
	double *err_rms, double *err_pv) {

	calculate_plot (plot_cont,
		plate_plot_n_points,
		plate_plot_n_triangles,
		plate_plot_z_displacement,
		plate_plot_point_x,
		plate_plot_point_y,
		plate_plot_triangle_points,
		plate_plot_tri_slope_x,
		plate_plot_tri_slope_y,
		&plate_plot_z_min,
		&plate_plot_z_max,
		pic_z_range,
		picture,
		pic_size);

	*pic_z_min = plate_plot_z_min;
	*pic_z_max = plate_plot_z_max;
	*err_rms = plate_err_vis_rms;
	*err_pv = plate_err_vis_p_v;
}


void calculate_z88_plot (int plot_cont, double *pic_z_min, double *pic_z_max,
	double *err_rms, double *err_pv) {

	calculate_plot (plot_cont,
		z88_plot_n_points,
		z88_plot_n_triangles,
		z88_plot_z_displacement,
		z88_plot_point_x,
		z88_plot_point_y,
		z88_plot_triangle_points,
		z88_plot_tri_slope_x,
		z88_plot_tri_slope_y,
		&z88_plot_z_min,
		&z88_plot_z_max,
		pic_z_range,
		picture,
		pic_size);


	*pic_z_min = z88_plot_z_min;
	*pic_z_max = z88_plot_z_max;
	*err_rms = z88_plot_err_vis_rms;
	*err_pv = z88_plot_err_vis_p_v;
}


#ifdef use_gd_graphics

void write_picture (
	char *pic_name,
    plop_cmap *pcm)
{
	int i;
	int j;
    int gd_color_table [max_colors];

	FILE *pfile;
    char const *fmode;

    gd_image = gdImageCreate (pic_size, pic_size);
    for (i = 0; i < pcm->n_colors; i++)
    {	gd_color_table [i] = gdImageColorAllocate(gd_image, pcm->color_r [i],
    		 pcm->color_g [i], pcm->color_b [i]);
    }


	fmode = "w";
	if ((pfile = fopen (pic_name, fmode)) == (FILE *) NULL)
	{	fprintf (stderr, "can't open %s\n", pic_name);
		exit (1);
	}
    for (i = 0; i < pic_size; i++)
    {	for (j = 0; j < pic_size; j++)
    	{	gdImageSetPixel (gd_image, i, j, picture [j] [i]);
        }
    }

	gdImageGif (gd_image, pfile);
	fclose (pfile);
}


#else


void write_picture (
	char *pic_name)
{
	int i;
	int j;

	FILE *pfile;
    char *fmode;

#ifdef __BORLANDC__
	fmode = "wb";
#else
	fmode = "w";
#endif
	if ((pfile = fopen (pic_name, fmode)) == (FILE *) NULL)
	{	fprintf (stderr, "can't open %s\n", pic_name);
		exit (1);
	}
#ifdef ppm_p3_format
	fprintf (pfile, "P3 %d %d %d\n", pic_size, pic_size, color_max_inten);

	for (i = 0; i < pic_size; i++)
	{	for (j = 0; j < pic_size; j++)
		{	fprintf (pfile, "%d %d %d\n",
				pcm->color_r [picture [i] [j]],
				pcm->color_g [picture [i] [j]],
				pcm->color_b [picture [i] [j]]);
		}
	}
#else
	fprintf (pfile, "P6 %d %d %d\n", pic_size, pic_size, color_max_inten);
	for (i = 0; i < pic_size; i++)
	{	for (j = 0; j < pic_size; j++)
		{	fputc (pcm->color_r [picture [i] [j]], pfile);
			fputc (pcm->color_g [picture [i] [j]], pfile);
			fputc (pcm->color_b [picture [i] [j]], pfile);
		}
	}
#endif
	fclose (pfile);
}
#endif


#ifdef standalone
main (
	int argc,
	char **argv)
{	
	refocus_flag = 0;
	contour_flag = 0;

 	alloc_plate_globals ();

	init_global_graphics ();

	obstruction_radius = 0;
	while (argc > 1 && argv [1] [0] == '-')
	{	switch (argv [1] [1])
		{	case 'c':
				contour_flag = 1;
				if (argv [1] [2] == 'n')
				{	sscanf (argv [2], "%d", &n_contours);
					argc--;
					argv++;
				}
				break;

			case 'n':
				sscanf (argv [2], "%d", &n_colors);
				argc--;
				argv++;
				break;

			case 'o':
				sscanf (argv [2], "%lg", &obstruction_radius);
				argc--;
				argv++;
				break;

			case 'r':
				refocus_flag = 1;
				break;

			case 's':
				sscanf (argv [2], "%d", &pic_size);
				if (pic_size > graph_xy_max)
				{	fprintf (stderr, "-s option too big\n");
					exit (1);
				}
				argc--;
				argv++;

			case 'x':
				mirror_x = 1;
				break;
		}
		argc--;
		argv++;
	}
	if (argc < 2)
	{	fprintf (stderr, "plot: usage: plot datafile picture\n");
		exit (2);
	}
	init_graphics ();

	read_surface (argv [1]);

	if (mirror_x)
		reflect_x ();

	interpolate_tris ();

	calc_rms ();
	printf ("raw rms error = %g visible rms = %g\n", plate_err_rms, plate_err_vis_rms);
	printf ("raw p-v error = %lg, visble p-v = %lg\n", plate_err_p_v, plate_err_vis_p_v);

    if (calculate_zernike_flag)
        calculate_zernike ();

	if (refocus_flag)
	{	refocus_mirror ();
		printf ("refocus a = %lg b = %lg\n", plate_refocus_a0, plate_refocus_a2);
		interpolate_tris ();
		calc_rms ();
		printf ("refocused rms error = %g visible rms = %g\n", plate_err_rms, plate_err_vis_rms);
		printf ("refocused p-v error = %lg, visible p-v = %lg\n", plate_err_p_v, plate_err_vis_p_v);
	}

	if (argc > 2)
	{	calculate_plop_plot (0, &plate_plot_z_min, &plate_plot_z_max);

		write_picture (argv [2]);
	}
}
#endif




