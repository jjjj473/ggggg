#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include "sysmgr.h"

static SystemManager sysmgr;
static char *default_folder = NULL;

/* simple setup screen to choose a default extraction folder */
static void show_setup_screen(void) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Setup", NULL, GTK_DIALOG_MODAL,
        "_Start", GTK_RESPONSE_OK,
        NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new("Select default extract folder:");
    GtkWidget *chooser = gtk_file_chooser_button_new(
        "Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    gtk_box_pack_start(GTK_BOX(content), label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(content), chooser, FALSE, FALSE, 5);
    gtk_widget_show_all(dialog);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        default_folder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
    }
    gtk_widget_destroy(dialog);
}

/* quick splash window shown while the application loads */
static void show_loading_screen(void) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated(GTK_WINDOW(win), FALSE);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(win), 200, 100);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *label = gtk_label_new("Loading...");
    GtkWidget *spinner = gtk_spinner_new();
    gtk_container_add(GTK_CONTAINER(win), box);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(box), spinner, FALSE, FALSE, 10);
    gtk_widget_show_all(win);
    gtk_spinner_start(GTK_SPINNER(spinner));
    while (gtk_events_pending())
        gtk_main_iteration();
    g_usleep(1000000);
    gtk_widget_destroy(win);
}

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

typedef void (*CmdCallback)(gchar *output, GError *error, gpointer data);

typedef struct {
    GtkWidget *dialog;
    char *cmd;
    gchar *output;
    GError *error;
    CmdCallback cb;
    gpointer data;
} CmdTask;

static gboolean cmd_task_finished(gpointer userdata) {
    CmdTask *task = userdata;
    gtk_widget_destroy(task->dialog);
    task->cb(task->output, task->error, task->data);
    g_free(task->cmd);
    g_free(task->output);
    if (task->error)
        g_error_free(task->error);
    g_free(task);
    return FALSE;
}

static gpointer cmd_task_thread(gpointer userdata) {
    CmdTask *task = userdata;
    task->output = sysmgr_run(&sysmgr, task->cmd, &task->error);
    g_idle_add(cmd_task_finished, task);
    return NULL;
}

static void run_command_async(GtkWidget *window, const char *cmd,
                              CmdCallback cb, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Working...",
                                                    GTK_WINDOW(window),
                                                    GTK_DIALOG_MODAL,
                                                    NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *spinner = gtk_spinner_new();
    gtk_box_pack_start(GTK_BOX(content), spinner, FALSE, FALSE, 20);
    gtk_widget_show_all(dialog);
    gtk_spinner_start(GTK_SPINNER(spinner));

    CmdTask *task = g_new0(CmdTask, 1);
    task->dialog = dialog;
    task->cmd = g_strdup(cmd);
    task->cb = cb;
    task->data = data;
    g_thread_new("syscmd", cmd_task_thread, task);
}

/* Callback to open a zip file and display contents */
static void open_zip_finished(gchar *output, GError *err, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    if (err) {
        show_error(window, err->message);
    } else {
        GtkWidget *textview = g_object_get_data(G_OBJECT(window), "textview");
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
        gtk_text_buffer_set_text(buffer, output, -1);
        set_status(window, "ZIP contents listed");
    }
}

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
        run_command_async(window, cmd, open_zip_finished, window);
        g_free(cmd);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

/* Callback to extract selected zip */
static void extract_zip_finished(gchar *output, GError *err, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    if (err) {
        show_error(window, err->message);
    } else {
        GtkWidget *textview = g_object_get_data(G_OBJECT(window), "textview");
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
        gtk_text_buffer_set_text(buffer, output, -1);
        set_status(window, "Archive extracted");
    }
}

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
            run_command_async(window, cmd, extract_zip_finished, window);
            g_free(cmd);
            g_free(dest);
        }
        gtk_widget_destroy(dest_dialog);
        g_free(zipname);
    }
    gtk_widget_destroy(dialog);
}

