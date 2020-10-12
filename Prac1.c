#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

struct tree {
	char *word;
	int count;
	struct tree *l_node;
	struct tree *r_node;
};

struct list {
	char *word;
	int count;
	struct list *next;
};

typedef struct tree Tree;
typedef struct list List;

char *get_word(FILE *inp) {
	int space = 1;
	int not_str = 1;
	int size = 5;
	int i;
	char *word = (char *) malloc(size * sizeof(char));
	char c;
	for (i = 0; c = fgetc(inp); i++) {
		if (i + 1 >= size) {
			size *= 2;
			word = (char *) realloc(word, size * sizeof(char));
		}
		if (isalnum(c)) {
			word[i] = c;
			not_str = 0;
			continue;
		} else if (isspace(c)) {
			word[i] = '\0';
			break;
		} else if (ispunct(c) && !not_str) {
			ungetc(c, inp);
			word[i] = '\0';
			break;
		} else if (ispunct(c) && not_str) {
			word[i] = c;
			word[i + 1] = '\0';
			break; 	
		} 
	}	 
	while (space) {
		if (isspace(c=fgetc(inp)) || iscntrl(c)) {
			; //skip spaces
		} else {
			ungetc(c, inp);
			space = 0;
		}
	}

	return word;
}

Tree *add_to_tree(Tree *tree, char *word) {
	if (tree == NULL) {
		tree = (Tree *) malloc(sizeof(Tree));
		tree -> word = word;
		tree -> count = 1;
		tree -> l_node = NULL;
		tree -> r_node = NULL;
		
	} else {
		int cmp_res = strcmp(word, tree -> word);
		if (!cmp_res) {
			tree -> count += 1;
			free(word);
		} else if (cmp_res > 0)	{	// word > tree -> word 
			tree -> r_node = add_to_tree(tree -> r_node, word);
		} else if (cmp_res < 0) {	// word < tree -> word
			tree -> l_node = add_to_tree(tree -> l_node, word);
		}
	}

	return tree;
}	

List *add_to_list(Tree *tree, List *list) {
	if (list == NULL) {
		list = (List *) malloc(sizeof(List));
		list -> next = NULL;
		list -> word = tree -> word;
		list -> count = tree -> count;
	} else if (tree -> count < list -> count) {
		list -> next = add_to_list(tree, list -> next);
	} else {
		List *help_elem = (List *) malloc(sizeof(List));
		help_elem -> word = tree -> word;
		help_elem -> count = tree -> count;
		help_elem -> next = list;
		list = help_elem;
	}
	return list;
		
}

List *ctl(Tree *tree, List *list) {	//change tree to list
	if (tree != NULL) {
		list = ctl(tree -> l_node, list);
		list = ctl(tree -> r_node, list);
		list = add_to_list(tree, list);
	}
	
	return list;
}

void print_res(List *main_list, int all_words, FILE *out) {
	if (main_list != NULL) {
		double frequency = (double) main_list -> count / (double) all_words;
		fprintf(out, "%s %d %f\n", main_list -> word, main_list -> count, frequency);
		print_res(main_list -> next, all_words, out);
		free(main_list);
	}
}

void clear_tree(Tree *tree) {
	if (tree != NULL) {
		clear_tree(tree -> l_node);
		clear_tree(tree -> r_node);
		free(tree -> word);
		free(tree -> l_node);
		free(tree -> r_node);
	}
}
int main(int argc, char **argv) {
	FILE *inp = stdin, *out = stdout;
	int i;
	if (argc > 1) {
		for (i = 1; i <= argc; i++) {
			if ((i + 1 <= argc) && !strcmp(argv[i], "-i")) {
				inp = fopen(argv[i + 1], "r");
				if (inp == NULL) {
					fprintf(stderr, "Can't open file for reading\n");
					exit(1);
				}
			}
			if ((i + 1 <= argc) && !strcmp(argv[i], "-o")) {
				out = fopen(argv[i + 1], "w");
				if (out == NULL) {
					fprintf(stderr, "Can't open file for writing\n");
					exit(1);
				}
			}
		}
	}
	int all_words = 0;
	char *word = NULL;
	Tree *main_tree = NULL;
	List *main_list = NULL;
	while (!feof(inp)) {
		word = get_word(inp);
		main_tree = add_to_tree(main_tree, word);
		all_words++;
	}
	main_list = ctl(main_tree, main_list);
	print_res(main_list, all_words, out);
	
	clear_tree(main_tree);
	free(main_tree);	
	fclose(inp);
	fclose(out);
	
	return 0;
}
				 
	
