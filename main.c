#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#define _SS_IMPLEMENT
#include "ss.h"

#define DECIMAL_PLACES 2
#define EXTRA_CELL_SPACE 2
#define GRAPH_NODE_BUFFER_SIZE 32           // node buffer of one entire expression contains this many nodes
#define NODE_LIST_SIZE 32                   // node stack contains this many nodes
#define SYNTAX_ERRORS_BUFF_SIZE 2048         // buffer for printing all syntax errors
#define GRAPH_CYCLE_BUFFER_SIZE 256              // buffer for printing a dependency cycle
#define INVALID_DEPENDENCY_BUFFER_SIZE 256  // buffer for printing invalid dependency
#define EQUATION_TOKENS                 16

#define ANSI_RESET      "\x1b[0m"
#define ANSI_RED        "\x1b[31m"
#define ANSI_GREEN      "\x1b[32m"
#define ANSI_BOLD_RED   "\x1b[1;31m"

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

#define EQ_INIT {       \
    .n_first = 0,       \
    .n_second = 0,      \
    .n_third = 0,       \
    .o_first = '\0',    \
    .o_second = '\0'    \
}

int max_cell_width = 0;
int expression_count = 0; //for dynamic allocation of graph nodes later

typedef enum {
    EXPR_DEFAULT = 0,
    EXPR_VALID,
    EXPR_INVALID
} ExprKind;

typedef struct {
    ExprKind kind;
    StringStruct expr;
} Expr;

typedef enum {
    KIND_EMPTY = 0,
    KIND_TEXT,
    KIND_NUM,
    KIND_EXPR
} CellKind;

typedef union {
    StringStruct text;
    double number;
    Expr expression;
} Cell_as;

typedef struct {
    CellKind kind;
    Cell_as as;
} Cell;

typedef struct {
    Cell* cells;
    int rows;
    int cols;
} Table;

char* consume_file(const char* input_file_path, int* count) {


    char* buffer = NULL;
    int len = 0;

    FILE* file = fopen(input_file_path, "rb");

    if(file == NULL) goto end;

    if(fseek(file, 0, SEEK_END) < 0) goto end;

    len = ftell(file);
    
    if(len < 0) goto end;

    buffer = malloc(sizeof(char) * len);

    if(buffer == NULL) goto end;

    if(fseek(file, 0, SEEK_SET) < 0) goto end;

    size_t read = fread(buffer, sizeof(char), len, file);
    assert(read == len);

    if(count)
        *count = len;

    return buffer;

    end:
        if(file) fclose(file);
        if(buffer) free(buffer);

        return NULL;
}

void approx_table_size(StringStruct input, int* out_rows, int* out_cols) {

    int max_rows = 0;
    int max_cols = 0;

    while(input.count > 0) {

        max_rows++;

        StringStruct line = ss_cut_by_delim(&input, '\n');

        int cols = 0;

        while(line.count > 0) {

            StringStruct cell_in_row = ss_cut_by_delim(&line, '|');
            cols++;
        }

        if(max_cols < cols) max_cols = cols;
    }

    *out_cols = max_cols;
    *out_rows = max_rows;
}

Table alloc_table(int rows, int cols) {

    Table table = {0};
    table.rows = rows;
    table.cols = cols;

    table.cells = malloc(sizeof(Cell) * rows * cols);

    assert(table.cells != NULL);

    memset(table.cells, 0, sizeof(Cell) * rows * cols);

    return table;
}

Cell* cell_at(Table* table, int row, int col) {

    if(row > table->rows || col > table->cols) return NULL;
    return &table->cells[row * table->cols + col];
}

void print_cell(Cell* cell) {

    switch(cell->kind) {

        case KIND_EMPTY: {

            printf("%*.s|", max_cell_width, " ");
            return;
        }

        case KIND_NUM: {
            
            if(cell->as.number - (int)cell->as.number > 0)
                printf("%*.*f|", max_cell_width, DECIMAL_PLACES, cell->as.number);
            else
                printf("%*.d|", max_cell_width, (int)cell->as.number);

            return;
        }

        case KIND_TEXT: {

            printf("%*.s"SSFormat"|", max_cell_width - (int)cell->as.text.count, " ",  SSArg(cell->as.text));
            return;
        }

        case KIND_EXPR: {

            if(cell->as.expression.kind == EXPR_INVALID) {

                printf(ANSI_RED "%*.s"SSFormat ANSI_RESET "|", max_cell_width - (int)cell->as.expression.expr.count, " ", SSArg(cell->as.expression.expr));
            } else {

                printf("%*.s"SSFormat"|", max_cell_width - (int)cell->as.expression.expr.count, " ", SSArg(cell->as.expression.expr));
            }
            
            return;
        }

        default: {

            assert(0 && "You did the undoable. Great job!");
        }
    }
}

