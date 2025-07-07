#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * This simple GUI allows Arch Linux users to inspect zip archives,
 * extract them, and create new ones.  It can also query package
 * information from pacman to showcase basic Arch Linux API usage.
 */

/* Helper to run a command and capture output */
static gchar* run_command(const gchar *cmd) {
    gchar *stdout_str = NULL, *stderr_str = NULL;
    gint exit_status;

    if (!g_spawn_command_line_sync(cmd, &stdout_str, &stderr_str, &exit_status, NULL)) {
        g_free(stdout_str);
        g_free(stderr_str);
        return g_strdup_printf("Failed to run command: %s", cmd);
    }
    if (exit_status != 0) {
        gchar *msg = g_strdup_printf("Error running '%s': %s", cmd, stderr_str);
        g_free(stdout_str);
        g_free(stderr_str);
        return msg;
    }
    g_free(stderr_str);
    return stdout_str; /* must be freed by caller */
}

/* Callback to open a zip file and display contents */
static void on_open_zip(GtkButton *button, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open ZIP", GTK_WINDOW(window),
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   "_Cancel", GTK_RESPONSE_CANCEL,
                                                   "_Open", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        char *cmd = g_strdup_printf("unzip -l '%s'", filename);
        gchar *output = run_command(cmd);
        GtkWidget *textview = g_object_get_data(G_OBJECT(window), "textview");
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
        gtk_text_buffer_set_text(buffer, output, -1);
        g_free(output);
        g_free(cmd);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

/* Callback to extract selected zip */
static void on_extract_zip(GtkButton *button, gpointer user_data) {
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
            gchar *output = run_command(cmd);
            GtkWidget *textview = g_object_get_data(G_OBJECT(window), "textview");
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
            gtk_text_buffer_set_text(buffer, output, -1);
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
static void on_create_zip(GtkButton *button, gpointer user_data) {
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
            gchar *output = run_command(cmd);
            GtkWidget *textview = g_object_get_data(G_OBJECT(window), "textview");
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
            gtk_text_buffer_set_text(buffer, output, -1);
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
static void on_pacman_info(GtkButton *button, gpointer user_data) {
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
        gchar *output = run_command(cmd);
        GtkWidget *textview = g_object_get_data(G_OBJECT(window), "textview");
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
        gtk_text_buffer_set_text(buffer, output, -1);
        g_free(output);
        g_free(cmd);
    }
    gtk_widget_destroy(dialog);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GTK3 Zip Tool");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

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

    g_object_set_data(G_OBJECT(window), "textview", textview);

    g_signal_connect(open_btn, "clicked", G_CALLBACK(on_open_zip), window);
    g_signal_connect(extract_btn, "clicked", G_CALLBACK(on_extract_zip), window);
    g_signal_connect(create_btn, "clicked", G_CALLBACK(on_create_zip), window);
    g_signal_connect(pacman_btn, "clicked", G_CALLBACK(on_pacman_info), window);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}

