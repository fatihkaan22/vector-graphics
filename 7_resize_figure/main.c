#include"../mylib.h"


	/* CIRCLE */

int main() {
	Figure fig;
	fig = start_figure(300, 200);
	set_thickness_resolution(&fig, 1, 1);
	set_color(&fig, "ff0000");

	Point center[1] = {{0,0}};
	fig.poly_line.points = center;

	draw_circle(&fig.poly_line, 20);

	Point start_roi = {-25,0};
	Point end_roi = {0,25};
	resize_figure(&fig, start_roi, end_roi);

	export_svg(fig, "circle_resized.svg");
	export_eps(fig, "circle_resized.eps");
	free_figure(&fig);
	return 0;
}

