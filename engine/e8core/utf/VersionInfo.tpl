1 VERSIONINFO
FILEVERSION     $version_comma,$build_number
PRODUCTVERSION  $version_comma,$build_number
BEGIN
  BLOCK "StringFileInfo"
  BEGIN
    BLOCK "041904E3"
    BEGIN
      VALUE "CompanyName", "dmpas.ru"
      VALUE "FileDescription", "E8 Script Unicode library"
      VALUE "FileVersion", "$version_dot.$build_number"
      VALUE "InternalName", "e8std"
      VALUE "LegalCopyright", "dmpas.ru"
      VALUE "OriginalFilename", "e8core-utf.dll"
      VALUE "ProductName", "E8 Script Tools"
      VALUE "ProductVersion", "$product_version"
    END
  END

  BLOCK "VarFileInfo"
  BEGIN
    VALUE "Translation", 0x419, 1251
  END
END