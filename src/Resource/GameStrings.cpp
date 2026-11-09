#pragma ipa file
#if !defined(jpn)

#include "Resource/GameStrings.h"
#include "Memory/SafeAllocator.h"
#include "Filesystem/BackgroundLoader.h"
#include "Filesystem/FileIO.h"

extern "C"
{
    void func_02012da4(AllocatorUnion*, const void*);
}
extern AllocatorUnion data_02114e20;

extern StringList data_02153694[2];
extern SafeAllocator data_02153680;

void InitializeGameStrings()
{
    for (int i = 0; i < 2; i++)
        data_02153694[i].Reset();
    data_02153680.ResetAllocatorPointer();
}

void DestroyGameStrings()
{
    for (int i = 0; i < 2; i++)
        data_02153694[i].Destroy();
    SignedAllocatorHeader* signedAlloc = data_02153680.GetSignedAllocator();
    if (signedAlloc != NULL)
    {
        func_02012da4(&data_02114e20, signedAlloc);
        data_02153680.Destroy();
    }
}

void ReserveGameStringMemory(void* data, unsigned int length)
{
    data_02153680.CreateTypeA(data, length);
    data_02153680.Reset();
}

bool LoadGameStrings()
{
    data_02153680.Reset();
    BackgroundLoader::AddLockGlobal();
    BackgroundLoader::FreeAllocationsGlobal();

    for (int i = 0; i < 2; i++)
    {
        void* data;
        unsigned int fileSize;
        StringList* list = &data_02153694[i];
        list->Reset();
        if (i != 0)
            data = ExtractFileFromGP2("data/prm/article.gp2", "article_<LG>.nat", &fileSize);
        else
            data = ExtractFileFromGP2("data/bin/strstd.gp2", "strstd_<LG>.nat", &fileSize);
        if (data != NULL)
            list->LoadFromMemory(&data_02153680, data, fileSize);
    }

    BackgroundLoader::RemoveLockGlobal();
    return false;
}

const char* GetGameString(eGameString id)
{
    return data_02153694[0].GetString((int)id);
}

const char* GetArticleTemplate(int id)
{
    return data_02153694[1].GetString(id);
}

#endif