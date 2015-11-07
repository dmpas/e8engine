#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include <sstream>

#include "e8core/utf/utf.h"
#include "e8core/env/_array.h"

#include "e8core/plugin/plugin.hpp"

#include "file_class.hpp"
#include "file_reader_class.hpp"
#include "file_writer_class.hpp"

#define DEFAULT_FILE_BUF_SIZE 1024

#ifdef __WINNT
#define CASE_SENSITIVE_DEFAULT false
#else
#define CASE_SENSITIVE_DEFAULT true
#endif

#include "main.hpp"


using namespace std;
namespace fs = boost::filesystem;

template<typename T>
int __strlen(const T *s)
{
    int l = 0;
    while (s[l])
        ++l;
    return l;
}

template<typename T>
bool match(const T *mask, const T *str)
{
    int N = __strlen(mask);
    int M = __strlen(str);

    bool A[100][100];

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < M; ++j)
            A[i][j] = false;

    for (int i = 0; i < N; ++i) { // mask iterator

        for (int j = 0; j < M; ++j) { // string iterator

            if (mask[i] == '*') {
                bool Ai1j = i == 0 ? true : A[i-1][j];
                bool Aij1 = j == 0 ? false : A[i][j-1];
                A[i][j] = Ai1j || Aij1;
            } else if (mask[i] == '?') {
                bool last = true;
                if (i != 0 && j != 0)
                    last = A[i-1][j-1];
                else if (j == 0)
                    last = false;
                A[i][j] = last;
            } else {
                bool Ai1j = i == 0 && j == 0 ? true : A[i-1][j];
                bool Aij1 = j == 0 ? false : A[i][j-1];
                bool Ai1j2 = (i && j) ? A[i-1][j-1] : false;
                bool last = Aij1 || Ai1j || Ai1j2;
                A[i][j] = last && (mask[i] == str[j]);
            } // if mask[i]

        } // for j: string iterator
    } // for i: mask iterator

    return A[N-1][M-1];
}

E8::Variant FindFiles(E8_IN Path, E8_IN Mask, E8_IN Recursive)
{

    E8::Variant A = E8::NewObject("Array");

    bool recursive = false;

    wstring path = Path.to_string().to_wstring();

    fs::path p(path);
    if (Mask == E8::Variant::Undefined()) {
        // Ищем один файл
        if (fs::exists(p)) {
            E8::Variant f = E8::NewObject("File", Path);
            A.__Call("Add", f);
        }
        return A;
    }
    if (Recursive != E8::Variant::Undefined())
        recursive = Recursive.to_bool();

    if (recursive);

    boost::system::error_code error;
    if (!fs::exists(p, error))
        return A;

    fs::directory_iterator itr(p);
    fs::directory_iterator end_iter;

    E8::string s_Mask = Mask.to_string();

    for (; itr != end_iter; ++itr) {
        if (fs::is_regular_file(itr->status())) {
            E8::string fs(itr->path().leaf().wstring());

            if (match<e8_uchar>(s_Mask.c_str(), fs.c_str())) {
                wstring path = itr->path().wstring();
                E8::Variant f = E8::NewObject("File", path);
                A.__Call("Add", f);
            }
        }
    } // for itr

    return A;
}

void FileCopy(E8_IN v_src, E8_IN v_dst)
{
    wstring
        src = v_src.to_string().to_wstring(),
        dst = v_dst.to_string().to_wstring()
    ;

    fs::path p_src(src), p_dst(dst);
    fs::copy_file(p_src, p_dst, fs::copy_option::overwrite_if_exists);

}
static const char* random_suffix()
{
    return ".aUvHn7";
}


void MoveFile(E8_IN v_src, E8_IN v_dst)
{
    wstring src = v_src.to_string().to_wstring();
    wstring dst = v_dst.to_string().to_wstring();

    fs::path p_src(src), p_dst(dst);

    fs::path p_tmp(p_src);
    p_tmp += random_suffix();

    fs::rename(p_src, p_tmp);
    fs::copy_file(p_tmp, p_dst, fs::copy_option::overwrite_if_exists);
    fs::remove(p_tmp);
}

void DeleteFiles(E8_IN Path, E8_IN Mask)
{

    E8::Variant Files = FindFiles(Path, Mask, E8::Variant(false));

    E8::Variant sz = Files.__Call("Count"), i;

    for (i = 0; i < sz; ++i) {

        E8::Variant File = Files.__Call("Get", i);
        E8::Variant FullName = File.__get("FullName");
        wstring utf_path = FullName.to_string().to_wstring();

        fs::path m_p(utf_path);
        fs::remove(m_p);
    }
}


