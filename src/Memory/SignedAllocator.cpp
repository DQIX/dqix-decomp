#include "Memory/SignedAllocator.h"

namespace AllocatorTree
{
    int initializedTopLevel;
    SignedAllocatorList topLevel;
}

// USA: func_020aeec8
void SignedAllocatorList::Initialize(unsigned short sigLength)
{
    pFirst = NULL;
    pLast = NULL;
    numElements = 0;
    signatureLength = sigLength;
}

// USA: func_020aeee0
void SignedAllocatorList::DoInitialInsertion(SignedAllocatorHeader* what)
{
    SignedAllocatorHeader** whatPtrs = (SignedAllocatorHeader**)((char*)what + signatureLength);
    whatPtrs[1] = NULL;
    whatPtrs[0] = NULL;

    pFirst = what;
    pLast = what;

    numElements++;
}

// USA: func_020aef0c
void SignedAllocatorList::InsertAtEnd(SignedAllocatorHeader* what)
{
    if (pFirst == NULL)
        return DoInitialInsertion(what);

    SignedAllocatorHeader** whatPtrs = (SignedAllocatorHeader**)((char*)what + signatureLength);
    whatPtrs[0] = pLast;
    whatPtrs[1] = NULL;

    ((SignedAllocatorHeader**)((char*)pLast + signatureLength))[1] = what;
    pLast = what;

    numElements++;
}

// USA: func_020aef60
void SignedAllocatorList::InsertAtStart(SignedAllocatorHeader* what)
{
    if (pFirst == NULL)
        return DoInitialInsertion(what);

    SignedAllocatorHeader** whatPtrs = (SignedAllocatorHeader**)((char*)what + signatureLength);

    whatPtrs[0] = NULL;
    whatPtrs[1] = pFirst;

    *(SignedAllocatorHeader**)((char*)pFirst + signatureLength) = what;
    pFirst = what;

    numElements++;
}

// USA: func_020aefb0
void SignedAllocatorList::InsertBefore(SignedAllocatorHeader* where, SignedAllocatorHeader* what)
{
    if (where == NULL)
        return InsertAtEnd(what);

    if (where == pFirst)
        return InsertAtStart(what);
    
    SignedAllocatorHeader** whatPtrs = (SignedAllocatorHeader**)((char*)what + signatureLength);
    SignedAllocatorHeader* wherePrev = *(SignedAllocatorHeader**)((char*)where + signatureLength);
    SignedAllocatorHeader** wherePrevPtrs = (SignedAllocatorHeader**)((char*)wherePrev + signatureLength);

    whatPtrs[0] = wherePrev;
    whatPtrs[1] = where;
    wherePrevPtrs[1] = what;
    *(SignedAllocatorHeader**)((char*)where + signatureLength) = what;

    numElements++;
}

// USA: func_020af014
void SignedAllocatorList::Remove(SignedAllocatorHeader* what)
{
    SignedAllocatorHeader** prevAndNext = (SignedAllocatorHeader**)((char*)what + signatureLength);
    
    if (prevAndNext[0] == NULL)
        pFirst = prevAndNext[1];
    else
        *(SignedAllocatorHeader**)((char*)prevAndNext[0] + signatureLength + 4) = prevAndNext[1];

    if (prevAndNext[1] == NULL)
        pLast = prevAndNext[0];
    else
        *(SignedAllocatorHeader**)((char*)prevAndNext[1] + signatureLength) = prevAndNext[0];

    prevAndNext[0] = NULL;
    prevAndNext[1] = NULL;
    numElements--;
}

// USA: func_020af074
SignedAllocatorHeader* SignedAllocatorList::ElementAfter(SignedAllocatorHeader* what)
{
    if (what == NULL)
        return pFirst;
    
    return *(SignedAllocatorHeader**)((char*)what + signatureLength + 4);
}

// USA: func_020af08c
SignedAllocatorHeader* SignedAllocatorList::ElementBefore(SignedAllocatorHeader* what)
{
    if (what == NULL)
        return pLast;

    return *(SignedAllocatorHeader**)((char*)what + signatureLength);
}

// USA: func_020af0a0
SignedAllocatorHeader* SignedAllocatorList::GetNthElement(unsigned int n)
{
    unsigned int loopIdx = 0;
    SignedAllocatorHeader* loopNode = this->ElementAfter(NULL);
    if (loopNode != NULL)
    {
        do
        {
            if (loopIdx == n)
                return loopNode;

            loopIdx++;
            loopNode = this->ElementAfter(loopNode);
        } while (loopNode != NULL);
    }

    return NULL;
}

// USA: func_020af0e8
SignedAllocatorHeader* AllocatorTree::GetParent(SignedAllocatorList* searchList, SignedAllocatorHeader* alloc)
{
    // Loop through the list to find which element is an ancestor
    SignedAllocatorHeader* ancestor = searchList->ElementAfter(NULL);
    if (ancestor != NULL)
    {
        do
        {
            // Allocators manage a contiguous block, and siblings in the list
            // don't share any space, so this range condition is satisfied by
            // at most one ancestor.
            if (ancestor->allocBegin <= alloc && alloc < ancestor->allocEnd)
            {
                // Having found the ancestor, look among its children & descendants
                SignedAllocatorHeader* answer = GetParent(&ancestor->children, alloc);
                if (answer != NULL)
                    return answer;
                // If we couldn't find it among the descendants, then it must be that 
                // ancestor is actually the node we're looking for
                return ancestor;
            }
    
            ancestor = searchList->ElementAfter(ancestor);
        } while (ancestor != NULL);
    }

    return NULL;
}

// USA: func_020af150
SignedAllocatorList* AllocatorTree::GetListContainingAllocator(SignedAllocatorHeader* alloc)
{
    SignedAllocatorList* list = &topLevel;
    SignedAllocatorHeader* parent = GetParent(list, alloc);
    if (parent != NULL)
        list = &parent->children;
    return list;
}

void AllocatorTree::InsertNewAndPopulateHeader(SignedAllocatorHeader* allocator,
    unsigned int sig, void* allocBegin, void* allocEnd, unsigned short clearFlags)
{
    SignedAllocatorHeader* header = (SignedAllocatorHeader*)allocator;
    header->signature = sig;
    header->allocBegin = allocBegin;
    header->allocEnd = allocEnd;
    // wtf?!
    header->clearFlags = 0;
    header->clearFlags &= ~0xFF;
    header->clearFlags |= clearFlags & 0xFF;
    
    header->children.Initialize(4);
    if (!initializedTopLevel)
    {
        topLevel.Initialize(4);
        initializedTopLevel = 1;
    }

    SignedAllocatorList* containList = GetListContainingAllocator(header);
    containList->InsertAtEnd(header);
}

void AllocatorTree::Remove(SignedAllocatorHeader* alloc)
{
    SignedAllocatorList* containList = GetListContainingAllocator(alloc);
    containList->Remove(alloc);
}