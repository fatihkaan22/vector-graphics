#include"../mylib.h"

	/* POLYLINE */

int main() {
	Figure fig;
	fig = start_figure(200, 200);
	set_thickness_resolution(&fig, 0.1, 0.01);
	set_color(&fig, "ff0000");

	Point poly_line[4] = {{-30,-10}, {-10,10}, {15,10}, {20,-10}};
	fig.poly_line.points = poly_line;

	draw_polyline(&fig.poly_line, 4);

	export_svg(fig, "polyline.svg");
	export_eps(fig, "polyline.eps");
	free_figure(&fig);
	return 0;
}
