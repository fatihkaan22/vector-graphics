#include"../mylib.h"

	/* CIRCLE */

int main() {
	Figure fig;
	fig = start_figure(300, 200);
	set_thickness_resolution(&fig, 0.5, 1);
	set_color(&fig, "ff0000");

	Point center[1] = {{0,0}};
	fig.poly_line.points = center;

	draw_circle(&fig.poly_line, 20);
	scale_figure(&fig,3,2);			/* expands the figure three times along the x axis 
									   and 2 times along the y axis */


	export_svg(fig, "circle_scaled.svg");
	export_eps(fig, "circle_scaled.eps");
	free_figure(&fig);
	return 0;
}

