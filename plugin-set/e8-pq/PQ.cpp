#include "PQ.hpp"
#include <iostream>

//namespace E8 {

PqConnection::PqConnection()
    : conn(NULL)
{

}
PqConnection::~PqConnection()
{
    Disconnect();
}

void PqConnection::Connect(E8_IN ConnectionString)
{
    if (conn != NULL)
        Disconnect();

    std::string scinfo = ConnectionString.to_string().to_string();
    conn = PQconnectdb(scinfo.c_str());

    ConnStatusType ct = PQstatus(conn);
    if (ct != CONNECTION_OK) {
        // НАДО: Внятное сообщеніе
        throw E8::Exception(E8::string("PQ: failed to connect!"));
    }
}

void PqConnection::Disconnect()
{
    if (conn != NULL)
        PQfinish(conn);
    conn = NULL;
}

PqConnection *PqConnection::Constructor(E8_IN ConnectionString)
{
    PqConnection *R = new PqConnection();
    if (ConnectionString != E8::Undefined())
        R->Connect(ConnectionString);
    return R;
}

E8::Variant PqConnection::Query(E8_IN Text)
{
    PqQuery *Q = new PqQuery(this);
    E8::GC::Register(Q, E8::Env.VMT["PqQuery"]);

    if (Text != E8::Undefined())
        Q->SetText(Text);

    return E8::Variant::Object(Q);
}


PqQuery::PqQuery(PqConnection *conn)
    : conn(conn), Text("")
{
    E8::GC::Use(conn);
}
PqQuery::~PqQuery()
{
    E8::GC::Free(conn);
}

void PqQuery::SetText(E8_IN Value)
{
    Text = E8::Variant(Value.to_string());
}
E8::Variant PqQuery::GetText() const
{
    return Text;
}

E8::Variant PqQuery::Execute()
{
    PGresult *res = PQexec(conn->conn, Text.to_string().to_string().c_str());
    //PGresult *res = PQexec(conn->conn, "select * from config");

    ExecStatusType stt = PQresultStatus(res);
    if (stt != PGRES_TUPLES_OK && stt != PGRES_COMMAND_OK) {
        char *err = PQresultErrorMessage(res);
        throw E8::Exception(E8::string(err));
    }

    PqQueryResult *R = new PqQueryResult(res);
    E8::GC::Register(R, E8::Env.VMT["PqQueryResult"]);

    return E8::Variant::Object(R);
}


PQ::PQ()
{
    //ctor
}

PQ::~PQ()
{
    //dtor
}

PqQueryResult::PqQueryResult(PGresult *res)
    : res(res)
{

}
PqQueryResult::~PqQueryResult()
{
    PQclear(res);
    res = NULL;
}


//} // namespace E8
