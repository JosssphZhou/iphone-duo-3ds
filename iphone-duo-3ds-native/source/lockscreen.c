#include "lockscreen.h"
#include <math.h>
#include <string.h>
void lockMotionCancel(LockMotion*m){memset(m,0,sizeof(*m));}
bool lockMotionBegin(LockMotion*m,float x,float y,float h){
    if(m->phase==LOCK_LEAVING||y<h-64||y>=h)return false;
    lockMotionCancel(m);m->phase=LOCK_DRAGGING;m->startX=x;m->startY=y;return true;
}
void lockMotionMove(LockMotion*m,float x,float y){
    if(m->phase!=LOCK_DRAGGING||m->rejected)return;
    float dx=fabsf(x-m->startX),up=m->startY-y;
    if(!m->axisChosen){
        if(up>=8&&up>=dx*1.2f)m->axisChosen=true;
        else if((dx>=8&&dx>=fabsf(up)*1.2f)||up<=-8){m->rejected=true;return;}
    }
    if(m->axisChosen)m->offset=fmaxf(0,up);
}
void lockMotionEnd(LockMotion*m,float x,float y,float h,uint64_t now){
    if(m->phase!=LOCK_DRAGGING)return;
    lockMotionMove(m,x,y);
    float up=m->startY-y,dx=fabsf(x-m->startX);
    bool unlock=m->axisChosen&&!m->rejected&&up>=64&&up>=dx*1.2f;
    m->from=m->offset;m->to=unlock?h+8:0;m->started=now;m->phase=unlock?LOCK_LEAVING:LOCK_RETURNING;
}
bool lockMotionStep(LockMotion*m,uint64_t now){
    if(m->phase!=LOCK_LEAVING&&m->phase!=LOCK_RETURNING)return false;
    float p=now>=m->started?(now-m->started)/180.f:0;if(p>1)p=1;
    float ease=1-(1-p)*(1-p)*(1-p);m->offset=m->from+(m->to-m->from)*ease;
    if(p<1)return false;
    bool unlocked=m->phase==LOCK_LEAVING;lockMotionCancel(m);return unlocked;
}
