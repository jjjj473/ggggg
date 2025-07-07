#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <sqlite3.h>
#include <time.h>
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>

static GtkWidget *web_view;
static GtkWidget *url_entry;

static const char *page_style =
    "<style>\n"
    "body{font-family:sans-serif;margin:20px;}\n"
    "h1{color:#333;}\n"
    "table{border-collapse:collapse;width:100%;margin-bottom:1em;}\n"
    "th,td{padding:4px;border:1px solid #ccc;text-align:left;}\n"
    "a{color:#06c;text-decoration:none;}\n"
    "a:hover{text-decoration:underline;}\n"
    ".nav{background:#f3f3f3;padding:10px;margin-bottom:20px;}\n"
    ".nav a{margin-right:10px;}\n"
    "body.dark{background:#222;color:#ddd;}\n"
    ".dark a{color:#9cf;}\n"
    /* Generated padding classes */\

    ".s001{padding:1px}\n"
    ".s002{padding:2px}\n"
    ".s003{padding:3px}\n"
    ".s004{padding:4px}\n"
    ".s005{padding:5px}\n"
    ".s006{padding:6px}\n"
    ".s007{padding:7px}\n"
    ".s008{padding:8px}\n"
    ".s009{padding:9px}\n"
    ".s010{padding:10px}\n"
    ".s011{padding:11px}\n"
    ".s012{padding:12px}\n"
    ".s013{padding:13px}\n"
    ".s014{padding:14px}\n"
    ".s015{padding:15px}\n"
    ".s016{padding:16px}\n"
    ".s017{padding:17px}\n"
    ".s018{padding:18px}\n"
    ".s019{padding:19px}\n"
    ".s020{padding:20px}\n"
    ".s021{padding:21px}\n"
    ".s022{padding:22px}\n"
    ".s023{padding:23px}\n"
    ".s024{padding:24px}\n"
    ".s025{padding:25px}\n"
    ".s026{padding:26px}\n"
    ".s027{padding:27px}\n"
    ".s028{padding:28px}\n"
    ".s029{padding:29px}\n"
    ".s030{padding:30px}\n"
    ".s031{padding:31px}\n"
    ".s032{padding:32px}\n"
    ".s033{padding:33px}\n"
    ".s034{padding:34px}\n"
    ".s035{padding:35px}\n"
    ".s036{padding:36px}\n"
    ".s037{padding:37px}\n"
    ".s038{padding:38px}\n"
    ".s039{padding:39px}\n"
    ".s040{padding:40px}\n"
    ".s041{padding:41px}\n"
    ".s042{padding:42px}\n"
    ".s043{padding:43px}\n"
    ".s044{padding:44px}\n"
    ".s045{padding:45px}\n"
    ".s046{padding:46px}\n"
    ".s047{padding:47px}\n"
    ".s048{padding:48px}\n"
    ".s049{padding:49px}\n"
    ".s050{padding:50px}\n"
    ".s051{padding:51px}\n"
    ".s052{padding:52px}\n"
    ".s053{padding:53px}\n"
    ".s054{padding:54px}\n"
    ".s055{padding:55px}\n"
    ".s056{padding:56px}\n"
    ".s057{padding:57px}\n"
    ".s058{padding:58px}\n"
    ".s059{padding:59px}\n"
    ".s060{padding:60px}\n"
    ".s061{padding:61px}\n"
    ".s062{padding:62px}\n"
    ".s063{padding:63px}\n"
    ".s064{padding:64px}\n"
    ".s065{padding:65px}\n"
    ".s066{padding:66px}\n"
    ".s067{padding:67px}\n"
    ".s068{padding:68px}\n"
    ".s069{padding:69px}\n"
    ".s070{padding:70px}\n"
    ".s071{padding:71px}\n"
    ".s072{padding:72px}\n"
    ".s073{padding:73px}\n"
    ".s074{padding:74px}\n"
    ".s075{padding:75px}\n"
    ".s076{padding:76px}\n"
    ".s077{padding:77px}\n"
    ".s078{padding:78px}\n"
    ".s079{padding:79px}\n"
    ".s080{padding:80px}\n"
    ".s081{padding:81px}\n"
    ".s082{padding:82px}\n"
    ".s083{padding:83px}\n"
    ".s084{padding:84px}\n"
    ".s085{padding:85px}\n"
    ".s086{padding:86px}\n"
    ".s087{padding:87px}\n"
    ".s088{padding:88px}\n"
    ".s089{padding:89px}\n"
    ".s090{padding:90px}\n"
    ".s091{padding:91px}\n"
    ".s092{padding:92px}\n"
    ".s093{padding:93px}\n"
    ".s094{padding:94px}\n"
    ".s095{padding:95px}\n"
    ".s096{padding:96px}\n"
    ".s097{padding:97px}\n"
    ".s098{padding:98px}\n"
    ".s099{padding:99px}\n"
    ".s100{padding:100px}\n"

    "</style>";

