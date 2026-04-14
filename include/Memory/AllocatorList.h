#pragma once

#include <globaldefs.h>

class LinkedList
{
public:
    struct Node
    {
        char unknown_0[0x70];
        void* value;
        int unknown_74;
        LinkedList* pOwner;
        Node* pPrev;
        Node* pNext;
    };

    // The list appears to be sorted in increasing order of Node::value
    void Insert(Node* node);
    Node* PopFront();
    // Returns the passed node if found, and null if not
    Node* Remove(Node* node);

public:
    Node* pFirst;
    Node* pLast;
};