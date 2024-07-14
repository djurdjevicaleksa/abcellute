#include "element.h"


#define _SS_IMPLEMENT
#include "ss.h"


void stack_push(ElementStack* stack, Element element) {

    assert(stack->count < ELEMENT_STACK_SIZE);
    stack->elements[stack->count++] = element;
}

Element stack_pop(ElementStack* stack) {

    if(stack->count > 0) {
        return stack->elements[--stack->count];;
    }

    Element inv = INVALID_ELEMENT;
    return inv;
}

void queue_push(ElementQueue* queue, Element element) {

    assert(queue->last < ELEMENT_QUEUE_SIZE);
    queue->elements[queue->last++] = element;
    queue->count++;
}

Element queue_pop(ElementQueue* queue) {

    if(queue->last - queue->first != 0) {

        queue->count--;
        return queue->elements[queue->first++];
    }
    
    Element inv = INVALID_ELEMENT;
    return inv;
}
