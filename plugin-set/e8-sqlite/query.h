#ifndef E8_SQLITE_PORT_QUERY
#define E8_SQLITE_PORT_QUERY

#include "e8core/plugin/plugin.h"
#include "sqlite3.h"
#include "connection.h"
#include <malloc.h>

typedef struct __sqlite_connection e8_sqlite_connection;

#define MAX_PARAMS 10
typedef struct __sqlite_query {
    e8_sqlite_connection           *conn;
    sqlite3_stmt                   *stmt;
    bool                            done;
    int                             prestep;
    e8_uchar                       *text;

    e8_var                          params[MAX_PARAMS];
} e8_sqlite_query;

extern e8_vtable vmt_sqlite_query;

void __init_query_vmt();

#endif // E8_SQLITE_PORT_QUERY
