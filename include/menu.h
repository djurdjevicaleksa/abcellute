#ifndef _MENU_H
#define _MENU_H

#include "table.h"

void print_menu_options();
void free_table(Table* table);
void handle_import(Table* table, char* loaded_content, bool* loaded_table);
void handle_create(Table* table, char* loaded_content, bool* loaded_table);
void handle_modify(Table* table, bool* loaded_table);
void handle_export(Table* table, bool* loaded_table);
void handle_solve(Table* table, bool* loaded_table);
void show_menu();






#endif //_MENU_H