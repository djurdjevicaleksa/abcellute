#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define _SS_IMPLEMENT
#include "ss.h"

#define DECIMAL_PLACES 2
#define EXTRA_CELL_SPACE 2
#define GRAPH_NODE_BUFFER_SIZE 32 // this should be the buffe
#define NODE_LIST_SIZE 32
#define SYNTAX_ERRORS_BUFF_LEN 2048


#define ANSI_RESET      "\x1b[0m"
#define ANSI_RED        "\x1b[31m"
#define ANSI_GREEN      "\x1b[32m"
#define ANSI_BOLD_RED   "\x1b[1;31m"

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

            printf("%*s|", 5, "EXPR");
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

bool token_iscellref(StringStruct token, int* out_row, int* out_column) {

    StringStruct input = ss_trim(token);

    char c = *((char*)(ss_cut_n(&input, 1).data));
    if(!c_isupper(c)) goto not;

    if(!ss_isnumber(input)) goto not;

    int num = (int)ss_tod(input);

    if(num < 0 || num > 999) goto not;

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

}

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

    Node nodes[NODE_LIST_SIZE];
    size_t count;
} NodeList;

typedef NodeList VisitedNodes;

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

        if(root->row == visited->nodes[i].row && root->col == visited->nodes[i].col) return true;
    }
    return false;
}

Node* dfs(Node* root, Node* target, VisitedNodes* visited) {

    assert(root != NULL);

    //proveri da li je vec poseceno
    if(was_visited(root, visited)) return NULL;

    if(root->col == target->col && root->row == target->row) {

        //nasao ga, vrati ga
        //printf("Ovde je poznata celija koja zavisi od nadjenog cvora\n");
        return root;
    }

    visited->nodes[visited->count++] = *root;
    //printf("[DFS] Visited node %c%d.\n", 'A' + root->col, root->row);

    for(size_t i = 0; i < root->count; i++) {

        Node* ret = dfs(root->dependencies[i], target, visited);

        if(ret != NULL) return ret;
    }

    return NULL;
}

Node* find_node(Node* root, Node* target) {

    //printf("[FIND] Begin:\n");

    VisitedNodes visited = {0};

    return dfs(root, target, &visited);
}

void add_node(Node* source, Node* target) {

    Node* newNode = malloc(sizeof(Node));
    assert(newNode != NULL);

    newNode->row = target->row;
    newNode->col = target->col;
    newNode->count = 0;
    newNode->dependencies = NULL;

    if(source->dependencies == NULL) 
        source->dependencies = malloc(sizeof(Node*));
    else 
        source->dependencies = realloc(source->dependencies, sizeof(source->dependencies) + sizeof(Node*));
        
    assert(source->dependencies != NULL);
    source->dependencies[source->count++] = newNode;
    printf("[ADD] %c%d -> %c%d.\n", 'A' + source->col, source->row, 'A' + target->col, target->row);
}

void handle_expression(Node* root, Node* values, size_t count) {

    Node* source = find_node(root, &values[0]);

    if(source == NULL) {

        Node* newNode = malloc(sizeof(Node));
        assert(newNode != NULL);
        
        newNode->row = (&values[0])->row;
        newNode->col = (&values[0])->col;
        newNode->count = 0;
        newNode->dependencies = malloc(sizeof(Node*) * count - 1);
        assert(newNode->dependencies != NULL);
        memset(newNode->dependencies, 0, sizeof(Node*) * count - 1);

        source = newNode;

        root->dependencies[root->count++] = newNode;
        printf("[ADD] Cell %c%d wasn't found so it was appended to the root.\n", 'A' + (&values[0])->col, (&values[0])->row);
    } 

    for(size_t i = 1; i < count; i++) {

        add_node(source, &values[i]);
    }
}


