#include "Graphics/NSBXX/NSBXX.h"

extern "C" void* NSBXXNameList_Search(const NSBXXNameList* nameList, const char* name)
{
    const uint32_t* target = (const uint32_t*)name;
    if (name == NULL)
        return NULL;

    if (nameList->numEntries_ < 16) // list is short, do linear search
    {
        unsigned int searchIndex;
        uint32_t target0 = target[0];
        uint32_t target1 = target[1];
        uint32_t target2 = target[2];
        uint32_t target3 = target[3];
        
        for (searchIndex = 0; searchIndex < nameList->numEntries_; searchIndex++)
        {
            const uint32_t* source = (const uint32_t*)nameList->GetNameByIndex(searchIndex);
            if (source[0] == target0 && source[1] == target1 &&
                source[2] == target2 && source[3] == target3)
            {
                return nameList->GetEntryByIndex_notVolatile<void>(searchIndex);
            }
        }
    }
    else // list is long, use the binary search tree
    {   
        NSBXXNameList::SearchTreeEntry* entryArray = (NSBXXNameList::SearchTreeEntry*)&nameList->treeRoot_8_;
        int firstChild = entryArray[0].children_[0];
        if (firstChild != 0)
        {
            NSBXXNameList::SearchTreeEntry* searchCursor = &entryArray[firstChild];
            int bitIndex = entryArray[firstChild].bitIndex_;
            unsigned int prevBitIndex = entryArray[0].bitIndex_;
            while (prevBitIndex > bitIndex)
            {
                int integerToQuery = bitIndex >> 5;
                int bitToQuery = bitIndex & 0x1f;
                int bitValue = (target[integerToQuery] >> bitToQuery) & 1;
                int childID = searchCursor->children_[bitValue];
                bitIndex = entryArray[childID].bitIndex_;
                prevBitIndex = searchCursor->bitIndex_;
                searchCursor = &entryArray[childID];
            }
            
            unsigned int candidateIndex = searchCursor->resourceIndex_;
            const uint32_t* source = (const uint32_t*)nameList->GetNameByIndex(candidateIndex);
            
            if (source[0] == target[0] && source[1] == target[1] &&
                source[2] == target[2] && source[3] == target[3])
            {
                return nameList->GetEntryByIndex_notVolatile<void>(candidateIndex);
            }
        }
    }

    return NULL;
}

extern "C" int NSBXXNameList_SearchIndex(const NSBXXNameList* nameList, const char* name)
{
    const uint32_t* target = (const uint32_t*)name;
    if (name == NULL)
        return -1;

    if (nameList->numEntries_ < 16) // list is short, do linear search
    {
        unsigned int searchIndex;
        uint32_t target0 = target[0];
        uint32_t target1 = target[1];
        uint32_t target2 = target[2];
        uint32_t target3 = target[3];
        for (searchIndex = 0; searchIndex < nameList->numEntries_; searchIndex++)
        {
            const uint32_t* source = (const uint32_t*)nameList->GetNameByIndex(searchIndex);
            if (source[0] == target0 && source[1] == target1 &&
                source[2] == target2 && source[3] == target3)
            {
                return searchIndex;
            }
        }
    }
    else // list is long, use the binary search tree
    {   
        NSBXXNameList::SearchTreeEntry* entryArray = (NSBXXNameList::SearchTreeEntry*)&nameList->treeRoot_8_;
        int firstChild = entryArray[0].children_[0];
        if (firstChild != 0)
        {
            NSBXXNameList::SearchTreeEntry* searchCursor = &entryArray[firstChild];
            int bitIndex = entryArray[firstChild].bitIndex_;   
            unsigned int prevBitIndex = entryArray[0].bitIndex_;
            while (prevBitIndex > bitIndex)
            {
                int integerToQuery = bitIndex >> 5;
                int bitToQuery = bitIndex & 0x1f;
                int bitValue = (target[integerToQuery] >> bitToQuery) & 1;
                int childID = searchCursor->children_[bitValue];
                bitIndex = entryArray[childID].bitIndex_;
                prevBitIndex = searchCursor->bitIndex_;
                searchCursor = &entryArray[childID];
            }
            
            const uint32_t* sourceIntArray = (const uint32_t*)nameList->GetNameByIndex(searchCursor->resourceIndex_);
            if (sourceIntArray[0] == target[0] && sourceIntArray[1] == target[1] &&
                sourceIntArray[2] == target[2] && sourceIntArray[3] == target[3])
                return searchCursor->resourceIndex_;
            
        }
    }

    return -1;
}