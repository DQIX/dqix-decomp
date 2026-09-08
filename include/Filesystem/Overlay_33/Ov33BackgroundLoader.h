#pragma once

#include "../BackgroundLoader.h"

struct Ov33BackgroundLoader : public BackgroundLoader
{
    Ov33BackgroundLoader();
    virtual int Process();
};

void PopulateOv33BackgroundLoader(void* fileLoadSpace, unsigned int capacity, int relativePrio);