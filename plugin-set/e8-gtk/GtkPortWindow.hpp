#ifndef GTKPORTWINDOW_HPP
#define GTKPORTWINDOW_HPP

#include <e8core/plugin/plugin.hpp>
#include <gtk/gtk.h>
#include "TemplateClass.hpp"

/*E8.Include GtkPortWindow.hpp
*/

/**E8.Enum GtkPortWindowType Gtk::WindowType|Гтк::ТипОкна AutoInit GTK_WINDOW_
*/
class GtkPortWindowType {
public:

    /**E8.Element TOPLEVEL|TopLevel|Обычное
    */
    static E8::Variant TOPLEVEL;

    /**E8.Element POPUP|Popup|Всплывающее
    */
    static E8::Variant POPUP;

    static E8::Variant Cast(const GtkWindowType index);

};
/*E8.EndEnum
*/


//!E8.Class GtkPortWindow Gtk::Window|Гтк::Окно
class GtkPortWindow : public GtkPortBin
{
//!E8.Inherit GtkPortBin
public:
    GtkPortWindow(GtkWindowType type);
    virtual ~GtkPortWindow();

    /**E8.Method Close|Закрыть void
    */
    void Close();

    /**E8.Method ShowAll|ПоказатьВсе void
    */
    void ShowAll();

    /**E8.Property Title|Заголовок get:GetTitle set:SetTitle
    */
    E8::Variant GetTitle() const;

    void SetTitle(E8_IN Title);

    /**E8.Property Width|Ширина get:GetWidth set:SetWidth
    */

    E8::Variant GetWidth() const;
    void SetWidth(E8_IN Width);

    /**E8.Property Height|Высота get:GetHeight set:SetHeight
    */

    E8::Variant GetHeight() const;
    void SetHeight(E8_IN Height);

    virtual GtkWidget *GetWidget();

protected:
private:

    GtkWindow *obj;

};
//!E8.EndClass

#endif // GTKPORTWINDOW_HPP
