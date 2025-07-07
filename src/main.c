#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include "sysmgr.h"

static SystemManager sysmgr;

static void show_error(GtkWidget *parent, const char *msg) {
    GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(parent),
                                          GTK_DIALOG_MODAL,
                                          GTK_MESSAGE_ERROR,
                                          GTK_BUTTONS_CLOSE,
                                          "%s", msg);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

/*
 * This simple GUI allows Arch Linux users to inspect zip archives,
 * extract them, and create new ones.  It can also query package
 * information from pacman to showcase basic Arch Linux API usage.
 */


/* helper to update the status bar */
static void set_status(GtkWidget *window, const char *msg) {
    GtkWidget *status = g_object_get_data(G_OBJECT(window), "statusbar");
    if (status) {
        guint ctx = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(window), "status_ctx"));
        gtk_statusbar_pop(GTK_STATUSBAR(status), ctx);
        gtk_statusbar_push(GTK_STATUSBAR(status), ctx, msg);
    }
}

/* Callback to open a zip file and display contents */
static void on_open_zip(GtkWidget *widget, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open ZIP", GTK_WINDOW(window),
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   "_Cancel", GTK_RESPONSE_CANCEL,
                                                   "_Open", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        char *cmd = g_strdup_printf("unzip -l '%s'", filename);
        GError *err = NULL;
        gchar *output = sysmgr_run(&sysmgr, cmd, &err);
        if (err) {
            show_error(window, err->message);
            g_error_free(err);
        } else {
            GtkWidget *textview = g_object_get_data(G_OBJECT(window), "textview");
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
            gtk_text_buffer_set_text(buffer, output, -1);
            set_status(window, "ZIP contents listed");
        }
        g_free(output);
        g_free(cmd);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

/* Callback to extract selected zip */
static void on_extract_zip(GtkWidget *widget, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Select ZIP to Extract", GTK_WINDOW(window),
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   "_Cancel", GTK_RESPONSE_CANCEL,
                                                   "_Select", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *zipname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        GtkWidget *dest_dialog = gtk_file_chooser_dialog_new("Choose Destination", GTK_WINDOW(window),
                                                            GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                                            "_Cancel", GTK_RESPONSE_CANCEL,
                                                            "_Extract", GTK_RESPONSE_ACCEPT,
                                                            NULL);
        if (gtk_dialog_run(GTK_DIALOG(dest_dialog)) == GTK_RESPONSE_ACCEPT) {
            char *dest = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dest_dialog));
            char *cmd = g_strdup_printf("unzip '%s' -d '%s'", zipname, dest);
            GError *err = NULL;
            gchar *output = sysmgr_run(&sysmgr, cmd, &err);
            if (err) {
                show_error(window, err->message);
                g_error_free(err);
            } else {
                GtkWidget *textview = g_object_get_data(G_OBJECT(window), "textview");
                GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
                gtk_text_buffer_set_text(buffer, output, -1);
                set_status(window, "Archive extracted");
            }
            g_free(output);
            g_free(cmd);
            g_free(dest);
        }
        gtk_widget_destroy(dest_dialog);
        g_free(zipname);
    }
    gtk_widget_destroy(dialog);
}

/* Callback to create zip from folder */
static void on_create_zip(GtkWidget *widget, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    GtkWidget *src_dialog = gtk_file_chooser_dialog_new("Select Folder to ZIP", GTK_WINDOW(window),
                                                      GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                                      "_Cancel", GTK_RESPONSE_CANCEL,
                                                      "_Select", GTK_RESPONSE_ACCEPT,
                                                      NULL);
    if (gtk_dialog_run(GTK_DIALOG(src_dialog)) == GTK_RESPONSE_ACCEPT) {
        char *src_folder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(src_dialog));
        GtkWidget *save_dialog = gtk_file_chooser_dialog_new("Save ZIP As", GTK_WINDOW(window),
                                                           GTK_FILE_CHOOSER_ACTION_SAVE,
                                                           "_Cancel", GTK_RESPONSE_CANCEL,
                                                           "_Save", GTK_RESPONSE_ACCEPT,
                                                           NULL);
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(save_dialog), "archive.zip");
        if (gtk_dialog_run(GTK_DIALOG(save_dialog)) == GTK_RESPONSE_ACCEPT) {
            char *zipname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(save_dialog));
            char *cmd = g_strdup_printf("zip -r '%s' '%s'", zipname, src_folder);
            GError *err = NULL;
            gchar *output = sysmgr_run(&sysmgr, cmd, &err);
            if (err) {
                show_error(window, err->message);
                g_error_free(err);
            } else {
                GtkWidget *textview = g_object_get_data(G_OBJECT(window), "textview");
                GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
                gtk_text_buffer_set_text(buffer, output, -1);
                set_status(window, "ZIP file created");
            }
            g_free(output);
            g_free(cmd);
            g_free(zipname);
        }
        gtk_widget_destroy(save_dialog);
        g_free(src_folder);
    }
    gtk_widget_destroy(src_dialog);
}

