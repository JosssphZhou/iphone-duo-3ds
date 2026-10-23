#include "language.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
static uint32_t sum(const unsigned char*b){uint32_t h=2166136261u;for(int i=0;i<8;i++){h^=b[i];h*=16777619u;}return h;}
bool languageLoad(const char*path,DuoLanguage*language){
    if(!language)return false;
    FILE*f=fopen(path,"rb");if(!f)return false;
    unsigned char b[12];size_t n=fread(b,1,sizeof(b),f);int extra=fgetc(f);bool okay=!ferror(f);if(fclose(f))okay=false;
    if(!okay||n!=12||extra!=EOF||memcmp(b,"DULG",4)||b[4]!=1||b[5]>1||b[6]||b[7])return false;
    uint32_t checksum=0;for(int i=0;i<4;i++)checksum|=(uint32_t)b[8+i]<<(i*8);
    if(checksum!=sum(b))return false;
    *language=(DuoLanguage)b[5];return true;
}
bool languageSave(const char*path,DuoLanguage language){
    if(language!=DUO_ENGLISH&&language!=DUO_CHINESE)return false;
    char tmp[512];int len=snprintf(tmp,sizeof(tmp),"%s.tmp",path);if(len<0||(size_t)len>=sizeof(tmp))return false;
    unsigned char b[12]={'D','U','L','G',1,(unsigned char)language,0,0};uint32_t checksum=sum(b);
    for(int i=0;i<4;i++)b[8+i]=(unsigned char)(checksum>>(8*i));
    FILE*f=fopen(tmp,"wb");if(!f)return false;
    bool okay=fwrite(b,1,sizeof(b),f)==sizeof(b);if(okay)okay=fflush(f)==0;if(okay)okay=fsync(fileno(f))==0;
    if(fclose(f))okay=false;
    if(okay)okay=rename(tmp,path)==0;
    if(!okay)remove(tmp);
    return okay;
}
unsigned languageKeysUpdate(LanguageKeys*k,bool selectDown,bool selectUp,bool selectHeld,bool yHeld){
    unsigned result=0;bool combo=selectHeld&&yHeld;
    if(selectDown)k->selectPending=true;
    if(combo){result|=LANGUAGE_CONSUME_Y;k->selectPending=false;if(!k->comboHeld)result|=LANGUAGE_TOGGLE;}
    if(selectUp){if(k->selectPending)result|=LANGUAGE_SELECT;k->selectPending=false;}
    k->comboHeld=combo;return result;
}
