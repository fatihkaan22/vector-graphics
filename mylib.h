#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

typedef enum {empty, function, polyline, tree, snowflake} figure_type;

typedef struct {
	char color[15];
	char fill[15];
	double r;
	double g;
	double b;
} Color;

typedef struct {
	double x1;
	double x2;
	double y1;
	double y2;
	Color c;
	double thickness; 
} Axis;

typedef struct {
	double x;
	double y;
} Point;

typedef struct {
	figure_type type;
	Point * points;
	double shift_x;
	double shift_y;
	int n;
	double resolution;
} Point2D;

typedef struct {
	Point * joint_points;		//center of circles
	int depth;
	int n;
	struct nodePosition * head;	//holds the x and y coordinates of joint points of tree
} Tree_figure;

typedef struct node {
	int key;
	struct node *left;
	struct node *right;
}Tree;

typedef struct figure{
	figure_type type;
	double width, height;
	Axis xAxis, yAxis; 
	//function parameters
	Color c;	
	double thickness;		//stroke-width
	double resolution;	//how much to increment x at each step 
	double *functionPoints;
	double start_x, end_x;
	Point2D poly_line;
	double viewBox[4];				// svg
	double translate_scale[4];		//eps
	Point start_roi, end_roi;
	struct figure *figNext;		//to append figures
	Tree_figure tree;
	Tree tree_numbers;			//holds the user data
} Figure;


struct nodePosition {		// it holds x and y coordinates of each leaf
	double x;
	double y;
	struct nodePosition *left;
	struct nodePosition *right;
};

// additional functions for drawing binary tree
void print_lines_svg(struct nodePosition * node, Tree * root, FILE * fptr);
void print_circles_svg(struct nodePosition * node, Tree * root, FILE * fptr); 
void print_lines_eps(struct nodePosition * node, Tree * root, FILE * fptr);
void print_circles_eps(struct nodePosition * node, Tree * root, FILE * fptr); 

void hex_to_rgb(Color * color){
	int r, g, b;
	sscanf((*color).color, "%02x%02x%02x", &r, &g, &b);
	(*color).r = (double)r/255;
	(*color).g = (double)g/255;
	(*color).b = (double)b/255;
	return;
}

Figure start_figure(double width, double height){
	// INITILIZATION AND DEFALUTS
	Figure fig;
	fig.width = width;
	fig.height = height;
	fig.xAxis.x1 = width/2.0; 
	fig.xAxis.x2 = width/2.0; 
	fig.xAxis.y1 = 0; 
	fig.xAxis.y2 = height; 
	fig.yAxis.x1 = 0; 
	fig.yAxis.x2 = width; 
	fig.yAxis.y1 = height/2.0; 
	fig.yAxis.y2 = height/2.0; 
	strcpy(fig.xAxis.c.color, "0000ff");
	strcpy(fig.yAxis.c.color, "0000ff");
	fig.xAxis.thickness = 1;
	fig.yAxis.thickness = 1;

	fig.poly_line.points = NULL;
	fig.poly_line.shift_x = width/2;
	fig.poly_line.shift_y = height/2;

	fig.type = empty;
	fig.poly_line.type = empty;
	fig.thickness=1;

	fig.viewBox[0] = 0;
	fig.viewBox[1] = 0;
	fig.viewBox[2] = width;
	fig.viewBox[3] = height;

	fig.translate_scale[0] = 0;
	fig.translate_scale[1] = 0;
	fig.translate_scale[2] = 1;
	fig.translate_scale[3] = 1;

	fig.start_roi.x = 0;
	fig.start_roi.y = 0;
	fig.end_roi.x = width;
	fig.end_roi.y = height;

	fig.functionPoints = NULL;
	fig.tree.joint_points = NULL;
	fig.tree.head = NULL;

	fig.figNext = NULL;

	return fig;
}

void set_thickness_resolution(Figure * fig, double thickness, double resolution){
	(*fig).thickness=thickness;
	(*fig).resolution=resolution;
	(*fig).poly_line.resolution=resolution;
	return;
}

void set_color(Figure * fig, char * c){
	strcpy((*fig).c.color, c); 
	strcpy((*fig).c.fill, "none"); 
	return;
}

