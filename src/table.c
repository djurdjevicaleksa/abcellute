#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "table.h"
#include "constants.h"

int max_cell_width = 0;
int expression_count = 0; //for dynamic allocation of graph nodes later


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

void calculate_new_cell_width(Table* table) {

    int new_cell_width = 0;

    for(size_t row = 0; row < table->rows; row++) {

        for(size_t col = 0; col < table->cols; col++) {

            Cell* current_cell = cell_at(table, row, col);
            if(current_cell->kind == KIND_TEXT) {

                if(new_cell_width < current_cell->as.text.count) new_cell_width = current_cell->as.text.count; 
            }
            else if(current_cell->kind == KIND_NUM) {

                char buffer[32];
                sprintf(buffer, "%.*lf", DECIMAL_PLACES, current_cell->as.number);
                if(new_cell_width < strlen(buffer)) new_cell_width = strlen(buffer);
            }
            else if(current_cell->kind == KIND_EXPR) {

                if(new_cell_width < current_cell->as.expression.expr.count) new_cell_width = current_cell->as.expression.expr.count;
            }
        }
    }

    new_cell_width += 2;
    max_cell_width = new_cell_width;   
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