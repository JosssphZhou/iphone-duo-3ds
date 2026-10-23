#include "orientation.h"
#include <assert.h>
#include <stdio.h>

static bool sample(OrientationFilter*f,int x,int y,int z,uint64_t now,bool busy,bool current){bool next=current;bool result=orientationUpdate(f,x,y,z,now,busy,current,&next);if(result)assert(next!=current);return result;}
static void stable(OrientationFilter*f,int x,int y,int z,uint64_t start,bool current){
    for(unsigned t=0;t<500;t+=100)assert(!sample(f,x,y,z,start+t,false,current));
    assert(sample(f,x,y,z,start+500,false,current));
}
int main(void){
    assert(orientationClassify(0,-512,0)==ORIENTATION_UNKNOWN);
    assert(orientationClassify(0,512,0)==ORIENTATION_UNKNOWN);
    assert(orientationClassify(0,0,512)==ORIENTATION_LANDSCAPE);
    assert(orientationClassify(-512,0,0)==ORIENTATION_BOOK);
    assert(orientationClassify(512,0,0)==ORIENTATION_UNKNOWN);
    assert(orientationClassify(0,0,-512)==ORIENTATION_UNKNOWN);
    assert(orientationClassify(0,0,0)==ORIENTATION_UNKNOWN);
    assert(orientationClassify(2147483647,0,0)==ORIENTATION_UNKNOWN);
    assert(orientationClassify(-1000,0,0)==ORIENTATION_UNKNOWN);
    assert(orientationClassify(-360,0,360)==ORIENTATION_UNKNOWN);
    assert(orientationClassify(-230,-450,0)==ORIENTATION_UNKNOWN);
    OrientationFilter f;orientationInit(&f);assert(f.automatic);
    for(unsigned t=0;t<500;t+=100)assert(!sample(&f,-512,0,0,1000+t,false,false));
    assert(!sample(&f,-512,0,0,1499,false,false));assert(sample(&f,-512,0,0,1500,false,false));
    stable(&f,0,0,512,2000,true);
    /* Noise in the same sector still converges without delaying a valid rotation. */
    for(unsigned t=0;t<500;t+=100)assert(!sample(&f,-500+(int)t/50,-40,15,3000+t,false,false));
    assert(sample(&f,-505,20,-20,3500,false,false));
    /* Ambiguous readings interrupt a candidate instead of counting flat time. */
    assert(!sample(&f,-512,0,0,4000,false,false));assert(!sample(&f,-512,0,0,4200,false,false));
    assert(!sample(&f,0,-512,0,4300,false,false));stable(&f,-512,0,0,4400,false);
    /* Any touch resets the 500 ms interval; releasing does not trigger immediately. */
    assert(!sample(&f,-512,0,0,6000,false,false));assert(!sample(&f,-512,0,0,6200,false,false));
    assert(!sample(&f,-512,0,0,6300,true,false));stable(&f,-512,0,0,6400,false);
    /* Long sample gaps and clock rollback cannot complete a stale candidate. */
    assert(!sample(&f,-512,0,0,8000,false,false));assert(!sample(&f,-512,0,0,9000,false,false));
    assert(!sample(&f,-512,0,0,8500,false,false));orientationReset(&f);assert(f.automatic);
    stable(&f,-512,0,0,10000,false);
    orientationSetAutomatic(&f,false);for(unsigned t=0;t<1000;t+=100)assert(!sample(&f,-512,0,0,12000+t,false,false));
    orientationSetAutomatic(&f,true);stable(&f,-512,0,0,14000,false);
    orientationReset(&f);assert(f.candidate==ORIENTATION_UNKNOWN&&f.automatic);
    for(unsigned t=0;t<1000;t+=100)assert(!sample(&f,0,0,512,16000+t,false,false));
    puts("orientation_test: PASS (signed sectors, flat rejection, dwell, noisy readings, touch/manual reset, sample gaps)");
}
