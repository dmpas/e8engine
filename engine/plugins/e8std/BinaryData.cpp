#include "BinaryData.hpp"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

BinaryData::BinaryData()
    : m_data(0), m_size(0)
{

}

void BinaryData::Load(E8_IN FileName)
{
    boost::filesystem::path m_path(FileName.to_string().to_wstring());
    boost::filesystem::ifstream is(m_path, std::ios_base::binary);

    is.seekg(0, is.end);
    m_size = is.tellg();
    is.seekg(0, is.beg);

    if (m_size) {
        m_data = new char[m_size];
        is.readsome(m_data, m_size);
    }
}

void BinaryData::SetString(const std::string &data)
{
    m_size = data.size();
    if (m_size) {
        m_data = new char[m_size];
        memcpy(m_data, data.c_str(), m_size);
    }
}

void BinaryData::Write(E8_IN FileName)
{
    boost::filesystem::path m_path(FileName.to_string().to_wstring());
    boost::filesystem::ofstream os(m_path, std::ios_base::binary);

    os.write(m_data, m_size);
}

BinaryData* BinaryData::Constructor(E8_IN FileName)
{
    BinaryData* d = new BinaryData();
    d->Load(FileName);
    return d;
}

BinaryData::~BinaryData()
{
    if (m_data)
        delete m_data;
}

int BinaryData::Size() const
{
    return m_size;
}

BinaryDataManager::BinaryDataManager()
{

}
BinaryDataManager::~BinaryDataManager()
{

}

E8::Variant m_BinaryData;

E8::Variant BinaryDataManager::FromString(E8_IN String, E8_IN Encoding)
{
    std::string enc = "utf-8";
    if (Encoding != E8::Undefined())
        enc = Encoding.to_string().to_string("utf-8");

    std::string data = String.to_string().to_string(enc.c_str());


    BinaryData *bd = new BinaryData();
    bd->SetString(data);

    return E8::Variant::Object(bd, E8::Env.VMT["BinaryData"]);
}
