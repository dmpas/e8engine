#ifndef TEMPLATECLASS_HPP
#define TEMPLATECLASS_HPP

#include <gtk/gtk.h>
#include <e8core/plugin/plugin.hpp>

//!E8.Include TemplateClass.hpp

//!E8.Class GtkPortWidget Gtk::Widget
class GtkPortWidget {
public:

    virtual GtkWidget *GetWidget() = 0;
    virtual ~GtkPortWidget();

};
//!E8.EndClass

//!E8.Class GtkPortMisc Gtk::Misc
class GtkPortMisc : public GtkPortWidget {
//!E8.Inherit GtkPortWidget
public:
};
//!E8.EndClass

//!E8.Class GtkPortContainer Gtk::Container
class GtkPortContainer : public GtkPortWidget {
//!E8.Inherit GtkPortWidget
public:

    //!E8.Method Add|Добавить void IN:ДочернийЭлемент
    void Add(E8_IN Child);

    std::vector<E8::Variant> Children;

};
//!E8.EndClass


//!E8.Class GtkPortBin Gtk::Bin
class GtkPortBin : public GtkPortContainer {
//!E8.Inherit GtkPortContainer
public:

};
//!E8.EndClass

#endif // TEMPLATECLASS_HPP
