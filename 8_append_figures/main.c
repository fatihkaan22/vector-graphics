#include"../mylib.h"

int main() {
	Figure fig1;
	Figure fig2;
	Figure fig3;

	Point center[1] = {{0,0}};

	fig1 = start_figure(300, 500);
	set_thickness_resolution(&fig1, 1, 1);
	set_color(&fig1, "ff0000");
	fig1.poly_line.points = center;

	draw_circle(&fig1.poly_line, 20);

	fig2 = start_figure(600, 200);
	fig2.poly_line.points = center;
	set_thickness_resolution(&fig2, 3, 1);
	set_color(&fig2, "00ff00");

	draw_ellipse(&fig2.poly_line, 80, 40);

	fig3 = start_figure(300, 200);
	fig3.poly_line.points = center;
	set_thickness_resolution(&fig3, 1, 20);
	set_color(&fig3, "000f0f");

	draw_ellipse(&fig3.poly_line, 20, 50);

	append_figures(&fig1, &fig2);
	append_figures(&fig2, &fig3);

	export_svg(fig1, "append.svg");
	export_eps(fig1, "append.eps");
	free_figure(&fig1);
	free_figure(&fig2);
	return 0;
}

