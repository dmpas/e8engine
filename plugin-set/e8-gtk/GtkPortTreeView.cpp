#include "GtkPortTreeView.hpp"

GtkPortTreeView::GtkPortTreeView(E8::Variant ValueTable)
    : ValueTable(ValueTable)
{
    GtkTreeModel *model = NULL;
    obj = (GtkTreeView *)gtk_tree_view_new_with_model(model);
}

GtkPortTreeView::~GtkPortTreeView()
{
    //dtor
}


GtkWidget *GtkPortTreeView::GetWidget()
{
    return GTK_WIDGET(obj);
}
