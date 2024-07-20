#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "table.h"
#include "constants.h"
#include "equation_solver.h"



extern int max_cell_width;
extern int expression_count;

void print_menu_options() {
    
    printf("\n\n\n----------------------------------\nChoose an option:\n");
    printf("1. Import\n");
    printf("2. Create\n");
    printf("3. Modify cell\n");
    printf("4. Export\n");
    printf("5. Solve\n");
    printf("6. Exit\n\n");
    printf("> ");
}

void free_table(Table* table) {

    free(table->cells);
}

void handle_import(Table* table, char* loaded_content, bool* loaded_table) {

    if(*loaded_table) {

        free(table->cells);
        printf(ANSI_YELLOW "Freed memory used by prior table." ANSI_RESET "\n");
        *loaded_table = false;
    }

    if(loaded_content) {

        free(loaded_content);
        loaded_content = NULL;
    }

    printf("Enter the name of the input file, including its extension.\n> ");

    char input_file_name[128];
    memset(input_file_name, '\0', sizeof(input_file_name));
    fgets(input_file_name, sizeof(input_file_name), stdin);

    size_t new_line = 0;
    while(input_file_name[new_line] != '\n') new_line++;
    input_file_name[new_line] = '\0';

    int len = 0;
    loaded_content = consume_file(input_file_name, &len);

    if(loaded_content == NULL) {

        printf("Couldn't find that file.\n");
        return;
    }

    StringStruct input = ss_form_string(loaded_content, len);

    int rows;
    int cols;

    approx_table_size(input, &rows, &cols);

    if(cols > 26) printf("[ERROR] This program only supports up to 26 columns.\n");
    assert(cols <= 26);
    if(rows > 999) printf("[ERROR] This program only supports up to 999 rows.\n");
    assert(rows <= 999);

    *table = alloc_table(rows, cols);

    populate_table(table, input);
    print_table(table, stdout);

    *loaded_table = true;
    printf(ANSI_GREEN "Table loaded successfully." ANSI_RESET "\n");
}

void handle_create(Table* table, char* loaded_content, bool* loaded_table) {

    if(*loaded_table) {

        free(table->cells);
        printf(ANSI_YELLOW "Freed memory used by prior table." ANSI_RESET "\n");
    }

    if(loaded_content) {
        free(loaded_content);
        loaded_content = NULL;
    }

    printf("Specify dimensions of the custom table. Expected format: <columns> <rows>.\n> ");
                
    char dimensions_buffer[16];
    memset(dimensions_buffer, '\0', sizeof(dimensions_buffer));
    fgets(dimensions_buffer, sizeof(dimensions_buffer), stdin);

    for(size_t i = 0; i < 16; i++) {

        if(dimensions_buffer[i] == '\n') {

            dimensions_buffer[i] = '\0';
            break;
        }
    }

    int rows = 0;
    int cols = 0;

    if(sscanf(dimensions_buffer, "%d %d", &cols, &rows) != 2) {

        printf("Invalid input.\n");
        return;
    }

    if(rows <= 0 || cols <= 0) {

        printf("Row or column count can't be non-positive.\n");
        return;
    }

    if(cols > 26) printf("[ERROR] This program only supports up to 26 columns.\n");
    assert(cols <= 26);
    if(rows > 999) printf("[ERROR] This program only supports up to 999 rows.\n");
    assert(rows <= 999);

    *table = alloc_table(rows, cols);

    print_table(table, stdout);

    *loaded_table = true;
    printf(ANSI_GREEN "Table created successfully." ANSI_RESET "\n");
}