void draw_fx(Figure * fig, double (*f)(double x), double start_x, double end_x){
	int pointCount=1;
	double y=0;

	(*fig).type = function;

	if (start_x<-(*fig).width/2){
		printf("Start x is not in range of canvas. Start position is shifted.\n");
		start_x = -(*fig).width/2+(*fig).thickness*2;
	}
	if (end_x>(*fig).width/2){
		printf("End x is not in range of canvas. End position is shifted.\n");
		end_x = (*fig).width/2-(*fig).thickness*2;
	}
		
	(*fig).start_x = start_x;
	(*fig).end_x = end_x;

	(*fig).functionPoints = (double *) malloc(sizeof(double) * 2);

	while(start_x<=end_x){
		y = f(start_x);
		if (y<=(*fig).height/2 && y>=-(*fig).height/2){
			(*fig).functionPoints[pointCount] = start_x + (*fig).width/2;
			(*fig).functionPoints[pointCount+1] = -(y) + (*fig).height/2;
			pointCount+=2;
			(*fig).functionPoints = (double *) realloc((*fig).functionPoints, sizeof(double) * (pointCount+1));
		}
		start_x += (*fig).resolution; 
	}

	(*fig).functionPoints[0] = pointCount-2;

	return;
}

void export_svg(Figure fig, char * file_name){
	FILE *fptr;
	char svgFormat[] = "<\?xml version=\"1.0\" standalone=\"no\"\?>\n";
	int x=0;
	int i;
	int j;
	int checkFig2=0;
	Figure *figP;
	Figure *tmp;

	//check if there is two figures or not
	if (fig.figNext != NULL) {
		tmp = &fig;
		while (tmp->figNext) {
			tmp=tmp->figNext;
			checkFig2++;
		}
	}

	fig.xAxis.thickness = fig.thickness;
	fig.yAxis.thickness = fig.thickness;

	fptr = fopen(file_name,"w+");
	if (fptr==NULL){
		printf("Error to open file.");
	}

	fprintf(fptr, "%s\n", svgFormat);
	
	//canvas
	fprintf(fptr, "<svg width='%f' height='%f' version='1.1' xmlns='http://www.w3.org/2000/svg'>\n", fig.width, fig.height);

	//resize defs
	fprintf(fptr, "<defs>\n");
	fprintf(fptr, "\t<clipPath id='resize'>\n");
	fprintf(fptr, "\t<rect x='%f' y='%f' width='%f' height='%f'/>\n",fig.start_roi.x, fig.start_roi.y, fig.end_roi.x, fig.end_roi.y);
	fprintf(fptr, "\t</clipPath>\n");
	fprintf(fptr, "</defs>\n");
	
	figP = &fig;

	for (j = 0; j <= checkFig2 ; j++) {

		if (figP->type == function) {

			//x axis
			fprintf(fptr, "\t<line x1='%f' x2='%f' y1='%f' y2='%f' stroke='#%s' stroke-width='%f'/>\n", figP->xAxis.x1, figP->xAxis.x2, figP->xAxis.y1, figP->xAxis.y2, figP->xAxis.c.color, figP->xAxis.thickness);
				
			//y axis
			fprintf(fptr, "\t<line x1='%f' x2='%f' y1='%f' y2='%f' stroke='#%s' stroke-width='%f'/>\n", figP->yAxis.x1, figP->yAxis.x2, figP->yAxis.y1, figP->yAxis.y2, figP->yAxis.c.color, figP->yAxis.thickness);

			//function of x
			fprintf(fptr, "\t<polyline points='");
			for(x=1; x<=(figP->functionPoints[0]); x+=2){
				fprintf(fptr, "%f %f, ", figP->functionPoints[x], figP->functionPoints[x+1]);
			}
			fprintf(fptr, "' stroke='#%s' fill='%s' stroke-width='%f'/>\n", figP->c.color, figP->c.fill, figP->thickness);

			//dash
			fprintf(fptr, "\t<line x1='%f' x2='%f' y1='%f' y2='%f' stroke='#ffa500' stroke-dasharray='0.5' stroke-width='%f'/>\n", figP->start_x+figP->width/2, figP->start_x+figP->width/2, figP->functionPoints[2], figP->height/2, figP->xAxis.thickness);
			fprintf(fptr, "\t<line x1='%f' x2='%f' y1='%f' y2='%f' stroke='#ffa500' stroke-dasharray='0.5' stroke-width='%f'/>\n", figP->end_x+figP->width/2, figP->end_x+figP->width/2, figP->functionPoints[x-1], figP->height/2, figP->xAxis.thickness);

			//start - end x
			fprintf(fptr,"\t<circle r='%f' cx='%f' cy='%f' fill='#%s'/>\n" , figP->xAxis.thickness*2, figP->width/2+figP->start_x, figP->height/2, figP->c.color);
			fprintf(fptr,"\t<circle r='%f' cx='%f' cy='%f' fill='#%s'/>\n" , figP->xAxis.thickness*2, figP->width/2+figP->end_x, figP->height/2, figP->c.color);

		}

		if (figP->poly_line.type == polyline) {
			//x axis
			fprintf(fptr, "\t<line x1='%f' x2='%f' y1='%f' y2='%f' stroke='#%s' stroke-width='%f'/>\n", figP->xAxis.x1, figP->xAxis.x2, figP->xAxis.y1, figP->xAxis.y2, figP->xAxis.c.color, figP->xAxis.thickness);
			
			//y axis
			fprintf(fptr, "\t<line x1='%f' x2='%f' y1='%f' y2='%f' stroke='#%s' stroke-width='%f'/>\n", figP->yAxis.x1, figP->yAxis.x2, figP->yAxis.y1, figP->yAxis.y2, figP->yAxis.c.color, figP->yAxis.thickness);

			fprintf(fptr, "\t<svg preserveAspectRatio='none' viewBox='%f %f %f %f'>\n", figP->viewBox[0], figP->viewBox[1], figP->viewBox[2], figP->viewBox[3]);
			fprintf(fptr, "\t<polyline points='");
			for(i=0; i< figP->poly_line.n ; i++){
				fprintf(fptr, "%f %f, ", figP->poly_line.points[i].x, figP->poly_line.points[i].y);
			}
			fprintf(fptr, "' stroke='#%s' fill='%s' stroke-width='%f' clip-path='url(#resize)'/>\n", figP->c.color, figP->c.fill, figP->thickness);
		fprintf(fptr, "\t%s\n", "</svg>");

		}
		if (figP->figNext != NULL) {
			figP = figP->figNext;
		}
	}

		if (figP->type == tree) {
			print_lines_svg(figP->tree.head, &figP->tree_numbers, fptr);
			print_circles_svg(figP->tree.head, &figP->tree_numbers , fptr);

		}
	//end of file
	fprintf(fptr, "%s\n", "</svg>");
	fclose(fptr);
	printf("%s created.\n", file_name);
}

