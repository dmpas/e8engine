#include "XmlWriter.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <sstream>

XmlWriter::XmlWriter()
    : m_encoding(L""), m_element_opened(false), m_tab_size(2), m_use_tabs(true)
{
}

XmlWriter::~XmlWriter()
{
}

XmlWriter *XmlWriter::Constructor()
{
    return new XmlWriter();
}

void XmlWriter::SetString(E8_IN Encoding)
{
    m_filename = L"";
    if (Encoding != E8::Variant::Undefined())
        m_encoding = Encoding.to_string().Lower().to_wstring();
}

void XmlWriter::OpenFile(E8_IN FileName, E8_IN Encoding)
{
    m_filename = FileName.to_string().to_wstring();
    if (Encoding != E8::Variant::Undefined())
        m_encoding = Encoding.to_string().Lower().to_wstring();
}

E8::Variant XmlWriter::Close()
{
    E8::string R("");
    if (m_filename.size() != 0) {

        boost::filesystem::path m_path(m_filename);
        boost::filesystem::wofstream os(m_path);
        os << m_sbuilder.str();

        os.close();
    } else {
        R = m_sbuilder.str();
    }

    return R;
}

void XmlWriter::WriteXmlDeclaration()
{
    m_sbuilder << L"<?xml version=\"1.0\"";
    if (m_encoding.size() != 0)
        m_sbuilder << " encoding=\"" << m_encoding << "\"";
    m_sbuilder << "?>" << std::endl;
}

void XmlWriter::WriteStartElement(E8_IN Name, E8_IN URI)
{
    E8::Variant FName = FullName(Name, URI);

    if (m_element_opened) {
        /* Начали вложенный */
        m_sbuilder << ">" << std::endl;
    }

    MakeTab();
    m_sbuilder << "<";
    WriteRaw(FName);
    m_names.push(FName);
    m_element_opened = true;
}

E8::Variant XmlWriter::FindPrefix(E8_IN URI)
{
    return E8::Variant("");
}

void XmlWriter::WriteRaw(E8_IN Text)
{
    std::wstring _data = Text.to_string().to_wstring();
    m_sbuilder << _data;
}

void XmlWriter::CheckElementClose(bool new_data)
{
    if (!m_element_opened)
        return;

    m_sbuilder << " />";
}

void XmlWriter::WriteEndElement()
{
    if (m_element_opened) {

        m_sbuilder << " />" << std::endl;
        m_element_opened = false;

    } else {

        m_sbuilder << "</";

        WriteRaw(m_names.top());
        m_sbuilder << ">" << std::endl;

    }
    m_names.pop();
}

void XmlWriter::MakeTab()
{
    size_t i;
    int j;

    for (i = 0; i < m_names.size(); ++i) {
        if (m_use_tabs)
            m_sbuilder << L"\t";
        else {
            for (j = 0; j < m_tab_size; ++j)
                m_sbuilder << L" ";
        }
    }
}

void XmlWriter::WriteText(E8_IN Text)
{
    if (!m_element_opened && !m_attribute_opened)
        throw std::exception(); /* TODO: Внятное исключение */

    if (m_attribute_opened) {
        /* Для атрибутов накапливаем текст */
        m_text_raw += escape_xml(Text.to_string().to_wstring());
    } else {
        m_element_opened = false;

        E8::Variant Raw(escape_xml(Text.to_string().to_wstring()));
        WriteRaw(Raw);
    }
}

std::wstring XmlWriter::escape_xml(const std::wstring &src)
{
    std::wstringstream buffer;
    for(size_t pos = 0; pos != src.size(); ++pos) {
        switch(src[pos]) {
            case '&':  buffer << ("&amp;");       break;
            case '\"': buffer << ("&quot;");      break;
            case '\'': buffer << ("&apos;");      break;
            case '<':  buffer << ("&lt;");        break;
            case '>':  buffer << ("&gt;");        break;
            default:   buffer << src[pos];        break;
        }
    }
    return buffer.str();
}

E8::Variant XmlWriter::FullName(E8_IN Name, E8_IN URI)
{
    return
        (URI != E8::Variant::Undefined())
        ? (FindPrefix(URI) + E8::Variant(":") + Name)
        : Name
    ;
}

void XmlWriter::WriteStartAttribute(E8_IN Name, E8_IN URI)
{

    if (m_attribute_opened || !m_element_opened)
        throw std::exception(); /* TODO: Внятное исключение */

    E8::Variant FName = FullName(Name, URI);
    m_attributes.push(FName);
    m_attribute_opened = true;

    m_text_raw = L"";

}

void XmlWriter::WriteEndAttribute()
{
    if (!m_attribute_opened)
        throw std::exception(); /* TODO: Внятное исключение */

    if (m_text_raw.size() != 0) {
        m_sbuilder << L" ";
        WriteRaw(m_attributes.top());
        m_sbuilder << L"=\"";
        m_sbuilder << m_text_raw;
        m_sbuilder << L"\"";

        m_text_raw = L"";
    }

    m_attributes.pop();
    m_attribute_opened = false;
}
