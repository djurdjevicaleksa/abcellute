#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#define DECIMAL_PLACES 3
#define EXTRA_CELL_SPACE 2
#define GRAPH_NODE_BUFFER_SIZE 32           // node buffer of one entire expression contains this many nodes
#define NODE_LIST_SIZE 32                   // node stack contains this many nodes
#define SYNTAX_ERRORS_BUFF_SIZE 2048         // buffer for printing all syntax errors
#define GRAPH_CYCLE_BUFFER_SIZE 256              // buffer for printing a dependency cycle
#define INVALID_DEPENDENCY_BUFFER_SIZE 256  // buffer for printing invalid dependency
#define EQUATION_TOKENS                 16

#define ANSI_RESET              "\x1b[0m"
#define ANSI_RED                "\x1b[31m"
#define ANSI_GREEN              "\x1b[32m"
#define ANSI_BOLD_RED           "\x1b[1;31m"
#define ANSI_YELLOW             "\x1b[33m"
#define ANSI_BLUE               "\x1b[34m"
#define ANSI_MAGENTA            "\x1b[35m"
#define ANSI_CYAN               "\x1b[36m"
#define ANSI_FAINT_RED          "\x1b[2;31m"
#define ANSI_BLACK              "\033[0;30m"
#define ANSI_WHITE              "\033[0;37m"      

#endif //_CONSTANTS_H