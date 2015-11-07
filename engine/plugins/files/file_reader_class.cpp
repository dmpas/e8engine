#include "file_reader_class.hpp"
#include <boost/filesystem.hpp>
#include "e8core/env/_array.h"
#include <sys/types.h>
#include <sys/stat.h>

namespace fs = boost::filesystem;
using namespace std;

void FileReaderClass::Close()
{
    stream.close();
}

void FileReaderClass::Open(E8_IN FileName, E8_IN Exclusive)
{
    if (IsOpen())
        Close();

    wstring ws = FileName.to_string().to_wstring();
    fs::path m_path(ws);

    stream.open(m_path, std::ios_base::binary);

    stream.seekg(0, stream.end);
    m_full_size = stream.tellg();
    stream.seekg(0, stream.beg);

    m_tail_size = m_full_size;
}

/* .Прочитать([Размер]) -> ФиксированныйМассив */
E8::Variant FileReaderClass::Read(E8_IN Size)
{
    if (!IsOpen()) {
        throw E8::EnvironmentException();
        //E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, "File not opened!");
    }

    //std::cout << "inSize: " << Size << std::endl << std::flush;

    size_t size = 0;
    if (Size != E8::Variant::Undefined()) {
        size = Size.to_long();
    }
    if (size == 0 || size > m_tail_size)
        size = m_tail_size;

    char *buf = new char[size];

    int read = stream.readsome(buf, size);
    m_tail_size -= read;
    if (m_tail_size < 0)
        m_tail_size = 0;

    E8::Variant A = E8::NewObject("Array", size);

    int i = 0;
    for (i = 0; i < read; ++i) {
        long l_value = (unsigned char)buf[i];
        E8::Variant V(l_value);
        A.__Call("Set", i, V);
    }

    delete buf;

    A = E8::NewObject("FixedArray", A);
    return A;
}


FileReaderClass *FileReaderClass::Constructor(E8_IN FileName, E8_IN Exclusive)
{
    FileReaderClass *R = new FileReaderClass();

    if (FileName != E8::Variant::Undefined())
        R->Open(FileName, Exclusive);

    return R;
}

bool FileReaderClass::IsOpen() const
{
    return stream.is_open();
}

FileReaderClass::FileReaderClass()
{

}

FileReaderClass::~FileReaderClass()
{

}
