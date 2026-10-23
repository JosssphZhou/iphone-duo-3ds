#include "orientation.h"

void orientationReset(OrientationFilter*f){
    f->candidate=ORIENTATION_UNKNOWN;f->candidateSince=0;f->lastSample=0;f->sampleCount=0;
}
void orientationInit(OrientationFilter*f){f->automatic=true;orientationReset(f);}
void orientationSetAutomatic(OrientationFilter*f,bool enabled){f->automatic=enabled;orientationReset(f);}
OrientationTarget orientationClassify(int x,int y,int z){
    if(x < -32768 || x > 32767 || y < -32768 || y > 32767 || z < -32768 || z > 32767)return ORIENTATION_UNKNOWN;
    /* HID samples are signed 16-bit; 64-bit products keep the sums bounded. */
    int64_t xx=(int64_t)x*x,yy=(int64_t)y*y,zz=(int64_t)z*z;
    int64_t norm=xx+yy+zz;
    /* Reject near-zero samples and motion dominated by acceleration, not gravity. */
    if(norm<230LL*230||norm>845LL*845)return ORIENTATION_UNKNOWN;
    /* Flat or near-flat: the component normal to the screen dominates. */
    if(xx+zz<=yy)return ORIENTATION_UNKNOWN;
    /* Only the two supported signed sectors. The gap between them is hysteresis. */
    if(z>=256&&zz*4>=xx*9)return ORIENTATION_LANDSCAPE;
    if(x<=-256&&xx*4>=zz*9)return ORIENTATION_BOOK;
    return ORIENTATION_UNKNOWN;
}
bool orientationUpdate(OrientationFilter*f,int x,int y,int z,uint64_t now,
                       bool interactionBusy,bool currentPortrait,bool*nextPortrait){
    if(!f->automatic||interactionBusy){orientationReset(f);return false;}
    OrientationTarget target=orientationClassify(x,y,z);
    if(target==ORIENTATION_UNKNOWN||(target==ORIENTATION_BOOK)==currentPortrait){orientationReset(f);return false;}
    if(f->candidate!=target||now<f->lastSample||now-f->lastSample>ORIENTATION_SAMPLE_GAP_MS){
        f->candidate=target;f->candidateSince=now;f->sampleCount=1;
    }else if(f->sampleCount<1000)f->sampleCount++;
    f->lastSample=now;
    if(f->sampleCount>=3&&now-f->candidateSince>=ORIENTATION_STABLE_MS){
        if(nextPortrait)*nextPortrait=target==ORIENTATION_BOOK;
        orientationReset(f);return true;
    }
    return false;
}
