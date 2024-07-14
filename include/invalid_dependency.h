#ifndef _INVALID_DEPENDENCY_H
#define _INVALID_DEPENDENCY_H


void report_invalid_dependency(Recstack* recstack, Cell* target_cell, const char* err_message);
bool dfs_invalid_dependency(Table* table, Node* root, VisitedNodes* visited, Recstack* recstack);
bool invalid_dependencies_exist(Table* table, Node* root);


#endif //_INVALID_DEPENDENCY_H