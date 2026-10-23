#pragma once
#include <stdbool.h>
typedef enum { DUO_ENGLISH=0, DUO_CHINESE=1 } DuoLanguage;
bool languageLoad(const char *path,DuoLanguage *language);
bool languageSave(const char *path,DuoLanguage language);
typedef struct { bool selectPending,comboHeld; } LanguageKeys;
enum { LANGUAGE_TOGGLE=1, LANGUAGE_SELECT=2, LANGUAGE_CONSUME_Y=4 };
unsigned languageKeysUpdate(LanguageKeys *keys,bool selectDown,bool selectUp,bool selectHeld,bool yHeld);
