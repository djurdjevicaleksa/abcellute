#include <stdbool.h>
#include <stdio.h>

#include "graph.h"
#include "ss.h"
#include "table.h"
#include "equation_solver.h"

extern int expression_count;

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