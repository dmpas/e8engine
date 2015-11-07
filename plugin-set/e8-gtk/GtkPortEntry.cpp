#include "GtkPortEntry.hpp"

GtkPortEntry::GtkPortEntry()
{
    obj = (GtkEntry*)gtk_entry_new();
}

GtkPortEntry::~GtkPortEntry()
{
    //dtor
}

GtkWidget *GtkPortEntry::GetWidget()
{
    return GTK_WIDGET(obj);
}

E8::Variant GtkPortEntry::GetText() const
{
    const char *text = gtk_entry_get_text(obj);
    return E8::Variant(E8::string(text));
}

void GtkPortEntry::SetText(E8_IN Text)
{
    std::string s_text = Text.to_string().to_string();
    gtk_entry_set_text(obj, s_text.c_str());
}
