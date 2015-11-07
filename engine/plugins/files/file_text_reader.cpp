#include "file_text_reader.hpp"
#include <fstream>
#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;

e8_uchar *ws_to_e8(const wstring &ws);
wstring e8_to_ws(const e8_uchar *uc);


class FileTextReader {
public:
    FileTextReader();
    virtual ~FileTextReader();

    void Open(const wstring &filename, const char *encoding);
    void SetLineDelimiter(const char *delimiter);
    void Close();

    wstring ReadLine();
    wstring Read(size_t n);

    ifstream is;
    bool _eof_flag;
    const char *encoding;
    const char *line_delimiter;
};


FileTextReader::FileTextReader() :
    _eof_flag(false), encoding("utf-8"), line_delimiter("\n")
{}

void FileTextReader::SetLineDelimiter(const char *delimiter)
{
    line_delimiter = delimiter;
}

void FileTextReader::Open(const wstring &filename, const char *encoding)
{
    fs::path m_path(filename);
    is.open(m_path.string().c_str(), ifstream::binary);
    this->encoding = encoding;
}

void FileTextReader::Close()
{
    if (is.is_open())
        is.close();
}

FileTextReader::~FileTextReader()
 {
     Close();
 }