/* Callback to query pacman for package information */
static void on_pacman_info(GtkWidget *widget, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Pacman Package Info",
                                                   GTK_WINDOW(window),
                                                   GTK_DIALOG_MODAL,
                                                   "_Cancel", GTK_RESPONSE_CANCEL,
                                                   "_Query", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "package name");
    gtk_box_pack_start(GTK_BOX(content), entry, FALSE, FALSE, 0);
    gtk_widget_show_all(dialog);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char *pkg = gtk_entry_get_text(GTK_ENTRY(entry));
        char *cmd = g_strdup_printf("pacman -Si '%s'", pkg);
        GError *err = NULL;
        gchar *output = sysmgr_run(&sysmgr, cmd, &err);
        if (err) {
            show_error(window, err->message);
            g_error_free(err);
        } else {
            GtkWidget *textview = g_object_get_data(G_OBJECT(window), "textview");
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
            gtk_text_buffer_set_text(buffer, output, -1);
            set_status(window, "Pacman info retrieved");
        }
        g_free(output);
        g_free(cmd);
    }
    gtk_widget_destroy(dialog);
}

static void on_about(GtkWidget *widget, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    GtkWidget *dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "ArchZip");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog),
        "Archive manager for Arch Linux users");
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "https://archlinux.org");
    gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(dialog), "application-zip");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* Callback to open the offline website */
static void on_show_website(GtkWidget *widget, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    char *path = g_build_filename(g_get_current_dir(), "site", "index.html", NULL);
    char *uri = g_filename_to_uri(path, NULL, NULL);
    if (uri) {
        gtk_show_uri_on_window(GTK_WINDOW(window), uri, GDK_CURRENT_TIME, NULL);
        g_free(uri);
        set_status(window, "Opened offline website");
    }
    g_free(path);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    sysmgr_init(&sysmgr);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GTK3 Zip Tool");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    /* fancy header bar with branding */
    GtkWidget *header = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header), "ArchZip");
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header), "Arch Linux Zip Manager");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
    GtkWidget *logo = gtk_image_new_from_icon_name("application-zip", GTK_ICON_SIZE_BUTTON);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), logo);
    gtk_window_set_titlebar(GTK_WINDOW(window), header);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    /* menu bar */
    GtkWidget *menubar = gtk_menu_bar_new();
    GtkWidget *file = gtk_menu_item_new_with_label("File");
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *open_item = gtk_menu_item_new_with_label("Open...");
    GtkWidget *extract_item = gtk_menu_item_new_with_label("Extract...");
    GtkWidget *create_item = gtk_menu_item_new_with_label("Create...");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), open_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), extract_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), create_item);

    GtkWidget *tools = gtk_menu_item_new_with_label("Tools");
    GtkWidget *tools_menu = gtk_menu_new();
    GtkWidget *pacman_item = gtk_menu_item_new_with_label("Pacman Info");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(tools), tools_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(tools_menu), pacman_item);

    GtkWidget *help = gtk_menu_item_new_with_label("Help");
    GtkWidget *help_menu = gtk_menu_new();
    GtkWidget *about_item = gtk_menu_item_new_with_label("About");
    GtkWidget *website_item = gtk_menu_item_new_with_label("Offline Website");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), help_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), website_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), tools);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help);
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);

    GtkWidget *open_btn = gtk_button_new_with_label("Open ZIP");
    GtkWidget *extract_btn = gtk_button_new_with_label("Extract ZIP");
    GtkWidget *create_btn = gtk_button_new_with_label("Create ZIP");
    GtkWidget *pacman_btn = gtk_button_new_with_label("Pacman Info");
    gtk_box_pack_start(GTK_BOX(toolbar), open_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), extract_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), create_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), pacman_btn, FALSE, FALSE, 0);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

    GtkWidget *textview = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled), textview);

    /* watermark label for Arch Linux */
    GtkWidget *arch_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(arch_label), "<span alpha='50%'>For Arch Linux Users</span>");
    gtk_widget_set_halign(arch_label, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(vbox), arch_label, FALSE, FALSE, 0);

    /* status bar */
    GtkWidget *statusbar = gtk_statusbar_new();
    guint ctx = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), "app");
    gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, FALSE, 0);

    g_object_set_data(G_OBJECT(window), "statusbar", statusbar);
    g_object_set_data(G_OBJECT(window), "status_ctx", GUINT_TO_POINTER(ctx));

    g_object_set_data(G_OBJECT(window), "textview", textview);

    g_signal_connect(open_btn, "clicked", G_CALLBACK(on_open_zip), window);
    g_signal_connect(extract_btn, "clicked", G_CALLBACK(on_extract_zip), window);
    g_signal_connect(create_btn, "clicked", G_CALLBACK(on_create_zip), window);
    g_signal_connect(pacman_btn, "clicked", G_CALLBACK(on_pacman_info), window);
    g_signal_connect(open_item, "activate", G_CALLBACK(on_open_zip), window);
    g_signal_connect(extract_item, "activate", G_CALLBACK(on_extract_zip), window);
    g_signal_connect(create_item, "activate", G_CALLBACK(on_create_zip), window);
    g_signal_connect(pacman_item, "activate", G_CALLBACK(on_pacman_info), window);
    g_signal_connect(about_item, "activate", G_CALLBACK(on_about), window);
    g_signal_connect(website_item, "activate", G_CALLBACK(on_show_website), window);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    set_status(window, "Ready");

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}

