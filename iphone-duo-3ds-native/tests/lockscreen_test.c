#include "lockscreen.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
int main(void){
    LockMotion m={0};
    for(int h=240;h<=320;h+=80){
        assert(!lockMotionBegin(&m,100,h-65,h));assert(m.phase==LOCK_STILL);
        assert(lockMotionBegin(&m,100,h-12,h));lockMotionMove(&m,102,h-42);
        assert(m.phase==LOCK_DRAGGING&&m.offset==30);lockMotionEnd(&m,102,h-42,h,1000);
        assert(m.phase==LOCK_RETURNING);assert(!lockMotionStep(&m,1090)&&m.offset>0&&m.offset<30);
        assert(!lockMotionStep(&m,1180)&&m.phase==LOCK_STILL&&m.offset==0);
        assert(lockMotionBegin(&m,100,h-10,h));lockMotionMove(&m,130,h-12);
        assert(m.rejected);lockMotionMove(&m,130,h-100);lockMotionEnd(&m,130,h-100,h,2000);
        assert(m.phase==LOCK_RETURNING);assert(!lockMotionStep(&m,2180));
        assert(lockMotionBegin(&m,100,h-10,h));lockMotionMove(&m,102,h-80);
        assert(m.offset==70);lockMotionEnd(&m,102,h-80,h,3000);assert(m.phase==LOCK_LEAVING);
        assert(!lockMotionBegin(&m,100,h-10,h));assert(!lockMotionStep(&m,3090));
        assert(lockMotionStep(&m,3180)&&m.phase==LOCK_STILL);
        assert(!lockMotionStep(&m,3200));
        assert(lockMotionBegin(&m,100,h-10,h));lockMotionMove(&m,100,h-90);lockMotionCancel(&m);
        lockMotionEnd(&m,100,h-110,h,4000);assert(!lockMotionStep(&m,4200)&&m.offset==0);
        assert(lockMotionBegin(&m,100,h-10,h));lockMotionEnd(&m,100,h-74,h,5000);assert(m.phase==LOCK_LEAVING);
        lockMotionCancel(&m);assert(!lockMotionStep(&m,6000));
        assert(lockMotionBegin(&m,100,h-10,h));lockMotionEnd(&m,100,h-73,h,7000);assert(m.phase==LOCK_RETURNING);
        lockMotionCancel(&m);
    }
    puts("lockscreen_test: PASS (bottom-only start, follow, 64px release, horizontal rejection, rebound, cancel, one-shot completion)");
}
