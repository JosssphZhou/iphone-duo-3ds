#include "interaction.h"
#include <math.h>
#include <string.h>
void gestureBegin(UiGesture*g,GestureKind kind,UiRect region,float x,float y){
    *g=(UiGesture){.kind=kind,.region=region,.startX=x,.startY=y,.x=x,.y=y,.active=true};
}
void gestureMove(UiGesture*g,float x,float y){if(g->active){g->x=x;g->y=y;}}
void gestureCancel(UiGesture*g){memset(g,0,sizeof(*g));}
GestureResult gestureEnd(UiGesture*g,float x,float y){
    if(!g->active)return GESTURE_IGNORED;
    gestureMove(g,x,y);float dx=x-g->startX,dy=y-g->startY;
    GestureResult result=GESTURE_IGNORED;
    if(g->kind==GESTURE_SEEK||g->kind==GESTURE_PHOTO)result=GESTURE_CHANGED;
    else if(g->kind==GESTURE_EDGE&&dy<=-35&&-dy>fabsf(dx)*1.2f)result=GESTURE_HOME;
    else if(g->kind==GESTURE_TAP&&fabsf(dx)<18&&fabsf(dy)<18&&uiContains(g->region,x,y))result=GESTURE_CLICK;
    g->active=false;return result;
}
float gestureFraction(const UiGesture*g){if(g->region.w<=0)return 0;return fminf(1,fmaxf(0,(g->x-g->region.x)/g->region.w));}
UiRect gestureSeekTarget(UiRect r){float mid=r.y+r.h/2;r.y=mid-12;r.h=24;return r;}
bool gestureEdgeStart(float y,float height,bool locked,bool desktop){return !locked&&!desktop&&y>=height-10&&y<height;}
float transitionEase(float p){p=fminf(1,fmaxf(0,p));return 1-(1-p)*(1-p)*(1-p);}
