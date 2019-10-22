#include"../mylib.h"

	/* ELLIPSE */

int main() {
	Figure fig;
	fig = start_figure(200, 200);
	set_thickness_resolution(&fig, 0.1, 1);
	set_color(&fig, "ff0000");

	Point center[1] = {{0,0}};
	fig.poly_line.points = center;

	draw_ellipse(&fig.poly_line, 50, 30);

	export_svg(fig, "ellipse.svg");
	export_eps(fig, "ellipse.eps");
	free_figure(&fig);
	return 0;
}
