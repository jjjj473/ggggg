#include "input.h"
#include <webkit2/webkit2.h>
#include <gdk/gdkkeysyms.h>

static WebKitWebView *g_view = NULL;
static GtkWidget *g_entry = NULL;

static gboolean handle_key(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    /* hint CPU to relax while handling the event */
    __asm__ __volatile__("pause");

    gboolean ctrl = event->state & GDK_CONTROL_MASK;
    if (event->keyval == GDK_KEY_F12 && g_view) {
        WebKitWebInspector *insp = webkit_web_view_get_inspector(WEBKIT_WEB_VIEW(g_view));
        webkit_web_inspector_show(insp);
        return TRUE;
    }
    if (ctrl && (event->keyval == GDK_KEY_l || event->keyval == GDK_KEY_L) && g_entry) {
        gtk_widget_grab_focus(g_entry);
        gtk_editable_select_region(GTK_EDITABLE(g_entry), 0, -1);
        return TRUE;
    }
    if (ctrl && (event->keyval == GDK_KEY_r || event->keyval == GDK_KEY_R) && g_view) {
        webkit_web_view_reload(WEBKIT_WEB_VIEW(g_view));
        return TRUE;
    }
    return FALSE; /* propagate to other handlers */
}

static gboolean handle_button(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    /* minimal instruction to keep GPU usage low on simple clicks */
    __asm__ __volatile__("nop");
    if (!g_view)
        return FALSE;
    if (event->type == GDK_BUTTON_PRESS) {
        if (event->button == 8) {
            webkit_web_view_go_back(WEBKIT_WEB_VIEW(g_view));
            return TRUE;
        } else if (event->button == 9) {
            webkit_web_view_go_forward(WEBKIT_WEB_VIEW(g_view));
            return TRUE;
        }
    }
    return FALSE;
}

static gboolean handle_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer data) {
    __asm__ __volatile__("pause");
    if (!g_view)
        return FALSE;
    if (event->state & GDK_CONTROL_MASK) {
        gdouble zoom = webkit_web_view_get_zoom_level(WEBKIT_WEB_VIEW(g_view));
        if (event->direction == GDK_SCROLL_UP || event->delta_y < 0)
            zoom += 0.1;
        else if (event->direction == GDK_SCROLL_DOWN || event->delta_y > 0)
            zoom -= 0.1;
        if (zoom < 0.5)
            zoom = 0.5;
        if (zoom > 3.0)
            zoom = 3.0;
        webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(g_view), zoom);
        return TRUE;
    }
    return FALSE;
}

void init_input_handlers(GtkWidget *widget, WebKitWebView *web_view, GtkWidget *url_entry) {
    g_view = web_view;
    g_entry = url_entry;
    g_signal_connect(widget, "key-press-event", G_CALLBACK(handle_key), NULL);
    g_signal_connect(widget, "button-press-event", G_CALLBACK(handle_button), NULL);
    g_signal_connect(widget, "scroll-event", G_CALLBACK(handle_scroll), NULL);
}