void print_cell_kind(Cell* cell) {

    switch(cell->kind) {

        case KIND_EMPTY: {

            printf("%*s|", 5, "EMPTY");
            return;
        }

        case KIND_NUM: {

            printf("%*s|", 5, "NUM");
            return;
        }

        case KIND_TEXT: {

            printf("%*s|", 5, "TEXT");
            return;
        }

        case KIND_EXPR: {

            if(cell->as.expression.kind == EXPR_DEFAULT)
                printf("%*s|", 5, "EXPR");
            else if(cell->as.expression.kind == EXPR_VALID)
                printf("%*s|", 5, "EXPR+");
            else
                printf("%*s|", 5, "EXPR-");
            
            return;
        }
        
        default: {

            assert(0 && "unreachable code.");
        }
    }
}

void populate_table(Table* table, StringStruct input) {

    for(int rows = 0; input.count > 0; rows++) {

        StringStruct line = ss_cut_by_delim(&input, '\n');

        for(int cols = 0; line.count > 0; cols++) {

            StringStruct token = ss_trim(ss_cut_by_delim(&line, '|'));
            if(token.count > max_cell_width) max_cell_width = token.count;

            if(ss_starts_with(&token, '=')) {

                cell_at(table, rows, cols)->as.expression.expr = token;
                cell_at(table, rows, cols)->as.expression.kind = EXPR_DEFAULT;
                cell_at(table, rows, cols)->kind = KIND_EXPR;
                expression_count++;
            } else if(ss_isnumber(token)) {

                cell_at(table, rows, cols)->as.number = ss_tod(token);
                cell_at(table, rows, cols)->kind = KIND_NUM;
            } else {

                cell_at(table, rows, cols)->as.text = token;
                if(token.count == 0)
                    cell_at(table, rows, cols)->kind = KIND_EMPTY;
                else
                    cell_at(table, rows, cols)->kind = KIND_TEXT;
            }
        }
    }

    max_cell_width += EXTRA_CELL_SPACE;
}

void print_table(Table* table) {

    printf("\n LE |");

    int left_padding = 0;
    int right_padding = 0;

    if(max_cell_width % 2) {

        left_padding = max_cell_width / 2;
        right_padding = max_cell_width / 2;
        
    } else {


        left_padding = max_cell_width / 2 - 1;
        right_padding = max_cell_width / 2;
    }

    for(int i = 0; i < table->cols; i++) { // columns header

        printf("%*s%c%*s|", left_padding, " ", 'A' + i, right_padding, " ");
    }

    printf("\n");
    for(int i = 0; i < table->cols * (max_cell_width + 1) + 5; i++) printf("%c", '-'); //line separator
    printf("\n");

    for(int row = 0; row < table->rows; row++) {

        printf("|%*d|", 3, row); //row separator

        for(int col = 0; col < table->cols; col++) {

            print_cell(cell_at(table, row, col));
        }
        printf("\n");
    }
    

    for(int i = 0; i < table->cols * (max_cell_width + 1) + 5; i++) printf("%c", '-'); //line separator
    printf("\n");
}

void print_table_kind(Table* table) {

    printf("\n");
    for(int i = 0; i < table->cols * 6 + 1; i++) printf("%c", '-'); 
    printf("\n");

    for(int row = 0; row < table->rows; row++) {

        printf("|");

        for(int col = 0; col < table->cols; col++) {

            print_cell_kind(cell_at(table, row, col));
        }
        printf("\n");
    }
    
    for(int i = 0; i < table->cols * 6 + 1; i++) printf("%c", '-'); 
    printf("\n");
}

//returns true ONLY IF it is VALID AND FITS INSIDE THE TABLE
bool token_iscellref(Table* table, StringStruct token, int* out_row, int* out_column) {

    StringStruct input = ss_trim(token);

    char c = *((char*)(ss_cut_n(&input, 1).data));
    if(!c_isupper(c)) goto not;

    if(!ss_isnumber(input)) goto not;

    int num = (int)ss_tod(input);

    if(num < 0 || num > 999) goto not;


    int column = (int)(c - 'A');
    if(column >= table->cols) goto out_of_bounds_col;
    if(num >= table->rows) goto out_of_bounds_row;


    if(out_column) *out_column = (int)(c - 'A');
    if(out_row) *out_row = num;

    return true;

    not:
        if(out_row) {
            *out_row = -1;
        }
        if(out_column) {
            *out_column = -1;
        }

        return false;

    out_of_bounds_col:
        if(out_row) {
            *out_row = -2;
        }
        if(out_column) {
            *out_column = -2;
        }

        return false;

    out_of_bounds_row:
        if(out_row) {
            *out_row = -3;
        }
        if(out_column) {
            *out_column = -3;
        }

        return false;
}

//returns the first operator or \0
char find_first_operator(StringStruct ss) {

    for(int i = 0; i < ss.count; i++) {

        if(!c_isoperator(c_charat(&ss, i))) continue;
        return c_charat(&ss, i);
    }

    return '\0';
}


/*
    GRAPH THINGS
*/

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

Node* make_root(int expression_count) {

    Node* root = malloc(sizeof(Node));
    assert(root != NULL);

    root->row = -2;
    root->col = -2;
    root->dependencies = NULL;
    root->count;

    if(expression_count > 0) {

        root->dependencies = malloc(sizeof(Node*) * expression_count);
        assert(root->dependencies != NULL);
        memset(root->dependencies, 0, sizeof(Node*) * expression_count);
    }

    return root;
}

