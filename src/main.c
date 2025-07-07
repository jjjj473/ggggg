#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <sqlite3.h>
#include <time.h>
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>
#include <libxml/parser.h>
#include <openssl/sha.h>
#include <archive.h>
#include <archive_entry.h>

static GtkWidget *notebook;
static GtkWidget *web_view;
static GtkWidget *url_entry;
static GtkWidget *progress_bar;
static GPtrArray *network_logs;
static WebKitWebContext *web_context;
static WebKitSettings *global_settings;

static gboolean load_internal(const char *uri);
static gboolean decide_policy_cb(WebKitWebView *web_view,
                                 WebKitPolicyDecision *decision,
                                 WebKitPolicyDecisionType type,
                                 gpointer user_data);

static char *trim_whitespace(const char *s) {
    if (!s)
        return g_strdup("");
    while (g_ascii_isspace(*s))
        s++;
    const char *end = s + strlen(s);
    while (end > s && g_ascii_isspace(*(end - 1)))
        end--;
    return g_strndup(s, end - s);
}

static gboolean show_startup_home(gpointer data) {
    load_internal("archbrowser://home");
    return G_SOURCE_REMOVE;
}

static void sha256_hex(const char *in, char out[65]) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)in, strlen(in), hash);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(out + i*2, "%02x", hash[i]);
    out[64] = '\0';
}

static const char *page_style =
    "<style>\n"
    "body{font-family:sans-serif;margin:20px;}\n"
    ".nav{background:#f3f3f3;padding:10px;margin-bottom:20px;}\n"
    ".nav a{margin-right:10px;color:#06c;text-decoration:none;}\n"
    ".nav a:hover{text-decoration:underline;}\n"
    ".btn{padding:4px 8px;border:1px solid #888;background:#eee;cursor:pointer;}\n"
    "body.dark{background:#222;color:#ddd;}\n"
    "body.dark a{color:#9cf;}\n"
    "</style>";

static const char *page_script =
    "<script>\n"
    "function setTheme(dark){\n"
    "  document.body.classList.toggle('dark', dark);\n"
    "  localStorage.setItem('theme', dark ? 'dark' : 'light');\n"
    "}\n"
    "function init(){\n"
    "  if(localStorage.getItem('theme')==='dark')setTheme(true);\n"
    "}\n"
    "function go(u){\n"
    "  window.location.href=u;\n"
    "}\n"
    "document.addEventListener('DOMContentLoaded',init);\n"
    "</script>";

static const char *nav_links =
    "<a href=\"archbrowser://home\">Home</a>"
    "<a href=\"archbrowser://history\">History</a>"
    "<a href=\"archbrowser://downloads\">Downloads</a>"
    "<a href=\"archbrowser://bookmarks\">Bookmarks</a>"
    "<a href=\"archbrowser://notes\">Notes</a>"
    "<a href=\"archbrowser://network\">Network</a>"
    "<a href=\"archbrowser://settings\">Settings</a>"
    "<a href=\"archbrowser://extensions\">Extensions</a>"
    "<a href=\"archbrowser://about\">About</a>"
    "<a href=\"archbrowser://help\">Help</a>";

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
    "googletagservices.com",
    "cdn.taboola.com",
    "tr.outbrain.com",
    "pixel.outbrain.com",
    "ads.linkedin.com",
    "analytics.linkedin.com",
    "analytics.snapchat.com",
    "ads.snapchat.com",
    "adservice.amazon.com",
    "amazon-adsystem.com",
    "adrolladvertising.com",
    "moatads.com",
    "zedo.com",
    "exelator.com",
    "crwdcntrl.net",
    "switchadhub.com",
    "imrworldwide.com",
    "turn.com",
    "pubmatic.com",
    "rubiconproject.com",
    "casalemedia.com",
    "matomymedia.com",
    "adcolony.com",
    "appsflyer.com",
    "branch.io",
    "mixpanel.com",
    "segment.io",
    "intercom.io",
    "chartbeat.com",
    "chartbeat.net",
    "optimizely.com",
    "crazyegg.com",
    "hotjar.com",
    "marketo.com",
    "pardot.com",
    "yandex.ru",
    "mc.yandex.ru",
    "yandexadexchange.net",
    "dmp.tremorhub.com",
    "audienceconnect.tremorhub.com",
    "tracking.m6r.eu",
    "pixel.quantserve.com",
    "doubleverify.com",
    "yieldlab.net",
    "ads.servebom.com",
    "iadsdk.apple.com",
    "metrics.apple.com",
    "app-measurement.com",
    "fast.appcues.com",
    "cdn.optimizely.com",
    "pixel.wp.com",
    "stats.wp.com",
    "omtrdc.net",
    "adobedtm.com",
    "adnxs.com",
    "l.doubleclick.net",
    "m.doubleclick.net",
    "t.doubleclick.net",
    "innovid.com",
    "adserver.org",
    "advertising.com",
    "inmobi.com",
    "adform.com",
    "trackinghost.io",
    "bidswitch.net",
    "adition.com",
    "flashtalking.com",
    "bidswitch.com",
    "adsrvr.com",
    "agkn.com",
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
    sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS bookmarks(" \
        "id INTEGER PRIMARY KEY AUTOINCREMENT," \
        "title TEXT," \
        "url TEXT NOT NULL);",
        NULL, NULL, NULL);
    sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS notes(" \
        "id INTEGER PRIMARY KEY AUTOINCREMENT," \
        "content TEXT," \
        "created_at INTEGER NOT NULL);",
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

