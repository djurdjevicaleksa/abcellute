#ifndef _ELEMENT_H
#define _ELEMENT_H

#include <aio.h>
#include <assert.h>

#define INVALID_ELEMENT {   \
    .kind = ELEMENT_INV     \
}

#define PRINT_ELEMENT(element) {                                                    \
    if((element).kind == ELEMENT_NUM) printf("Element: %f\n", (element).as.number); \
    else printf("Element: %c\n", (element).as.operator);                            \
}

#define ELEMENT_STACK_SIZE 16
#define STACK_INIT {        \
    .elements = {0},        \
    .count = 0              \
}

#define ELEMENT_QUEUE_SIZE 32
#define QUEUE_INIT {        \
    .elements = {0},        \
    .first = 0,             \
    .last = 0,              \
    .count = 0              \
}

typedef union {
    double number;
    char operator;
} ElementAs;

typedef enum {
    ELEMENT_NUM = 0,
    ELEMENT_OP,
    ELEMENT_INV
} ElementKind;

typedef struct {
    ElementKind kind;
    ElementAs as;
} Element;

typedef struct {
    Element elements[ELEMENT_STACK_SIZE];
    size_t count;
} ElementStack;

typedef struct {
    Element elements[ELEMENT_QUEUE_SIZE];
    size_t first;
    size_t last;
    size_t count;
} ElementQueue;

void    stack_push(ElementStack* stack, Element element);
Element stack_pop(ElementStack* stack);

void    queue_push(ElementQueue* queue, Element element);
Element queue_pop(ElementQueue* queue);












#endif //_ELEMENT_H