bool was_visited(Node* root, VisitedNodes* visited) {

    for(size_t i = 0; i < visited->count; i++) {

        if(visited->nodes[i] == root) return true;
    }
    return false;
}

Node* dfs(Node* root, Node* target, VisitedNodes* visited) {

    assert(root != NULL);

    //check if it was visited already
    if(was_visited(root, visited)) return NULL;

    if(root->col == target->col && root->row == target->row) {

        //found it
        return root;
    }

    //mark as visited
    push_stack(visited, root);

    for(size_t i = 0; i < root->count; i++) {

        Node* ret = dfs(root->dependencies[i], target, visited);

        if(ret != NULL) return ret;
    }

    return NULL;
}

Node* find_node(Node* root, Node* target) {

    VisitedNodes visited = {0};

    return dfs(root, target, &visited);
}

Node* alloc_new_node(Node* target) {

    Node* newNode = malloc(sizeof(Node));
    assert(newNode != NULL);

    newNode->row = target->row;
    newNode->col = target->col;
    newNode->count = 0;
    newNode->dependencies = NULL;

    return newNode;
}

void add_node(Node* root, Node* source, Node* target) {

    Node* found_target = find_node(root, target);

    if(found_target != NULL) { //exists, just append dependency

        source->dependencies[source->count++] = found_target;
        return;
    }
        
    source->dependencies[source->count++] = alloc_new_node(target); //doesn't, create new dependency
}

void handle_expression(Node* root, Node* values, size_t count) {

    Node* source = find_node(root, &values[0]);

    if(source == NULL) { //lhs of the equation doesnt exist in the graph

        Node* newNode = alloc_new_node(&values[0]);
        newNode->dependencies = malloc(sizeof(Node*) * count - 1);
        assert(newNode->dependencies != NULL);
        
        memset(newNode->dependencies, 0, sizeof(Node*) * count - 1);

        source = newNode;

        root->dependencies[root->count++] = newNode;
    }
    else {

        if(source->dependencies == NULL) {

            source->dependencies = malloc(sizeof(Node*) * count - 1);
            assert(source->dependencies != NULL);
        }
    }

    for(size_t i = 1; i < count; i++) {

        add_node(root, source, &values[i]);
    }
}

