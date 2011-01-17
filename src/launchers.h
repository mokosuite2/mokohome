#ifndef __LAUNCHERS_H
#define __LAUNCHERS_H

#include <Elementary.h>
#include "globals.h"

#define LAUNCHER_ICON_THEME     "shr"
#define LAUNCHER_ICON_SIZE      (30 * SCALE_FACTOR)
#define LAUNCHER_HEIGHT         (65 * SCALE_FACTOR)

#ifdef QVGA
#define LAUNCHER_WIDTH          55
#else
#define LAUNCHER_WIDTH          115
#endif


Evas_Object* make_launchers(Evas_Object* win, Evas_Object* layout_edje);

Evas_Object* launcher_new(Evas_Object* parent, Efreet_Desktop* d);

#endif  /* __LAUNCHERS_H */
