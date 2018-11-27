#ifndef plotH
#define plotH
#include "global.h"

#define max_colors			256

#define color_map_size      200

#define graph_xy_max		1024

#define n_contour_sub_sample	3
#define n_contour_colors		2

#define contour_thresh_ratio	.1

#define default_pic_size	500
#define default_n_colors	200
#define default_n_contours	10
#define max_contours        50

#define default_max_inten	255

typedef enum {e_refocus_parab, e_refocus_xy_parab, e_refocus_zernike} e_refocus_functions;

typedef struct {
	int n_colors;
	int color_r [max_colors];
	int color_g [max_colors];
	int color_b [max_colors];
	} plop_cmap;


void load_z88_deflection_data (int n_mesh_depth);

void solve_matrix (
	double a [max_refocus_distortion_order] [max_refocus_distortion_order],
	double rhs [max_refocus_distortion_order],
	int n);


void calculate_plop_plot (int, double *z_range_min, double *z_range_max, double *err_rms, double *err_pv);

void calculate_z88_plot (int, double *z_range_min, double *z_range_max, double *err_rms, double *err_pv);
void write_picture (char *pic_name, plop_cmap *pcm);

#endif