/* Callback to create zip from folder */
static void create_zip_finished(gchar *output, GError *err, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    if (err) {
        show_error(window, err->message);
    } else {
        GtkWidget *textview = g_object_get_data(G_OBJECT(window), "textview");
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
        gtk_text_buffer_set_text(buffer, output, -1);
        set_status(window, "ZIP file created");
    }
}

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
            run_command_async(window, cmd, create_zip_finished, window);
            g_free(cmd);
            g_free(zipname);
        }
        gtk_widget_destroy(save_dialog);
        g_free(src_folder);
    }
    gtk_widget_destroy(src_dialog);
}

/* Callback to query pacman for package information */
static void pacman_info_finished(gchar *output, GError *err, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    if (err) {
        show_error(window, err->message);
    } else {
        GtkWidget *textview = g_object_get_data(G_OBJECT(window), "textview");
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
        gtk_text_buffer_set_text(buffer, output, -1);
        set_status(window, "Pacman info retrieved");
    }
}

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
        run_command_async(window, cmd, pacman_info_finished, window);
        g_free(cmd);
    }
    gtk_widget_destroy(dialog);
}

/* --- Pacman updater with rotating tips --- */

static const char *arch_tips[] = {
    "Tip 1: pacman -Ss searches packages.",
    "Tip 2: pacman -Ql lists package files.",
    "Tip 3: Use pacman -Syu to stay updated.",
    "Tip 4: pacman -Rns removes unused deps.",
    "Tip 5: Keep your mirrorlist fresh.",
    "Tip 6: Read pacman.conf for options.",
    "Tip 7: Use -U to install local files.",
    "Tip 8: Combine pacman with makepkg.",
    "Tip 9: pacman -Qi shows package info.",
    "Tip 10: Use pacman -Qdt to find orphans.",
    "Tip 11: pacman -F finds files in repos.",
    "Tip 12: Cache cleanup with pacman -Sc.",
    "Tip 13: pacman -Qkk verifies packages.",
    "Tip 14: Use pacman -D --asdeps to mark deps.",
    "Tip 15: Mirror status at archlinux.org.",
    "Tip 16: pacman -Qm lists AUR installs.",
    "Tip 17: PGP keys keep packages secure.",
    "Tip 18: pacman -Si reveals dependencies.",
    "Tip 19: Update frequently for security.",
    "Tip 20: Use -Syu before -S to avoid breakage.",
    "Tip 21: Query the pacman man page often.",
    "Tip 22: pacman -Qs searches package names.",
    "Tip 23: Use pacman -S --needed to skip reinstall.",
    "Tip 24: Keep root partition large for cache.",
    "Tip 25: pacman -Qo finds owner of a file.",
    "Tip 26: pacman -Qet lists explicitly installed.",
    "Tip 27: Use rankmirrors to speed downloads.",
    "Tip 28: pacman hooks automate tasks.",
    "Tip 29: Use pacman -Qen for native packages.",
    "Tip 30: Keep pacman and keyring up to date.",
    "Tip 31: pacman -Sg shows package groups.",
    "Tip 32: pacman -Qk checks install integrity.",
    "Tip 33: Use -Rdd carefully to force removal.",
    "Tip 34: pacman -Qi | less for scrolling info.",
    "Tip 35: Remember to read /var/log/pacman.log.",
    "Tip 36: pacman -Qs linux lists kernel packages.",
    "Tip 37: Use pacman -Fy before pacman -Fx.",
    "Tip 38: pacman -Rsnc removes config files too.",
    "Tip 39: Use pacman -Syyu to force sync.",
    "Tip 40: pacman -D --asexplicit marks explicit.",
    "Tip 41: pacman -Qdtq shows orphaned packages.",
    "Tip 42: Combine pacman -Ql with grep.",
    "Tip 43: Use pacman -Dk to check database.",
    "Tip 44: pacman -Rsu handles dependencies.",
    "Tip 45: pacman -Qi base gives system basics.",
    "Tip 46: Keep /etc/pacman.d/gnupg backed up.",
    "Tip 47: pacman -Qn shows repo packages only.",
    "Tip 48: Use pacman -Syy if mirrors changed.",
    "Tip 49: pacman -Qs '^vim' lists Vim packages.",
    "Tip 50: pacman -Qk package verifies install.",
    "Tip 51: Use pacman -Ql package | less.",
    "Tip 52: pacman -Rns $(pacman -Qdtq) cleans orphans.",
    "Tip 53: pacman -S package --overwrite fixes conflicts.",
    "Tip 54: pacman -Qkk checks for missing files.",
    "Tip 55: pacman -S --noconfirm for scripting.",
    "Tip 56: Use pacman -Qi pacman for version.",
    "Tip 57: pacman -Qe lists explicit packages.",
    "Tip 58: Back up /var/cache/pacman/pkg.",
    "Tip 59: pacman -Syu before reporting bugs.",
    "Tip 60: The Arch Wiki is your best guide."
};