Node* perform_syntax_analysis(Table* table) {

    //root uvek postoji
    char syntax_errors[SYNTAX_ERRORS_BUFF_SIZE];
    size_t syntax_buffer_iterator = 0;
    memset(syntax_errors, '\0', sizeof(syntax_errors));

    Node* root = make_root(expression_count);
    
    for(int row = 0; row < table->rows; row++) {

        for(int col = 0; col < table->cols; col++) {

            Cell* cell = cell_at(table, row, col);

            if(cell->kind == KIND_EXPR) {

                //node buffer, implemented here in temporary memory to avoid having to allocate an unknown amount of spaces for
                //cell dependencies. also wanted to avoid duplicating the entire function.

                Node current_cell = MKNode(row, col);
                cell->as.expression.kind = EXPR_VALID; //invalidated during processing if its invalid

                Node referenced_cells[GRAPH_NODE_BUFFER_SIZE] = {0}; 
                size_t buffer_count = 0;
                referenced_cells[buffer_count++] = current_cell;

                StringStruct expr = cell->as.expression.expr;
                ss_cut_n(&expr, 1); //cut the '=' sign
                expr = ss_trim(expr);

                if(expr.count == 0) {

                    cell->as.expression.kind = EXPR_INVALID;
                    syntax_buffer_iterator += sprintf(syntax_errors + syntax_buffer_iterator, ANSI_RED"[SYNTAX ERROR] Empty expression in cell "ANSI_BOLD_RED"%c%d"ANSI_RESET ANSI_RED".\n"ANSI_RESET, (char)('A' + col), row);
                    continue;
                }

                while(expr.count > 0) {
                    
                    //bool negative = false;
                    if(c_charat(&expr, 0) == '-') {

                        //negative = true;
                        ss_cut_n(&expr, 1);
                        expr = ss_trim(expr);
                    }

                    char c = find_first_operator(expr);
                    int out_row = 0;
                    int out_col = 0;

                    if(c != '\0') {

                        //ima operator

                        StringStruct token = ss_trim(ss_cut_by_delim(&expr, c));
                        expr = ss_trim(expr); //ready the next token

                        if(expr.count == 0) { //posle cuttovanja operatora nema nista -> dangling operator

                            cell->as.expression.kind = EXPR_INVALID;
                            syntax_buffer_iterator += sprintf(syntax_errors + syntax_buffer_iterator, ANSI_RED"[SYNTAX ERROR] Dangling operator in cell "ANSI_BOLD_RED"%c%d"ANSI_RESET ANSI_RED" :\""SSFormat"\"\n" ANSI_RESET, (char)('A' + col), row, SSArg(cell->as.expression.expr));
                            break;
                        }
 

                        if(ss_isnumber(token)) {} //if its a number, do nothing
                        else if(token_iscellref(table, token, &out_row, &out_col)) { //if its a cell ref add it

                            Node dep = MKNode(out_row, out_col);

                            referenced_cells[buffer_count++] = dep;
                            assert(buffer_count < GRAPH_NODE_BUFFER_SIZE); //to crash before segfault to alert the user of there being
                                                                           //too many cell references.   
                        }
                        else { //if its neither, report and continue

                            if(out_row == -1 && out_col == -1){

                                cell->as.expression.kind = EXPR_INVALID;
                                syntax_buffer_iterator += sprintf(syntax_errors + syntax_buffer_iterator, ANSI_RED"[SYNTAX ERROR] Invalid expression in cell "ANSI_BOLD_RED"%c%d"ANSI_RESET ANSI_RED" :\""SSFormat"\"\n" ANSI_RESET, (char)('A' + col), row, SSArg(cell->as.expression.expr));
                                break;
                            }
                            else if(out_row == -2 && out_col == -2) { //out_of_bounds_col

                                cell->as.expression.kind = EXPR_INVALID;
                                syntax_buffer_iterator += sprintf(syntax_errors + syntax_buffer_iterator, 
                                ANSI_RED"[OOB ERROR] Cell " ANSI_BOLD_RED SSFormat ANSI_RESET ANSI_RED" used in expression in "ANSI_BOLD_RED"%c%d"ANSI_RESET ANSI_RED" but column "ANSI_BOLD_RED"%c"ANSI_RESET ANSI_RED" doesn't exist in the table." ANSI_RESET"\n",
                                SSArg(token), 'A' + col, row, c_charat(&token, 0));
                                break;

                            } 
                            else if(out_row == -3 && out_col == -3) { //out_of_bounds_row

                                StringStruct copy = token;
                                ss_cut_n(&copy, 1);

                                cell->as.expression.kind = EXPR_INVALID;
                                syntax_buffer_iterator += sprintf(syntax_errors + syntax_buffer_iterator, 
                                ANSI_RED"[OOB ERROR] Cell " ANSI_BOLD_RED SSFormat ANSI_RESET ANSI_RED" used in expression in "ANSI_BOLD_RED"%c%d"ANSI_RESET ANSI_RED" but row "ANSI_BOLD_RED SSFormat ANSI_RESET ANSI_RED" doesn't exist in the table." ANSI_RESET"\n",
                                SSArg(token), 'A' + col, row, SSArg(copy));
                                break;
                            }  
                        }

                    } else {

                        //ceo ostatak je token                          //+1 to make sure its the entire thing
                        StringStruct token = ss_trim(ss_cut_n(&expr, expr.count + 1));
                        assert(expr.count == 0);

                        if(ss_isnumber(token)) {} //do nothing if its a number
                        else if(token_iscellref(table, token, &out_row, &out_col)) {

                            Node dep = MKNode(out_row, out_col);
                            referenced_cells[buffer_count++] = dep;

                        }
                        else {

                            if(out_row == -1 && out_col == -1){

                                cell->as.expression.kind = EXPR_INVALID;
                                syntax_buffer_iterator += sprintf(syntax_errors + syntax_buffer_iterator, ANSI_RED"[SYNTAX ERROR] Invalid expression in cell "ANSI_BOLD_RED"%c%d"ANSI_RESET ANSI_RED" :\""SSFormat"\"\n" ANSI_RESET, (char)('A' + col), row, SSArg(cell->as.expression.expr));
                                break;
                            }
                            else if(out_row == -2 && out_col == -2) { //out_of_bounds_col

                                cell->as.expression.kind = EXPR_INVALID;
                                syntax_buffer_iterator += sprintf(syntax_errors + syntax_buffer_iterator, 
                                ANSI_RED"[OOB ERROR] Cell " ANSI_BOLD_RED SSFormat ANSI_RESET ANSI_RED" used in expression in "ANSI_BOLD_RED"%c%d"ANSI_RESET ANSI_RED" but column "ANSI_BOLD_RED"%c"ANSI_RESET ANSI_RED" doesn't exist in the table." ANSI_RESET"\n",
                                SSArg(token), 'A' + col, row, c_charat(&token, 0));
                                break;

                            } 
                            else if(out_row == -3 && out_col == -3) { //out_of_bounds_row

                                StringStruct copy = token;
                                ss_cut_n(&copy, 1);

                                cell->as.expression.kind = EXPR_INVALID;
                                syntax_buffer_iterator += sprintf(syntax_errors + syntax_buffer_iterator, 
                                ANSI_RED"[OOB ERROR] Cell " ANSI_BOLD_RED SSFormat ANSI_RESET ANSI_RED" used in expression in "ANSI_BOLD_RED"%c%d"ANSI_RESET ANSI_RED" but row "ANSI_BOLD_RED SSFormat ANSI_RESET ANSI_RED" doesn't exist in the table." ANSI_RESET"\n",
                                SSArg(token), 'A' + col, row, SSArg(copy));
                                break;
                            } 
                        }     
                    }
                }

                if(cell->as.expression.kind == EXPR_INVALID) continue;
                handle_expression(root, referenced_cells, buffer_count);
            }
        }
    }

    fprintf(stderr, "%s\n", syntax_errors);
    return root;
}

