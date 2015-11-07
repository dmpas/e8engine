#ifndef E8_FILES_MAIN_HPP
#define E8_FILES_MAIN_HPP

/*
E8.Include main.hpp
*/

/**
E8.Method FindFiles|FindFiles|НайтиФайлы variant IN [IN] [IN]
*/
E8::Variant FindFiles(E8_IN Path, E8_IN Mask, E8_IN Recursive);

/**
E8.Method FileCopy|КопироватьФайл void IN IN
*/
void FileCopy(E8_IN v_src, E8_IN v_dst);

/**
E8.Method MoveFile|ПереместитьФайл void IN IN
*/
void MoveFile(E8_IN src, E8_IN dst);

/**
E8.Method DeleteFiles|УдалитьФайлы void IN [IN]
*/
void DeleteFiles(E8_IN Path, E8_IN Mask);

/**
E8.Method CreateDirectory|СоздатьКаталог void IN
*/
void CreateDirectory(E8_IN Name);

/**
E8.Method TempFilesDir|КаталогВременныхФайлов string
*/
E8::string TempFilesDir();

/**
E8.Method SplitFile|РазделитьФайл variant IN IN [IN]
*/
E8::Variant SplitFile(E8_IN FileName, E8_IN PartSize, E8_IN Path);

/**
E8.Method MergeFiles|ОбъединитьФайлы void IN IN
*/
void MergeFiles(E8_IN Parts, E8_IN FileName);

/**
E8.Method File_f|File|Файл variant IN
*/
E8::Variant File_f(E8_IN FileName);

/**
E8.Method MatchMask|СоответствуетМаске bool IN IN [IN]
*/
bool MatchMask(E8_IN Name, E8_IN Mask, E8_IN Sensitive);

/**
E8.Method SystemCommand|КомандаСистемы void IN [IN]
*/
void SystemCommand(E8_IN cmd_line, E8_IN current_dir);

#endif // E8_FILES_MAIN_HPP
