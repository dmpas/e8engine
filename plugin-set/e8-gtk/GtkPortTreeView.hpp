#ifndef GTKPORTTREEVIEW_HPP
#define GTKPORTTREEVIEW_HPP

#include "TemplateClass.hpp"

//E8.Include GtkPortTreeView.hpp

//!E8.Class GtkPortTreeView Gtk::TreeView|Гтк::ОтображениеДерева
class GtkPortTreeView : public GtkPortContainer
{
//!E8.Inherit GtkPortContainer
public:
    GtkPortTreeView(E8::Variant ValueTable);
    virtual ~GtkPortTreeView();


    virtual GtkWidget *GetWidget();

protected:
private:

    E8::Variant ValueTable;
    GtkTreeView *obj;

};
//E8.EndClass

#endif // GTKPORTTREEVIEW_HPP
