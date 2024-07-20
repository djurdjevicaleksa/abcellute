#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "equation_solver.h"
#include "element.h"
#include "ss.h"
#include "table.h"
#include "graph.h"
#include "invalid_dependency.h"

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

char find_first_operator(StringStruct ss) {

    for(int i = 0; i < ss.count; i++) {

        if(!c_isoperator(c_charat(&ss, i))) continue;
        return c_charat(&ss, i);
    }

    return '\0';
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
        
        //child's name
        char buffer[5];
        sprintf(buffer, "%c", 'A' + root->dependencies[i]->col);
        sprintf(buffer + 1, "%d", root->dependencies[i]->row);
        StringStruct cell_ref = ss_form_string_nt(buffer);
            
        Cell* parent_cell = cell_at(table, root->row, root->col);
        Cell* child_cell = cell_at(table, root->dependencies[i]->row, root->dependencies[i]->col);

        if(!was_visited(root->dependencies[i], visited)) {
            
            parent_cell->as.expression.expr = d_find_and_replace(parent_cell->as.expression.expr, cell_ref, dfs_solve(table, root->dependencies[i], visited));
        }
        else {

            parent_cell->as.expression.expr = d_find_and_replace(parent_cell->as.expression.expr, cell_ref, child_cell->as.number);
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

    printf(ANSI_GREEN "\n[SOLVE] No errors found. Solving..." ANSI_RESET"\n");

    solve_expressions(table, root);

    calculate_new_cell_width(table);
}