#ifndef BINARYDATA_H
#define BINARYDATA_H

#include "e8core/plugin/plugin.hpp"

/*E8.Include BinaryData.hpp
*/

/**E8.Class BinaryData BinaryData|ДвоичныеДанные
 */
class BinaryData
{
public:
    BinaryData();
    virtual ~BinaryData();

    /**E8.Constructor IN:ИмяФайла
     */
    static BinaryData* Constructor(E8_IN FileName);

    /**E8.Method Size|Размер int
     */
    int Size() const;

    /**E8.Method Write|Записать void IN:ИмяФайла
     */
    void Write(E8_IN FileName);

    void Load(E8_IN FileName);
    void SetString(const std::string &data);

private:

    char           *m_data;
    int             m_size;
};
/*E8.EndClass*/

/**E8.Class BinaryDataManager BinaryDataManager
 */
class BinaryDataManager{
public:

    BinaryDataManager();
    ~BinaryDataManager();

    /**E8.Method FromString|ИзСтроки variant IN:Строка [IN:Кодировка]
     */
    E8::Variant FromString(E8_IN String, E8_IN Encoding);

};
/*E8.EndClass*/

/**E8.Global m_BinaryData|BinaryData|ДвоичныеДанные
*/
extern E8::Variant m_BinaryData;
/*
E8.Custom InitBinaryDataManager
    m_BinaryData = E8::Variant::Object(new BinaryDataManager(), E8::Env.VMT["BinaryDataManager"]);
E8.EndCustom InitBinaryDataManager
*/

#endif // BINARYDATA_H
