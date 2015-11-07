#include "e8core/plugin/plugin.h"
#include "connection.h"

static e8_property p_sqlite;
static e8_var v_sqlite;

static e8_vtable vmt_sqlite_provider;

static E8_DECLARE_SUB(e8_sqlite_connect)
{
    return e8_sqlite_connection_constructor(obj, user_data, argc, argv, result);
}

static E8_METHOD_LIST_START(sqlite_provider_methods)
    E8_METHOD_LIST_ADD("Подключить", "Connect", e8_sqlite_connect)
E8_METHOD_LIST_END


static E8_DECLARE_GET_METHOD(sqlite_provider_get_method)
{
    e8_simple_get_method(name, sqlite_provider_methods, fn);
    E8_RETURN_OK;
}

static void
__init_vmt()
{
    e8_prepare_call_elements(sqlite_provider_methods, "utf-8");

    e8_vtable_template(&vmt_sqlite_provider);
    vmt_sqlite_provider.get_method = sqlite_provider_get_method;
}

static int provider;

E8_PLUGIN_MAIN
{
    __init_vmt();

    e8_gc_register_object(&provider, &vmt_sqlite_provider);
    e8_var_object(&v_sqlite, &provider);
    e8_create_variant_property(&p_sqlite, &v_sqlite);
    p_sqlite.can_set = false;

    env->vmt->register_property(env, "Sqlite", &p_sqlite);
    E8_RETURN_OK;
}
