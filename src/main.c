#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <sqlite3.h>
#include <time.h>
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>

/*
 * List of known tracking hosts. These are not meant to block entire platforms,
 * only common analytics and advertising domains used by Google, Microsoft and
 * TikTok to track users across the web.
 */
static const char* blocked_domains[] = {
    "google-analytics.com",
    "www.google-analytics.com",
    "ssl.google-analytics.com",
    "analytics.google.com",
    "googletagmanager.com",
    "tagmanager.google.com",
    "gtm-msr.appspot.com",
    "googleadservices.com",
    "pagead2.googlesyndication.com",
    "tpc.googlesyndication.com",
    "adservice.google.com",
    "ads.google.com",
    "ad.doubleclick.net",
    "g.doubleclick.net",
    "doubleclick.net",
    "googlesyndication.com",
    "measurement.googleapis.com",
    "firebaseinstallations.googleapis.com",
    "firebase-settings.crashlytics.com",
    "youtubeads.google.com",
    "clarity.ms",
    "bat.bing.com",
    "ads.microsoft.com",
    "bingads.microsoft.com",
    "a.ads1.msn.com",
    "a.ads2.msn.com",
    "ads.msn.com",
    "c.msn.com",
    "self.events.data.microsoft.com",
    "browser.events.data.microsoft.com",
    "telecommand.int.microsoft.com",
    "ads.bing.com",
    "analytics.tiktok.com",
    "ads.tiktok.com",
    "business.tiktok.com",
    "log.tiktok.com",
    "tiktok-analytics.com",
    "connect.facebook.net",
    "graph.facebook.com",
    "pixel.facebook.com",
    "static.ads-twitter.com",
    "analytics.twitter.com",
    "ads-twitter.com",
    "marketing.twitter.com",
    "syndication.twitter.com",
    "analytics.yahoo.com",
    "ads.yahoo.com",
    "gemini.yahoo.com",
    "sp.analytics.yahoo.com",
    "adroll.com",
    "pixel.advertising.com",
    "adsrvr.org",
    "atdmt.com",
    "demdex.net",
    "everesttech.net",
    "quantserve.com",
    "scorecardresearch.com",
    "mathtag.com",
    "openx.net",
    "serving-sys.com",
    "criteo.com",
    "ads.criteo.com",
    "bidr.io",
    "adform.net",
    "emjcd.com",
    "ml314.com",
    "contextweb.com",
    "tribalfusion.com",
    "adbrite.com",
    "advertisement.com",
    "2mdn.net",
    "bluekai.com",
    "tapad.com",
    "tracking-protection.cdn.mozilla.net",
    "usermetrics.teams.cdn.office.net",
    "myvisualiq.net",
    "simpli.fi",
    "upltv.com",
    "webtrends.com",
    "quantcount.com",
    NULL
};

static sqlite3 *db = NULL;

static void init_db(void) {
    if (sqlite3_open("archbrowser.db", &db) != SQLITE_OK) {
        g_warning("Failed to open database: %s", sqlite3_errmsg(db));
        db = NULL;
        return;
    }
    sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS history(" \
        "id INTEGER PRIMARY KEY AUTOINCREMENT," \
        "url TEXT NOT NULL," \
        "visited_at INTEGER NOT NULL);",
        NULL, NULL, NULL);
    sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS downloads(" \
        "id INTEGER PRIMARY KEY AUTOINCREMENT," \
        "url TEXT NOT NULL," \
        "path TEXT," \
        "downloaded_at INTEGER NOT NULL);",
        NULL, NULL, NULL);
}

