#pragma once
#include <stddef.h>
#include "model.h"

/* Four untouched website crops: landscape top/bottom, portrait top/bottom. */
typedef struct { App app;const char *atlasPath; } OfficialAppAssets;
extern const OfficialAppAssets *officialApps;
extern const size_t officialAppCount;
