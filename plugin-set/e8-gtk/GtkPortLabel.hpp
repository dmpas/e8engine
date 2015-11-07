#ifndef GTKPORTLABEL_HPP
#define GTKPORTLABEL_HPP

#include "TemplateClass.hpp"

//E8.Include GtkPortLabel.hpp

//!E8.Class GtkPortLabel Gtk::Label|Гтк::Надпись
class GtkPortLabel : public GtkPortMisc
{
public:
    GtkPortLabel(const std::string &text);
    virtual ~GtkPortLabel();

    //!E8.Property Text|Текст get:GetText set:SetText
    E8::Variant GetText() const;
    void SetText(E8_IN Text);

    virtual GtkWidget *GetWidget();
protected:
private:
    GtkLabel *obj;
};
//!E8.EndClass

#endif // GTKPORTLABEL_HPP