bool dfs_cycle(Node* root, VisitedNodes* visited, Recstack* recstack) {

    assert(root != NULL);

    push_stack(visited, root); // add it do list of visited nodes so that i call dfs for every node once
    push_stack(recstack, root); //add it to the path currently being taken

    for(size_t i = 0; i < root->count; i++) {

        if(!was_visited(root->dependencies[i], visited)) { //if not visited, visit

            if(dfs_cycle(root->dependencies[i], visited, recstack)) return true;
        }
        else if(was_visited(root->dependencies[i], recstack)) { //IS IT IN RECSTACK

            push_stack(recstack, root->dependencies[i]);
            return true;
        }
    }

    pop_stack(recstack); //pop from the path if we're backtracking
    return false;
}

void report_cycle(Recstack* recstack) {

    char buffer[GRAPH_CYCLE_BUFFER_SIZE];
    memset(buffer, '\0', sizeof(buffer));
    size_t buffer_iterator = 0;
                
    buffer_iterator += sprintf(buffer + buffer_iterator, ANSI_RED "[CYCLE CHECK] A dependency cycle was found:\n" ANSI_RESET);

    for(size_t i = 0; i < recstack->count - 1; i++) {

        buffer_iterator += sprintf(buffer + buffer_iterator, "%c%d -> ", 'A' + recstack->nodes[i]->col, recstack->nodes[i]->row);
    }

    sprintf(buffer + buffer_iterator, "%c%d\n", 'A' + recstack->nodes[recstack->count - 1]->col, recstack->nodes[recstack->count - 1]->row);
                
    fprintf(stderr, "%s", buffer);
}

bool cycles_exist(Node* root) {

    VisitedNodes visited = {0};
    Recstack recstack = {0};

    for(size_t i = 0; i < root->count; i++) {

        if(!was_visited(root->dependencies[i], &visited)) {

            if(dfs_cycle(root->dependencies[i], &visited, &recstack)) {

                report_cycle(&recstack);
                return true;
            }
        }
    }

    return false;
}


/*
    MATH PARSING
*/




/*
    SHUNTING YARD ALGORITHM
*/

typedef union {
    double number;
    char operator;
} ElementAs;

typedef enum {
    ELEMENT_NUM = 0,
    ELEMENT_OP,
    ELEMENT_INV
} ElementKind;

typedef struct {
    ElementKind kind;
    ElementAs as;
} Element;

#define INVALID_ELEMENT {   \
    .kind = ELEMENT_INV     \
}

#define PRINT_ELEMENT(element) {\
if((element).kind == ELEMENT_NUM) printf("Element: %f\n", (element).as.number);\
else printf("Element: %c\n", (element).as.operator);}


#define ELEMENT_STACK_SIZE 16

#define STACK_INIT {        \
    .elements = {0},        \
    .count = 0              \
}

typedef struct {
    Element elements[ELEMENT_STACK_SIZE];
    size_t count;
} ElementStack;

void stack_push(ElementStack* stack, Element element) {

    assert(stack->count < ELEMENT_STACK_SIZE);
    stack->elements[stack->count++] = element;
}

Element stack_pop(ElementStack* stack) {

    if(stack->count > 0) {
        return stack->elements[--stack->count];;
    }

    Element inv = INVALID_ELEMENT;
    return inv;
}

#define ELEMENT_QUEUE_SIZE 32
#define QUEUE_INIT {        \
    .elements = {0},        \
    .first = 0,             \
    .last = 0,              \
    .count = 0              \
}

typedef struct {
    Element elements[ELEMENT_QUEUE_SIZE];
    size_t first;
    size_t last;
    size_t count;
} ElementQueue;

void queue_push(ElementQueue* queue, Element element) {

    assert(queue->last < ELEMENT_QUEUE_SIZE);
    queue->elements[queue->last++] = element;
    queue->count++;
}

Element queue_pop(ElementQueue* queue) {

    if(queue->last - queue->first != 0) {

        queue->count--;
        return queue->elements[queue->first++];
    }
    
    Element inv = INVALID_ELEMENT;
    return inv;
}

bool stack_top_higher_precedence(ElementStack* stack, Element operator) {

    if(stack->count == 0) return false;

    Element op = stack->elements[stack->count - 1];

    if(operator.as.operator == '+' || operator.as.operator == '-') {

        if(op.as.operator == '+' || op.as.operator == '-') return false;
        else return true;
    }
    else if(operator.as.operator == '*' || operator.as.operator == '/') {

        if(op.as.operator == '^') return true;
        else return false;
    }
    else return false;
}

