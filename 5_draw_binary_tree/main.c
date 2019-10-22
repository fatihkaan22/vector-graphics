#include"../mylib.h"

struct node *newNode (int item) { 
    struct node *temp =  (struct node *)malloc(sizeof(struct node)); 
    temp->key = item; 
    temp->left = temp->right = NULL; 
    return temp; 
}

struct node* insert(struct node* node, int key) 
{ 
    if (node == NULL) return newNode(key); 

    if (key < node->key) 
        node->left  = insert(node->left, key); 
    else if (key > node->key) 
        node->right = insert(node->right, key);    

    return node; 
} 


int main() {
	Figure fig;
	Tree * root;

	fig = start_figure(2000, 500);

    root = NULL; 

	root = insert(root, 20);
	insert(root, 10);
	insert(root, 30);
	insert(root, 5);
	insert(root, 15);
	insert(root, 25);
	insert(root, 35);
	insert(root, 999);
	insert(root, 1);
	insert(root, 16);
	insert(root, 24);
	insert(root, 17);
	insert(root, 23);
	insert(root, 13);
	insert(root, 3);
	insert(root, 1);
	insert(root, 7);
	insert(root, 6);
	insert(root, 8);
	insert(root, 0);

	draw_binary_tree(root, &fig);

	export_svg(fig, "binary-tree.svg");
	export_eps(fig, "binary-tree.eps");
	free_figure(&fig);
	return 0;
}

