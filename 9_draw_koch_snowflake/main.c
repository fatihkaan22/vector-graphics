#include"../mylib.h"

int main () {
	Figure fig;
	fig = start_figure(400, 400);
	set_color(&fig, "000000");

	Point center = {0,0};
	draw_koch_snowflake (&fig, center, 0.01, 200, 6);

	export_svg(fig, "snowflake.svg");
	export_eps(fig, "snowflake.eps");
	free_figure(&fig);

	return 0;
}
