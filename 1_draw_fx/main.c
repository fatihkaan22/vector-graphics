#include"../mylib.h"

double f (double x){
	return (x*x);
}

	/* FUNCTION */

int main() {
	Figure fig;
	fig = start_figure(150, 100);

	set_thickness_resolution(&fig, 0.1, 0.01);
	set_color(&fig, "ff0000");

	draw_fx(&fig, f, -5, 14);

	export_svg(fig, "function.svg");
	export_eps(fig, "function.eps");
	free_figure(&fig);
	return 0;
}