double shunting_yard(StringStruct* sseq) {

    ElementStack stack = STACK_INIT;
    ElementQueue queue = QUEUE_INIT;

    while(sseq->count > 0) {

        bool negative = false;
        if(c_charat(sseq, 0) == '-') {

            negative = true;
            ss_cut_n(sseq, 1);
            *sseq = ss_trim(*sseq);
        }

        if(find_first_operator(*sseq) != '\0') { //postoji operator

            StringStruct number = ss_trim(ss_cut_to_delim(sseq, find_first_operator(*sseq)));
            
            Element num = {

                .kind = ELEMENT_NUM,
                .as.number = ss_tod(number)
            };

            if(negative) num.as.number *= -1;

            queue_push(&queue, num); //enqueue the number

            Element operator = {

                .kind = ELEMENT_OP,
                .as.operator = c_charat(sseq, 0)
            };

            ss_cut_n(sseq, 1);
            *sseq = ss_trim(*sseq);

            while(stack.count > 0 && stack_top_higher_precedence(&stack, operator)) {

                Element popped_operator = stack_pop(&stack);
                queue_push(&queue, popped_operator);
            }

            stack_push(&stack, operator);
        }
        else {

            StringStruct number = ss_trim(ss_cut_n(sseq, sseq->count + 1));

            Element num = {

                .kind = ELEMENT_NUM,
                .as.number = ss_tod(number)
            };

            if(negative) num.as.number *= -1;

            queue_push(&queue, num); //enqueue the number
        }
    }

    while(stack.count > 0) {

        Element popped = stack_pop(&stack);
        queue_push(&queue, popped);
    }

    assert(stack.count == 0);

    while(queue.count > 0) {

        Element popped1 = queue_pop(&queue);

        if(popped1.kind == ELEMENT_NUM) stack_push(&stack, popped1);
        else if(popped1.kind == ELEMENT_OP) {

            switch(popped1.as.operator) {

                case '^': {

                    Element ret = {

                        .kind = ELEMENT_NUM,
                        .as.number = pow(stack.elements[stack.count - 2].as.number, stack.elements[stack.count - 1].as.number)
                    };

                    stack_pop(&stack);
                    stack_pop(&stack);

                    stack_push(&stack, ret);
                    break;
                }

                case '*': {

                    Element ret = {

                        .kind = ELEMENT_NUM,
                        .as.number = stack.elements[stack.count - 2].as.number * stack.elements[stack.count - 1].as.number
                    };

                    stack_pop(&stack);
                    stack_pop(&stack);

                    stack_push(&stack, ret);
                    break;
                }

                case '/': {

                    Element ret = {

                        .kind = ELEMENT_NUM,
                        .as.number = stack.elements[stack.count - 2].as.number / stack.elements[stack.count - 1].as.number
                    };

                    stack_pop(&stack);
                    stack_pop(&stack);

                    stack_push(&stack, ret);
                    break;
                }

                case '+': {

                    Element ret = {

                        .kind = ELEMENT_NUM,
                        .as.number = stack.elements[stack.count - 2].as.number + stack.elements[stack.count - 1].as.number
                    };

                    stack_pop(&stack);
                    stack_pop(&stack);

                    stack_push(&stack, ret);
                    break;
                }

                case '-': {

                    Element ret = {

                        .kind = ELEMENT_NUM,
                        .as.number = stack.elements[stack.count - 2].as.number - stack.elements[stack.count - 1].as.number
                    };

                    stack_pop(&stack);
                    stack_pop(&stack);

                    stack_push(&stack, ret);
                    break;
                }

                default: {

                    assert(0 && "Unreachable code.");
                }
            }
        }
    }

    Element solution = stack_pop(&stack);
    return solution.as.number;
}


double solve_equation_with_numbers(Table* table, Node* node) {

    assert(table != NULL && node != NULL);

    Cell* target_cell = cell_at(table, node->row, node->col);

    StringStruct e = target_cell->as.expression.expr;
    ss_cut_n(&e, 1); //strip off the '=' sign
    e = ss_trim(e);

    return shunting_yard(&e);
}

double solve_expression(Table* table, Node* node) {

    Cell* target_cell = cell_at(table, node->row, node->col);
    assert(target_cell != NULL);

    assert(target_cell->kind != KIND_TEXT);
    assert(target_cell->kind != KIND_EMPTY);

    switch(target_cell->kind) { //poslednji u lancu moze biti ILI NUM ILI EXPRESSION SA BROJEVIMA

        case KIND_NUM: {

            return target_cell->as.number;
        }

        case KIND_EXPR: {

            target_cell->kind = KIND_NUM;
            target_cell->as.number =  solve_equation_with_numbers(table, node);
            return target_cell->as.number;
        }

        default: {

            assert(0 && "Unreachable code.");
        }
    }
}

