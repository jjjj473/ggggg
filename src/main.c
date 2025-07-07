#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

/*
 * List of known tracking hosts. These are not meant to block entire platforms,
 * only common analytics and advertising domains used by Google, Microsoft and
 * TikTok to track users across the web.
 */
static const char* blocked_domains[] = {
    "google-analytics.com",
    "googletagmanager.com",
    "googleadservices.com",
    "doubleclick.net",
    "googlesyndication.com",
    "clarity.ms",
    "bat.bing.com",
    "analytics.tiktok.com",
    "ads.tiktok.com",
    NULL
};

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

static void load_url(GtkEntry *entry, gpointer user_data) {
    const char *url = gtk_entry_get_text(entry);
    if (!g_str_has_prefix(url, "http://") && !g_str_has_prefix(url, "https://")) {
        char *tmp = g_strdup_printf("https://%s", url);
        webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), tmp);
        g_free(tmp);
    } else {
        webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), url);
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    WebKitWebsiteDataManager *manager = webkit_website_data_manager_new_ephemeral();
    WebKitWebContext *context = webkit_web_context_new_with_website_data_manager(manager);
    g_signal_connect(context, "send-request", G_CALLBACK(send_request_cb), NULL);

    web_view = webkit_web_view_new_with_context(context);
    g_signal_connect(web_view, "decide-policy", G_CALLBACK(decide_policy_cb), NULL);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *entry = gtk_entry_new();
    g_signal_connect(entry, "activate", G_CALLBACK(load_url), NULL);

    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), web_view, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(window), vbox);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    gtk_main();
    return 0;
}
