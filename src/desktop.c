/*
 * MokoHome
 * Desktop view
 * Copyright (C) 2009-2010 Daniele Ricci <daniele.athome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <Elementary.h>
#include <glib.h>

#include "globals.h"
#include "desktop.h"
#include "widgets.h"

#define WIDGETS_COLUMNS     4
#define WIDGETS_ROWS        4

#ifdef QVGA
#define WIDGETS_PADDING0_HEIGHT     3
#define WIDGETS_PADDING0_WIDTH      3
#else
#define WIDGETS_PADDING0_HEIGHT     5
#define WIDGETS_PADDING0_WIDTH      5
#endif

#define LAUNCHER_HEIGHT (65 * SCALE_FACTOR)

// per fare una cosa piu' precisa...
#ifdef QVGA
#define LAUNCHER_WIDTH  58
#else
#define LAUNCHER_WIDTH  115
#endif

#define LONG_PRESS_TIME     750

static Evas_Object* desktop_scroller = NULL;
static Evas_Object* desktops[NUM_DESKTOPS] = {};

static Evas_Object* widgets[NUM_DESKTOPS][WIDGETS_COLUMNS][WIDGETS_ROWS] = {};

// current desktop in dragging mode (-1 dragging disabled)
int drag_status = -1;

/**
 * Searches for the first free place starting from coordinates [x,y] if given.
 * x and y are used as return buffers too.
 * @param desktop_id
 * @param x
 * @param y
 */
static void get_free_place(int desktop_id, int* x, int* y)
{
    g_return_if_fail((x != NULL || y != NULL) && desktop_id < NUM_DESKTOPS);

    int i, j;
    int si = (x && *x >= 0) ? *x : 0;
    int sj = (y && *y >= 0) ? *y : 0;

    for (j = sj; j < WIDGETS_ROWS; j++) {
        for (i = si; i < WIDGETS_COLUMNS; i++) {
            if (!widgets[desktop_id][i][j]) {
                if (x) *x = i;
                if (y) *y = j;
                return;
            }
        }
    }

    // no more places :(
    if (x) *x = -1;
    if (y) *y = -1;
}

static void add_widget(int desktop_id, Evas_Object* wd, int cx, int cy)
{
    int wx = cx, wy = cy;
    get_free_place(desktop_id, &wx, &wy);
    if (wx < 0 || wy < 0) {
        EINA_LOG_WARN("No more space for widget, skipping");
        return;
    }

    EINA_LOG_DBG("Packing widget %p to desktop %d, position %dx%d", wd, desktop_id + 1, wx, wy);
    elm_table_pack(desktops[desktop_id], wd, wx, wy, 1, 1);
    evas_object_data_set(wd, "desktop_id", (void*) desktop_id);

    widgets[desktop_id][wx][wy] = wd;
}

#if 0
static void add_custom_widget(Evas_Object* win, int desktop_id, const char* icon, const char* name, Evas_Object* scroller)
{
    Evas_Object *ic, *lb;

    char* ic_path = NULL;
    if (!ic_path && (icon != NULL ? icon[0] != '/' : TRUE)) return;   // non aggiungere se l'icona non e' stata trovata

    ic = elm_icon_add(win);

    gboolean is_edj = (ic_path == NULL && g_str_has_suffix(icon, ".edj"));

    elm_icon_file_set(ic, (ic_path == NULL) ? icon : ic_path, is_edj ? "icon" : NULL );
    g_free(ic_path);

    elm_icon_smooth_set(ic, TRUE);
    elm_icon_scale_set(ic, TRUE, TRUE);
    evas_object_show(ic);
    //evas_object_size_hint_min_set(ic, 70, 70);

    Evas_Object *bt = edje_object_add(evas_object_evas_get(win));
    edje_object_file_set(bt, MOKOHOME_DATADIR "/theme.edj", "launcher");
    edje_object_part_swallow(bt, "icon", ic);

    lb = elm_label_add(win);
    elm_object_style_set(lb, "marker");
    elm_object_scale_set(lb, 0.65);

    elm_label_line_wrap_set(lb, TRUE);
    elm_label_label_set(lb, name);

    evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(lb, 0.5, 0.0);
    evas_object_show(lb);
    edje_object_part_swallow(bt, "title", lb);

    evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    //evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_min_set(bt, LAUNCHER_WIDTH, LAUNCHER_HEIGHT);
    //evas_object_size_hint_max_set(bt, 100, 100);
    evas_object_show(bt);

    // contenitore widget dragabile
    Evas_Object *wd = edje_object_add(evas_object_evas_get(win));
    edje_object_file_set(wd, MOKOHOME_DATADIR "/theme.edj", "widget");
    edje_object_part_swallow(wd, "widget", bt);
    evas_object_size_hint_min_set(wd, LAUNCHER_WIDTH, LAUNCHER_HEIGHT);

    #if 0
    evas_object_event_callback_add(wd, EVAS_CALLBACK_MOUSE_DOWN, widget_pressed, scroller);
    evas_object_event_callback_add(wd, EVAS_CALLBACK_MOUSE_UP, widget_released, scroller);
    #endif

    evas_object_show(bt);
    evas_object_show(wd);

    add_widget(desktop_id, wd, -1, -1);
}
#endif

