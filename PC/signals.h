//Writen by TjarkG and published under the MIT License

#include <stdio.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <errno.h>

#define ABOUT "Biosphere Monitor by TjarkG\ngithub.com/tjarkG\nGTK 3\n"

GtkWidget *infoWindow;
GtkWidget *delete_window;

gboolean windowDelete(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) GdkEvent  *event, __attribute__((unused)) gpointer   data)
{
	gtk_main_quit();
	return TRUE;   
}

void aboutOpen(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    GtkWidget *text = GTK_WIDGET (gtk_builder_get_object (builder,"infoText"));
    infoWindow = GTK_WIDGET (gtk_builder_get_object (builder,"infoWindow"));
    
    gtk_label_set_label(GTK_LABEL(text), ABOUT);
    gtk_widget_show_all(infoWindow);
    gtk_label_set_selectable(GTK_LABEL(text), TRUE);
}

void closeWindow(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    gtk_widget_hide_on_delete(infoWindow);
}

void syncTime(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    printf("sync time\n");
}

void deleteOpen(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    delete_window = GTK_WIDGET (gtk_builder_get_object (builder,"deleteWindow"));

    gtk_widget_show_all(delete_window);
}

void delete(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    //TODO: delete
    printf("delete\n");
    gtk_widget_hide_on_delete(delete_window);

    //Show Confirmation/Error
    GtkWidget *text = GTK_WIDGET (gtk_builder_get_object (builder,"infoText"));
    infoWindow = GTK_WIDGET (gtk_builder_get_object (builder,"infoWindow"));

    gtk_label_set_label(GTK_LABEL(text), "TODO: delete");
    gtk_widget_show_all(infoWindow);
}

void deleteAbort(__attribute__((unused)) GtkWidget *widget, __attribute__((unused)) gpointer   data)
{
    gtk_widget_hide_on_delete(delete_window);
}