static void add_history(const char *url) {
    if (!db || !url)
        return;
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db,
            "INSERT INTO history(url, visited_at) VALUES (?, strftime('%s','now'))",
            -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, url, -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

static void add_download(const char *url, const char *path) {
    if (!db || !url)
        return;
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db,
            "INSERT INTO downloads(url, path, downloaded_at) VALUES (?, ?, strftime('%s','now'))",
            -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, url, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, path ? path : "", -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

static void clear_data(void) {
    if (!db)
        return;
    sqlite3_exec(db, "DELETE FROM history;", NULL, NULL, NULL);
    sqlite3_exec(db, "DELETE FROM downloads;", NULL, NULL, NULL);
}

static void show_history(void) {
    GString *html = g_string_new("<html><body><h1>History</h1><ul>");
    if (db) {
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db,
                "SELECT url, datetime(visited_at,'unixepoch','localtime')"
                " FROM history ORDER BY visited_at DESC",
                -1, &stmt, NULL) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                const char *u = (const char*)sqlite3_column_text(stmt, 0);
                const char *ts = (const char*)sqlite3_column_text(stmt, 1);
                g_string_append_printf(html, "<li>%s - <a href=\"%s\">%s</a></li>",
                                        ts ? ts : "", u, u);
            }
            sqlite3_finalize(stmt);
        }
    }
    g_string_append(html, "</ul></body></html>");
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html->str, "archbrowser://history");
    g_string_free(html, TRUE);
}

static void show_downloads(void) {
    GString *html = g_string_new("<html><body><h1>Downloads</h1><ul>");
    if (db) {
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db,
                "SELECT url, path, datetime(downloaded_at,'unixepoch','localtime')"
                " FROM downloads ORDER BY downloaded_at DESC",
                -1, &stmt, NULL) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                const char *u = (const char*)sqlite3_column_text(stmt,0);
                const char *p = (const char*)sqlite3_column_text(stmt,1);
                const char *ts = (const char*)sqlite3_column_text(stmt,2);
                g_string_append_printf(html,
                    "<li>%s - <a href=\"file://%s\">%s</a> (%s)</li>",
                    ts ? ts : "", p ? p : "", u, u);
            }
            sqlite3_finalize(stmt);
        }
    }
    g_string_append(html, "</ul></body></html>");
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html->str, "archbrowser://downloads");
    g_string_free(html, TRUE);
}

static void show_settings(void) {
    const char *html =
        "<html><body><h1>Settings</h1>"
        "<p><a href=\"archbrowser://clear\">Clear all data</a></p>"
        "</body></html>";
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html, "archbrowser://settings");
}

static void show_clear_page(void) {
    clear_data();
    const char *html =
        "<html><body><h1>Data cleared</h1>"
        "<p><a href=\"archbrowser://settings\">Back to settings</a></p>"
        "</body></html>";
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html, "archbrowser://clear");
}

static gboolean load_internal(const char *uri) {
    if (g_strcmp0(uri, "archbrowser://history") == 0) {
        show_history();
        return TRUE;
    } else if (g_strcmp0(uri, "archbrowser://downloads") == 0) {
        show_downloads();
        return TRUE;
    } else if (g_strcmp0(uri, "archbrowser://settings") == 0) {
        show_settings();
        return TRUE;
    } else if (g_strcmp0(uri, "archbrowser://clear") == 0) {
        show_clear_page();
        return TRUE;
    }
    return FALSE;
}

static void load_changed_cb(WebKitWebView *view, WebKitLoadEvent event, gpointer data) {
    if (event == WEBKIT_LOAD_FINISHED) {
        const char *uri = webkit_web_view_get_uri(view);
        if (uri && g_str_has_prefix(uri, "http"))
            add_history(uri);
    }
}

static void download_started_cb(WebKitWebContext *context, WebKitDownload *download, gpointer data) {
    const char *uri = webkit_uri_request_get_uri(webkit_download_get_request(download));
    char *basename = g_path_get_basename(uri);
    const char *dir = g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD);
    if (!dir)
        dir = g_get_tmp_dir();
    g_mkdir_with_parents(dir, 0755);
    char *filepath = g_build_filename(dir, basename, NULL);
    char *dest = g_strdup_printf("file://%s", filepath);
    webkit_download_set_destination(download, dest);
    add_download(uri, filepath);
    g_free(dest);
    g_free(filepath);
    g_free(basename);
}