static void add_bookmark(const char *title, const char *url) {
    if (!db || !url)
        return;
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db,
            "INSERT INTO bookmarks(title, url) VALUES (?, ?)",
            -1, &stmt, NULL) == SQLITE_OK) {
        char hash[65];
        sha256_hex(url, hash);
        sqlite3_bind_text(stmt, 1, title ? title : hash, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, url, -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

static void add_note(const char *content) {
    if (!db || !content)
        return;
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db,
            "INSERT INTO notes(content, created_at) VALUES (?, strftime('%s','now'))",
            -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, content, -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

static void clear_data(void) {
    if (!db)
        return;
    sqlite3_exec(db, "DELETE FROM history;", NULL, NULL, NULL);
    sqlite3_exec(db, "DELETE FROM downloads;", NULL, NULL, NULL);
    sqlite3_exec(db, "DELETE FROM bookmarks;", NULL, NULL, NULL);
    sqlite3_exec(db, "DELETE FROM notes;", NULL, NULL, NULL);
}

static void show_error_page(const char *uri, const char *message) {
    GString *html = g_string_new("<html><head>");
    g_string_append(html, page_style);
    g_string_append_printf(html,
        "</head><body><h1>Failed to load</h1><p>%s</p><p>%s</p></body></html>",
        uri ? uri : "", message ? message : "");
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html->str, "archbrowser://error");
    g_string_free(html, TRUE);
}

static gboolean is_valid_uri(const char *uri) {
    if (!uri)
        return FALSE;
    GUri *guri = g_uri_parse(uri, G_URI_FLAGS_NONE, NULL);
    if (!guri)
        return FALSE;
    g_uri_unref(guri);
    return TRUE;
}

static void show_history(void) {
    GString *html = g_string_new("<html><head>");
    g_string_append(html, page_style);
    g_string_append(html, page_script);
    g_string_append(html, "</head><body><div id='nav' class='nav'>");
    g_string_append(html, nav_links);
    g_string_append(html, "<button onclick=\"setTheme(document.body.className!=='dark')\">Toggle Theme</button>");
    g_string_append(html, "</div><h1>History</h1><table><tr><th>Time</th><th>URL</th></tr>");
    if (db) {
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db,
                "SELECT url, datetime(visited_at,'unixepoch','localtime')"
                " FROM history ORDER BY visited_at DESC",
                -1, &stmt, NULL) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                const char *u = (const char*)sqlite3_column_text(stmt, 0);
                const char *ts = (const char*)sqlite3_column_text(stmt, 1);
                g_string_append_printf(html,
                        "<tr><td>%s</td><td><a href=\"%s\">%s</a></td></tr>",
                        ts ? ts : "", u, u);
            }
            sqlite3_finalize(stmt);
        }
    }
    g_string_append(html, "</table></body></html>");
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html->str, "archbrowser://history");
    g_string_free(html, TRUE);
}

static void show_downloads(void) {
    GString *html = g_string_new("<html><head>");
    g_string_append(html, page_style);
    g_string_append(html, page_script);
    g_string_append(html, "</head><body><div id='nav' class='nav'>");
    g_string_append(html, nav_links);
    g_string_append(html, "<button onclick=\"setTheme(document.body.className!=='dark')\">Toggle Theme</button>");
    g_string_append(html, "</div><h1>Downloads</h1><table><tr><th>Time</th><th>File</th><th>URL</th></tr>");
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
                    "<tr><td>%s</td><td><a href=\"file://%s\">%s</a></td><td>%s</td></tr>",
                    ts ? ts : "", p ? p : "", p ? p : "", u);
            }
            sqlite3_finalize(stmt);
        }
    }
    g_string_append(html, "</table></body></html>");
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html->str, "archbrowser://downloads");
    g_string_free(html, TRUE);
}

