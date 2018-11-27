#ifndef xdcl_once
#include "global.h"
#define xdcl_once
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
	double *mxy_p);

xdcl_once int total_mem_alloced;

xdcl_once int max_points;
xdcl_once int max_triangles;

xdcl_once int max_quads;

xdcl_once int max_sparse_els;

xdcl_once double *plate_point_x;        /* [max_points] */
xdcl_once double *plate_point_y;        /* [max_points] */

xdcl_once double *plate_forces;         /* [max_points] */
xdcl_once int *plate_force_points;      /* [max_points] */

xdcl_once double *plate_basis_forces [max_basis_tests];
xdcl_once int *plate_basis_force_points [max_basis_tests];

xdcl_once tri_int_corners *plate_triangle_points;   /*[max_triangles]  [tri_pts]; */

xdcl_once int plate_n_points;
xdcl_once int plate_n_triangles;
xdcl_once int plate_n_constraints;
xdcl_once int plate_n_forces;

xdcl_once int z88_plot_n_points;
xdcl_once int z88_plot_n_triangles;
xdcl_once int z88_plot_n_constraints;
xdcl_once int z88_plot_n_forces;

xdcl_once int *plate_basis_n_forces;

xdcl_once int plate_matrix_size;

xdcl_once double plate_modulus;
xdcl_once double plate_poisson;

xdcl_once double *plate_thickness;		    /* [max_triangle] */
xdcl_once double *plate_tri_pressure;	    /* [max_triangles */
xdcl_once double *plate_area;				/* [max_triangles] */
xdcl_once double *plate_mx;				    /* [max_triangles] */
xdcl_once double *plate_my;				    /* [max_triangles] */
xdcl_once double *plate_mxy;				/* [max_triangles] */
xdcl_once tri_dbl_corners *plate_tri_x_cg;  /* [max_triangles] [tri_pts];	*/ 	/* distance from x to x_cg */
xdcl_once tri_dbl_corners *plate_tri_y_cg;  /* [max_triangles] [tri_pts];  */
xdcl_once double *plate_xcg;				/* [max_triangles] */		/* distance from x to x_cg */
xdcl_once double *plate_ycg;				/* [max_triangles] */

xdcl_once double *plate_rhs;			    /* [max_eqn] */
xdcl_once double *plate_result;			    /* [max_eqn] */

xdcl_once double *plate_basis_rhs  [max_basis_tests];

xdcl_once double *plate_z_displacement;	/* [max_points] */

xdcl_once double plate_shear;

xdcl_once int plate_constraint_point [max_constraint];
xdcl_once int plate_constraint_code [max_constraint];

xdcl_once double plate_refocus_a0;
xdcl_once double plate_refocus_a2;

xdcl_once double plate_parab_a2;
xdcl_once double plate_focal_length;
xdcl_once double plate_refocus_focal_length;

xdcl_once double plate_err_rms;
xdcl_once double plate_err_vis_rms;
xdcl_once double plate_err_p_v;
xdcl_once double plate_err_vis_p_v;

xdcl_once double z88_plot_err_rms;
xdcl_once double z88_plot_err_vis_rms;
xdcl_once double z88_plot_err_p_v;
xdcl_once double z88_plot_err_vis_p_v;

xdcl_once int plate_n_basis_tests;

xdcl_once int warp_sphere_flag;
xdcl_once int refocus_flag;
xdcl_once int refocus_xyr_flag;
xdcl_once int calculate_zernike_flag;
xdcl_once int zernike_order;
xdcl_once int generate_z88_input_flag;

xdcl_once double zernike_coeff [max_refocus_distortion_order];

xdcl_once double fact_table [max_refocus_distortion_order];

xdcl_once int pic_size;
xdcl_once int n_colors;
xdcl_once int n_contours;

xdcl_once double pic_z_range;
xdcl_once double contour_space;


xdcl_once double radius;

xdcl_once double obstruction_radius;
xdcl_once double obstruction_diam;
xdcl_once double rel_obstruction_radius;

// copy of data for plotting in gui version
// the solver will update the plate_ version and gui accesses the plate_plot_
// version. need to get the lock on plop data to update plate_plot_ version.

xdcl_once double *tmp_function_vals [max_refocus_distortion_order];

xdcl_once int plate_plot_n_points;
xdcl_once int plate_plot_n_triangles;

xdcl_once double *plate_plot_point_x;        /* [max_points] */
xdcl_once double *plate_plot_point_y;        /* [max_points] */

xdcl_once double *plate_plot_z_displacement;	/* [max_points] */

xdcl_once double plate_plot_z_min;
xdcl_once double plate_plot_z_max;

xdcl_once tri_int_corners *plate_plot_triangle_points;   /*[max_triangles]  [tri_pts]; */

xdcl_once double tri_fract_inv;
xdcl_once double tri_fract_sq_inv;

// copy of z88 data for plotting in gui version

xdcl_once int z88_input_files_valid;

xdcl_once double *z88_plot_point_x;        /* [max_points] */
xdcl_once double *z88_plot_point_y;        /* [max_points] */

xdcl_once double *z88_plot_z_displacement;	/* [max_points] */

xdcl_once double z88_plot_z_min;
xdcl_once double z88_plot_z_max;

xdcl_once tri_int_corners *z88_plot_triangle_points;   /*[max_triangles]  [tri_pts]; */