void export_eps(Figure fig, char * file_name){
	FILE *fptr;
	char epsFormat[] = "\%!PS-Adobe-3.0 EPSF-3.0";
	int x=0;
	int i;
	int j;
	int checkFig2=0;
	Figure *figP;
	Figure *tmp;

	figP = &fig;
	tmp = &fig;

	//check if there is two figures or not
	if (fig.figNext != NULL) {
		tmp = &fig;
		while (tmp) {
			tmp=tmp->figNext;
			checkFig2++;
		}
	}

	fig.xAxis.thickness = fig.thickness;
	fig.yAxis.thickness = fig.thickness;

	fptr = fopen(file_name,"w+");
	if (fptr==NULL){
		printf("Error to open file.");
	}

	fprintf(fptr, "%s\n", epsFormat);
	
	//canvas
	fprintf(fptr, "%%%%BoundingBox: 0 -%f %f 0\n", fig.height, fig.width);

	if (fig.poly_line.type == polyline) {
			//define clip-path
			fprintf(fptr, "\n/clip-path {\n");
			fprintf(fptr, "newpath\n");
			fprintf(fptr, "%f -%f moveto\n", figP->start_roi.x, figP->start_roi.y);
			fprintf(fptr, "%f -%f lineto\n", figP->start_roi.x + figP->end_roi.x, figP->start_roi.y);
			fprintf(fptr, "%f -%f lineto\n", figP->start_roi.x + figP->end_roi.x, figP->start_roi.y + figP->end_roi.y);
			fprintf(fptr, "%f -%f lineto\n", figP->start_roi.x, figP->start_roi.y + figP->end_roi.y);
			fprintf(fptr, "closepath\n");
			fprintf(fptr, "} def\n");
	}

	for (j = 0; j <= checkFig2 ; j++) {
		
		if (figP->type == function) {

			//x axis
			hex_to_rgb(&figP->xAxis.c);

			fprintf(fptr, "\nnewpath\n%f %f moveto\n%f -%f lineto\n%f %f %f setrgbcolor\n%f setlinewidth\nstroke\n", figP->xAxis.x1, figP->xAxis.y1, figP->xAxis.x2, figP->xAxis.y2, figP->xAxis.c.r, figP->xAxis.c.g, figP->xAxis.c.b, figP->xAxis.thickness);

			//y axis

			hex_to_rgb(&figP->yAxis.c);

			fprintf(fptr, "\nnewpath\n%f -%f moveto\n%f -%f lineto\n%f %f %f setrgbcolor\n%f setlinewidth\nstroke\n", figP->yAxis.x1, figP->yAxis.y1, figP->yAxis.x2, figP->yAxis.y2, figP->yAxis.c.r, figP->yAxis.c.g, figP->yAxis.c.b, figP->yAxis.thickness);


			//function of x

			hex_to_rgb(&figP->c);

			fprintf(fptr, "\nnewpath\n");
			fprintf(fptr, "%f -%f moveto\n", figP->functionPoints[1], figP->functionPoints[2]);
			for(x=3; x<=(figP->functionPoints[0]); x+=2){
				fprintf(fptr, "%f -%f lineto\n", figP->functionPoints[x], figP->functionPoints[x+1]);
			}
			fprintf(fptr, "%f %f %f setrgbcolor \n%f setlinewidth\nstroke\n", figP->c.r, figP->c.g, figP->c.b, figP->thickness);


			//dash
			fprintf(fptr, "\nnewpath\n%f -%f moveto\n%f -%f lineto\n1 0.647 0 setrgbcolor\n%f setlinewidth\n[0.5] 0 setdash\nstroke\n", figP->start_x+figP->width/2, figP->functionPoints[2], figP->start_x+figP->width/2, figP->height/2, figP->xAxis.thickness);

			fprintf(fptr, "\nnewpath\n%f -%f moveto\n%f -%f lineto\n1 0.647 0 setrgbcolor\n%f setlinewidth\n[0.5] 0 setdash\nstroke\n", figP->end_x+figP->width/2, figP->functionPoints[x-1], figP->end_x+figP->width/2, figP->height/2, figP->xAxis.thickness);

			//start - end x
			fprintf(fptr,"\nnewpath\n1 setlinecap\n%f -%f moveto\n0 0 rlineto\n%f setlinewidth\n%f %f %f setrgbcolor\nstroke\n" ,figP->width/2+figP->start_x, figP->height/2, figP->xAxis.thickness*3, figP->c.r, figP->c.g, figP->c.b);
			fprintf(fptr,"\nnewpath\n1 setlinecap\n%f -%f moveto\n0 0 rlineto\n%f setlinewidth\n%f %f %f setrgbcolor\nstroke\n" ,figP->width/2+figP->end_x, figP->height/2, figP->xAxis.thickness*3, figP->c.r, figP->c.g, figP->c.b);

		}

		if (figP->poly_line.type == polyline) {

			//x axis
			hex_to_rgb(&figP->xAxis.c);

			fprintf(fptr, "\nnewpath\n%f %f moveto\n%f -%f lineto\n%f %f %f setrgbcolor\n%f setlinewidth\nstroke\n", figP->xAxis.x1, figP->xAxis.y1, figP->xAxis.x2, figP->xAxis.y2, figP->xAxis.c.r, figP->xAxis.c.g, figP->xAxis.c.b, figP->xAxis.thickness);
			
			//y axis

			hex_to_rgb(&figP->yAxis.c);

			fprintf(fptr, "\nnewpath\n%f -%f moveto\n%f -%f lineto\n%f %f %f setrgbcolor\n%f setlinewidth\nstroke\n", figP->yAxis.x1, figP->yAxis.y1, figP->yAxis.x2, figP->yAxis.y2, figP->yAxis.c.r, figP->yAxis.c.g, figP->yAxis.c.b, figP->yAxis.thickness);

			hex_to_rgb(&figP->c);

			if (figP->type != snowflake) {
				fprintf(fptr, "\n-%f %f translate\n", figP->translate_scale[0], figP->translate_scale[1]);
				fprintf(fptr, "%f %f scale\n", figP->translate_scale[2], figP->translate_scale[3]);
			}
			
			fprintf(fptr, "\nclip-path clip");
			fprintf(fptr, "\nnewpath\n");
			fprintf(fptr, "%f -%f moveto\n", figP->poly_line.points[0].x, figP->poly_line.points[0].y);
			for(i=1; i<figP->poly_line.n; i++){
				fprintf(fptr, "%f -%f lineto\n", figP->poly_line.points[i].x, figP->poly_line.points[i].y);
			}
			fprintf(fptr, "%f %f %f setrgbcolor \n%f setlinewidth\nstroke\n", figP->c.r, figP->c.g, figP->c.b, figP->thickness);
		}
		if (figP->figNext != NULL) {
			figP = figP->figNext;
		}

		if (figP->type == tree) {
			fprintf(fptr, "\n\nnewpath\n");
			print_lines_eps(figP->tree.head, &figP->tree_numbers, fptr);
			fprintf(fptr, "1 setlinewidth\n");
			fprintf(fptr, "stroke\n\n");

			fprintf(fptr, "/monospace findfont\n");
			fprintf(fptr, "15 scalefont\n");
			fprintf(fptr, "setfont\n");

			print_circles_eps(figP->tree.head, &figP->tree_numbers , fptr);
		}
	}

	//end of file
	fprintf(fptr, "\nshowpage\n%%%%EOF" );

	fclose(fptr);
	printf("%s created.\n", file_name);
}

