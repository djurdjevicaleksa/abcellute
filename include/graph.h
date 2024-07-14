#ifndef _GRAPH_H
#define _GRAPH_H

#include <aio.h>
#include <stdlib.h>
#include <assert.h>

#include "constants.h"
#include "table.h"


#define push_stack(stack, node) {           \
    assert((stack)->count < NODE_LIST_SIZE);\
    (stack)->nodes[(stack)->count++] = node;\
}

#define pop_stack(stack) {                  \
    assert((stack)->count > 0);             \
    (stack)->count--;                       \
}

#define MKNode(_row, _col) {                \
    .row = _row,                            \
    .col = _col,                            \
    .count = 0,                             \
    .dependencies = NULL                    \
}


struct Node;

typedef struct Node {
    int row, col;
    struct Node** dependencies;
    size_t count;
} Node;

typedef struct {
    Node* nodes[NODE_LIST_SIZE];
    size_t count;
} NodeList;

typedef NodeList VisitedNodes;
typedef NodeList Recstack;

Node* make_root(int expression_count);
bool was_visited(Node* root, VisitedNodes* visited);
Node* dfs(Node* root, Node* target, VisitedNodes* visited);
Node* find_node(Node* root, Node* target);
Node* alloc_new_node(Node* target);
void add_node(Node* root, Node* source, Node* target);
void handle_expression(Node* root, Node* values, size_t count);
Node* perform_syntax_analysis(Table* table);
bool dfs_cycle(Node* root, VisitedNodes* visited, Recstack* recstack);
void report_cycle(Recstack* recstack);
bool cycles_exist(Node* root);


#endif //_GRAPH_H