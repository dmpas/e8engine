#ifndef GTKPORTBUTTON_HPP
#define GTKPORTBUTTON_HPP

#include "TemplateClass.hpp"

//E8.Include GtkPortButton.hpp

//!E8.Class GtkPortButton Gtk::Button|Гтк::Кнопка
class GtkPortButton : public GtkPortBin
{
//!E8.Inherit GtkPortButton
public:
    GtkPortButton();
    virtual ~GtkPortButton();

    virtual GtkWidget *GetWidget();

    //!E8.Property Text|Текст get:GetText set:SetText
    E8::Variant GetText() const;
    void SetText(E8_IN Text);

    //!E8.Method Click|Нажатие variant [IN:Обработчик]
    E8::Variant Click(E8_IN Handler);

    E8::Variant m_Clicked;

protected:

private:

    GtkButton *obj;

};
//E8.EndClass

#endif // GTKPORTBUTTON_HPP