void draw_polyline (Point2D * poly_line, int n) {
	poly_line->n = n;
	n--;
	poly_line->type = polyline;
	while (n>=0) {
		poly_line->points[n].x += poly_line->shift_x;
		poly_line->points[n].y += poly_line->shift_y;
		n--;
	}
	return;
}

void draw_circle (Point2D * center, double r) {
	double length = (2*M_PI*r);
	double n = length/center->resolution;
	int i;
	double angle, angle_increment;

	center->shift_x += center->points->x;
	center->shift_y -= center->points->y;

	center->points = (Point *) malloc((n+1) * sizeof(Point));
	center->n = (int)n;
	center->type = polyline;

	angle = 0;
	angle_increment = 2*M_PI/center->n;

	for (i = 0; i < center->n; ++i) {
		center->points[i].x = center->shift_x + r * sin(angle);
		center->points[i].y = center->shift_y + r * cos(angle);
		angle += angle_increment;
	}
	center->points[i].x = center->points[0].x;
	center->points[i].y = center->points[0].y;
	center->n ++;
	
	return;
}

void draw_ellipse (Point2D * center, double r_max, double r_min) {
	double length = (2*M_PI*(r_max+r_min)/2);
	double n = length/center->resolution;
	double angle ;
	double angle_increment;
	int i;

	center->shift_x += center->points->x;
	center->shift_y -= center->points->y;

	center->points = (Point *) malloc((n+1) * sizeof(Point));
	center->n = (int)n;
	center->type = polyline;

	angle = 0;
	angle_increment = 2*M_PI/center->n;

	for (i = 0; i < center->n; ++i) {
		center->points[i].x = center->shift_x + r_max * sin(angle);
		center->points[i].y = center->shift_y + r_min * cos(angle);
		angle += angle_increment;
	}
	center->points[i].x = center->points[0].x;
	center->points[i].y = center->points[0].y;
	center->n ++;
	
	return;
}