static void show_settings(void) {
    gchar *contents = NULL;
    if (g_file_get_contents("data/settings.html", &contents, NULL, NULL)) {
        webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), contents, "archbrowser://settings");
        g_free(contents);
    } else {
        show_error_page("archbrowser://settings", "Failed to load settings file");
    }
}

static void show_clear_page(void) {
    clear_data();
    GString *html = g_string_new("<html><head>");
    g_string_append(html, page_style);
    g_string_append(html, page_script);
    g_string_append(html, "</head><body><div id='nav' class='nav'>");
    g_string_append(html, nav_links);
    g_string_append(html, "<button onclick=\"setTheme(document.body.className!=='dark')\">Toggle Theme</button>");
    g_string_append(html, "</div><h1>Data cleared</h1>");
    g_string_append(html, "<p><a href=\"archbrowser://settings\">Back to settings</a></p>");
    g_string_append(html, "</body></html>");
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html->str, "archbrowser://clear");
    g_string_free(html, TRUE);
}

static void show_home(void) {
    GString *html = g_string_new("<html><head>");
    g_string_append(html, page_style);
    g_string_append(html, page_script);
    g_string_append(html,
        "</head><body>"
        "<div id='nav' class='nav'>"
        );
    g_string_append(html, nav_links);
    g_string_append(html,
        "<button onclick=\"setTheme(document.body.className!=='dark')\">Toggle Theme</button>"
        "</div>"
        "<h1>Welcome to Arch Browser</h1>"
        "<p>This is the home page.</p>"
        "<form onsubmit=\"go(document.getElementById('u').value);return false;\" style='margin-top:20px'>"
        "<input id='u' style='width:60%' placeholder='Enter URL'>"
        "<button type='submit'>Go</button>"
        "</form>"
        "<div id='clock' style='margin-top:20px;font-weight:bold'></div>"
        "<script>setInterval(function(){document.getElementById('clock').textContent=new Date().toLocaleString();},1000);</script>"
        "</body></html>");
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html->str, "archbrowser://home");
    g_string_free(html, TRUE);
}

static void show_about(void) {
    GString *html = g_string_new("<html><head>");
    g_string_append(html, page_style);
    g_string_append(html, page_script);
    g_string_append(html,
        "</head><body>"
        "<div id='nav' class='nav'>");
    g_string_append(html, nav_links);
    g_string_append(html,
        "<button onclick=\"setTheme(document.body.className!=='dark')\">Toggle Theme</button>"
        "</div>"
        "<h1>About Arch Browser</h1>"
        "<p>A minimal privacy‑focused browser built for Arch Linux.</p>"
        "</body></html>");
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html->str, "archbrowser://about");
    g_string_free(html, TRUE);
}

static void show_bookmarks(void) {
    GString *html = g_string_new("<html><head>");
    g_string_append(html, page_style);
    g_string_append(html, page_script);
    g_string_append(html, "</head><body><div id='nav' class='nav'>");
    g_string_append(html, nav_links);
    g_string_append(html, "<button onclick=\"setTheme(document.body.className!=='dark')\">Toggle Theme</button>");
    g_string_append(html, "</div><h1>Bookmarks</h1><ul>");
    if (db) {
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, "SELECT title,url FROM bookmarks", -1, &stmt, NULL) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                const char *title = (const char*)sqlite3_column_text(stmt,0);
                const char *url = (const char*)sqlite3_column_text(stmt,1);
                g_string_append_printf(html, "<li><a href=\"%s\">%s</a></li>", url, title ? title : url);
            }
            sqlite3_finalize(stmt);
        }
    }
    g_string_append(html, "</ul></body></html>");
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html->str, "archbrowser://bookmarks");
    g_string_free(html, TRUE);
}