typedef struct {
    GtkWidget *dialog;
    GtkWidget *label;
    char *cmd;
    gchar *output;
    GError *error;
    int tip_idx;
    guint timer_id;
    CmdCallback cb;
    gpointer data;
} UpdateTask;

static gboolean update_tip(gpointer userdata) {
    UpdateTask *task = userdata;
    task->tip_idx = (task->tip_idx + 1) % G_N_ELEMENTS(arch_tips);
    gtk_label_set_text(GTK_LABEL(task->label), arch_tips[task->tip_idx]);
    return TRUE;
}

static gboolean update_finished(gpointer userdata) {
    UpdateTask *task = userdata;
    if (task->timer_id)
        g_source_remove(task->timer_id);
    gtk_widget_destroy(task->dialog);
    task->cb(task->output, task->error, task->data);
    g_free(task->cmd);
    g_free(task->output);
    if (task->error)
        g_error_free(task->error);
    g_free(task);
    return FALSE;
}

static gpointer update_thread(gpointer userdata) {
    UpdateTask *task = userdata;
    task->output = sysmgr_run(&sysmgr, task->cmd, &task->error);
    g_idle_add(update_finished, task);
    return NULL;
}

static void run_pacman_update(GtkWidget *window, CmdCallback cb, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Updating System", GTK_WINDOW(window),
                                                   GTK_DIALOG_MODAL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *spinner = gtk_spinner_new();
    GtkWidget *label = gtk_label_new(arch_tips[0]);
    gtk_box_pack_start(GTK_BOX(content), spinner, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(content), label, FALSE, FALSE, 10);
    gtk_widget_show_all(dialog);
    gtk_spinner_start(GTK_SPINNER(spinner));

    UpdateTask *task = g_new0(UpdateTask, 1);
    task->dialog = dialog;
    task->label = label;
    task->cmd = g_strdup("pacman -Syu");
    task->cb = cb;
    task->data = data;
    task->tip_idx = 0;
    task->timer_id = g_timeout_add(2000, update_tip, task);
    g_thread_new("pacupdate", update_thread, task);
}

static void pacman_update_finished(gchar *output, GError *err, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    if (err)
        show_error(window, err->message);
    else
        set_status(window, "System updated");
}

static void on_update_system(GtkWidget *widget, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    run_pacman_update(window, pacman_update_finished, window);
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
    show_setup_screen();
    show_loading_screen();

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
    GtkWidget *update_item = gtk_menu_item_new_with_label("System Update");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(tools), tools_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(tools_menu), pacman_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(tools_menu), update_item);

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
    GtkWidget *update_btn = gtk_button_new_with_label("Update System");
    gtk_box_pack_start(GTK_BOX(toolbar), open_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), extract_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), create_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), pacman_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), update_btn, FALSE, FALSE, 0);

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
    g_signal_connect(update_btn, "clicked", G_CALLBACK(on_update_system), window);
    g_signal_connect(open_item, "activate", G_CALLBACK(on_open_zip), window);
    g_signal_connect(extract_item, "activate", G_CALLBACK(on_extract_zip), window);
    g_signal_connect(create_item, "activate", G_CALLBACK(on_create_zip), window);
    g_signal_connect(pacman_item, "activate", G_CALLBACK(on_pacman_info), window);
    g_signal_connect(update_item, "activate", G_CALLBACK(on_update_system), window);
    g_signal_connect(about_item, "activate", G_CALLBACK(on_about), window);
    g_signal_connect(website_item, "activate", G_CALLBACK(on_show_website), window);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    set_status(window, "Ready");

    gtk_widget_show_all(window);
    gtk_main();
    sysmgr_clear(&sysmgr);
    g_free(default_folder);
    return 0;
}