void scale_figure (Figure * fig , double scale_x, double scale_y) {
	// It should be shifted (x/2 - x/2k, y/2 - y/2k) 
	fig->viewBox[0] = fig->width/2 - fig->width/(2*scale_x);
	fig->viewBox[1] = fig->height/2 - fig->height/(2*scale_y);
	fig->viewBox[2] = fig->width/scale_x;
	fig->viewBox[3] = fig->height/scale_y;

	fig->translate_scale[0] = fig->viewBox[0] * scale_x; 
	fig->translate_scale[1] = fig->viewBox[1] * scale_y;
	fig->translate_scale[2] = scale_x;
	fig->translate_scale[3] = scale_y;

	return;
}

void resize_figure (Figure *fig, Point start_roi, Point end_roi) {
	fig->start_roi.x = start_roi.x + fig->width/2;
	fig->start_roi.y = fig->height/2 - end_roi.y;

	fig->end_roi.x = fabs(end_roi.x-start_roi.x);
	fig->end_roi.y = fabs(end_roi.y-start_roi.y);
	return;
}

void rearange_width_height (Figure * fig, double width, double height) {
	fig->xAxis.x1 = width/2.0; 
	fig->xAxis.x2 = width/2.0; 
	fig->xAxis.y1 = 0; 
	fig->xAxis.y2 = height; 
	fig->yAxis.x1 = 0; 
	fig->yAxis.x2 = width; 
	fig->yAxis.y1 = height/2.0; 
	fig->yAxis.y2 = height/2.0; 

	fig->poly_line.shift_x = width/2;
	fig->poly_line.shift_y = height/2;

	fig->viewBox[2] = width;
	fig->viewBox[3] = height;

	fig->end_roi.x = width;
	fig->end_roi.y = height;
	return;
}