static void show_notes(void) {
    GString *html = g_string_new("<html><head>");
    g_string_append(html, page_style);
    g_string_append(html, page_script);
    g_string_append(html, "</head><body><div id='nav' class='nav'>");
    g_string_append(html, nav_links);
    g_string_append(html, "<button onclick=\"setTheme(document.body.className!=='dark')\">Toggle Theme</button>");
    g_string_append(html, "</div><h1>Notes</h1><ul>");
    if (db) {
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, "SELECT content, datetime(created_at,'unixepoch','localtime') FROM notes ORDER BY created_at DESC", -1, &stmt, NULL) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                const char *content = (const char*)sqlite3_column_text(stmt,0);
                const char *ts = (const char*)sqlite3_column_text(stmt,1);
                g_string_append_printf(html, "<li>[%s] %s</li>", ts ? ts : "", content ? content : "");
            }
            sqlite3_finalize(stmt);
        }
    }
    g_string_append(html, "</ul></body></html>");
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html->str, "archbrowser://notes");
    g_string_free(html, TRUE);
}

static void show_network(void) {
    GString *html = g_string_new("<html><head>");
    g_string_append(html, page_style);
    g_string_append(html, page_script);
    g_string_append(html, "</head><body><div id='nav' class='nav'>");
    g_string_append(html, nav_links);
    g_string_append(html, "<button onclick=\"setTheme(document.body.className!=='dark')\">Toggle Theme</button>");
    g_string_append(html, "</div><h1>Network Log</h1><ul>");
    if (network_logs) {
        for (guint i = 0; i < network_logs->len; i++) {
            char *u = g_ptr_array_index(network_logs, i);
            g_string_append_printf(html, "<li>%s</li>", u);
        }
    }
    g_string_append(html, "</ul></body></html>");
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html->str, "archbrowser://network");
    g_string_free(html, TRUE);
}

static void show_extensions(void) {
    GString *html = g_string_new("<html><head>");
    g_string_append(html, page_style);
    g_string_append(html, page_script);
    g_string_append(html, "</head><body><div id='nav' class='nav'>");
    g_string_append(html, nav_links);
    g_string_append(html, "<button onclick=\"setTheme(document.body.className!=='dark')\">Toggle Theme</button>");
    g_string_append(html, "</div><h1>Extensions</h1><p>Extension system placeholder.</p></body></html>");
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html->str, "archbrowser://extensions");
    g_string_free(html, TRUE);
}

static void show_help(void) {
    GString *html = g_string_new("<html><head>");
    g_string_append(html, page_style);
    g_string_append(html, page_script);
    g_string_append(html, "</head><body><div id='nav' class='nav'>");
    g_string_append(html, nav_links);
    g_string_append(html, "<button onclick=\"setTheme(document.body.className!=='dark')\">Toggle Theme</button>");
    g_string_append(html, "</div><h1>Help</h1><p>Use the address bar to navigate. Built-in pages start with archbrowser://</p></body></html>");
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html->str, "archbrowser://help");
    g_string_free(html, TRUE);
}

static gboolean match_page(const char *uri, const char *page)
{
    size_t n = strlen(page);
    return g_str_has_prefix(uri, page) &&
           (uri[n] == '\0' || uri[n] == '/' || uri[n] == '?');
}

static gboolean load_internal(const char *uri) {
    if (match_page(uri, "archbrowser://home")) {
        show_home();
        return TRUE;
    } else if (match_page(uri, "archbrowser://history")) {
        show_history();
        return TRUE;
    } else if (match_page(uri, "archbrowser://downloads")) {
        show_downloads();
        return TRUE;
    } else if (match_page(uri, "archbrowser://settings")) {
        show_settings();
        return TRUE;
    } else if (match_page(uri, "archbrowser://clear")) {
        show_clear_page();
        return TRUE;
    } else if (match_page(uri, "archbrowser://bookmarks")) {
        show_bookmarks();
        return TRUE;
    } else if (match_page(uri, "archbrowser://notes")) {
        show_notes();
        return TRUE;
    } else if (match_page(uri, "archbrowser://network")) {
        show_network();
        return TRUE;
    } else if (match_page(uri, "archbrowser://extensions")) {
        show_extensions();
        return TRUE;
    } else if (match_page(uri, "archbrowser://help")) {
        show_help();
        return TRUE;
    } else if (match_page(uri, "archbrowser://about")) {
        show_about();
        return TRUE;
    }
    return FALSE;
}

static void load_changed_cb(WebKitWebView *view, WebKitLoadEvent event, gpointer data) {
    if (event == WEBKIT_LOAD_STARTED) {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0);
    } else if (event == WEBKIT_LOAD_COMMITTED) {
        const char *uri = webkit_web_view_get_uri(view);
        if (uri)
            gtk_entry_set_text(GTK_ENTRY(url_entry), uri);
    } else if (event == WEBKIT_LOAD_FINISHED) {
        const char *uri = webkit_web_view_get_uri(view);
        if (uri && g_str_has_prefix(uri, "http"))
            add_history(uri);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 1.0);
    }
}

