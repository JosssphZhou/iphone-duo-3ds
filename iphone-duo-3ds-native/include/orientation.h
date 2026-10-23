#pragma once
#include <stdbool.h>
#include <stdint.h>

#define ORIENTATION_STABLE_MS 500
#define ORIENTATION_SAMPLE_GAP_MS 250

typedef enum { ORIENTATION_UNKNOWN=-1, ORIENTATION_LANDSCAPE=0, ORIENTATION_BOOK=1 } OrientationTarget;
typedef struct {
    bool automatic;
    OrientationTarget candidate;
    uint64_t candidateSince,lastSample;
    unsigned sampleCount;
} OrientationFilter;

void orientationInit(OrientationFilter *filter);
void orientationReset(OrientationFilter *filter);
void orientationSetAutomatic(OrientationFilter *filter,bool enabled);
OrientationTarget orientationClassify(int x,int y,int z);
/* Returns true only after an eligible different orientation has stayed stable.
 * Input is the libctru/Azahar accelerometer vector (about 512 units per g).
 * X/Z are the screen plane in the source-derived convention; Y is its normal.
 * Physical hardware signs still require calibration against an actual console.
 */
bool orientationUpdate(OrientationFilter *filter,int x,int y,int z,uint64_t now,
                       bool interactionBusy,bool currentPortrait,bool *nextPortrait);