void append_figures(Figure *fig1, Figure *figNext) {
	//take the larger width and height

	if (fig1->width > figNext->width) figNext->width = fig1->width;
	else fig1->width = figNext->width;

	if (fig1->height > figNext->height) figNext->height = fig1->height;
	else fig1->height = figNext->height;

	rearange_width_height(fig1, fig1->width, fig1->height);
	rearange_width_height(figNext, figNext->width, figNext->height);

	fig1->figNext = (Figure *) malloc(sizeof(Figure));
	fig1->figNext = figNext;
	return;
}

Point position(int index, int depth, int maxDepth, double width, double height) {
	Point point;
	point.x = index * (width/(pow(2,depth)+1));
	point.y = depth*height/maxDepth + 25; 			//by default radius of circles are 20 units.
	return point;
}


struct nodePosition* new_node(double x, double y) { 
    struct nodePosition* node = (struct nodePosition*)malloc(sizeof(struct nodePosition)); 
    node->x = x; 
    node->y = y; 
    node->left = node->right = NULL; 
    return (node); 
} 

struct nodePosition* insert_level_order (Point arr[], struct nodePosition* root, int i, int n) { 
    if (i < n) { 
        struct nodePosition* temp = new_node(arr[i].x, arr[i].y); 
        root = temp; 
  
        root->left = insert_level_order (arr, root->left, 2*i + 1, n); 
  
        root->right = insert_level_order (arr, root->right, 2*i + 2, n); 
    }
    return root; 
}

void print_lines_svg (struct nodePosition * node, Tree * root, FILE * fptr) { 
    if (node->right == NULL) return; 
    if (node->left == NULL) return;
	if (root == NULL) return;

	print_lines_svg(node->left, root->left, fptr); 
  
	if (root->left != NULL) 
	fprintf(fptr, "\t<line x1='%f' y1='%f' x2='%f' y2='%f' stroke='black'/>\n", node->x, node->y, node->left->x, node->left->y);

	if (root->right != NULL) 
	fprintf(fptr, "\t<line x1='%f' y1='%f' x2='%f' y2='%f' stroke='black'/>\n", node->x, node->y, node->right->x, node->right->y);
  
    print_lines_svg(node->right, root->right, fptr); 
}