Node* perform_syntax_analysis(Table* table) {

    //root uvek postoji
    char syntax_errors[SYNTAX_ERRORS_BUFF_LEN];
    size_t syntax_buffer_iterator = 0;

    Node* root = make_root(expression_count);
    
    for(int row = 0; row < table->rows; row++) {

        for(int col = 0; col < table->cols; col++) {

            Cell* cell = cell_at(table, row, col);

            if(cell->kind == KIND_EXPR) {

                //node buffer, implemented here in temporary memory to avoid having to allocate an unknown amount of spaces for
                //cell dependencies. also wanted to avoid duplicating the entire function.

                Node current_cell = {

                    .row = row,
                    .col = col,
                    .count = 0,
                    .dependencies = NULL
                };

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

                    char c = find_first_operator(expr);
                    int out_row = 0;
                    int out_col = 0;

                    if(c != '\0') {

                        //ima operator

                        StringStruct token = ss_cut_by_delim(&expr, c);
                        expr = ss_trim(expr); //ready the next token

                        if(expr.count == 0) { //posle cuttovanja operatora nema nista -> dangling operator

                            cell->as.expression.kind = EXPR_INVALID;
                            syntax_buffer_iterator += sprintf(syntax_errors + syntax_buffer_iterator, ANSI_RED"[SYNTAX ERROR] Dangling operator in cell "ANSI_BOLD_RED"%c%d"ANSI_RESET ANSI_RED" :\""SSFormat"\"\n" ANSI_RESET, (char)('A' + col), row, SSArg(cell->as.expression.expr));
                            break;
                        }

                        if(!token_iscellref(token, &out_row, &out_col) && !ss_isnumber(token)) {

                            cell->as.expression.kind = EXPR_INVALID;
                            syntax_buffer_iterator += sprintf(syntax_errors + syntax_buffer_iterator, ANSI_RED"[SYNTAX ERROR] Invalid expression in cell "ANSI_BOLD_RED"%c%d"ANSI_RESET ANSI_RED" :\""SSFormat"\"\n" ANSI_RESET, (char)('A' + col), row, SSArg(cell->as.expression.expr));
                            break;
                        }

                        if(out_col != -1 && out_row != -1) {

                            //token je cellref
                            //ovde dodaj dependency u buffer
                            Node dep = {

                                .row = out_row,
                                .col = out_col,
                                .count = 0,
                                .dependencies = NULL
                            };

                            referenced_cells[buffer_count++] = dep;
                            assert(buffer_count < GRAPH_NODE_BUFFER_SIZE); //to crash before segfault to alert the user of there being
                                                                           //too many cell references. 
                        }                            
                         


                    } else {

                        //ceo ostatak je token                          //+1 to make sure its the entire thing
                        StringStruct token = ss_cut_n(&expr, expr.count + 1);
                        assert(expr.count == 0);

                        if(!token_iscellref(token, &out_row, &out_col) && !ss_isnumber(token)) {

                            cell->as.expression.kind = EXPR_INVALID;
                            syntax_buffer_iterator += sprintf(syntax_errors + syntax_buffer_iterator, ANSI_RED"[SYNTAX ERROR] Invalid expression in cell "ANSI_BOLD_RED"%c%d"ANSI_RESET ANSI_RED" :\""SSFormat"\"\n" ANSI_RESET, (char)('A' + col), row, SSArg(cell->as.expression.expr));
                            break;
                        }

                        if(out_col != -1 && out_row != -1) {

                            //token je cellref
                            //ovde dodaj dependency u buffer
                            Node dep = {

                                .row = out_row,
                                .col = out_col,
                                .count = 0,
                                .dependencies = NULL
                            };

                            referenced_cells[buffer_count++] = dep;
                        }
                    }
                }

                if(cell->as.expression.kind == EXPR_INVALID) continue;

                handle_expression(root, referenced_cells, buffer_count);

            }
        }
    }

    fprintf(stderr, "%s\n", syntax_errors);
}

/*
    EXPRESSION EVALUATION STEPS:

    1. SYNTAX ANALYSIS (BRACKETS TBA, MAY BE EXPANDED ON LATER)
    2. SOME SORT OF DEPENDENCY GRAPH CREATION
    3. CYCLE CHECKING
    4. REPORTING CYCLES IN DEPENDENCIES
    5. TAKING CLEAN DEPENDENCIES AND 
       SUBSTITUTING CELL NAMES WITH VALUES
    6. EVALUATING MATH EQUATION
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

    1. SS APPENDING : TEST, VALIDATE, FIND BUGS, SOLVE IF ANY

    2. GRAPH :
        - 1ST PASS : GO THROUGH ALL EXPRESSIONS AND EXTRACT DEPENDENCIES
        -? ALLOCATE MEMORY FOR ALL OF THEM
        - 2ND PASS: GO THROUGH ALL EXPRESSIONS AND ADD DEPENDENCIES AS NODES IN THE GRAPH
            --UPDATE DEPENDENCIES ALONG THE WAY
        
        - 3RD PASS : ANALYZE AND DETECT CYCLES (SOME ALGO FROM PROJEKTOVANJE ALGORITAMA, LOOK 'TOPOLOGICAL SORTING', 'CYCLIC GRAPHS')
        ...
    ...
*/

/*
    NOTES

    1. MATH EVALUATION CAN BE DONE USING A STACK STRUCTURE, THINK OF IT
        AS DOING THE EQUATION IN YOUR HEAD BUT WITH A PIECE OF PAPER COVERING 
        IT AND UNVEILING ONE THING AT A TIME
    
    2. TEST THE NEW CHANGES TO ISOPERATOR() SINCE I DIDNT HAVE IT IN MIND WHEN DEVELOPING
        ALGORITHM FOR THE PARSER

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
    perform_syntax_analysis(&table);
    print_table(&table);
    //print_table_kind(&table);

    return 0;
}