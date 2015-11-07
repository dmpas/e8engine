#ifndef E8_LIB_PQ_H
#define E8_LIB_PQ_H

#include "e8core/plugin/plugin.hpp"
#include <libpq-fe.h>

/*E8.Include PQ.hpp
 */

//namespace E8 {

/**E8.Class PqConnection Pq:Connection|Пг:Соединение
 */
class PqConnection {
public:
    PqConnection();
    ~PqConnection();

    /**E8.Constructor [IN:СтрокаСоединения]
     */
    static PqConnection *
    Constructor(E8_IN ConnectionString);

    /**E8.Method Connect|Подключить void IN:СтрокаСоединения
     */
    void Connect(E8_IN ConnectionString);

    /**E8.Method Disconnect|Отключить void
     */
    void Disconnect();

    /**E8.Method Query|Запрос variant [IN:ТекстЗапроса]
     */
    E8::Variant Query(E8_IN Text);

    PGconn             *conn;
};
/*E8.EndClass*/

/**E8.Class PqQueryResult Pq:QueryResult|Пг:РезультатЗапроса
 */
class PqQueryResult {
public:
    PqQueryResult(PGresult *res);
    ~PqQueryResult();

private:
    PGresult *res;
};
/*E8.EndClass*/

/**E8.Class PqQuery Pq:Query|Пг:Запрос
 */
class PqQuery {
public:

    PqQuery(PqConnection *conn);
    ~PqQuery();

    /**E8.Property Text|Текст get:GetText set:SetText
     */
    void SetText(E8_IN Value);
    E8::Variant GetText() const;

    /**E8.Method Execute|Выполнить variant
     */
    E8::Variant Execute();

private:
    PqConnection   *conn;
    E8::Variant     Text;

};
/*E8.EndClass*/

/**E8.Class PQ PQ|Постгрес
 */
class PQ
{
public:
    PQ();
    ~PQ();
};
/*E8.EndClass*/

//} // namespace E8

#endif // E8_LIB_PQ_H
