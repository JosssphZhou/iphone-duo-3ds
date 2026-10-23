#pragma once
#include "model.h"
typedef enum { LAYOUT_LOADED, LAYOUT_MISSING, LAYOUT_INVALID, LAYOUT_IO_ERROR, LAYOUT_MIGRATED } LayoutLoadResult;
LayoutLoadResult layoutLoad(Model *model,const char *path);
bool layoutSave(const Model *model,const char *path);
