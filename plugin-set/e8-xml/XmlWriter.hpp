#ifndef XMLWRITER_HPP
#define XMLWRITER_HPP

#include "e8core/plugin/plugin.hpp"
#include <sstream>
#include <stack>

/*
E8.Include XmlWriter.hpp
 */

/**E8.Class XmlWriter XmlWriter|ЗаписьXml
 */
class XmlWriter
{
public:
    XmlWriter();
    virtual ~XmlWriter();

    /**E8.Constructor
     */
    static XmlWriter *Constructor();

    /**E8.Method SetString|УстановитьСтроку void [IN:Кодировка]
     */
    void SetString(E8_IN Encoding);

    /**E8.Method OpenFile|ОткрытьФайл void IN:ИмяФайла [IN:Кодировка]
     */
    void OpenFile(E8_IN FileName, E8_IN Encoding);

    /**E8.Method Close|Закрыть variant
     */
    E8::Variant Close();

    /**E8.Method WriteXmlDeclaration|ЗаписатьОбъявлениеXml void
     */
    void WriteXmlDeclaration();

    /**E8.Method WriteStartElement|ЗаписатьНачалоЭлемента void IN:Имя [IN:URI]
     */
    void WriteStartElement(E8_IN Name, E8_IN URI);

    /**E8.Method WriteEndElement|ЗаписатьКонецЭлемента void
     */
    void WriteEndElement();

    /**E8.Method FindPrefix|НайтиПрефикс variant IN:URI
     */
    E8::Variant FindPrefix(E8_IN URI);

    /**E8.Method WriteRaw|ЗаписатьБезОбработки void IN:Текст
     */
    void WriteRaw(E8_IN Text);

    /**E8.Method WriteText|ЗаписатьТекст void IN:Текст
     */
    void WriteText(E8_IN Text);

    /**E8.Method WriteStartAttribute|ЗаписатьНачалоАтрибута void IN:Name [IN:URI]
     */
    void WriteStartAttribute(E8_IN Name, E8_IN URI);

    /**E8.Method WriteEndAttribute|ЗаписатьКонецАтрибута void
     */
    void WriteEndAttribute();


    std::wstring escape_xml(const std::wstring &src);

protected:
private:

    void CheckElementClose(bool new_data);
    void MakeTab();

    E8::Variant FullName(E8_IN Name, E8_IN URI);

    std::wstringstream      m_sbuilder;
    std::wstring            m_filename;
    std::wstring            m_encoding;
    bool                    m_element_opened;
    bool                    m_attribute_opened;
    std::stack<E8::Variant> m_names;
    std::stack<E8::Variant> m_attributes;
    int                     m_tab_size;
    bool                    m_use_tabs;

    std::wstring            m_text_raw;

};
/*E8.EndClass
 */

#endif // XMLWRITER_HPP
