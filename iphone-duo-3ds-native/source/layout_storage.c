#include "layout_storage.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define RECORD_SIZE (8+LAYOUT_SLOTS+4)
static uint32_t checksum(const unsigned char*bytes,size_t n){uint32_t h=2166136261u;for(size_t i=0;i<n;i++){h^=bytes[i];h*=16777619u;}return h;}
static void put32(unsigned char*b,uint32_t v){for(int i=0;i<4;i++)b[i]=(unsigned char)(v>>(i*8));}
static uint32_t get32(const unsigned char*b){uint32_t v=0;for(int i=0;i<4;i++)v|=(uint32_t)b[i]<<(i*8);return v;}
LayoutLoadResult layoutLoad(Model*m,const char*path){
    modelResetLayout(m);
    FILE*f=fopen(path,"rb");if(!f)return errno==ENOENT?LAYOUT_MISSING:LAYOUT_IO_ERROR;
    unsigned char bytes[RECORD_SIZE];size_t n=fread(bytes,1,sizeof(bytes),f);int extra=fgetc(f);bool io=ferror(f)!=0;bool closed=fclose(f)==0;
    if(io||!closed)return LAYOUT_IO_ERROR;
    if(n!=sizeof(bytes)||extra!=EOF||memcmp(bytes,"DULY",4)||(bytes[4]!=1&&bytes[4]!=2)||bytes[5]!=LAYOUT_SLOTS||bytes[6]||bytes[7]||get32(bytes+RECORD_SIZE-4)!=checksum(bytes,RECORD_SIZE-4))return LAYOUT_INVALID;
    int8_t candidate[LAYOUT_SLOTS];for(int i=0;i<LAYOUT_SLOTS;i++)candidate[i]=(int8_t)bytes[8+i];
    if(!modelValidLayout(candidate))return LAYOUT_INVALID;
    if(bytes[4]==1){
        int music=-1,mail=-1;
        for(int i=0;i<LAYOUT_SLOTS;i++){if(candidate[i]==APP_MUSIC)music=i;else if(candidate[i]==APP_MAIL)mail=i;}
        int8_t app=candidate[music];candidate[music]=candidate[mail];candidate[mail]=app;
    }
    memcpy(m->layout,candidate,sizeof(candidate));return bytes[4]==1?LAYOUT_MIGRATED:LAYOUT_LOADED;
}
bool layoutSave(const Model*m,const char*path){
    if(!modelValidLayout(m->layout))return false;
    char temp[512];int len=snprintf(temp,sizeof(temp),"%s.tmp",path);if(len<0||(size_t)len>=sizeof(temp))return false;
    unsigned char bytes[RECORD_SIZE]={0};memcpy(bytes,"DULY",4);bytes[4]=2;bytes[5]=LAYOUT_SLOTS;
    for(int i=0;i<LAYOUT_SLOTS;i++)bytes[8+i]=(unsigned char)m->layout[i];
    put32(bytes+RECORD_SIZE-4,checksum(bytes,RECORD_SIZE-4));
    FILE*f=fopen(temp,"wb");if(!f)return false;
    bool okay=fwrite(bytes,1,sizeof(bytes),f)==sizeof(bytes);
    if(okay)okay=fflush(f)==0;
    if(okay)okay=fsync(fileno(f))==0;
    if(fclose(f)!=0)okay=false;
    if(okay)okay=rename(temp,path)==0;
    if(!okay)remove(temp);
    return okay;
}
