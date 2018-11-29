#ifndef globalH
#define globalH

#define gui_plop
#define use_sparse
#define use_gd_graphics

//#define DEBUGGING_ENVARS
//#define log_malloc
//#define obsolete
//#define solve_by_col
//#define ppm_p3_format
//#define standalone
//#define foo
//#define foooo
//#define debug
//#define debug_ainv_middle
//#define debug_detailed
//#define standalone_plate
//#define DebugLocale
//#define richardgetfiledir

#define M_PI 3.14159265358979323846264338327

#define z88_struct_file_name_base "z88i1.txt"
#define z88_load_file_name_base "z88i2.txt"
#define z88_deflection_file_name_base "z88o2.txt"

#define z88i1_bin_name "\\z88i1plop.exe"
#define z88i2_bin_name "\\z88i2plop.exe"


#define max_line_len	200

#define max_plop_file_len   30000

#define max_comments        100
#define max_comments_len    15000

#define default_max_points		5000

#define max_constraint	60

#define degrees				360		/* process angles in degrees to avoid roundoff */

#define tri_pts			3
#define n_deg_free		3			/* number of degrees of freedom in this universe */
#define dim_x			0
#define dim_y			1
#define dim_z			2

#define quad_pts		4			/* numer of points in quadrilateral */
#define tetra_pts		8			/* number of points in a tetra */


#define constraint_z		100
#define constraint_theta_x	10
#define constraint_theta_y	1

typedef int tri_int_corners [tri_pts];
typedef double tri_dbl_corners [tri_pts];

typedef int quad_int_corners [quad_pts];

typedef double dvec [n_deg_free];

#define max_eqn			(max_points * n_deg_free)

#define mat_index(point,deg)	((point) * n_deg_free + (deg))

#define tri_deg_free	(tri_pts * n_deg_free)		/* number of degrees of freedom of a triangle */

#define band_size			250


#define tri_fract		1			/* num linear pieces to fracture triangle for rms error */
#define tri_fract_sq	(tri_fract * tri_fract)

#define max_basis_size	20		/* max number of independent support configurations used as basis */
#define max_mesh_radii	100		/* maximum number of radii of points */
#define max_mesh_depth	20

#define max_points_per_ring	30	/* not presently checked; we could croak on this */


#define max_basis_tests	(max_basis_size * max_mesh_radii)   /* should actually use and check max supports on a mesh ring.*/
#define max_components	(max_basis_tests * 4)		/* upper bound, not guaranteed unless above is valid */

#define max_parm_names		50
                                                   
#define max_variables		100

#define max_parts           50

#define max_mesh_perim		1000	/* maximum n of points around each circle */

#define max_refocus_distortion_order    100


#define maxwords			100
#define wordlen				100

#define parm_type_int		0
#define parm_type_double	1
#define parm_type_flag		2

#define max_support_radii	50

#define max_opt_vars		100		/* max number of parms to optimize */
#define max_scan_vars		100		/* max number of parms to scan */
#define max_scan_set_vars	100		/* max number of scan-set vars */
#define max_monte_vars		100
#define max_scan_set		50		/* max number in set of values to scan; must be <= maxwords */


#define grid_gen_never		0			/* only once, actually, but never again! */
#define grid_gen_basis		1			/* generate a basis and use it */

#define part_type_triangle        0
#define part_type_bar             1

#define part_point_ring           0
#define part_point_part           1

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

extern int total_mem_alloced;

extern int max_points;
extern int max_triangles;

extern int max_quads;

extern int max_sparse_els;

extern double *plate_point_x;        /* [max_points] */
extern double *plate_point_y;        /* [max_points] */

extern double *plate_forces;         /* [max_points] */
extern int *plate_force_points;      /* [max_points] */

extern double *plate_basis_forces [max_basis_tests];
extern int *plate_basis_force_points [max_basis_tests];

extern tri_int_corners *plate_triangle_points;   /*[max_triangles]  [tri_pts]; */

extern int plate_n_points;
extern int plate_n_triangles;
extern int plate_n_constraints;
extern int plate_n_forces;

extern int z88_plot_n_points;
extern int z88_plot_n_triangles;
extern int z88_plot_n_constraints;
extern int z88_plot_n_forces;

extern int *plate_basis_n_forces;

extern int plate_matrix_size;

extern double plate_modulus;
extern double plate_poisson;

extern double *plate_thickness;		    /* [max_triangle] */
extern double *plate_tri_pressure;	    /* [max_triangles */
extern double *plate_area;				/* [max_triangles] */
extern double *plate_mx;				    /* [max_triangles] */
extern double *plate_my;				    /* [max_triangles] */
extern double *plate_mxy;				/* [max_triangles] */
extern tri_dbl_corners *plate_tri_x_cg;  /* [max_triangles] [tri_pts];	*/ 	/* distance from x to x_cg */
extern tri_dbl_corners *plate_tri_y_cg;  /* [max_triangles] [tri_pts];  */
extern double *plate_xcg;				/* [max_triangles] */		/* distance from x to x_cg */
extern double *plate_ycg;				/* [max_triangles] */

extern double *plate_rhs;			    /* [max_eqn] */
extern double *plate_result;			    /* [max_eqn] */

extern double *plate_basis_rhs  [max_basis_tests];

extern double *plate_z_displacement;	/* [max_points] */

extern double plate_shear;

extern int plate_constraint_point [max_constraint];
extern int plate_constraint_code [max_constraint];

extern double plate_refocus_a0;
extern double plate_refocus_a2;

extern double plate_parab_a2;
extern double plate_focal_length;
extern double plate_refocus_focal_length;

extern double plate_err_rms;
extern double plate_err_vis_rms;
extern double plate_err_p_v;
extern double plate_err_vis_p_v;

extern double z88_plot_err_rms;
extern double z88_plot_err_vis_rms;
extern double z88_plot_err_p_v;
extern double z88_plot_err_vis_p_v;

extern int plate_n_basis_tests;

extern int warp_sphere_flag;
extern int refocus_flag;
extern int refocus_xyr_flag;
extern int calculate_zernike_flag;
extern int zernike_order;
extern int generate_z88_input_flag;

extern double zernike_coeff [max_refocus_distortion_order];

extern double fact_table [max_refocus_distortion_order];

extern int pic_size;
extern int n_colors;
extern int n_contours;

extern double pic_z_range;
extern double contour_space;


extern double radius;

extern double obstruction_radius;
extern double obstruction_diam;
extern double rel_obstruction_radius;

// copy of data for plotting in gui version
// the solver will update the plate_ version and gui accesses the plate_plot_
// version. need to get the lock on plop data to update plate_plot_ version.

extern double *tmp_function_vals [max_refocus_distortion_order];

extern int plate_plot_n_points;
extern int plate_plot_n_triangles;

extern double *plate_plot_point_x;        /* [max_points] */
extern double *plate_plot_point_y;        /* [max_points] */

extern double *plate_plot_z_displacement;	/* [max_points] */

extern double plate_plot_z_min;
extern double plate_plot_z_max;

extern tri_int_corners *plate_plot_triangle_points;   /*[max_triangles]  [tri_pts]; */

extern double tri_fract_inv;
extern double tri_fract_sq_inv;

// copy of z88 data for plotting in gui version

extern int z88_input_files_valid;

extern double *z88_plot_point_x;        /* [max_points] */
extern double *z88_plot_point_y;        /* [max_points] */

extern double *z88_plot_z_displacement;	/* [max_points] */

extern double z88_plot_z_min;
extern double z88_plot_z_max;

extern tri_int_corners *z88_plot_triangle_points;   /*[max_triangles]  [tri_pts]; */

#endif
