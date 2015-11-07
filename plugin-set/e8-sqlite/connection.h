#ifndef E8_SQLITE_PORT_CONNECTION
#define E8_SQLITE_PORT_CONNECTION

#include "e8core/plugin/plugin.h"
#include "sqlite3.h"

typedef struct __sqlite_connection {
    sqlite3         *db;
} e8_sqlite_connection;

extern e8_vtable vmt_sqlite_connection;
extern e8_type_info type_sqlite_connection;

E8_DECLARE_SUB(e8_sqlite_connection_constructor);

#endif // E8_SQLITE_PORT_CONNECTION
