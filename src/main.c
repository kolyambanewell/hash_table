//Hello, my name is Nikolay and this is my First project at GitHub

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

struct tree_item {
    struct tree_item *left;
    struct tree_item *right;
    char *key;
    size_t count;
};



struct hash_table {
    size_t size;
    struct tree_item **data;
};


void add_tree_item(struct tree_item **root, const char *key) {
    if(*root == NULL) {
        struct tree_item *new_tree = calloc(1, sizeof(struct tree_item));
        new_tree->key = calloc(strlen(key) + 1, sizeof(char));
        strcpy(new_tree->key, key);
        new_tree->count = 1;
        *root = new_tree;
        return;
    }

    if(strcmp(key, (*root)->key) < 0) {
        add_tree_item(&(*root)->left, key);
    } else if(strcmp(key, (*root)->key) == 0) {
        (*root)->count++;
    } else {
        add_tree_item(&(*root)->right, key);
    }
}

struct tree_item *find(struct tree_item *root, const char *key) {
    if(strcmp(key, root->key) < 0) {
        return find(root->left, key);
    } else if(strcmp(key, root->key) == 0) {
        return root;
    } else {
        return find(root->right, key);
    }
}

struct tree_item *min_tree_item(struct tree_item *root, struct tree_item **param) {
    if(root->left == NULL) return root;

    *param = root;
    return min_tree_item(root->left, param);
}

int delete_tree_item(struct tree_item **root, const char *key) {

    struct tree_item *tmp;
    struct tree_item *min_item;
    struct tree_item *param;

    if(*root == NULL) return 0;

    if(strcmp(key, (*root)->key) < 0) {
        return delete_tree_item(&(*root)->left, key);
    } else if(strcmp(key, (*root)->key) == 0) {
        if((*root)->left == NULL) {
            tmp = *root;
            *root = (*root)->right;
            free(tmp->key);
            free(tmp);
        } else if((*root)->right == NULL) {
            tmp = *root;
            *root = (*root)->left;
            free(tmp->key);
            free(tmp);
        } else {
            tmp = *root;
            param = *root;
            min_item = min_tree_item((*root)->right, &param);
            if(param != *root) {
                param->left = min_item->right;
                min_item->right = (*root)->right;
            }
            min_item->left = (*root)->left;
            *root = min_item;
            free(tmp->key);
            free(tmp);
        }
    } else {
        return delete_tree_item(&(*root)->right, key);
    }

    return 1;
}

void print_tree(const struct tree_item *root) {
    if(root == NULL) {
        return;
    }
    print_tree(root->left);
    printf("%s %lu\n", root->key, root->count);
    print_tree(root->right);
}

void delete_leaf(struct tree_item **root) {
    if(*root == NULL) return;

    delete_leaf(&(*root)->left);
    delete_leaf(&(*root)->right);
    free((*root)->key);
    free(*root);
    *root = NULL;
}

static size_t hash_func(const char *key, size_t tab_size) {
    size_t sum = 0;
    size_t len = strlen(key);

    for (size_t i = 0; i < len; i++) {
        sum += key[i];
    }

    return sum % tab_size;
}

int hash_allocate(struct hash_table *h, size_t size) {
    h->data = calloc(size, sizeof(struct tree_item*));

    if (h->data == NULL) {
        return 0;
    }

    h->size = size;
    return 1;
}

static int hash_fill_item(struct tree_item **it, const char *key) {

    add_tree_item(it, key);
    if (it == NULL) return 0;

    return 1;
}

void hash_free(struct hash_table *h) {
    for (size_t i = 0; i < h->size; i++) {
        delete_leaf(&h->data[i]);
    }

    free(h->data);
}

struct tree_item **hash_add(struct hash_table *h, const char *key) {
    size_t loc = hash_func(key, h->size);
    struct tree_item **it = &h->data[loc];


    if (!hash_fill_item(it, key)) {
        return NULL;
    }
    return it;

}

struct tree_item **hash_search(const struct hash_table *h, const char *key) {
    size_t loc = hash_func(key, h->size);


    if (h->data[loc] == NULL) {
        return NULL;
    }

    if (find(h->data[loc], key)) {
        return &h->data[loc];
    }

    return NULL;
}



void hash_print(const struct hash_table *const h) {
    printf("The numbers of word occurrences\n");
    for (size_t i = 0; i < h->size; i++) {
        if (h->data[i] != NULL) {
            print_tree(h->data[i]);

        }
    }
}

static void to_lower(unsigned char *const s) {
    for (size_t i = 0; s[i] != 0; i++) {
        s[i] = tolower(s[i]);
    }
}

int process_line(struct hash_table *h, char *line) {

    char word[257];
    const char *delim = ".,;:-+=~[]{}() \n\r\b\t\"'`@#$^&*!?%<>|\\/"
                        "0123456789";
    char *token = strtok(line, delim);
    struct tree_item **it;

    while (token != NULL) {
        memset(word, 0, sizeof(word));
        strncpy(word, token, sizeof(word) - 1);
        to_lower((unsigned char *const) word);
        it = hash_add(h, word);

        if (it == NULL) {
            fprintf(stderr, "Insufficient memory\n");
            hash_free(h);
            return 0;
        }

        token = strtok(NULL, delim);

    }
    return 1;
}

int main(int argc, char **argv) {
    FILE *input;
    char buffer[257];
    struct hash_table hash_table;

    if (argc < 2) {
        fprintf(stderr, "File path wasn't specified \n");

    }

    if (argc != 2) {
        fprintf(stderr, "Invalid number of arguments\n");

    }

    input = fopen(argv[1], "r");

    if (input == NULL) {
        fprintf(stderr, "Unable to open the file '%s'\n", argv[1]);
    }

    if (!hash_allocate(&hash_table, 256)) {
        fprintf(stderr, "Insufficient memory \n");
    }

    while (fgets(buffer, sizeof(buffer), input) != NULL) {
        if (!process_line(&hash_table, buffer)) {
            fclose(input);
            fprintf(stderr, "Error in reading file\n");
        }
    }
    fclose(input);

    hash_print(&hash_table);
    hash_free(&hash_table);

    return 0;
}
