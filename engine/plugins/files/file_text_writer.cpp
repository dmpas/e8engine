#include "file_text_writer.hpp"

FileTextWriter::FileTextWriter() :
        m_delimiter(L"\n"), m_convert_delimiter(L""), m_encoding("utf-8")
{
}

FileTextWriter::~FileTextWriter()
{
    Close();
}

void FileTextWriter::Write(E8_IN text)
{
    std::wstringstream m_ss;

    if (m_convert_delimiter.size()) {
        /* Сменим символ символ ПС на указанный разделитель */
        std::wstring m_text(text.to_string().to_wstring());
        const std::wstring m_PS(L"\n");

        std::wstring::size_type p = 0, oldp = 0;
        do {
            p = m_text.find(m_PS, oldp);
            if (p != std::wstring::npos) {
                m_ss << m_text.substr(oldp, p - oldp) << m_convert_delimiter;
                oldp = p + m_PS.size();
            } else {
                m_ss << m_text.substr(oldp);
                break;
            }
        } while (true);

    } else
        m_ss << text.to_string().to_wstring();

    E8::string us (m_ss.str());
    std::string b = us.to_string(m_encoding);
    m_os.write(b.c_str(), b.size());

}

void FileTextWriter::WriteLine(E8_IN text, E8_IN delimiter)
{
    Write(text);

    E8::Variant v_delimiter(delimiter);
    if (v_delimiter == E8::Variant::Undefined()) {
        v_delimiter = E8::Variant(m_delimiter);
    }

    std::string b = v_delimiter.to_string().to_string(m_encoding);
    m_os.write(b.c_str(), b.size());
}
void FileTextWriter::SetEncoding(const std::string &encoding)
{
    m_encoding = encoding;
}
void FileTextWriter::SetLineDelimiter(const std::wstring &delimiter)
{
    m_delimiter = delimiter;
}
void FileTextWriter::SetConvertLineDelimiter(const std::wstring &delimiter)
{
    m_convert_delimiter = delimiter;
}

void FileTextWriter::Close()
{
    if (m_os.is_open())
        m_os.close();
}

void FileTextWriter::Open(E8_IN filename, E8_IN append)
{
    Close();

    fs::path m_path(filename.to_string().to_wstring());

    m_os.open(
                m_path,
                std::ios_base::binary | std::ios_base::out
                | (append ? std::ios_base::app | std::ios_base::ate : std::ios_base::out)
    );

}

FileTextWriter *FileTextWriter::Constructor(E8_IN FileName, E8_IN Encoding, E8_IN Delimiter, E8_IN Append, E8_IN LineDelimiter)
{
    FileTextWriter *result = new FileTextWriter();

    if (FileName != E8::Undefined())
        result->Open(FileName, Append);

    if (Encoding != E8::Undefined())
        result->SetEncoding(Encoding.to_string().to_string("utf-8"));

    if (Delimiter != E8::Undefined())
        result->SetLineDelimiter(Delimiter.to_string().to_wstring());

    if (LineDelimiter != E8::Undefined())
        result->SetConvertLineDelimiter(LineDelimiter.to_string().to_wstring());

    return result;
}
