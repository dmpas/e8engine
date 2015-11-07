#ifndef XMLREADER_H
#define XMLREADER_H

#include <libxml/xmlreader.h>
#include "XmlNodeType.hpp"

/*
E8.Include XmlReader.hpp
*/

/**
E8.Class XmlReader XmlReader|ЧтениеXml
*/
class XmlReader
{
public:
    XmlReader();

    /**E8.Constructor
     */
    static XmlReader *Constructor();

    /**E8.Method OpenFile|ОткрытьФайл void IN:ИмяФайла [IN:Параметры] [IN:НаборСхем] [IN:Кодировка]
     */
    void OpenFile(E8_IN FileName, E8_IN Params, E8_IN Schemas, E8_IN Encoding);

    /**E8.Method SetString|УстановитьСтроку void IN:Строка [IN:Параметры] [IN:НаборСхем]
     */
    void SetString(E8_IN Text, E8_IN Params, E8_IN Schemas);

    /**E8.Method Close|Закрыть void
     */
    void Close();

    /**E8.Method Read|Прочитать bool
     */
    bool Read();

    /**E8.Property Name|Имя get:GetName
     */
    E8::Variant GetName() const;

    /**E8.Property LocalName|ЛокальноеИмя get:GetLocalName
     */
    E8::Variant GetLocalName() const;

    /**E8.Property NodeType|ТипУзла get:GetNodeType
     */
    E8::Variant GetNodeType() const;

    /**E8.Property Prefix|Префикс get:GetPrefix
     */
    E8::Variant GetPrefix() const;

    /**E8.Property HasValue|ИмеетЗначение get:HasValue
     */
    E8::Variant HasValue() const;

    /**E8.Property NamespaceURI|URIПространстваИмен get:GetNamespaceURI
     */
    E8::Variant GetNamespaceURI() const;

    /**E8.Property IsDefaultAttribute|ЭтоАтрибутПоУмолчанию get:IsDefaultAttribute
     */
    E8::Variant IsDefaultAttribute() const;

    /**E8.Property Value|Значение get:GetValue
     */
    E8::Variant GetValue() const;

    /**E8.Method AttributeCount|КоличествоАтрибутов int
     */
    int AttributeCount() const;

    /**E8.Method ReadAttribute|ПрочитатьАтрибут bool
     */
    bool ReadAttribute();

    /**E8.Method GetAttribute|ПолучитьАтрибут|ЗначениеАтрибута|AttributeValue variant IN:Атрибут
     */
    E8::Variant GetAttribute(E8_IN Attr);

    /**E8.Method MoveToContent|ПерейтиКСодержимому variant [IN:ПробельныеСимволы]
     */
    E8::Variant MoveToContent(E8_IN Spaces);

    /**E8.Method AttributeName|ИмяАтрибута variant IN:Индекс
     */
    E8::Variant AttributeName(E8_IN Index);

    virtual ~XmlReader();

protected:
private:
    xmlTextReader              *reader;
    char                       *data;
    bool                        skip_whitespaces;
    int                         empty_tag;
};
/*
E8.EndClass
*/

#endif // XMLREADER_H