static const char *page_script =
    "<script>\n"
    "function setTheme(dark){\n"
    "  document.body.classList.toggle('dark', dark);\n"
    "  localStorage.setItem('theme', dark ? 'dark' : 'light');\n"
    "}\n"
    "function initTheme(){\n"
    "  var t=localStorage.getItem('theme');\n"
    "  if(t==='dark')setTheme(true);\n"
    "}\n"
    "function go(u){\n"
    "  window.location.href=u;\n"
    "}\n"
    "document.addEventListener('DOMContentLoaded',initTheme);\n"
    /* Extra logging */\
    
    /* inserted script */\
    "console.log('line1');\n"
    "console.log('line2');\n"
    "console.log('line3');\n"
    "console.log('line4');\n"
    "console.log('line5');\n"
    "console.log('line6');\n"
    "console.log('line7');\n"
    "console.log('line8');\n"
    "console.log('line9');\n"
    "console.log('line10');\n"
    "console.log('line11');\n"
    "console.log('line12');\n"
    "console.log('line13');\n"
    "console.log('line14');\n"
    "console.log('line15');\n"
    "console.log('line16');\n"
    "console.log('line17');\n"
    "console.log('line18');\n"
    "console.log('line19');\n"
    "console.log('line20');\n"
    "console.log('line21');\n"
    "console.log('line22');\n"
    "console.log('line23');\n"
    "console.log('line24');\n"
    "console.log('line25');\n"
    "console.log('line26');\n"
    "console.log('line27');\n"
    "console.log('line28');\n"
    "console.log('line29');\n"
    "console.log('line30');\n"
    "console.log('line31');\n"
    "console.log('line32');\n"
    "console.log('line33');\n"
    "console.log('line34');\n"
    "console.log('line35');\n"
    "console.log('line36');\n"
    "console.log('line37');\n"
    "console.log('line38');\n"
    "console.log('line39');\n"
    "console.log('line40');\n"
    "console.log('line41');\n"
    "console.log('line42');\n"
    "console.log('line43');\n"
    "console.log('line44');\n"
    "console.log('line45');\n"
    "console.log('line46');\n"
    "console.log('line47');\n"
    "console.log('line48');\n"
    "console.log('line49');\n"
    "console.log('line50');\n"
    "console.log('line51');\n"
    "console.log('line52');\n"
    "console.log('line53');\n"
    "console.log('line54');\n"
    "console.log('line55');\n"
    "console.log('line56');\n"
    "console.log('line57');\n"
    "console.log('line58');\n"
    "console.log('line59');\n"
    "console.log('line60');\n"
    "console.log('line61');\n"
    "console.log('line62');\n"
    "console.log('line63');\n"
    "console.log('line64');\n"
    "console.log('line65');\n"
    "console.log('line66');\n"
    "console.log('line67');\n"
    "console.log('line68');\n"
    "console.log('line69');\n"
    "console.log('line70');\n"
    "console.log('line71');\n"
    "console.log('line72');\n"
    "console.log('line73');\n"
    "console.log('line74');\n"
    "console.log('line75');\n"
    "console.log('line76');\n"
    "console.log('line77');\n"
    "console.log('line78');\n"
    "console.log('line79');\n"
    "console.log('line80');\n"
    "console.log('line81');\n"
    "console.log('line82');\n"
    "console.log('line83');\n"
    "console.log('line84');\n"
    "console.log('line85');\n"
    "console.log('line86');\n"
    "console.log('line87');\n"
    "console.log('line88');\n"
    "console.log('line89');\n"
    "console.log('line90');\n"
    "console.log('line91');\n"
    "console.log('line92');\n"
    "console.log('line93');\n"
    "console.log('line94');\n"
    "console.log('line95');\n"
    "console.log('line96');\n"
    "console.log('line97');\n"
    "console.log('line98');\n"
    "console.log('line99');\n"
    "console.log('line100');\n"
    "</script>";

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
    g_string_append(html, "</head><body><div class='nav'>");
    g_string_append(html,
        "<a href=\"archbrowser://home\">Home</a>"
        "<a href=\"archbrowser://history\">History</a>"
        "<a href=\"archbrowser://downloads\">Downloads</a>"
        "<a href=\"archbrowser://settings\">Settings</a>"
        "<a href=\"archbrowser://about\">About</a>"
        "<button onclick=\"setTheme(document.body.className!=='dark')\">Toggle Theme</button>");
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
    g_string_append(html, "</head><body><div class='nav'>");
    g_string_append(html,
        "<a href=\"archbrowser://home\">Home</a>"
        "<a href=\"archbrowser://history\">History</a>"
        "<a href=\"archbrowser://downloads\">Downloads</a>"
        "<a href=\"archbrowser://settings\">Settings</a>"
        "<a href=\"archbrowser://about\">About</a>"
        "<button onclick=\"setTheme(document.body.className!=='dark')\">Toggle Theme</button>");
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
    GString *html = g_string_new("<html><head>");
    g_string_append(html, page_style);
    g_string_append(html, page_script);
    g_string_append(html, "</head><body><div class='nav'>");
    g_string_append(html,
        "<a href=\"archbrowser://home\">Home</a>"
        "<a href=\"archbrowser://history\">History</a>"
        "<a href=\"archbrowser://downloads\">Downloads</a>"
        "<a href=\"archbrowser://settings\">Settings</a>"
        "<a href=\"archbrowser://about\">About</a>"
        "<button onclick=\"setTheme(document.body.className!=='dark')\">Toggle Theme</button>");
    g_string_append(html, "</div><h1>Settings</h1><ul>");
    g_string_append(html, "<li><a href=\"archbrowser://clear\">Clear all data</a></li>");
    g_string_append(html, "</ul></body></html>");
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html->str, "archbrowser://settings");
    g_string_free(html, TRUE);
}

