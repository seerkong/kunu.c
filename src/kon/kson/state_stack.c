#include "state_stack.h"

StateStack* StateStackInit()
{
    StateStack* stack = (StateStack*)malloc(sizeof(StateStack));
    if (stack == NULL) {
        return NULL;
    }
    stack->Length = 0;
    stack->Top = NULL;
    return stack;
}

void StateStackDestroy(StateStack* stack)
{
    // TODO iter node, free
    StateStackNode* top = stack->Top;
    while (top) {
        StateStackNode* oldTop = top;
        free(oldTop);
        top = top->Next;
    }
    free(stack);
}

void StateStackPush(StateStack* stack, long item)
{
    assert(stack);
    StateStackNode* oldTop = stack->Top;
    StateStackNode* newTop = (StateStackNode*)malloc(sizeof(StateStackNode));
    assert(newTop);
    newTop->Data = item;
    newTop->Next = oldTop;
    stack->Top = newTop;
    stack->Length = stack->Length + 1;
}

long StateStackPop(StateStack* stack)
{
    assert(stack);
    StateStackNode* top = stack->Top;
    assert(top);
    StateStackNode* next = stack->Top->Next;

    long data = top->Data;
    stack->Top = next;
    stack->Length = stack->Length - 1;

    free(top);
    return data;
}

long StateStackLength(StateStack* stack)
{
    assert(stack);
    return stack->Length;
}

long StateStackTop(StateStack* stack)
{
    assert(stack);
    StateStackNode* top = stack->Top;
    assert(top);
    return top->Data;
}

void StateStackSetTopValue(StateStack* stack, long newData)
{
    assert(stack);
    StateStackNode* top = stack->Top;
    assert(top);
    top->Data = newData;
}