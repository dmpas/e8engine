#include "GtkPortWindow.hpp"
#include <iostream>

GtkPortWindow::GtkPortWindow(GtkWindowType type)
{
    obj = (GtkWindow*)gtk_window_new(type);
    gtk_window_set_default_size(obj, 200, 200);
}

GtkPortWindow::~GtkPortWindow()
{
}

void GtkPortWindow::Close()
{
    gtk_widget_set_visible(GTK_WIDGET(obj), false);
}

void GtkPortWindow::ShowAll()
{
    gtk_widget_show_all(GTK_WIDGET(obj));
}


E8::Variant GtkPortWindow::GetTitle() const
{
    return E8::string(gtk_window_get_title(obj));
}

void GtkPortWindow::SetTitle(E8_IN Title)
{
    std::string stitle = Title.to_string().to_string();
    gtk_window_set_title(obj, stitle.c_str());
}

E8::Variant GtkPortWindow::GetWidth() const
{
    int width, height;
    gtk_window_get_default_size(obj, &width, &height);
    return E8::Variant(width);
}

void GtkPortWindow::SetWidth(E8_IN Width)
{
    if (Width.of_type(varNumeric)) {
        int width, height;
        gtk_window_get_default_size(obj, &width, &height);
        width = e8_numeric_as_long(&Width.plain().num);
        gtk_window_set_default_size(obj, width, height);
    } else
        throw E8::Exception(E8::string("Невѣрно задан параметр Ширина."));

}

E8::Variant GtkPortWindow::GetHeight() const
{
    int width, height;
    gtk_window_get_default_size(obj, &width, &height);
    return E8::Variant(height);
}

void GtkPortWindow::SetHeight(E8_IN Height)
{
    if (Height.of_type(varNumeric)) {
        int width, height;
        gtk_window_get_default_size(obj, &width, &height);
        height = e8_numeric_as_long(&Height.plain().num);
        gtk_window_set_default_size(obj, width, height);
    } else
        throw E8::Exception(E8::string("Невѣрно задан параметр Высота."));
}

GtkWidget *GtkPortWindow::GetWidget()
{
    return GTK_WIDGET(obj);
}

