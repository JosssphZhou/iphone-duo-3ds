#pragma once
#include "geometry.h"

typedef enum { GESTURE_NONE, GESTURE_TAP, GESTURE_SEEK, GESTURE_EDGE, GESTURE_PHOTO } GestureKind;
typedef enum { GESTURE_IGNORED, GESTURE_CLICK, GESTURE_CHANGED, GESTURE_HOME } GestureResult;
typedef struct {
    GestureKind kind;
    UiRect region;
    float startX,startY,x,y;
    bool active;
} UiGesture;
void gestureBegin(UiGesture *g,GestureKind kind,UiRect region,float x,float y);
void gestureMove(UiGesture *g,float x,float y);
void gestureCancel(UiGesture *g);
GestureResult gestureEnd(UiGesture *g,float x,float y);
float gestureFraction(const UiGesture *g);
UiRect gestureSeekTarget(UiRect visual);
bool gestureEdgeStart(float y,float height,bool locked,bool desktop);
float transitionEase(float progress);
