/*
 * MokoHome
 * Entry point
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

#include <Ecore_X.h>
#include <Elementary.h>
#include <mokosuite/utils/utils.h>
#include <mokosuite/utils/remote-config-service.h>
#include <mokosuite/utils/dbus.h>
#include <glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "globals.h"
#include "launchers.h"
#include "desktop.h"

#define MOKOHOME_NAME               "org.mokosuite.home"
#define MOKOHOME_CONFIG_PATH        MOKOSUITE_DBUS_PATH "/Home/Config"

#define CONFIG_SECTION_DESKTOP      "desktop"
#define CONFIG_WALLPAPER_IMAGE      "wallpaper"

// default log domain
int _log_dom = -1;

RemoteConfigService* home_config = NULL;

static void _close(void* data, Evas_Object* obj, void* event_info)
{
    // chiudi launcher
    edje_object_signal_emit((Evas_Object *) data, "collapse", "handle");

    drag_end();
}

void get_screen_size(int *w, int *h)
{
    Ecore_X_Window *roots = NULL;
    int num = 0;

    roots = ecore_x_window_root_list(&num);
    if (roots && num > 0) {
        ecore_x_window_geometry_get(roots[0], NULL, NULL, w, h);
    }

    g_free(roots);
}

int main(int argc, char* argv[])
{
    // TODO initialize Intl
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    // initialize log
    eina_init();
    _log_dom = eina_log_domain_register(PACKAGE, EINA_COLOR_CYAN);
    eina_log_domain_level_set(PACKAGE, LOG_LEVEL);

    EINA_LOG_INFO("%s version %s", PACKAGE_NAME, VERSION);
    elm_init(argc, argv);
    elm_need_efreet();

    /* other things */
    mokosuite_utils_init();
    mokosuite_utils_glib_init(TRUE);

    EINA_LOG_DBG("Loading data from %s", MOKOHOME_DATADIR);

    // dbus name
    DBusGConnection* session_bus = dbus_session_bus();
    if (!session_bus || !dbus_request_name(session_bus, MOKOHOME_NAME))
        return EXIT_FAILURE;

    // settings database
    char* cfg_file = g_strdup_printf("%s/.config/mokosuite/%s.conf", g_get_home_dir(), PACKAGE);
    home_config = remote_config_service_new(session_bus,
        MOKOHOME_CONFIG_PATH,
        cfg_file);
    g_free(cfg_file);


    Ecore_X_Window *roots = NULL;
    int num = 0;

    roots = ecore_x_window_root_list(&num);
    if ((!roots) || (num <= 0)) {
        EINA_LOG_ERR("Unable to get root window. Exiting.");
        return EXIT_FAILURE;
    }

    int x, y, w, h;
    ecore_x_window_geometry_get(roots[0], &x, &y, &w, &h);
    g_free(roots);

    elm_theme_extension_add(NULL, MOKOHOME_DATADIR "/theme.edj");
    elm_theme_overlay_add(NULL, "elm/scroller/base/desktop");

    Evas_Object* win = elm_win_add(NULL, "mokohome", ELM_WIN_DESKTOP);
    elm_win_title_set(win, "Home");
    elm_win_borderless_set(win, TRUE);
    //elm_win_layer_set(win, 90);    // home layer :)

    // for not accepting focus...
    //ecore_x_icccm_hints_set(elm_win_xwindow_get(win), 0, 0, 0, 0, 0, 0, 0);

    // layout principale home
    Evas_Object* layout = elm_layout_add(win);
    elm_layout_file_set(layout, MOKOHOME_DATADIR "/theme.edj", "home");
    Evas_Object* layout_edje = elm_layout_edje_get(layout);

    // non funzionera' mai... o forse no?
    evas_object_smart_callback_add(win, "delete,request", _close, layout_edje);
    evas_object_smart_callback_add(win, "focus,out", _close, layout_edje);

    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(win, layout);
    evas_object_show(layout);

    // collapsa subito i launcher
    edje_object_signal_emit(layout_edje, "collapse", "handle");

    // scroller del desktop
    Evas_Object* bgsc = elm_scroller_add(win);
    elm_object_style_set(bgsc, "desktop");

    elm_scroller_bounce_set(bgsc, FALSE, FALSE);
    elm_scroller_policy_set(bgsc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_page_relative_set(bgsc, 1.0, 0.0);

    evas_object_size_hint_weight_set(bgsc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_weight_set(bgsc, EVAS_HINT_FILL, EVAS_HINT_FILL);

    // lo scroller sara' la zona widgets della home
    elm_layout_content_set(layout, "widgets", bgsc);
    evas_object_show(bgsc);

    // layout del desktop
    Evas_Object* widgets = elm_layout_add(win);
    elm_layout_file_set(widgets, MOKOHOME_DATADIR "/theme.edj", "widgets");
    //widgets_edje = elm_layout_edje_get(widgets);

    evas_object_size_hint_weight_set(widgets, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(widgets, 0.0, 0.0);

    // questo barbatrucco e' necessario per far scorrere lo sfondo insieme ai widgets
    elm_scroller_content_set(bgsc, widgets);
    evas_object_show(widgets);

    Evas_Object* bg = elm_bg_add(win);
    char* bg_img = NULL;
    if (!remote_config_service_get_string(home_config, CONFIG_SECTION_DESKTOP, CONFIG_WALLPAPER_IMAGE, &bg_img))
        bg_img = g_strdup(MOKOHOME_DATADIR "/wallpaper_mountain.jpg");
    elm_bg_file_set(bg, bg_img, NULL);
    g_free(bg_img);

    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_min_set(bg, w * NUM_DESKTOPS, h);
    evas_object_show(bg);

    // setta lo sfondo come wallpaper dei widgets
    elm_layout_content_set(widgets, "wallpaper", bg);

    Evas_Object* widgets_table = make_widgets(win, bgsc);
    elm_layout_content_set(widgets, "widgets", widgets_table);

    Evas_Object* apps = make_launchers(win, layout_edje);
    elm_layout_content_set(layout, "applications", apps);

    evas_object_move(win, x, y);
    evas_object_resize(win, w, h);
    evas_object_show(win);

    elm_run();
    elm_shutdown();

    return EXIT_SUCCESS;
}
