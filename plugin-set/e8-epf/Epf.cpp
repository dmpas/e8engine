#include "Epf.hpp"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "V8File.h"

E8::Variant g_Epf;

EpfModule::EpfModule()
{
    //ctor
}

EpfModule::~EpfModule()
{
    //dtor
}


Epf::Epf(){}
Epf::~Epf(){}


E8::Variant Epf::AttachUnit(E8_IN FileName)
{

    boost::filesystem::path m_path(FileName.to_string().to_wstring());
    std::string text = CV8File::GetEpfModule(m_path);

    E8::Variant tmpfile = E8::Env.__Call("TempDir");
    tmpfile += E8::string("/unit.e8s");

    {
        boost::filesystem::path u_path(tmpfile.to_string().to_wstring());
        boost::filesystem::ofstream of(u_path, std::ios_base::binary);
        of << text;
        of.close();
    }

    return E8::Env.AttachUnit(tmpfile.to_string());
}
