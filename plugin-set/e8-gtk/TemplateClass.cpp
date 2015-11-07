#include "TemplateClass.hpp"

GtkPortWidget::~GtkPortWidget()
{}

void GtkPortContainer::Add(E8_IN Child)
{
    GtkPortWidget *W = (GtkPortWidget *)Child.plain().obj;
    gtk_container_add(GTK_CONTAINER(GetWidget()), W->GetWidget());

    Children.push_back(Child);
}