void CreateDirectory(E8_IN Name)
{
    fs::path m_path(Name.to_string().to_wstring());
    boost::filesystem::create_directories(m_path);
}

E8::string TempFilesDir()
{
    E8::string s (fs::temp_directory_path().wstring());
    return s;
}

E8::Variant SplitFile(E8_IN FileName, E8_IN PartSize, E8_IN Path)
{
    fs::path f_path(FileName.to_string().to_wstring());
    if (!f_path.is_absolute())
        f_path = fs::absolute(f_path).make_preferred();

    wstring split_dir = f_path.parent_path().wstring();

    if (Path != E8::Variant::Undefined())
        split_dir = fs::absolute( fs::path(Path.to_string().to_wstring()) ).wstring();

    E8::Variant A = E8::NewObject("Array");

    if (!fs::exists(f_path))
        throw std::exception();

    wstring l_path = f_path.wstring();
    wstring l_name = f_path.filename().wstring();
    wstring d_path = split_dir;

    int index = 0;

    ifstream is(f_path.string().c_str(), ios_base::binary);
    while (true) {

        wstringstream ss;
        ss << d_path << "/" << l_name << "." << (++index);
        fs::ofstream os(ss.str().c_str(), ios_base::trunc | ios_base::binary);

        long part_size = PartSize.to_long();

        long done = 0;
        bool eof = false;
        while (done < part_size) {
            char buf[DEFAULT_FILE_BUF_SIZE];
            long to_read = sizeof(buf);
            if (to_read + done > part_size)
                to_read = part_size - done;
            to_read = is.readsome(buf, to_read);
            if (to_read > 0) {
                os.write(buf, to_read);
                //os.flush();
                done += to_read;
            } else {
                eof = true;
                break;
            }
        }
        os.flush();
        os.close();
        {
            E8::Variant uws (ss.str());
            A.__Call("Add", uws);
        }
        if (eof)
            break;
    }
    is.close();

    return A;
}

void MergeFiles(E8_IN Parts, E8_IN FileName)
{
    E8::Variant A = Parts;
    if (A.of_type(varString)) {

        wstring path_mask = A.to_string().to_wstring();

        fs::path pm(path_mask);

        E8::Variant pp ( pm.parent_path().wstring() );
        E8::Variant fn ( pm.filename().wstring() );

        E8::Variant Files = FindFiles(pp, fn, E8::Variant::Undefined());

        A = E8::NewObject("Array");

        E8::Variant i, sz = Files.__Call("Count");
        for (i = 0; i < sz; ++i) {

            E8::Variant File = Files.__Call("Get", i);
            E8::Variant FullName = File.__get("FullName");
            A.__Call("Add", FullName);
        }
    }

    E8::Variant i, sz = A.__Call("Count");

    fs::path p_dst(FileName.to_string().to_wstring());
    if (!p_dst.is_absolute())
        p_dst = fs::absolute(p_dst);

    string fname = p_dst.string();

    ofstream os(fname.c_str(), ios_base::binary | ios_base::trunc);

    for (i = 0; i < sz; ++i) {

        E8::Variant FileName = A.__Call("Get", i);

        fs::path f_path(FileName.to_string().to_wstring());

        fs::ifstream is(f_path, ios_base::binary);

        char buf[DEFAULT_FILE_BUF_SIZE];
        long rd = is.readsome(buf, sizeof(buf));
        while (rd > 0) {
            os.write(buf, rd);
            rd = is.readsome(buf, sizeof(buf));
        }
        is.close();
        os.flush();
    } /* for (i) */

    os.close();
}

E8::Variant File_f(E8_IN FileName)
{
    return E8::NewObject("File", FileName);
}

bool MatchMask(E8_IN Name, E8_IN Mask, E8_IN Sensitive)
{
    bool cs;
    E8::string s_FileName = Name.to_string();
    E8::string s_Mask = Mask.to_string();

    if (Sensitive == E8::Variant::Undefined()) {
        cs = CASE_SENSITIVE_DEFAULT;
    } else
        cs = Sensitive.to_bool();

    if (!cs) {
        s_FileName = s_FileName.Lower();
        s_Mask = s_Mask.Lower();
    }

    bool r = match<e8_uchar>(s_FileName.c_str(), s_Mask.c_str());

    return r;
}

void SystemCommand(E8_IN cmd_line, E8_IN current_dir)
{
    (void)current_dir;
    std::string s_cmd = cmd_line.to_string().to_string("utf-8");
    system(s_cmd.c_str());
}
