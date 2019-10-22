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

	export_svg(fig, "circle.svg");
	export_eps(fig, "circle.eps");
	free_figure(&fig);
	return 0;
}

