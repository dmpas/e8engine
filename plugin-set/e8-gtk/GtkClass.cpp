#include "GtkClass.hpp"
#include "GtkPortWindow.hpp"
#include "GtkPortBox.hpp"
#include "GtkPortEntry.hpp"
#include "GtkPortLabel.hpp"
#include "GtkPortButton.hpp"
#include "GtkPortTreeView.hpp"

E8::Variant g_Gtk;

GtkPortClass::GtkPortClass()
{
    app = gtk_application_new("ru.dmpas.e8.gtkport", G_APPLICATION_FLAGS_NONE);
}

GtkPortClass::~GtkPortClass()
{
}

void GtkPortClass::NewApplication(E8_IN Identifier)
{
    std::string suid = Identifier.to_string().to_string();
    const char *uid = suid.c_str();

    if (!g_application_id_is_valid(uid))
        throw E8::Exception(E8::string("Невѣрный опредѣлитель приложенія!"));

    app = gtk_application_new(uid, G_APPLICATION_FLAGS_NONE);
}

E8::Variant GtkPortClass::WindowNew(E8_IN WindowType) const
{
    GtkWindowType type = GTK_WINDOW_TOPLEVEL;
    if (WindowType == GtkPortWindowType::TOPLEVEL)
        type = GTK_WINDOW_POPUP;

    GtkPortWindow *W = new GtkPortWindow(type);
    gtk_window_set_application(GTK_WINDOW(W->GetWidget()), app);
    return E8::Variant::Object(W, E8::Env.VMT["GtkPortWindow"]);
}

static void
application_activate_slot(GtkApplication *app, gpointer user_data)
{
    GtkPortClass *Data = (GtkPortClass*)user_data;
    Data->MainWindow = Data->Action.__Call("Call");
}

void GtkPortClass::Run(E8_IN Action)
{
    this->Action = Action;
    g_signal_connect(G_APPLICATION(app), "activate", G_CALLBACK(application_activate_slot), this);
    g_application_run(G_APPLICATION(app), 0, 0);
    this->Action = E8::Variant::Undefined();
    this->MainWindow = E8::Variant::Undefined();
}

E8::Variant GtkPortClass::BoxNew(E8_IN Orientation) const
{
    GtkOrientation orient = GTK_ORIENTATION_VERTICAL;
    if (Orientation == GtkPortOrientation::HORIZONTAL)
        orient = GTK_ORIENTATION_HORIZONTAL;

    GtkPortBox *B = new GtkPortBox(orient);
    return E8::Variant::Object(B, E8::Env.VMT["GtkPortBox"]);
}

E8::Variant GtkPortClass::EntryNew() const
{
    GtkPortEntry *E = new GtkPortEntry();
    return E8::Variant::Object(E, E8::Env.VMT["GtkPortEntry"]);
}

E8::Variant GtkPortClass::LabelNew(E8_IN Label) const
{
    std::string text;
    if (Label != E8::Variant::Undefined())
        text = Label.to_string().to_string();
    GtkPortLabel *L = new GtkPortLabel(text);
    return E8::Variant::Object(L, E8::Env.VMT["GtkPortLabel"]);
}

E8::Variant GtkPortClass::ButtonNew(E8_IN Text, E8_IN OnClick) const
{
    GtkPortButton *B = new GtkPortButton();

    if (Text != E8::Variant::Undefined())
        B->SetText(Text);

    return E8::Variant::Object(B, E8::Env.VMT["GtkPortButton"]);
}

E8::Variant GtkPortClass::TreeViewNew(E8_IN ValueTable) const
{
    GtkPortTreeView *TV = new GtkPortTreeView(ValueTable);
    return E8::Variant::Object(TV, E8::Env.VMT["GtkPortView"]);
}
