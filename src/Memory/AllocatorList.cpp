#include "Memory/AllocatorList.h"

void LinkedList::Insert(Node* node)
{
    Node* entryAfter = pFirst;
    while (entryAfter != NULL && entryAfter->value <= node->value)
    {
        if (entryAfter == node)
            return;
        entryAfter = entryAfter->pNext;
    }

    if (entryAfter == NULL)
    {
        Node* entryBefore = pLast;
        if (entryBefore == NULL)
        {
            pFirst = node;
        }
        else
        {
            entryBefore->pNext = node;
        }
        node->pPrev = entryBefore;
        node->pNext = NULL;
        pLast = node;
    }
    else
    {
        Node* entryBefore = entryAfter->pPrev;
        if (entryBefore == NULL)
        {
            pFirst = node;
        }
        else
        {
            entryBefore->pNext = node;
        }
        node->pPrev = entryBefore;
        node->pNext = entryAfter;
        entryAfter->pPrev = node;
    }
}

LinkedList::Node* LinkedList::PopFront()
{
    Node* front = pFirst;
    if (front != NULL)
    {
        Node* second = front->pNext;
        pFirst = second;
        if (second != NULL)
        {
            second->pPrev = NULL;
        }
        else
        {
            pLast = NULL;
            front->pOwner = NULL;
        }
    }
    return front;
}

LinkedList::Node* LinkedList::Remove(Node* node)
{
    Node* searchNode = pFirst;
    if (pFirst != NULL)
    {
        while (true)
        {
            Node* nodeAfter = searchNode->pNext;
            if (searchNode == node)
            {
                Node* nodeBefore = searchNode->pPrev;
                if (pFirst == searchNode)
                    pFirst = nodeAfter;
                else
                    nodeBefore->pNext = nodeAfter;
            
                if (pLast == searchNode)
                    pLast = nodeBefore;
                else
                    nodeAfter->pPrev = nodeBefore;
                break;
            }
    
            searchNode = nodeAfter;
            if (nodeAfter == NULL)
                break;
        }
    }
    return searchNode;
}