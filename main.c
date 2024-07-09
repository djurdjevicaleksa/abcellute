#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ss.h"

#define DECIMAL_PLACES 2

int max_cell_width = 0;

typedef enum {

    EXPR_NUM = 0,
    EXPR_CELL,
    EXPR_BIN_OP,
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

            printf("%*.s"SSFormat"|", max_cell_width - (int)cell->as.expression.expr.count, " ", SSArg(cell->as.expression.expr));
            return;
        }

        default: {

            assert(0 && "Unreachable code");
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
                cell_at(table, rows, cols)->kind = KIND_EXPR;
            } else if(ss_isnumber(token)) {

                cell_at(table, rows, cols)->as.number = ss_tod(token);
                cell_at(table, rows, cols)->kind = KIND_NUM;
            } else {

                cell_at(table, rows, cols)->as.text = token;
                cell_at(table, rows, cols)->kind = KIND_TEXT;
            }
        }
    }
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

    for(int i = 0; i < table->cols; i++) {

        printf("%*s%c%*s|", left_padding, " ", 'A' + i, right_padding, " ");
    }

    printf("\n");
    for(int i = 0; i < table->cols * (max_cell_width + 1) + 5; i++) printf("%c", '-'); 
    printf("\n");

    for(int row = 0; row < table->rows; row++) {

        printf("|%*d|", 3, row);

        for(int col = 0; col < table->cols; col++) {

            print_cell(cell_at(table, row, col));
        }
        printf("\n");
    }
    

    for(int i = 0; i < table->cols * (max_cell_width + 1) + 5; i++) printf("%c", '-'); 
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
/*
    =A1+B1


*/
void parse_expression(Expr* expression) {

    StringStruct expr = expression->expr;

    printf(SSFormat"\n", SSArg(expr));
}

void parse_expressions(Table* table) {

    for(int row = 0; row < table->rows; row++) {

        for(int col = 0; col < table->cols; col++) {

            Cell* cell = cell_at(table, row, col);

            if(cell->kind == KIND_EXPR) {

                parse_expression(&cell->as.expression);
            }
        }
    }
}

int main(int argc, char* argv[]) {

    //otvori fajl, saznaj koliki je, alociraj memoriju, prekopiraj

    if(argc < 2) {
        fprintf(stderr, "You must provide name of the input file:\n ./main <input_file>.csv\n");
        exit(1);
    }

    const char* input_file_path = argv[1];
    int len;
    char* content = consume_file(input_file_path, &len);

    //printf("%s\n", content);

    StringStruct input = ss_form_string(content, len);

    int rows;
    int cols;

    approx_table_size(input, &rows, &cols);

    if(cols > 26) printf("ERROR: This program currently only supports up to 26 columns.\n");

    Table table = alloc_table(rows, cols);

    populate_table(&table, input);

    max_cell_width += 2;

    print_table(&table);
    //print_table_kind(&table);

    //parse_expressions(&table);


    return 0;
}