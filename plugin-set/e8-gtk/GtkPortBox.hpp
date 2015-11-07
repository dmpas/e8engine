#ifndef GTKPORTBOX_HPP
#define GTKPORTBOX_HPP

#include <e8core/plugin/plugin.hpp>
#include <gtk/gtk.h>
#include "TemplateClass.hpp"

/*E8.Include GtkPortBox.hpp
*/

/**E8.Enum GtkPortOrientation Gtk::Orientation|Гтк::Ориентация AutoInit GTK_ORIENTATION_
*/
class GtkPortOrientation {
public:
    /**E8.Element VERTICAL|Vertical|Вертикально
    */
    static E8::Variant VERTICAL;
    /**E8.Element HORIZONTAL|Horizontal|Горизонтально
    */
    static E8::Variant HORIZONTAL;
};
/*E8.EndEnum
*/

/**E8.Class GtkPortBox Gtk::Box|Гтк::Ящик
*/
class GtkPortBox : public GtkPortContainer
{
//!E8.Inherit GtkPortContainer
public:
    GtkPortBox(GtkOrientation orientation);

    /**E8.Method PackStart|ДобавитьВНачало void IN:ДочернийОбъект [IN:Растягивать] [IN:Заполнять] [IN:Отступ]
    */
    void PackStart(E8_IN Child, E8_IN Expand, E8_IN Fill, E8_IN Padding);

    /**E8.Method PackEnd|ДобавитьВКонец void IN:ДочернийОбъект [IN:Растягивать] [IN:Заполнять] [IN:Отступ]
    */
    void PackEnd(E8_IN Child, E8_IN Expand, E8_IN Fill, E8_IN Padding);

    virtual ~GtkPortBox();

    virtual GtkWidget *GetWidget();


protected:
private:
    GtkBox *obj;
};
/*E8.EndClass
*/

#endif // GTKPORTBOX_HPP
