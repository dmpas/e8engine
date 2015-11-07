#ifndef GTKCLASS_HPP
#define GTKCLASS_HPP

#include <e8core/plugin/plugin.hpp>
#include <gtk/gtk.h>

//E8.Include GtkClass.hpp


//!E8.Class GtkPortClass Gtk|Гтк
class GtkPortClass
{
public:
    GtkPortClass();
    virtual ~GtkPortClass();

    /**E8.Method WindowNew|НовоеОкно variant [IN:ВидОкна]
    */
    E8::Variant WindowNew(E8_IN WindowType) const;

    /**E8.Method Run|Пуск void IN:Действие
    */
    void Run(E8_IN Action);

    /**E8.Method NewApplication|НовоеПриложение void IN:Опредѣлитель
    */
    void NewApplication(E8_IN Identifier);

    /**E8.Method BoxNew|НовыйЯщик variant [IN:Оріентація]
    */
    E8::Variant BoxNew(E8_IN Orientation) const;

    /**E8.Method EntryNew|НовоеПолеВвода variant
    */
    E8::Variant EntryNew() const;

    //!E8.Method LabelNew|НоваяНадпись variant [IN:Заголовок]
    E8::Variant LabelNew(E8_IN Label) const;


    //!E8.Method ButtonNew|НоваяКнопка variant [IN:Заголовок] [IN:ОбработчикНажатие]
    E8::Variant ButtonNew(E8_IN Text, E8_IN OnClick) const;

    //!E8.Method TreeViewNew|НовоеДерево variant IN:ТаблицаЗначений
    E8::Variant TreeViewNew(E8_IN ValueTable) const;


    E8::Variant MainWindow;
    E8::Variant Action;

protected:
private:

    GtkApplication *app;

};
//E8.EndClass

//!E8.Global g_Gtk|Gtk|Гтк Read NoWrite
extern E8::Variant g_Gtk;

#if 0
E8.Custom InitGtk
    g_Gtk = E8::Variant::Object(new GtkPortClass(), E8::Env.VMT["GtkPortClass"]);
E8.EndCustom InitGtk
#endif


#endif // GTKCLASS_HPP
