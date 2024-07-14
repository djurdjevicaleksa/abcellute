#ifndef _TABLE_H
#define _TABLE_H

#include "ss.h"

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

char* consume_file(const char* input_file_path, int* count);
void approx_table_size(StringStruct input, int* out_rows, int* out_cols);
Table alloc_table(int rows, int cols);
Cell* cell_at(Table* table, int row, int col);
void calculate_new_cell_width(Table* table);
void print_cell(Cell* cell);
void print_cell_kind(Cell* cell);
void populate_table(Table* table, StringStruct input);
void print_table(Table* table);
void print_table_kind(Table* table);
bool token_iscellref(Table* table, StringStruct token, int* out_row, int* out_column);

#endif //_TABLE_H