//returns the pointer to a referenced cell and its name in SS format
/*Node* find_next_cell_reference(Table* table, StringStruct equation, StringStruct* out_reference) {

    for(size_t i = 0; i < equation.count; i++) {

        if(c_isupper(c_charat(&equation, i))) {

            int col = c_charat(&equation, i) - 'A';
            
            size_t upperBound = i + 4;
            if(upperBound > equation.count)
                upperBound = equation.count;

            char buffer[5];
            sprintf(buffer, "%c", 'A' + col);

            for(size_t j = i + 1; j < upperBound; j++) {

                if(c_isdigit(c_charat(&equation, j)))
                    sprintf(buffer + j - i, "%c", c_charat(&equation, j));
                else break;
            }

            //now theres a number in the buffer that represents the row
            double row = strtod(buffer + 1, NULL);
            //cant be invalid since its already being checked during syntax analysis
            assert(row >= 0 && row <= 1000);

            if(out_reference) *out_reference = ss_form_string_nt(buffer);

            return cell_at(table, row, col);
        }
        else continue;
    }

    return NULL;
}*/

StringStruct d_find_and_replace(StringStruct _base, StringStruct _replace_this, double _with_this) {

    if(_base.count == 0 || _replace_this.count == 0) return _base;

    char buffer[128];
    memset(buffer, '\0', sizeof(buffer));
    size_t iterator = 0;

    char base[32];
    memset(base, '\0', sizeof(base));
    size_t base_iterator = 0;

    char replace_this[32];
    memset(replace_this, '\0', sizeof(replace_this));
    size_t replace_iterator = 0;

    char with_this[32];
    memset(with_this, '\0', sizeof(with_this));
    size_t with_iterator = 0;

    /*strncpy(base, _base.data, _base.count);
    strncpy(replace_this, _replace_this.data, _replace_this.count);
    strncpy(with_this, _with_this.data, _with_this.count);*/

    snprintf(base, _base.count + 1, "%s", _base.data);
    snprintf(replace_this, _replace_this.count + 1, "%s", _replace_this.data);
    sprintf(with_this, "%.04lf", _with_this);

    while(c_find_substring(base + base_iterator, replace_this) != -1) {

        int index = c_find_substring(base + base_iterator, replace_this);
        
        strncpy(buffer + iterator, base + base_iterator, index);
        iterator += index;
        base_iterator += index + strlen(replace_this);
        strncpy(buffer + iterator, with_this, strlen(with_this));
        iterator += strlen(with_this);
    }

    strncpy(buffer + iterator, base + base_iterator, strlen(base + base_iterator));
    iterator += strlen(base + base_iterator);

    buffer[iterator] = '\0';
    
    StringStruct ret = {0};
    ret.count = strlen(buffer);
    ret.data = strdup(buffer);
    return ret;
}

double dfs_solve(Table* table, Node* root, VisitedNodes* visited) {

    assert(root != NULL);

    push_stack(visited, root);

    //a cell can depend on a node thats either an expression or a number

    //starting from here, find and replace all cell references with their solutions

    for(size_t i = 0; i < root->count; i++) {

        if(!was_visited(root->dependencies[i], visited)) {
            
            //child's name
            char buffer[5];
            sprintf(buffer, "%c", 'A' + root->dependencies[i]->col);
            sprintf(buffer + 1, "%d", root->dependencies[i]->row);
            StringStruct cell_ref = ss_form_string_nt(buffer);
            
            Cell* parent_cell = cell_at(table, root->row, root->col);

            parent_cell->as.expression.expr = d_find_and_replace(parent_cell->as.expression.expr, cell_ref, dfs_solve(table, root->dependencies[i], visited));
        }
    }

    return solve_expression(table, root);
    
}

void solve_expressions(Table* table, Node* root) {

    VisitedNodes visited = {0};

    for(size_t i = 0; i < root->count; i++) {

        if(!was_visited(root->dependencies[i], &visited)) {

            dfs_solve(table, root->dependencies[i], &visited);
        }
    }
}








void report_invalid_dependency(Recstack* recstack, Cell* target_cell, const char* err_message) {

    char buffer[INVALID_DEPENDENCY_BUFFER_SIZE];
    memset(buffer, '\0', sizeof(buffer));
    size_t iterator = 0;

    iterator += sprintf(buffer + iterator, ANSI_RED "%s" ANSI_RESET, err_message);
    for(size_t i = 0; i < recstack->count - 1; i++) {

        iterator += sprintf(buffer + iterator, "%c%d -> ", 'A' + recstack->nodes[i]->col, recstack->nodes[i]->row);
    }

    switch(target_cell->kind) {

        case KIND_EXPR: {

            sprintf(buffer + iterator, "( %c%d = '"SSFormat"')\n",
            'A' + recstack->nodes[recstack->count - 1]->col, recstack->nodes[recstack->count - 1]->row, SSArg(target_cell->as.expression.expr));
            break;
        }

        case KIND_TEXT: {

            sprintf(buffer + iterator, "( %c%d = '"SSFormat"')\n",
            'A' + recstack->nodes[recstack->count - 1]->col, recstack->nodes[recstack->count - 1]->row, SSArg(target_cell->as.text));
            break;
        }

        case KIND_EMPTY: {

            sprintf(buffer + iterator, "( %c%d = '"SSFormat"')\n",
            'A' + recstack->nodes[recstack->count - 1]->col, recstack->nodes[recstack->count - 1]->row, SSArg(target_cell->as.text));
            break;
        }

        default: {

            assert(0 && "Unreachable code.");
        }
    }
    
    printf("%s", buffer);
}

