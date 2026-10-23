#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef enum { LOCK_STILL, LOCK_DRAGGING, LOCK_RETURNING, LOCK_LEAVING } LockPhase;
typedef struct {
    LockPhase phase;
    float startX,startY,offset,from,to;
    uint64_t started;
    bool axisChosen,rejected;
} LockMotion;
void lockMotionCancel(LockMotion *motion);
bool lockMotionBegin(LockMotion *motion,float x,float y,float height);
void lockMotionMove(LockMotion *motion,float x,float y);
void lockMotionEnd(LockMotion *motion,float x,float y,float height,uint64_t now);
bool lockMotionStep(LockMotion *motion,uint64_t now);
