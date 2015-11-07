#ifndef GTKPORTENTRY_HPP
#define GTKPORTENTRY_HPP

#include <e8core/plugin/plugin.hpp>
#include "TemplateClass.hpp"

/*
E8.Include GtkPortEntry.hpp
*/

/**E8.Class GtkPortEntry Gtk::Entry|Гтк::ПолеВвода
*/
class GtkPortEntry : public GtkPortWidget
{
//!E8.Inherit GtkPortWidget
public:
    GtkPortEntry();
    virtual ~GtkPortEntry();

    /**E8.Property Text|Текст get:GetText set:SetText
    */
    E8::Variant GetText() const;

    void SetText(E8_IN Text);

    virtual GtkWidget *GetWidget();

protected:
private:
    GtkEntry *obj;
};
/*E8.EndClass
*/

#endif // GTKPORTENTRY_HPP