void print_circles_svg (struct nodePosition * node, Tree * root, FILE * fptr) { 
    if (node == NULL) return; 
    if (root == NULL) return; 

    print_circles_svg(node->left, root->left, fptr); 
  
	fprintf(fptr, "\t<circle r='20' cx='%f' cy='%f' fill='white' stroke='black'/>\n",node->x, node->y);

	if (root->key > 99) fprintf(fptr, "\t<text x='%f' y='%f' fill='black'>%d</text>\n", node->x-11, node->y+5, root->key);
	else if (root->key > 9) fprintf(fptr, "\t<text x='%f' y='%f' fill='black'>%d</text>\n", node->x-8, node->y+5, root->key);
	else if (root->key >= 0) fprintf(fptr, "\t<text x='%f' y='%f' fill='black'>%d</text>\n", node->x-4, node->y+5, root->key);
 
    print_circles_svg(node->right, root->right, fptr); 
}

void print_lines_eps (struct nodePosition * node, Tree * root, FILE * fptr) { 
    if (node->right == NULL) return; 
    if (node->left == NULL) return;
	if (root == NULL) return;

    print_lines_eps(node->left, root->left, fptr); 
  
	if (root->left != NULL) {
		fprintf(fptr, "%f -%f moveto \n", node->x, node->y);
		fprintf(fptr, "%f -%f lineto \n",node->left->x, node->left->y);
	}

	if (root->right != NULL) {
		fprintf(fptr, "%f -%f moveto \n", node->x, node->y);
		fprintf(fptr, "%f -%f lineto \n",node->right->x, node->right->y);
	}
  
    print_lines_eps(node->right, root->right, fptr); 
}

void print_circles_eps (struct nodePosition * node, Tree * root, FILE * fptr) { 
    if (node == NULL) return; 
    if (root == NULL) return; 

    print_circles_eps(node->left, root->left, fptr); 
  
	fprintf(fptr, "%f -%f 20 0 360 arc closepath gsave 1 setgray fill grestore stroke\n",node->x, node->y);

	if (root->key > 99) fprintf(fptr, "%f -%f moveto (%d) show stroke\n", node->x-12, node->y+5, root->key);
	else if (root->key > 9) fprintf(fptr, "%f -%f moveto (%d) show stroke\n", node->x-9, node->y+5, root->key);
	else if (root->key >= 0) fprintf(fptr, "%f -%f moveto (%d) show stroke\n", node->x-5, node->y+5, root->key);
 
    print_circles_eps(node->right, root->right, fptr); 
}

void calculate_n_depth (struct node* node, int *depth, int *n, int  i) { 
    if (node == NULL) return; 
	else *n += 1;

	if (i> *depth) {
		*depth = i;
	}

    calculate_n_depth(node->left, depth, n, i+1); 
    calculate_n_depth(node->right, depth, n, i+1); 
}

void draw_binary_tree(Tree * root, Figure * fig) {
	int maxIndex, maxDepth=0;
	int n = 0;
	int k=0, i,j; 

	fig->type = tree;

	calculate_n_depth(root, &maxDepth, &n, 0);
	maxDepth++;
	fig->tree.n = pow(2,maxDepth);

	fig->tree.joint_points = (Point *) malloc(fig->tree.n * sizeof(Point));

	fig->tree_numbers = *root;

	for (j = 0; j < maxDepth; ++j) {
		maxIndex = pow(2,j);
		for (i = 1; i <= maxIndex; ++i) {
			fig->tree.joint_points[k].x = position(i, j, maxDepth, fig->width, fig->height).x;
			fig->tree.joint_points[k].y = position(i, j, maxDepth, fig->width, fig->height).y;
			k++;
		}
	}

	fig->tree.head = insert_level_order (fig->tree.joint_points, fig->tree.head, 0, fig->tree.n); 

	return;
}

void get_a_pattern (Point a, Point e, Point * arr, double size) {
	double angle;
	double slope = (e.y-a.y)/(e.x-a.x);
	arr[0]=a;	/*startind point*/
	arr[4]=e;	/*end point*/

	angle = atan(slope);
 
	if (a.x > e.x) angle += (M_PI/180.0)*(180);

	arr[1].x = arr[0].x + size * cos(angle);
	arr[1].y = arr[0].y + size * sin(angle);

	arr[3].x = arr[0].x + 2*size * cos(angle);
	arr[3].y = arr[0].y + 2*size * sin(angle);

	angle -= (M_PI/180.0)*(60);

	arr[2].x = arr[1].x + size * cos(angle);
	arr[2].y = arr[1].y + size * sin(angle);

	return;
}