bool dfs_invalid_dependency(Table* table, Node* root, VisitedNodes* visited, Recstack* recstack) {

    assert(root != NULL);

    push_stack(visited, root);
    push_stack(recstack, root);

    //check if current node is valid
    Cell* target_cell = cell_at(table, root->row, root->col);
    assert(target_cell != NULL);

    switch(target_cell->kind) {

        case KIND_NUM: {
            
            break;
        }

        case KIND_EXPR: {

            if(target_cell->as.expression.kind == EXPR_INVALID) { //report cells who cant depend on an invalid cell

                report_invalid_dependency(recstack, target_cell, "[DEPCHK] Expressions depend on an invalid expression cell.\n");
                return true;
            }
            
            break;
        }

        case KIND_TEXT: { //report that cells cant depend on a cell which is a string

            report_invalid_dependency(recstack, target_cell, "[DEPCHK] Expressions depend on a text cell.\n");
            return true;
        }

        case KIND_EMPTY: { //report that cells cant depend on an empty cell

            report_invalid_dependency(recstack, target_cell, "[DEPCHK] Expressions depend on an empty cell.\n");
            return true;
        }

        default: {

            assert(0 && "Unreachable code.");
        }
    }

    for(size_t i = 0; i < root->count; i++) {

        if(dfs_invalid_dependency(table, root->dependencies[i], visited, recstack)) return true;
    }

    pop_stack(recstack);
    return false;
}

bool invalid_dependencies_exist(Table* table, Node* root) {

    VisitedNodes visited = {0};
    Recstack recstack = {0};

    for(size_t i = 0; i < root->count; i++) {

        if(!was_visited(root->dependencies[i], &visited)) {

            if(dfs_invalid_dependency(table, root->dependencies[i], &visited, &recstack)) {
                
                return true;
            }
        }
    }

    return false;
}

void solve_table(Table* table) {

    Node* root = perform_syntax_analysis(table);

    if(root->count == 0) {

        printf(ANSI_GREEN "\n[SOLVE] There is nothing to solve.\n" ANSI_RESET);
        return;
    }

    if(cycles_exist(root)) {

        printf(ANSI_BOLD_RED"\n[SOLVE] Terminated abnormally.\n" ANSI_RESET);
        return;
    }

    if(invalid_dependencies_exist(table, root)) {

        printf(ANSI_BOLD_RED"\n[SOLVE] Terminated abnormally.\n" ANSI_RESET);
        return;
    }

    printf(ANSI_GREEN "\n[SOLVE] Solving..." ANSI_RESET"\n");

    solve_expressions(table, root);
}

/*
    EXPRESSION EVALUATION STEPS:

    1. [DONE]   SYNTAX ANALYSIS (BRACKETS TBA, MAY BE EXPANDED ON LATER)
    2. [DONE]   SOME SORT OF DEPENDENCY GRAPH CREATION
    3. [DONE]   CYCLE CHECKING
    4. [DONE]   REPORTING CYCLES IN DEPENDENCIES
    5.          TAKING CLEAN DEPENDENCIES AND SUBSTITUTING CELL NAMES WITH VALUES
    6.          EVALUATING MATH EQUATION
*/

/*
    QUIRKS TBA:

    ? COLOURS IN CELLS FOR PRETTYFYING
    - ADD SUPPORT FOR MORE COLUMNS LIKE AA, AAA, AAAA, ...
    ? SEPARATE FILE AND CONSOLE OUTPUT BECAUSE OF THE COLOUR CHARACTERS,
        OR CONSOLIDATE ON NOT HAVING FILE OUTPUT, USELESS ANYWAYS

*/

/*
    TODO

    
*/

/*
    NOTES

    1. MATH EVALUATION CAN BE DONE USING A STACK STRUCTURE, THINK OF IT
        AS DOING THE EQUATION IN YOUR HEAD BUT WITH A PIECE OF PAPER COVERING 
        IT AND UNVEILING ONE THING AT A TIME
    ...
*/

int main(int argc, char* argv[]) {

    if(argc < 2) {
        fprintf(stderr, "You must provide name of the input file:\n ./main <input_file>.csv\n");
        exit(1);
    }

    const char* input_file_path = argv[1];
    int len = 0;
    char* content = consume_file(input_file_path, &len);

    StringStruct input = ss_form_string(content, len);

    int rows;
    int cols;

    approx_table_size(input, &rows, &cols);

    if(cols > 26) printf("ERROR: This program only supports up to 26 columns.\n");
    assert(cols <= 26);

    Table table = alloc_table(rows, cols);

    populate_table(&table, input);
    solve_table(&table);
    
    print_table(&table);
    print_table_kind(&table);

    /*StringStruct source = SS("aleksa");
    StringStruct target = SS("k");
    double with = 50;

    StringStruct a = d_find_and_replace(source, target, with);
    printf(SSFormat, SSArg(a));*/

   
    return 0;
}