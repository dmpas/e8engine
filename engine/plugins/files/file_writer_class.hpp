#ifndef PLUGIN_FILE_WRITER_CLASS_H_INCLUDED
#define PLUGIN_FILE_WRITER_CLASS_H_INCLUDED

#include "e8core/plugin/plugin.hpp"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

/*
E8.Include file_writer_class.hpp
*/

/**
E8.Class FileWriterClass FileWriter|ЗаписьФайла
*/
class FileWriterClass {
public:

    FileWriterClass();
    ~FileWriterClass();

    /**
    E8.Constructor [IN:ИмяФайла] [IN:Монопольно]
    */
    static
    FileWriterClass *Constructor(E8_IN FileName, E8_IN Exclusive);

    /**
    E8.Method Open|Открыть void IN:ИмяФайла [IN:Монопольно]
    */
    void Open(E8_IN FileName, E8_IN Exclusive);

    /**
    E8.Method Close|Закрыть void
    */
    void Close();

    /**
    E8.Method Write|Записать void IN:Массив
    */
    void Write(E8_IN Data);

    bool IsOpen() const;

private:
    boost::filesystem::ofstream stream;
    uintmax_t m_full_size, m_tail_size;
};
/**
E8.EndClass
*/

#endif // PLUGIN_FILE_WRITER_CLASS_H_INCLUDED
