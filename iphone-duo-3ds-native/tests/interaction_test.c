#include "interaction.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
int main(void){
    UiGesture g={0};UiRect track={13,53.5f,294,3};UiRect target=gestureSeekTarget(track);
    assert(target.h>=16&&uiContains(target,60,45)&&uiContains(target,60,64));
    gestureBegin(&g,GESTURE_SEEK,track,20,55);gestureMove(&g,160,100);
    assert(fabsf(gestureFraction(&g)-.5f)<.001f);assert(gestureEnd(&g,307,55)==GESTURE_CHANGED);
    assert(gestureFraction(&g)==1);assert(gestureEnd(&g,100,55)==GESTURE_IGNORED);
    gestureBegin(&g,GESTURE_SEEK,track,100,55);gestureMove(&g,-100,55);assert(gestureFraction(&g)==0);
    gestureCancel(&g);assert(gestureEnd(&g,200,55)==GESTURE_IGNORED);
    assert(gestureEdgeStart(230,240,false,false));assert(!gestureEdgeStart(229,240,false,false));
    assert(!gestureEdgeStart(235,240,true,false));assert(!gestureEdgeStart(235,240,false,true));
    gestureBegin(&g,GESTURE_EDGE,(UiRect){0,230,320,10},160,236);assert(gestureEnd(&g,160,200)==GESTURE_HOME);
    gestureBegin(&g,GESTURE_EDGE,(UiRect){0,230,320,10},160,236);assert(gestureEnd(&g,160,205)==GESTURE_IGNORED);
    gestureBegin(&g,GESTURE_EDGE,(UiRect){0,230,320,10},160,236);assert(gestureEnd(&g,220,195)==GESTURE_IGNORED);
    gestureBegin(&g,GESTURE_TAP,(UiRect){10,10,40,40},20,20);assert(gestureEnd(&g,22,22)==GESTURE_CLICK);
    gestureBegin(&g,GESTURE_TAP,(UiRect){10,10,40,40},20,20);assert(gestureEnd(&g,60,20)==GESTURE_IGNORED);
    gestureBegin(&g,GESTURE_PHOTO,(UiRect){18,55,284,76},30,60);gestureMove(&g,280,60);assert(gestureFraction(&g)>.5f);assert(gestureEnd(&g,280,60)==GESTURE_CHANGED);
    assert(transitionEase(-1)==0&&transitionEase(2)==1&&transitionEase(.5f)>.5f);
    puts("interaction_test: PASS (captured seek, cancellation, edge-only home, tap bounds, photo scrub, easing)");
}