static gboolean load_failed_cb(WebKitWebView *view, WebKitLoadEvent event,
                                const char *uri, GError *error, gpointer data) {
    show_error_page(uri, error ? error->message : "Unknown error");
    return TRUE;
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

static void back_cb(GtkButton *button, gpointer data) {
    if (webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(web_view)))
        webkit_web_view_go_back(WEBKIT_WEB_VIEW(web_view));
}

static void forward_cb(GtkButton *button, gpointer data) {
    if (webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(web_view)))
        webkit_web_view_go_forward(WEBKIT_WEB_VIEW(web_view));
}

static void reload_cb(GtkButton *button, gpointer data) {
    webkit_web_view_reload(WEBKIT_WEB_VIEW(web_view));
}

static void home_cb(GtkButton *button, gpointer data) {
    load_internal("archbrowser://home");
}

static void new_tab_cb(GtkButton *button, gpointer data) {
    web_view = create_tab();
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), -1);
    load_internal("archbrowser://home");
}

static WebKitWebView* create_tab(void) {
    GtkWidget *view = webkit_web_view_new_with_context(web_context);
    if (global_settings)
        webkit_web_view_set_settings(WEBKIT_WEB_VIEW(view), global_settings);
    g_signal_connect(view, "decide-policy", G_CALLBACK(decide_policy_cb), NULL);
    g_signal_connect(view, "load-changed", G_CALLBACK(load_changed_cb), NULL);
    g_signal_connect(view, "load-failed", G_CALLBACK(load_failed_cb), NULL);
    g_signal_connect(view, "create", G_CALLBACK(create_web_view_cb), NULL);
    g_signal_connect(view, "resource-load-started", G_CALLBACK(resource_load_started_cb), NULL);
    g_signal_connect(view, "notify::estimated-load-progress", G_CALLBACK(progress_changed_cb), NULL);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled), view);

    int num = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
    char *label = g_strdup_printf("Tab %d", num + 1);
    GtkWidget *tab_label = gtk_label_new(label);
    g_free(label);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled, tab_label);
    gtk_widget_show_all(scrolled);
    return WEBKIT_WEB_VIEW(view);
}

static void tab_switched_cb(GtkNotebook *nb, GtkWidget *page, guint page_num, gpointer data) {
    GtkWidget *scrolled = gtk_notebook_get_nth_page(nb, page_num);
    GtkWidget *view = gtk_bin_get_child(GTK_BIN(scrolled));
    web_view = view;
    const char *uri = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(web_view));
    if (uri)
        gtk_entry_set_text(GTK_ENTRY(url_entry), uri);
    else
        gtk_entry_set_text(GTK_ENTRY(url_entry), "");
    double p = webkit_web_view_get_estimated_load_progress(WEBKIT_WEB_VIEW(web_view));
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), p);
}

static void progress_changed_cb(WebKitWebView *view, GParamSpec *pspec, gpointer data) {
    double p = webkit_web_view_get_estimated_load_progress(view);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), p);
}

static gboolean send_request_cb(WebKitWebResource *resource, WebKitURIRequest *request, WebKitURIResponse *response, gpointer user_data) {
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
    if (network_logs)
        g_ptr_array_add(network_logs, g_strdup(uri));
    return blocked; // TRUE means the request is handled and will not be sent
}

static void resource_load_started_cb(WebKitWebView *view, WebKitWebResource *resource, WebKitURIRequest *request, gpointer user_data) {
    g_signal_connect(resource, "send-request", G_CALLBACK(send_request_cb), NULL);
}

static void close_web_view_cb(WebKitWebView *view, GtkWidget *window) {
    gtk_widget_destroy(window);
}

static gboolean decide_policy_cb(WebKitWebView *web_view,
                                 WebKitPolicyDecision *decision,
                                 WebKitPolicyDecisionType type,
                                 gpointer user_data);

