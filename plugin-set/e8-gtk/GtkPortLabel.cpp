#include "GtkPortLabel.hpp"

GtkPortLabel::GtkPortLabel(const std::string &text)
{
    obj = (GtkLabel*)gtk_label_new(text.c_str());
}

GtkPortLabel::~GtkPortLabel()
{
    //dtor
}

GtkWidget *GtkPortLabel::GetWidget()
{
    return GTK_WIDGET(obj);
}

E8::Variant GtkPortLabel::GetText() const
{
    return E8::Variant(E8::string(gtk_label_get_text(obj)));
}

void GtkPortLabel::SetText(E8_IN Text)
{
    gtk_label_set_text(obj, Text.to_string().to_string().c_str());
}
