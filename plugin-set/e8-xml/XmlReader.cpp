#include "XmlReader.hpp"
#include "GPort.hpp"

#include <libxml/xmlreader.h>

#include <iostream>
#include <fstream>

XmlReader *XmlReader::Constructor()
{
    return new XmlReader();
}

XmlReader::XmlReader()
    : reader(NULL), data(NULL), skip_whitespaces(true), empty_tag(0)
{
}

void XmlReader::Close()
{
    if (reader) {
        xmlTextReaderClose(reader);
        xmlFreeTextReader(reader);
        reader = NULL;
    }
    if (data) {
        delete data;
        data = NULL;
    }
}

XmlReader::~XmlReader()
{
    Close();
}

void XmlReader::OpenFile(E8_IN FileName, E8_IN Params, E8_IN Schemas, E8_IN Encoding)
{
    Close();

    std::string sname = FileName.to_string().to_string();

    {
        std::ifstream f(sname.c_str(), std::ios_base::binary);
        if (!f) {
            //
            throw E8::Exception(E8::string("XML: File access error!"));
        }
    }

    std::string encoding("utf-8");
    if (Encoding != E8::Variant::Undefined())
        encoding = Encoding.to_string().to_string();

    reader = xmlReaderForFile(sname.c_str(), encoding.c_str(), 0);
}

void XmlReader::SetString(E8_IN Text, E8_IN Params, E8_IN Schemas)
{
    Close();

    std::string s_text = Text.to_string().to_string();
    size_t size = s_text.size();
    data = new char[size + 1];
    memcpy(data, s_text.c_str(), size + 1);

    reader = xmlReaderForMemory(data, size, "", "utf-8", 0);
}

bool XmlReader::Read()
{
    if (empty_tag == 1) {
        empty_tag = 2;
        return true;
    }

    int r;
    do {

        r = xmlTextReaderRead(reader);

        if (r == -1) {
            // НАДО: Внятное сообщеніе
            throw E8::Exception(E8::string("XML read error!"));
        }

        if (r == 0)
            return false;

    } while (skip_whitespaces
             && xmlTextReaderNodeType(reader) == XML_READER_TYPE_SIGNIFICANT_WHITESPACE);

    empty_tag = xmlTextReaderIsEmptyElement(reader) ? 1 : 0;

    return (r == 1);
}

E8::Variant XmlReader::GetName() const
{
    const xmlChar *name = xmlTextReaderConstName(reader);
    if (name == NULL) {
        // НАДО: Внятное сообщеніе
        throw E8::Exception(E8::string("XML read error!"));
    }

    E8::string s((const char *)name);

    return E8::Variant(s);
}

E8::Variant XmlReader::GetLocalName() const
{
    const xmlChar *name = xmlTextReaderConstLocalName(reader);
    if (name == NULL) {
        // НАДО: Внятное сообщеніе
        throw E8::Exception(E8::string("XML read error!"));
    }

    E8::string s((const char *)name);

    return E8::Variant(s);
}

E8::Variant XmlReader::GetNodeType() const
{
    if (empty_tag == 2) {
        return XmlNodeType::_END_ELEMENT;
    }

    int _type = xmlTextReaderNodeType(reader);
    return XmlNodeType::Cast(_type);
}

E8::Variant XmlReader::GetPrefix() const
{
    const xmlChar *name = xmlTextReaderConstPrefix(reader);
    if (name == NULL) {
        // НАДО: Внятное сообщеніе
        throw E8::Exception(E8::string("XML read error!"));
    }

    E8::string s((const char *)name);

    return E8::Variant(s);
}

E8::Variant XmlReader::HasValue() const
{
    int r = xmlTextReaderHasValue(reader);

    if (r == -1) {
        // НАДО: Внятное сообщеніе
        throw E8::Exception(E8::string("XML read error!"));
    }

    return E8::Variant(r == 1);
}

E8::Variant XmlReader::GetNamespaceURI() const
{
    xmlChar *name = xmlTextReaderNamespaceUri(reader);
    if (name == NULL) {
        // НАДО: Внятное сообщеніе
        throw E8::Exception(E8::string("XML read error!"));
    }

    E8::string s((const char *)name);

    xmlFree(name);

    return E8::Variant(s);
}

E8::Variant XmlReader::IsDefaultAttribute() const
{
    int r = xmlTextReaderIsDefault(reader);

    if (r == -1) {
        // НАДО: Внятное сообщеніе
        throw E8::Exception(E8::string("XML read error!"));
    }

    return E8::Variant(r == 1);
}

E8::Variant XmlReader::GetValue() const
{
    const xmlChar *name = xmlTextReaderConstValue(reader);
    if (name == NULL) {
        // НАДО: Внятное сообщеніе
        throw E8::Exception(E8::string("XML read error!"));
    }

    E8::string s((const char *)name);

    return E8::Variant(s);
}

int XmlReader::AttributeCount() const
{
    if (empty_tag == 2)
        return 0;

    int r = xmlTextReaderAttributeCount(reader);

    if (r == -1) {
        // НАДО: Внятное сообщеніе
        throw E8::Exception(E8::string("XML read error!"));
    }

    return r;
}

bool XmlReader::ReadAttribute()
{
    if (empty_tag == 2)
        return false;

    int r = xmlTextReaderMoveToNextAttribute(reader);

    if (r == -1) {
        // НАДО: Внятное сообщеніе
        throw E8::Exception(E8::string("XML read error!"));
    }

    return r == 1;
}

E8::Variant XmlReader::GetAttribute(E8_IN Attr)
{
    int move = 0;
    if (Attr.plain().type == varNumeric)
        move = xmlTextReaderMoveToAttributeNo(reader, Attr.plain().num.l_value);
    else {
        std::string s_attr = Attr.to_string().to_string();
        move = xmlTextReaderMoveToAttribute(reader, (const xmlChar *)s_attr.c_str());
    }

    if (move == -1) {
        // НАДО: Внятное сообщеніе
        throw E8::Exception(E8::string("XML read error!"));
    }

    E8::Variant Result;

    if (move)
        Result = GetValue();

    xmlTextReaderMoveToElement(reader);

    return Result;
}

E8::Variant XmlReader::MoveToContent(E8_IN Spaces)
{
    int _type;
    _type = xmlTextReaderNodeType(reader);
    while (
           _type != XML_READER_TYPE_TEXT
           && _type != XML_READER_TYPE_ELEMENT
           && _type != XML_READER_TYPE_END_ELEMENT
           && (_type != XML_READER_TYPE_SIGNIFICANT_WHITESPACE || skip_whitespaces)
           && _type != XML_READER_TYPE_ENTITY_REFERENCE
           && _type != XML_READER_TYPE_END_ENTITY
           ) {

        if (xmlTextReaderRead(reader) == 0)
            return XmlNodeType::_NONE;

        _type = xmlTextReaderNodeType(reader);
    }

    return XmlNodeType::Cast(_type);
}

E8::Variant XmlReader::AttributeName(E8_IN Index)
{
    if (Index.plain().type != varNumeric)
        throw E8::Exception(E8::string("Not a numeric!"));

    int move = xmlTextReaderMoveToAttributeNo(reader, Index.plain().num.l_value);
    if (move == -1) {
        // НАДО: Внятное сообщеніе
        throw E8::Exception(E8::string("XML read error!"));
    }

    E8::Variant Result(GetName());
    xmlTextReaderMoveToElement(reader);

    return Result;
}
