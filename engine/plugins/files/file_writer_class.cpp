#include "file_writer_class.hpp"
#include <sys/types.h>
#include <sys/stat.h>

#include "e8core/plugin/plugin.hpp"

using namespace std;
namespace fs = boost::filesystem;


void FileWriterClass::Close()
{
    stream.close();
}

bool FileWriterClass::IsOpen() const
{
    return stream.is_open();
}

/* .Открыть(ИмяФайла, [МонопольныйРежим]) */
void FileWriterClass::Open(E8_IN FileName, E8_IN Exclusive)
{
    if (IsOpen())
        Close();

    wstring ws = FileName.to_string().to_wstring();
    fs::path m_path(ws);

    stream.open(m_path, std::ios_base::binary);
}

/* .Записать(Массив) */
void FileWriterClass::Write(E8_IN Data)
{
    if (!stream.is_open())
        throw E8::EnvironmentException();


    if (!Data.of_type(varObject))
        E8::EnvironmentException(); //"Argument 1 is not an object!");

    E8::Variant mData(Data);

    E8::Variant v_size = mData.__Call("Count");
    size_t size = v_size.to_long();
    char *buf = new char[size];

    size_t i;
    char *cb = buf;
    for (i = 0; i < size; ++i) {
        E8::Variant el = mData.__Call("Get", i);
        long l_el = el.to_long();
        *cb = l_el;
        ++cb;
    }

    stream.write(buf, size);

    delete buf;
}


FileWriterClass *FileWriterClass::Constructor(E8_IN FileName, E8_IN Exclusive)
{

    FileWriterClass *W = new FileWriterClass();
    if (FileName != E8::Variant::Undefined())
        W->Open(FileName, Exclusive);
    return W;
}

FileWriterClass::FileWriterClass()
{

}

FileWriterClass::~FileWriterClass()
{

}
