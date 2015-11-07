#include "file_class.hpp"
#include <boost/filesystem.hpp>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;
namespace fs = boost::filesystem;

E8::Variant FileClass::GetModificationTime() const
{
    time_t mt = boost::filesystem::last_write_time(m_path);
    tm *fd = localtime(&(mt));
    return E8::Variant(fd);
}
bool FileClass::GetHidden() const
{
    /* TODO: FileClass::GetHidden */
    /* TODO: Windows-only */
    return E8::Variant(false);
}
bool FileClass::GetReadOnly() const
{
    /* TODO: FileClass::GetReadonly */
    return E8::Variant(false);
}
E8::Variant FileClass::GetModificationUniversalTime() const
{
    time_t mt = fs::last_write_time(m_path);
    tm *fd = gmtime(&(mt));
    return E8::Variant(fd);
}
size_t FileClass::Size() const
{
    return fs::file_size(m_path);
}
bool FileClass::Exist() const
{
    boost::system::error_code code;
    return E8::Variant(fs::exists(m_path, code));
}

bool FileClass::IsDirectory() const
{
    boost::system::error_code code;
    return E8::Variant(fs::is_directory(m_path, code));
}

bool FileClass::IsFile() const
{
    return E8::Variant(fs::is_regular_file(m_path));
}


void FileClass::SetModificationTime(E8_IN time)
{
    tm dt = time.get_tm();
    time_t t = mktime(&dt);
    fs::last_write_time(m_path, t);
}

void FileClass::SetHidden(E8_IN hidden)
{
    /*TODO:SetHidden*/
}

void FileClass::SetReadOnly(E8_IN read_only)
{
    /*TODO:SetReadOnly*/
}

void FileClass::SetModificationUniversalTime(E8_IN time)
{
    tm GT = time.get_tm();

    time_t gt = mktime(&GT);
    tm *mt = gmtime(&gt);
    GT.tm_hour += GT.tm_hour - mt->tm_hour;
    gt = mktime(&GT);

    fs::last_write_time(m_path, gt);
}



FileClass::FileClass(const std::wstring &file_name)
    : m_path(file_name)
{
    fs::path complete = fs::absolute(m_path).make_preferred();

    E8::string ub;

    ub = m_path.filename().wstring();
    Name = ub;

    ub = m_path.stem().wstring();
    BaseName = ub;

    ub = complete.wstring();
    FullName = ub;

    ub = complete.parent_path().wstring();
    Path = ub;

    ub = m_path.extension().wstring();
    Extension = ub;
}

FileClass::~FileClass()
{
}


FileClass *FileClass::Constructor(E8_IN FileName)
{
    std::wstring fname = FileName.to_string().to_wstring();
    return new FileClass(fname);
}

E8::Variant FileClass::GetName() const
{
    return Name;
}

E8::Variant FileClass::GetBaseName() const
{
    return BaseName;
}

E8::Variant FileClass::GetExtension() const
{
    return Extension;
}

E8::Variant FileClass::GetFullName() const
{
    return FullName;
}

void FileClass::SetFullName(const E8::Variant &value)
{
    throw E8::EnvironmentException();
}

E8::Variant FileClass::GetPath() const
{
    return Path;
}
