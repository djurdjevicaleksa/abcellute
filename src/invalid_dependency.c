#include <stdio.h>

#include "graph.h"
#include "table.h"

#include "invalid_dependency.h"


void report_invalid_dependency(Recstack* recstack, Cell* target_cell, const char* err_message) {

    char buffer[INVALID_DEPENDENCY_BUFFER_SIZE];
    memset(buffer, '\0', sizeof(buffer));
    size_t iterator = 0;

    iterator += sprintf(buffer + iterator, ANSI_RED "%s" ANSI_RESET, err_message);
    for(size_t i = 0; i < recstack->count - 1; i++) {

        iterator += sprintf(buffer + iterator, "%c%d -> ", 'A' + recstack->nodes[i]->col, recstack->nodes[i]->row);
    }

    switch(target_cell->kind) {

        case KIND_EXPR: {

            sprintf(buffer + iterator, "( %c%d = '"SSFormat"')\n",
            'A' + recstack->nodes[recstack->count - 1]->col, recstack->nodes[recstack->count - 1]->row, SSArg(target_cell->as.expression.expr));
            break;
        }

        case KIND_TEXT: {

            sprintf(buffer + iterator, "( %c%d = '"SSFormat"')\n",
            'A' + recstack->nodes[recstack->count - 1]->col, recstack->nodes[recstack->count - 1]->row, SSArg(target_cell->as.text));
            break;
        }

        case KIND_EMPTY: {

            sprintf(buffer + iterator, "( %c%d = '"SSFormat"')\n",
            'A' + recstack->nodes[recstack->count - 1]->col, recstack->nodes[recstack->count - 1]->row, SSArg(target_cell->as.text));
            break;
        }

        default: {

            assert(0 && "Unreachable code.");
        }
    }
    
    printf("%s", buffer);
}

bool dfs_invalid_dependency(Table* table, Node* root, VisitedNodes* visited, Recstack* recstack) {

    assert(root != NULL);

    push_stack(visited, root);
    push_stack(recstack, root);

    //check if current node is valid
    Cell* target_cell = cell_at(table, root->row, root->col);
    assert(target_cell != NULL);

    switch(target_cell->kind) {

        case KIND_NUM: {
            
            break;
        }

        case KIND_EXPR: {

            if(target_cell->as.expression.kind == EXPR_INVALID) { //report cells who cant depend on an invalid cell

                report_invalid_dependency(recstack, target_cell, "[DEPCHK] Expressions depend on an invalid expression cell.\n");
                return true;
            }
            
            break;
        }

        case KIND_TEXT: { //report that cells cant depend on a cell which is a string

            report_invalid_dependency(recstack, target_cell, "[DEPCHK] Expressions depend on a text cell.\n");
            return true;
        }

        case KIND_EMPTY: { //report that cells cant depend on an empty cell

            report_invalid_dependency(recstack, target_cell, "[DEPCHK] Expressions depend on an empty cell.\n");
            return true;
        }

        default: {

            assert(0 && "Unreachable code.");
        }
    }

    for(size_t i = 0; i < root->count; i++) {

        if(dfs_invalid_dependency(table, root->dependencies[i], visited, recstack)) return true;
    }

    pop_stack(recstack);
    return false;
}

bool invalid_dependencies_exist(Table* table, Node* root) {

    VisitedNodes visited = {0};
    Recstack recstack = {0};

    for(size_t i = 0; i < root->count; i++) {

        if(!was_visited(root->dependencies[i], &visited)) {

            if(dfs_invalid_dependency(table, root->dependencies[i], &visited, &recstack)) {
                
                return true;
            }
        }
    }

    return false;
}