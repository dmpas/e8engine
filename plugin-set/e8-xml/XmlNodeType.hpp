#ifndef XMLNODETYPE_H
#define XMLNODETYPE_H

#include "e8core/plugin/plugin.hpp"


/*E8.Include XmlNodeType.hpp
  E8.Include libxml/xmlreader.h
 */

/**E8.Enum XmlNodeType XMLNodeType|ТипУзлаXml AutoInit XML_READER_TYPE
 */
class XmlNodeType
{
public:
    XmlNodeType();

    /**E8.Element _ATTRIBUTE|Атрибут
     */
    static E8::Variant _ATTRIBUTE;

    /**E8.Element _PROCESSING_INSTRUCTION|ProcessingInstruction|ИнструкцияОбработки
     */
    static E8::Variant _PROCESSING_INSTRUCTION;

    /**E8.Element _COMMENT|Комментарий
     */
    static E8::Variant _COMMENT;

    /**E8.Element _END_ENTITY|EndEntity|КонецСущности
     */
    static E8::Variant _END_ENTITY;

    /**E8.Element _END_ELEMENT|EndElement|КонецЭлемента
     */
    static E8::Variant _END_ELEMENT;

    /**E8.Element _ELEMENT|Element|StartElement|НачалоЭлемента
     */
    static E8::Variant _ELEMENT;

    /**E8.Element _NONE|None|Ничего
     */
    static E8::Variant _NONE;

    /**E8.Element _NOTATION|Notation|Нотация
     */
    static E8::Variant _NOTATION;

    /**E8.Element _XML_DECLARATION|XmlDeclaration|ОбъявлениеXML
     */
    static E8::Variant _XML_DECLARATION;

    /**E8.Element _DOCUMENT_TYPE|DocumentType|DocumentTypeDefinition|ОпределениеТипаДокумента
     */
    static E8::Variant _DOCUMENT_TYPE;

    /**E8.Element _WHITESPACE|Whitespace|ПробельныеСимволы
     */
    static E8::Variant _WHITESPACE;

    /**E8.Element _CDATA|CDATASection|СекцияCDATA
     */
    static E8::Variant _CDATA;

    /**E8.Element _ENTITY_REFERENCE|EntityReference|СсылкаНаСущность
     */
    static E8::Variant _ENTITY_REFERENCE;

    /**E8.Element _ENTITY|Entity|Сущность
     */
    static E8::Variant _ENTITY;

    /**E8.Element _TEXT|Text|Текст
     */
    static E8::Variant _TEXT;

    /**E8.Cast int
     */
    static E8::Variant Cast(const int index);

};

/*E8.EndEnum
 */

#endif // XMLNODETYPE_H