static void show_clear_page(void) {
    clear_data();
    GString *html = g_string_new("<html><head>");
    g_string_append(html, page_style);
    g_string_append(html, page_script);
    g_string_append(html, "</head><body><div class='nav'>");
    g_string_append(html,
        "<a href=\"archbrowser://home\">Home</a>"
        "<a href=\"archbrowser://history\">History</a>"
        "<a href=\"archbrowser://downloads\">Downloads</a>"
        "<a href=\"archbrowser://settings\">Settings</a>"
        "<a href=\"archbrowser://about\">About</a>"
        "<button onclick=\"setTheme(document.body.className!=='dark')\">Toggle Theme</button>");
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
        "<div class='nav'>"
        "<a href=\"archbrowser://home\">Home</a>"
        "<a href=\"archbrowser://history\">History</a>"
        "<a href=\"archbrowser://downloads\">Downloads</a>"
        "<a href=\"archbrowser://settings\">Settings</a>"
        "<a href=\"archbrowser://about\">About</a>"
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
        "<div class='nav'>"
        "<a href=\"archbrowser://home\">Home</a>"
        "<a href=\"archbrowser://history\">History</a>"
        "<a href=\"archbrowser://downloads\">Downloads</a>"
        "<a href=\"archbrowser://settings\">Settings</a>"
        "<a href=\"archbrowser://about\">About</a>"
        "<button onclick=\"setTheme(document.body.className!=='dark')\">Toggle Theme</button>"
        "</div>"
        "<h1>About Arch Browser</h1>"
        "<p>A minimal privacy‑focused browser built for Arch Linux.</p>"
        "</body></html>");
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(web_view), html->str, "archbrowser://about");
    g_string_free(html, TRUE);
}

static gboolean load_internal(const char *uri) {
    if (g_strcmp0(uri, "archbrowser://home") == 0) {
        show_home();
        return TRUE;
    } else 
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
    } else if (g_strcmp0(uri, "archbrowser://about") == 0) {
        show_about();
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


static void load_url(GtkEntry *entry, gpointer user_data) {
    const char *url = gtk_entry_get_text(entry);
    if (g_str_has_prefix(url, "archbrowser://")) {
        load_internal(url);
        return;
    }
    char *full = NULL;
    if (!g_str_has_prefix(url, "http://") && !g_str_has_prefix(url, "https://"))
        full = g_strdup_printf("https://%s", url);
    else
        full = g_strdup(url);

    if (is_valid_uri(full))
        webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), full);
    else
        show_error_page(full, "Invalid URL");
    g_free(full);
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
    g_signal_connect(web_view, "load-failed", G_CALLBACK(load_failed_cb), NULL);

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

    show_home();

    gtk_main();
    if (db)
        sqlite3_close(db);
    return 0;
}
