#ifndef FILE_TEXT_WRITER_H
#define FILE_TEXT_WRITER_H

#include "e8core/plugin/plugin.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <sstream>

namespace fs = boost::filesystem;

typedef fs::basic_ofstream<char> ofstream;



/*
E8.Include file_text_writer.hpp
*/

/**
E8.Class FileTextWriter TextWriter|ЗаписьТекста
*/
class FileTextWriter {
public:

    FileTextWriter();
    ~FileTextWriter();

    /**
    E8.Constructor [IN:ИмяФайла] [IN:Кодировка] [IN:РазделительСтрок] [IN:Дописывать] [IN:РазделительСтрокВФайле]
    */
    static FileTextWriter *Constructor(E8_IN FileName, E8_IN Encoding, E8_IN Delimiter, E8_IN Append, E8_IN LineDelimiter);

    /**
    E8.Method Close|Закрыть void
    */
    void Close();

    /**
    E8.Method Open|Открыть void IN:ИмяФайла [IN:Дописывать]
    */
    void Open        (E8_IN filename, E8_IN append);

    /**
    E8.Method Write|Записать void IN:Строка
    */
    void Write       (E8_IN text);

    /**
    E8.Method WriteLine|ЗаписатьСтроку void IN:Строка [IN:Разделитель]
    */
    void WriteLine   (E8_IN text, E8_IN delimiter);

    void SetEncoding            (const std:: string &encoding);
    void SetLineDelimiter       (const std::wstring &delimiter);
    void SetConvertLineDelimiter(const std::wstring &delimiter);

private:
    std::wstring m_delimiter;
    std::wstring m_convert_delimiter;
    std::string m_encoding;
    ofstream m_os;
};
/**
E8.EndClass
*/

#endif // FILE_TEXT_WRITER_H
