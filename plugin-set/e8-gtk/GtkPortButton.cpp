#include "GtkPortButton.hpp"
#include <iostream>

static void __gtk_port_button_click(GtkButton *btn, gpointer user_data)
{
    GtkPortButton *B = (GtkPortButton*)user_data;
    E8::Variant Element = E8::Variant::Object(B);
    if (B->m_Clicked != E8::Variant::Undefined())
        B->m_Clicked.__Call("Call", Element);
}

GtkPortButton::GtkPortButton()
    : m_Clicked()
{
    obj = (GtkButton*)gtk_button_new();
    g_signal_connect(obj, "clicked", G_CALLBACK(__gtk_port_button_click), this);
}

GtkPortButton::~GtkPortButton()
{

}

GtkWidget *GtkPortButton::GetWidget()
{
    return GTK_WIDGET(obj);
}

E8::Variant GtkPortButton::GetText() const
{
    return E8::Variant(E8::string(gtk_button_get_label(obj)));
}

void GtkPortButton::SetText(E8_IN Text)
{
    gtk_button_set_label(obj, Text.to_string().to_string().c_str());
}

E8::Variant GtkPortButton::Click(E8_IN Handler)
{
    if (Handler == E8::Variant::Undefined()) {
        g_signal_emit_by_name(obj, "clicked");
    } else {
        m_Clicked = Handler;
    }

    return E8::Variant::Object(this);
}