static Evas_Object* add_launcher_widget(Evas_Object* parent, int desktop_id, const char* desktop_file, int cx, int cy)
{
    Efreet_Desktop* d = efreet_desktop_get(desktop_file);
    Evas_Object* wd = NULL;

    if (d) {
        wd = widget_launcher_new(parent, d);
        if (wd)
            add_widget(desktop_id, wd, cx, cy);

        efreet_desktop_free(d);
    }

    return wd;
}

void drag_start(int desktop_id)
{
    drag_status = desktop_id;

    int w = 0, h = 0;
    get_screen_size(&w, &h);
    elm_scroller_region_bring_in(desktop_scroller, desktop_id * w, 0, w, h);

    int i, j;
    for (i = 0; i < WIDGETS_COLUMNS; i++)
        for (j = 0; j < WIDGETS_ROWS; j++)
            if (widgets[desktop_id][i][j])
                edje_object_signal_emit(widgets[desktop_id][i][j], "drag_start", "widget");

    elm_object_scroll_freeze_push(desktop_scroller);
}

void drag_end(void)
{
    if (drag_status < 0) return;

    int i, j;
    for (i = 0; i < WIDGETS_COLUMNS; i++)
        for (j = 0; j < WIDGETS_ROWS; j++)
            if (widgets[drag_status][i][j])
                edje_object_signal_emit(widgets[drag_status][i][j], "drag_end", "widget");

    elm_object_scroll_freeze_pop(desktop_scroller);

    drag_status = -1;
}

static Evas_Object* prepare_table(Evas_Object* win, int cols, int rows)
{
    Evas_Object* tb;

    tb = elm_table_add(win);
    elm_table_homogenous_set(tb, TRUE);
    evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_table_padding_set(tb, WIDGETS_PADDING0_WIDTH, WIDGETS_PADDING0_HEIGHT);

    return tb;
}

static void load_launchers(int desktop_id, Evas_Object* win)
{
    char* entry, *entry2;
    int i = 1, cx, cy;
    char* key = g_strdup_printf("home/%d/launcher/1", desktop_id + 1);

    while ((entry = remote_settings_database_GetSetting(home_settings, key, NULL))) {
        g_free(key);

        // custom position :)
        cx = cy = -1;
        key = g_strdup_printf("home/%d/launcher/%d/position", desktop_id + 1, i);
        if ((entry2 = remote_settings_database_GetSetting(home_settings, key, NULL))) {
            char** xy = g_strsplit(entry2, ",", 2);
            if (xy && xy[0] != NULL) {
                cx = atoi(xy[0]);
                cy = atoi(xy[1]);
            }
            g_strfreev(xy);
            g_free(entry2);
        }

        add_launcher_widget(win, desktop_id, entry, cx, cy);

        g_free(entry);
        g_free(key);
        key = g_strdup_printf("home/%d/launcher/%d", desktop_id + 1, ++i);
    }
}

Evas_Object* make_widgets(Evas_Object* win, Evas_Object* scroller)
{
    Evas_Object *mb, *bx, *tb;

    bx = elm_box_add(win);
    elm_box_homogenous_set(bx, TRUE);
    elm_box_horizontal_set(bx, TRUE);

    evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_weight_set(bx, 0.0, 0.0);
    evas_object_show(bx);

    // riempi la tabella con le icone
    tb = prepare_table(win, WIDGETS_COLUMNS, WIDGETS_ROWS);
    desktops[0] = tb;

    // launcher da configurazione semplice
    load_launchers(0, win);

    mb = elm_mapbuf_add(win);
    evas_object_size_hint_weight_set(mb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(mb, 0.0, 0.0);

    elm_mapbuf_content_set(mb, tb);
    evas_object_show(tb);

    elm_box_pack_end(bx, mb);
    evas_object_show(mb);

    // tabella 2 :)
    tb = prepare_table(win, WIDGETS_COLUMNS, WIDGETS_ROWS);
    desktops[1] = tb;

    // launcher da configurazione semplice
    load_launchers(1, win);

    mb = elm_mapbuf_add(win);
    evas_object_size_hint_weight_set(mb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(mb, 0.0, 0.0);

    elm_mapbuf_content_set(mb, tb);
    evas_object_show(tb);

    elm_box_pack_end(bx, mb);
    evas_object_show(mb);

    desktop_scroller = scroller;

    return bx;
}
