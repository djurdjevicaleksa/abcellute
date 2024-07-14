#ifndef _EQUATION_SOLVER_H
#define _EQUATION_SOLVER_H

#include "element.h"
#include "ss.h"
#include "table.h"
#include "graph.h"

bool stack_top_higher_precedence(ElementStack* stack, Element operator);
char find_first_operator(StringStruct ss);
double shunting_yard(StringStruct* sseq);
double solve_equation_with_numbers(Table* table, Node* node);
double solve_expression(Table* table, Node* node);
StringStruct d_find_and_replace(StringStruct _base, StringStruct _replace_this, double _with_this);
double dfs_solve(Table* table, Node* root, VisitedNodes* visited);
void solve_expressions(Table* table, Node* root);



#endif //_EQUATION_SOLVER_H