void handle_modify(Table* table, bool* loaded_table) {

    if(!(*loaded_table)) {

        printf(ANSI_RED "You must load a table first." ANSI_RESET "\n");
        return;
    }

    printf("Name a cell you want to change.\n> ");

    char cell_ref_buffer[8];
    memset(cell_ref_buffer, '\0', sizeof(cell_ref_buffer));
    fgets(cell_ref_buffer, sizeof(cell_ref_buffer), stdin);

    for(size_t i = 0; i < 8; i++) {

        if(cell_ref_buffer[i] == '\n') {

            cell_ref_buffer[i] = '\0';
            break;
        }
    }

    StringStruct cell_ref = ss_form_string_nt(cell_ref_buffer);
                
    int row, col;

    if(!token_iscellref(table, cell_ref, &row, &col)) {

        printf(ANSI_RED "Token is invalid or its not inside the table." ANSI_RESET "\n");
        return;
    }

    printf("Provide a value for the targeted cell.\n> ");

    char _new_value[64];
    memset(_new_value, '\0', 64);
    fgets(_new_value, sizeof(_new_value), stdin);

    for(size_t i = 0; i < 64; i++) {

        if(_new_value[i] == '\n') {

            _new_value[i] = '\0';
            break;
        }
    }

    char* new_value = malloc(sizeof(char) * 64); //MEMORY LEAK
    memset(new_value, '\0', 64);
    memcpy(new_value, _new_value, 64);


    //find out what type it is and update its fields in the table
    StringStruct token = ss_form_string_nt(new_value);
    StringStruct colour = SS("");
    printf(SSFormat"\n", SSArg(token));

    if(is_colour(token, &colour)) {

        cell_at(table, row, col)->kind = KIND_COLOUR;
        cell_at(table, row, col)->as.colour = colour;
    }
    else if(ss_starts_with(&token, '=')) {

        if(token.count > max_cell_width) max_cell_width = token.count;

        cell_at(table, row, col)->as.expression.expr = token;
        cell_at(table, row, col)->as.expression.kind = EXPR_DEFAULT;
        cell_at(table, row, col)->kind = KIND_EXPR;
        expression_count++;
                    
    } else if(ss_isnumber(token)) {

        if(token.count > max_cell_width) max_cell_width = token.count;

        cell_at(table, row, col)->as.number = ss_tod(token);
        cell_at(table, row, col)->kind = KIND_NUM;
    } else {

        if(token.count > max_cell_width) max_cell_width = token.count;

        cell_at(table, row, col)->as.text = token;
        if(token.count == 0)
            cell_at(table, row, col)->kind = KIND_EMPTY;
        else
            cell_at(table, row, col)->kind = KIND_TEXT;
    }

    printf(ANSI_GREEN "Cell modified successfully." ANSI_RESET "\n");

    calculate_new_cell_width(table);
    print_table(table, stdout);
}

void handle_export(Table* table, bool* loaded_table) {

    if(!(*loaded_table)) {

        printf(ANSI_RED "You must load a table first." ANSI_RESET "\n");
        return;
    }

    printf("Provide a name and extension for the output file.\n");

    char output_file_name[32];
    memset(output_file_name, '\0', 32);
    fgets(output_file_name, sizeof(output_file_name), stdin);

    for(size_t i = 0; i < 32; i++) {

        if(output_file_name[i] == '\n') {

            output_file_name[i] = '\0';
            break;
        }
    }

    FILE* output_file = fopen(output_file_name, "wb");
    if(output_file == NULL) {

        printf("Couldn't open output file.\n");
        return;
    }

    print_table(table, output_file);

    fclose(output_file);

    printf(ANSI_GREEN "Table output successfully." ANSI_RESET "\n");
}

void handle_solve(Table* table, bool* loaded_table) {

    if(!loaded_table) {

        printf(ANSI_RED "You must load a table first." ANSI_RESET "\n");
        return;
    }

    solve_table(table);
    print_table(table, stdout);
}

void show_menu() {
    
    char option = '0';
    Table table = {0};
    char* loaded_content = NULL;
    bool loaded_table = false;

    while(true) {

        print_menu_options();

        option = (char)fgetc(stdin);
        char c;
        while ((c = getchar()) != '\n' && c != EOF) {}
    
        switch(option) {

            case '1': {

                handle_import(&table, loaded_content, &loaded_table);
                break;
            }

            case '2': {

                handle_create(&table, loaded_content, &loaded_table);
                break;
            }

            case '3': {

                handle_modify(&table, &loaded_table);
                break;
            }

            case '4': {

                handle_export(&table, &loaded_table);
                break;
            }

            case '5': {

                handle_solve(&table, &loaded_table);
                break;
            }

            case '6': {
                
                if(table.cells != NULL) free(table.cells);
                if(loaded_content != NULL) free(loaded_content);
                return;
            }

            default: {

                printf("Invalid input. Try again.\n");
                break;
            }
        }
    }
    
}