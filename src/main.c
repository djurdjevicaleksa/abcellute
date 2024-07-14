#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

//#define _SS_IMPLEMENT
#include "ss.h"

#include "element.h"
#include "graph.h"
#include "table.h"
#include "invalid_dependency.h"
#include "constants.h"
#include "equation_solver.h"

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

    calculate_new_cell_width(table);

    print_table(table);
}

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
    print_table(&table);
    solve_table(&table);
   
    return 0;
}