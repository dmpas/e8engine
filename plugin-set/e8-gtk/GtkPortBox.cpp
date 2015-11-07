#include "GtkPortBox.hpp"

GtkPortBox::GtkPortBox(GtkOrientation orientation)
{
    obj = (GtkBox*)gtk_box_new(orientation, 0);
}

GtkPortBox::~GtkPortBox()
{
    //dtor
}

void GtkPortBox::PackStart(E8_IN Child, E8_IN Expand, E8_IN Fill, E8_IN Padding)
{
    GtkPortWidget *Widget = (GtkPortWidget *)Child.plain().obj;

    bool expand = false;
    if (Expand != E8::Variant::Undefined())
        expand = Expand.to_bool();

    bool fill = false;
    if (Fill != E8::Variant::Undefined())
        fill = Fill.to_bool();

    int padding = 0;
    if (Padding != E8::Variant::Undefined())
        padding = e8_numeric_as_long(&Padding.plain().num);

    gtk_box_pack_start(obj, Widget->GetWidget(), expand, fill, padding);

    Children.push_back(Child);
}


void GtkPortBox::PackEnd(E8_IN Child, E8_IN Expand, E8_IN Fill, E8_IN Padding)
{
    GtkPortWidget *Widget = (GtkPortWidget *)Child.plain().obj;

    bool expand = false;
    if (Expand != E8::Variant::Undefined())
        expand = Expand.to_bool();

    bool fill = false;
    if (Fill != E8::Variant::Undefined())
        fill = Fill.to_bool();

    int padding = 0;
    if (Padding != E8::Variant::Undefined())
        padding = e8_numeric_as_long(&Padding.plain().num);

    gtk_box_pack_end(obj, Widget->GetWidget(), expand, fill, padding);

    Children.push_back(Child);
}


GtkWidget *GtkPortBox::GetWidget()
{
    return GTK_WIDGET(obj);
}