static WebKitWebView* create_web_view_cb(WebKitWebView *web_view, WebKitNavigationAction *action, gpointer user_data) {
    GtkWidget *popup_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(popup_window), 800, 600);

    WebKitWebContext *context = webkit_web_view_get_context(web_view);
    GtkWidget *new_view = webkit_web_view_new_with_context(context);

    g_signal_connect(new_view, "decide-policy", G_CALLBACK(decide_policy_cb), NULL);
    g_signal_connect(new_view, "load-changed", G_CALLBACK(load_changed_cb), NULL);
    g_signal_connect(new_view, "load-failed", G_CALLBACK(load_failed_cb), NULL);
    g_signal_connect(new_view, "close", G_CALLBACK(close_web_view_cb), popup_window);

    gtk_container_add(GTK_CONTAINER(popup_window), new_view);
    gtk_widget_show_all(popup_window);
    return WEBKIT_WEB_VIEW(new_view);
}

static gboolean decide_policy_cb(WebKitWebView *web_view, WebKitPolicyDecision *decision, WebKitPolicyDecisionType type, gpointer user_data) {
    if (type == WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION) {
        WebKitNavigationAction *nav = webkit_navigation_policy_decision_get_navigation_action(WEBKIT_NAVIGATION_POLICY_DECISION(decision));
        WebKitURIRequest *request = webkit_navigation_action_get_request(nav);
        const char *uri = webkit_uri_request_get_uri(request);
        if (uri) {
            GUri *guri = g_uri_parse(uri, G_URI_FLAGS_NONE, NULL);
            if (guri) {
                const char *scheme = g_uri_get_scheme(guri);
                if (scheme && g_strcmp0(scheme, "archbrowser") == 0) {
                    webkit_policy_decision_ignore(decision);
                    load_internal(uri);
                    g_uri_unref(guri);
                    return TRUE;
                }
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


static void load_url(GtkEntry *entry, gpointer user_data) {
    char *raw = trim_whitespace(gtk_entry_get_text(entry));
    if (g_str_has_prefix(raw, "archbrowser://")) {
        load_internal(raw);
        g_free(raw);
        return;
    }
    char *full = NULL;
    if (!g_str_has_prefix(raw, "http://") && !g_str_has_prefix(raw, "https://"))
        full = g_strdup_printf("https://%s", raw);
    else
        full = g_strdup(raw);
    if (is_valid_uri(full))
        webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), full);
    else
        show_error_page(full, "Invalid URL");
    g_free(full);
    g_free(raw);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    network_logs = g_ptr_array_new_with_free_func(g_free);

    init_db();

    WebKitWebsiteDataManager *manager = webkit_website_data_manager_new_ephemeral();
    web_context = webkit_web_context_new_with_website_data_manager(manager);
    g_signal_connect(web_context, "download-started", G_CALLBACK(download_started_cb), NULL);

    notebook = gtk_notebook_new();
    g_signal_connect(notebook, "switch-page", G_CALLBACK(tab_switched_cb), NULL);

    web_view = create_tab();

    global_settings = webkit_settings_new_with_settings("enable-developer-extras", TRUE, NULL);
    webkit_web_view_set_settings(WEBKIT_WEB_VIEW(web_view), global_settings);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    GtkWidget *back_btn = gtk_button_new_with_label("Back");
    GtkWidget *forward_btn = gtk_button_new_with_label("Forward");
    GtkWidget *reload_btn = gtk_button_new_with_label("Reload");
    GtkWidget *home_btn = gtk_button_new_with_label("Home");
    GtkWidget *new_tab_btn = gtk_button_new_with_label("New Tab");
    url_entry = gtk_entry_new();
    progress_bar = gtk_progress_bar_new();

    g_signal_connect(back_btn, "clicked", G_CALLBACK(back_cb), NULL);
    g_signal_connect(forward_btn, "clicked", G_CALLBACK(forward_cb), NULL);
    g_signal_connect(reload_btn, "clicked", G_CALLBACK(reload_cb), NULL);
    g_signal_connect(home_btn, "clicked", G_CALLBACK(home_cb), NULL);
    g_signal_connect(new_tab_btn, "clicked", G_CALLBACK(new_tab_cb), NULL);
    g_signal_connect(url_entry, "activate", G_CALLBACK(load_url), NULL);

    gtk_box_pack_start(GTK_BOX(toolbar), back_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), forward_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), reload_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), home_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), new_tab_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), url_entry, TRUE, TRUE, 2);

    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), progress_bar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(window), vbox);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(key_press_cb), NULL);

    gtk_widget_show_all(window);

    g_idle_add(show_startup_home, NULL);

    gtk_main();
    if (db)
        sqlite3_close(db);
    if (network_logs)
        g_ptr_array_free(network_logs, TRUE);
    if (global_settings)
        g_object_unref(global_settings);
    if (web_context)
        g_object_unref(web_context);
    return 0;
}