static gboolean key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    if (event->keyval == GDK_KEY_F12) {
        WebKitWebInspector *insp = webkit_web_view_get_inspector(WEBKIT_WEB_VIEW(web_view));
        webkit_web_inspector_show(insp);
        return TRUE;
    }
    return FALSE;
}

static gboolean send_request_cb(WebKitWebContext *context, WebKitURIRequest *request, WebKitURIResponse *response, gpointer user_data) {
    const char *uri = webkit_uri_request_get_uri(request);
    if (!uri)
        return FALSE;
    GUri *guri = g_uri_parse(uri, G_URI_FLAGS_NONE, NULL);
    if (!guri)
        return FALSE;
    const char *host = g_uri_get_host(guri);
    gboolean blocked = FALSE;
    if (host) {
        for (int i = 0; blocked_domains[i]; ++i) {
            if (g_str_has_suffix(host, blocked_domains[i])) {
                g_print("Blocked request to %s\n", uri);
                blocked = TRUE;
                break;
            }
        }
    }
    g_uri_unref(guri);
    return blocked; // TRUE means the request is handled and will not be sent
}

static gboolean decide_policy_cb(WebKitWebView *web_view, WebKitPolicyDecision *decision, WebKitPolicyDecisionType type, gpointer user_data) {
    if (type == WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION) {
        WebKitNavigationAction *nav = webkit_navigation_policy_decision_get_navigation_action(WEBKIT_NAVIGATION_POLICY_DECISION(decision));
        WebKitURIRequest *request = webkit_navigation_action_get_request(nav);
        const char *uri = webkit_uri_request_get_uri(request);
        if (uri) {
            GUri *guri = g_uri_parse(uri, G_URI_FLAGS_NONE, NULL);
            if (guri) {
                const char *host = g_uri_get_host(guri);
                for (int i = 0; blocked_domains[i]; ++i) {
                    if (host && g_str_has_suffix(host, blocked_domains[i])) {
                        g_print("Blocked navigation to %s\n", uri);
                        webkit_policy_decision_ignore(decision);
                        g_uri_unref(guri);
                        return TRUE;
                    }
                }
                g_uri_unref(guri);
            }
        }
    }
    return FALSE;
}

static GtkWidget *web_view;
static GtkWidget *url_entry;

static void load_url(GtkEntry *entry, gpointer user_data) {
    const char *url = gtk_entry_get_text(entry);
    if (g_str_has_prefix(url, "archbrowser://")) {
        load_internal(url);
    } else if (!g_str_has_prefix(url, "http://") && !g_str_has_prefix(url, "https://")) {
        char *tmp = g_strdup_printf("https://%s", url);
        webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), tmp);
        g_free(tmp);
    } else {
        webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), url);
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    init_db();

    WebKitWebsiteDataManager *manager = webkit_website_data_manager_new_ephemeral();
    WebKitWebContext *context = webkit_web_context_new_with_website_data_manager(manager);
    g_signal_connect(context, "send-request", G_CALLBACK(send_request_cb), NULL);
    g_signal_connect(context, "download-started", G_CALLBACK(download_started_cb), NULL);

    web_view = webkit_web_view_new_with_context(context);
    g_signal_connect(web_view, "decide-policy", G_CALLBACK(decide_policy_cb), NULL);
    g_signal_connect(web_view, "load-changed", G_CALLBACK(load_changed_cb), NULL);

    WebKitSettings *settings = webkit_settings_new_with_settings("enable-developer-extras", TRUE, NULL);
    webkit_web_view_set_settings(WEBKIT_WEB_VIEW(web_view), settings);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    url_entry = gtk_entry_new();
    g_signal_connect(url_entry, "activate", G_CALLBACK(load_url), NULL);

    gtk_box_pack_start(GTK_BOX(vbox), url_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), web_view, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(window), vbox);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(key_press_cb), NULL);

    gtk_widget_show_all(window);

    gtk_main();
    if (db)
        sqlite3_close(db);
    return 0;
}