int end (int t) {
	int i;
	int sum=0;

	for (i = 0; i < t; ++i) sum += pow(4,i);

	return sum;
}

int k_end (int t) {
	int i;
	int r = 5;
	int sum=0;
		for (i = 0; i < t; ++i) {
			sum += r;
			r = r*4;
		}
	return sum;
}

void draw_koch_snowflake (Figure * fig, Point center, double thickness, double size, int num_iterations) {
	double angle = (M_PI/180.0)*(60);
	int i=0;
	Point *arr;
	double size_tmp = size;
	int k=0, t, x, j, m;
	Figure * figP=fig;

	fig->poly_line.shift_x += center.x;
	fig->poly_line.shift_y -= center.y;

	arr = (Point*) malloc(k_end(num_iterations)*sizeof(Point));

	fig->figNext = (Figure *) malloc(sizeof(Figure));
	fig->figNext->figNext = (Figure *) malloc(sizeof(Figure));

	for (m = 0; m < 3; ++m) {	//for each side of triangle
		size = size_tmp ;

		if (m==0) { 		//top
			arr[0].x = 0;
			arr[0].y = 0;

			arr[4].x = size;
			arr[4].y = 0;
		}

		if (m==1) {			//right
			arr[0].x = size;
			arr[0].y = 0;

			arr[4].x = size*cos(angle);
			arr[4].y = size*sin(angle);
		}

		if (m==2) {			// left
			arr[0].x = size*cos(angle);
			arr[0].y = size*sin(angle);

			arr[4].x = 0;
			arr[4].y = 0;
		}

		i = 0;
		k = 0;
		for (t = 0; t < num_iterations ; t++) {
			size = size/3.0;
			x = k_end(t-1);

			for ( ; k < end(t+1) ; k++) {
				if (t!=0) {
					if ((x+1)%5==0) {
						x++;
					}
					arr[i] = arr[x];
					arr[i+4] = arr[x+1];
					x++;
				}
				get_a_pattern (arr[i],arr[i+4], &arr[i], size);
				i+=5;
			}
		}

		figP->type = snowflake;
		figP->poly_line.type = polyline;
		figP->thickness = thickness;
		set_color(figP, fig->c.color);
		strcpy(figP->c.fill, "none");

		figP->poly_line.points = (Point*) malloc((k_end(num_iterations)-k_end(num_iterations-1))*sizeof(Point));
		figP->poly_line.n = k_end(num_iterations)-k_end(num_iterations-1);

		j=0;
		for (i = k_end(t-1); i < k_end(t) ; i++) {
			figP->poly_line.points[j].x = arr[i].x + fig->poly_line.shift_x - size_tmp/2 * cos(0), 
			figP->poly_line.points[j].y = arr[i].y + fig->poly_line.shift_y - size_tmp/2 * cos(angle);
			j++;
		}

		figP = figP->figNext;
	}

	free(arr);
}

void free_tree(Tree * head) {
	if (head) return;

	free_tree(head->left);
	free_tree(head->right);
	free (head);

	return;
}

void free_node(struct nodePosition * head) {
	if (head == NULL) return;

	free_node(head->left);
	free_node(head->right);

	free (head);

	return;
}

void free_tree_fig (Tree_figure * fig) {
	if (fig->joint_points) free(fig->joint_points);
	free_node(fig->head);
	return;
}

void free_figure (Figure * fig) {
	Figure * tmp;

	while (fig) {
		free(fig->functionPoints);
		if (fig->type == snowflake) free(fig->poly_line.points);
		free_tree_fig(&fig->tree);
		free_tree(&fig->tree_numbers);

		fig = fig->figNext;
	}

	while(fig) {
		tmp = fig;
		fig = fig->figNext;
		free(tmp);
	}
}
