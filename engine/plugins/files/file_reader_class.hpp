#ifndef PLUGIN_FILE_READER_CLASS_H_INCLUDED
#define PLUGIN_FILE_READER_CLASS_H_INCLUDED

#include "e8core/plugin/plugin.hpp"
#include <boost/filesystem/fstream.hpp>

/*
E8.Include file_reader_class.hpp
*/


/**
E8.Class FileReaderClass FileReader|ЧтениеФайла
*/
class FileReaderClass {
public:

    FileReaderClass();
    ~FileReaderClass();

    /**
    E8.Constructor [IN:ИмяФайла] [IN:МонопольныйРежим]
    */
    static
    FileReaderClass *Constructor(E8_IN FileName, E8_IN Exclusive);

    /**
    E8.Method Open|Открыть void IN:ИмяФайла [IN:МонопольныйРежим]
    */
    void Open(E8_IN FileName, E8_IN Exclusive);

    /**
    E8.Method Close|Закрыть void
    */
    void Close();

    /**
    E8.Method Read|Прочитать variant [IN:Размер]
    */
    E8::Variant Read(E8_IN Size);

    bool IsOpen() const;

private:
    boost::filesystem::ifstream stream;
    uintmax_t m_full_size, m_tail_size;

};

/*
E8.EndClass
*/

#endif // PLUGIN_FILE_READER_CLASS_H_INCLUDED
