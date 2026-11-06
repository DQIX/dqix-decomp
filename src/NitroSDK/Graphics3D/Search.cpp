#include "NitroSDK/Graphics3D/Graphics3D.h"

void* Nitro3D_GetNameListEntry(const Nitro3DNameList* nameList, const char* name)
{
    const uint32_t* target = (const uint32_t*)name;
    if (name == NULL)
        return NULL;

    if (nameList->numEntries < 16) // list is short, do linear search
    {
        unsigned int searchIndex;
        uint32_t target0 = target[0];
        uint32_t target1 = target[1];
        uint32_t target2 = target[2];
        uint32_t target3 = target[3];
        
        for (searchIndex = 0; searchIndex < nameList->numEntries; searchIndex++)
        {
            const uint32_t* source = (const uint32_t*)nameList->GetNameByIndex(searchIndex);
            if (source[0] == target0 && source[1] == target1 &&
                source[2] == target2 && source[3] == target3)
            {
                return nameList->GetEntryByIndex<void>(searchIndex);
            }
        }
    }
    else // list is long, use the binary search tree
    {   
        Nitro3DNameList::SearchTreeEntry* entryArray = (Nitro3DNameList::SearchTreeEntry*)&nameList->treeRoot;
        int firstChild = entryArray[0].children[0];
        if (firstChild != 0)
        {
            Nitro3DNameList::SearchTreeEntry* searchCursor = &entryArray[firstChild];
            int bitIndex = entryArray[firstChild].bitIndex;
            unsigned int prevBitIndex = entryArray[0].bitIndex;
            while (prevBitIndex > bitIndex)
            {
                int integerToQuery = bitIndex >> 5;
                int bitToQuery = bitIndex & 0x1f;
                int bitValue = (target[integerToQuery] >> bitToQuery) & 1;
                int childID = searchCursor->children[bitValue];
                bitIndex = entryArray[childID].bitIndex;
                prevBitIndex = searchCursor->bitIndex;
                searchCursor = &entryArray[childID];
            }
            
            unsigned int candidateIndex = searchCursor->resourceIndex;
            const uint32_t* source = (const uint32_t*)nameList->GetNameByIndex(candidateIndex);
            
            if (source[0] == target[0] && source[1] == target[1] &&
                source[2] == target[2] && source[3] == target[3])
            {
                return nameList->GetEntryByIndex<void>(candidateIndex);
            }
        }
    }

    return NULL;
}

int Nitro3D_GetNameListIndex(const Nitro3DNameList* nameList, const char* name)
{
    const uint32_t* target = (const uint32_t*)name;
    if (name == NULL)
        return -1;

    if (nameList->numEntries < 16) // list is short, do linear search
    {
        unsigned int searchIndex;
        uint32_t target0 = target[0];
        uint32_t target1 = target[1];
        uint32_t target2 = target[2];
        uint32_t target3 = target[3];
        for (searchIndex = 0; searchIndex < nameList->numEntries; searchIndex++)
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
        Nitro3DNameList::SearchTreeEntry* entryArray = (Nitro3DNameList::SearchTreeEntry*)&nameList->treeRoot;
        int firstChild = entryArray[0].children[0];
        if (firstChild != 0)
        {
            Nitro3DNameList::SearchTreeEntry* searchCursor = &entryArray[firstChild];
            int bitIndex = entryArray[firstChild].bitIndex;   
            unsigned int prevBitIndex = entryArray[0].bitIndex;
            while (prevBitIndex > bitIndex)
            {
                int integerToQuery = bitIndex >> 5;
                int bitToQuery = bitIndex & 0x1f;
                int bitValue = (target[integerToQuery] >> bitToQuery) & 1;
                int childID = searchCursor->children[bitValue];
                bitIndex = entryArray[childID].bitIndex;
                prevBitIndex = searchCursor->bitIndex;
                searchCursor = &entryArray[childID];
            }
            
            const uint32_t* sourceIntArray = (const uint32_t*)nameList->GetNameByIndex(searchCursor->resourceIndex);
            if (sourceIntArray[0] == target[0] && sourceIntArray[1] == target[1] &&
                sourceIntArray[2] == target[2] && sourceIntArray[3] == target[3])
                return searchCursor->resourceIndex;
            
        }
    }

    return -1;
}


#define SUBFILE_OFFSETS(nsb) ((uint32_t*)((intptr_t)(nsb) + nsb->headerSize))

void* Nitro3D_GetFirstSubfile(Nitro3DFileHeader* nsbxx)
{
    return (char*)nsbxx + SUBFILE_OFFSETS(nsbxx)[0];
}

Nitro3DTextureSet* Nitro3D_GetTexturesFromFile(Nitro3DFileHeader* nsbxx)
{
    const uint32_t* subfiles = SUBFILE_OFFSETS(nsbxx);
    if (nsbxx->numSubfiles == 1)
    {
        if (nsbxx->signature == SIGNATURE_NSBTX)
            return (Nitro3DTextureSet*)((intptr_t)nsbxx + subfiles[0]);

        return NULL;
    }
    return (Nitro3DTextureSet*)((intptr_t)nsbxx + subfiles[1]);
}

void* Nitro3D_GetInnerObjectByIndex(Nitro3DFileHeader* nsbxx, unsigned int idx)
{
    if (nsbxx != NULL)
    {
        Nitro3DInnerFileHeader* innerData = (Nitro3DInnerFileHeader*)((intptr_t)nsbxx + SUBFILE_OFFSETS(nsbxx)[0]);
        uint32_t* pOffset = innerData->nameList.GetEntryByIndex<uint32_t>(idx);
        if (pOffset != NULL)
            return (char*)innerData + *pOffset;
    }
    return NULL;
}
