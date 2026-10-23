#pragma once
#include <stdbool.h>
#include <stdint.h>

#define GRID_SLOTS 32
#define DOCK_FIRST 32
#define LAYOUT_SLOTS 36
#define APP_COUNT 21
#define SLOT_EMPTY (-1)
#define LONG_PRESS_MS 450
#define EDGE_HOLD_MS 550

typedef enum { APP_HOME=-1, APP_FACETIME, APP_CALENDAR, APP_PHOTOS, APP_CAMERA,
    APP_MAIL, APP_NOTES, APP_CLOCK, APP_MAPS, APP_TV, APP_NEWS, APP_STORE,
    APP_GAMES, APP_HEALTH, APP_WALLET, APP_SIRI, APP_SETTINGS, APP_PHONE,
    APP_SAFARI, APP_MESSAGES, APP_MUSIC, APP_CALCULATOR } App;
typedef struct {
    double lhs;
    char input[32];
    char op;
    bool replace, error;
} Calculator;
typedef struct {
    bool active, dragging, moved, edgeLatched, musicTransfer;
    int origin, startX, startY, x, y, edge;
    uint64_t startedAt, edgeStartedAt;
} TouchGesture;
typedef struct {
    App app;
    int page, selection, photo;
    bool night, locked, selectionVisible, editing, portrait;
    int8_t layout[LAYOUT_SLOTS];
    TouchGesture touch;
    Calculator calc;
    char note[256];
} Model;
void modelInit(Model *m);
void modelResetLayout(Model *m);
bool modelValidLayout(const int8_t *layout);
void modelHome(Model *m);
void modelOpen(Model *m, App app);
void modelMove(Model *m, int dx, int dy);
void modelActivate(Model *m);
void modelNextPage(Model *m);
void modelBack(Model *m);
void modelCancelTouch(Model *m, bool exitEditing);
void modelSetLocked(Model *m, bool locked);
void modelToggleOrientation(Model *m);
void modelTouchBegin(Model *m, int x, int y, uint64_t now);
void modelTouchMove(Model *m, int x, int y, uint64_t now);
bool modelTouchEnd(Model *m, int x, int y, uint64_t now);
int modelSlotAt(const Model *m, int x, int y);
int modelSelectedSlot(const Model *m);
bool modelSwapSlots(Model *m, int from, int to);
void calcKey(Calculator *c, char key);
App modelSelected(const Model *m);

bool modelMusicTransferCandidate(const Model *model,int x,int y);
