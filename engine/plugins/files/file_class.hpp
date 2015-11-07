#ifndef PLUGIN_FILE_CLASS_H_INCLUDED
#define PLUGIN_FILE_CLASS_H_INCLUDED

#include "e8core/plugin/plugin.hpp"
#include <boost/filesystem.hpp>

extern e8_type_info file_class;

E8_DECLARE_SUB(File_Constructor);

/*
E8.Include file_class.hpp
*/

/**
E8.Class FileClass File|Файл
*/
class FileClass {
public:

    FileClass(const std::wstring &file_name);
    ~FileClass();

    /**
    E8.Constructor IN:ИмяФайла
    */
    static FileClass *Constructor(E8_IN FileName);

	/**
	E8.Property Name|Имя get:GetName
	*/
	E8::Variant GetName() const;

	/**
	E8.Property BaseName|ИмяБезРасширения get:GetBaseName
	*/
	E8::Variant GetBaseName() const;

	/**
	E8.Property Extension|Расширение get:GetExtension
	*/
	E8::Variant GetExtension() const;

	/** (setter just for test)
	E8.Property FullName|ПолноеИмя get:GetFullName set:SetFullName
	*/
	E8::Variant GetFullName() const;

	void SetFullName(const E8::Variant &value);

	/**
	E8.Property Path|Путь get:GetPath
	*/
	E8::Variant GetPath() const;

	/**
	E8.Method GetModificationTime|ПолучитьВремяИзменения variant
	*/
	E8::Variant GetModificationTime() const;

	/**
	E8.Method GetHidden|ПолучитьНевидимость bool
	*/
	bool GetHidden() const;

	/**
	E8.Method GetReadOnly|ПолучитьТолькоЧтение bool
	*/
	bool GetReadOnly() const;

	/**
	E8.Method GetModificationUniversalTime|ПолучитьУниверсальноеВремяИзменения variant
	*/
	E8::Variant GetModificationUniversalTime() const;

	/**
	E8.Method Size|Размер int
	*/
	size_t Size() const;

	/**
	E8.Method Exist|Существует bool
	*/
	bool Exist() const;

	/**
	E8.Method SetModificationTime|УстановитьВремяИзменения void IN:ВремяИзменения
	*/
	void SetModificationTime(E8_IN time);

	/**
	E8.Method SetHidden|УстановитьНевидимость void IN:Невидимость
	*/
	void SetHidden(E8_IN hidden);

	/**
	E8.Method SetReadOnly|УстановитьТолькоЧтение void IN:ТолькоЧтение
	*/
	void SetReadOnly(E8_IN read_only);

	/**
	E8.Method SetModificationUniversalTime|УстановитьУниверсальноеВремяИзменения void IN:ВремяИзменения
	*/
	void SetModificationUniversalTime(E8_IN time);

	/**
	E8.Method IsDirectory|ЭтоКаталог bool
	*/
	bool IsDirectory() const;

	/**
	E8.Method IsFile|ЭтоФайл bool
	*/
	bool IsFile() const;

private:
    boost::filesystem::path m_path;

    E8::Variant Name;
    E8::Variant BaseName;
    E8::Variant Extension;
    E8::Variant FullName;
    E8::Variant Path;
};
/**
E8.EndClass
*/

#endif // PLUGIN_FILE_CLASS_H_INCLUDED
