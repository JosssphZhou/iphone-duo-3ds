#define _DARWIN_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#include "language.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(void){
    char folder[]="/tmp/duo-language-XXXXXX";assert(mkdtemp(folder));char path[256];snprintf(path,sizeof(path),"%s/language.bin",folder);
    DuoLanguage language=DUO_CHINESE;assert(!languageLoad(path,&language)&&language==DUO_CHINESE);
    assert(languageSave(path,DUO_ENGLISH));assert(languageLoad(path,&language)&&language==DUO_ENGLISH);
    assert(languageSave(path,DUO_CHINESE));assert(languageLoad(path,&language)&&language==DUO_CHINESE);
    assert(!languageSave(path,(DuoLanguage)2));assert(languageLoad(path,&language)&&language==DUO_CHINESE);
    FILE*f=fopen(path,"ab");assert(f);fputc(0,f);fclose(f);language=DUO_ENGLISH;assert(!languageLoad(path,&language)&&language==DUO_ENGLISH);
    assert(languageSave(path,DUO_CHINESE));f=fopen(path,"r+b");assert(f);fseek(f,8,SEEK_SET);fputc(0,f);fclose(f);assert(!languageLoad(path,&language));
    assert(languageSave(path,DUO_CHINESE));f=fopen(path,"r+b");assert(f);fseek(f,5,SEEK_SET);fputc(5,f);fclose(f);assert(!languageLoad(path,&language));
    f=fopen(path,"wb");assert(f);fwrite("DULG",1,4,f);fclose(f);assert(!languageLoad(path,&language));
    assert(!languageSave("/invalid/directory/language.bin",DUO_ENGLISH));remove(path);rmdir(folder);
    LanguageKeys keys={0};
    assert(languageKeysUpdate(&keys,true,false,true,false)==0);
    assert(languageKeysUpdate(&keys,false,false,true,true)==(LANGUAGE_TOGGLE|LANGUAGE_CONSUME_Y));
    assert(languageKeysUpdate(&keys,false,false,true,true)==LANGUAGE_CONSUME_Y);
    assert(languageKeysUpdate(&keys,false,true,false,false)==0); // combo does not also lock
    assert(languageKeysUpdate(&keys,true,false,true,false)==0);
    assert(languageKeysUpdate(&keys,false,true,false,false)==LANGUAGE_SELECT); // solo SELECT fires on release
    assert(languageKeysUpdate(&keys,true,false,true,true)==(LANGUAGE_TOGGLE|LANGUAGE_CONSUME_Y));
    assert(languageKeysUpdate(&keys,false,true,false,false)==0);
    puts("language_test: PASS (English/Chinese persistence, independent bounded record, invalid/truncated/trailing data, fallback preservation, failed writes)");
}
