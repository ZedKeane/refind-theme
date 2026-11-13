#include "ui_language.h"
typedef struct {
    ui_char *target;
    const ui_char *zh;
    const ui_char *en;
} UI_TRANSLATION;
#include "../generated/ui_language_data.inc"

void ui_set_language(int english) {
    unsigned i;
    for (i = 0; i < sizeof(UiTranslations) / sizeof(UiTranslations[0]); i++) {
        const ui_char *source = english ? UiTranslations[i].en : UiTranslations[i].zh;
        ui_char *target = UiTranslations[i].target;
        do {
            *target++ = *source;
        } while (*source++);
    }
}
