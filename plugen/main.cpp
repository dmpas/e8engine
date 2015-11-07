#include <iostream>
#include <string.h>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <algorithm>

using std::string;
using std::vector;
using std::map;
using std::stringstream;

string in_file;
string out_h;
string out_cxx;

typedef boost::filesystem::ofstream ofstream;
typedef boost::filesystem::ifstream ifstream;

const string E8_Class           ("E8.Class");
const string E8_Method          ("E8.Method");
const string E8_EndClass        ("E8.EndClass");
const string E8_Include         ("E8.Include");
const string E8_Identifier      ("E8.Identifier");
const string E8_Attach          ("E8.Attach");
const string E8_Main            ("E8.Main");
const string E8_Constructor     ("E8.Constructor");
const string E8_Property        ("E8.Property");
const string E8_Enum            ("E8.Enum");
const string E8_EndEnum         ("E8.EndEnum");
const string E8_Element         ("E8.Element");
const string E8_Cast            ("E8.Cast");
const string E8_Global          ("E8.Global");
const string E8_Custom          ("E8.Custom");
const string E8_EndCustom       ("E8.EndCustom");
const string E8_Inherit         ("E8.Inherit");

class codegen_exception : public std::exception {
public:
    codegen_exception(const char *msg)
        : msg(msg)
    { }

    virtual ~codegen_exception() throw()
    { }

    codegen_exception(const string &msg)
        : msg(msg)
    { }

    void tell() const
    {
        std::cout << msg << std::endl;
    }

private:
    string msg;
};

class ParamInfo {
public:
    enum {IN, OUT};

    ParamInfo(int in_out, bool expected)
        : in_out(in_out), expected(expected)
    { }

    ParamInfo(const ParamInfo &src)
        : in_out(src.in_out), expected(src.expected)
    { }

    int         in_out;
    bool        expected;
};

enum ResultType {Void, Bool, Object, Int, Double, String, Variant};
map<string, ResultType> types;

template<typename T>
void assign(vector<T> &dst, const vector<T> &src)
{
    dst.assign(src.begin(), src.end());
}

class PropertyInfo {
public:

    PropertyInfo() {}
    PropertyInfo(const PropertyInfo &src)
        : Name(src.Name), Getter(src.Getter), Setter(src.Setter)
    {
        assign(Aliases, src.Aliases);
    }

    string Name;
    string Getter;
    string Setter;

    vector<string> Aliases;
};

class MethodInfo {
public:
    MethodInfo() {}
    MethodInfo(const MethodInfo &src)
        : Name(src.Name), type(src.type)
    {
        assign(Aliases, src.Aliases);
        assign(Params, src.Params);
    }

    string              Name;
    vector<string>      Aliases;
    vector<ParamInfo>   Params;
    ResultType          type;
};

class ClassInfo {
public:

    ClassInfo() : HasConstructor(false) {}
    ClassInfo(const ClassInfo &src)
        : Name(src.Name), HasConstructor(src.HasConstructor)
    {
        assign(Aliases, src.Aliases);
        assign(Methods, src.Methods);
        assign(ConstructorParams, src.ConstructorParams);
    }

    string               Name;
    vector<string>       Aliases;
    vector<MethodInfo>   Methods;
    bool                 HasConstructor;
    vector<ParamInfo>    ConstructorParams;
    vector<PropertyInfo> Properties;

    void inherit(const ClassInfo &super)
    {
        vector<MethodInfo>::const_iterator mit = super.Methods.begin();
        for (; mit != super.Methods.end(); ++mit)
            Methods.push_back(*mit);

        vector<PropertyInfo>::const_iterator pit = super.Properties.begin();
        for (; pit != super.Properties.end(); ++pit)
            Properties.push_back(*pit);
    }
};

class EnumElement {
public:
    string              Name;
    vector<string>      Aliases;
};

class EnumInfo {
public:

    EnumInfo() {}
    EnumInfo(const EnumInfo &src)
        : Name(src.Name)
    {
        assign(Aliases, src.Aliases);
        assign(Elements, src.Elements);
    }

    string              Name;
    vector<string>      Aliases;
    vector<EnumElement> Elements;
    string              CastType;
    string              AutoInit;
};

class GlobalVar {
public:

    GlobalVar()
        : Name(""), Read("false"), Write("false") {}
    GlobalVar(const string &Name)
        : Name(Name), Read("true"), Write("false") {}
    GlobalVar(const GlobalVar &src)
        : Name(src.Name), Read(src.Read), Write(src.Write)
    {
        assign(Aliases, src.Aliases);
    }

    string              Name;
    vector<string>      Aliases;
    string              Read;
    string              Write;
};


vector<string> delimit(const string &src, const string &del)
{

    vector<string> R;

    string::size_type pos = src.find(del), old_pos = 0;
    while (pos != string::npos) {

        string sub = src.substr(old_pos, pos - old_pos);
        R.push_back(sub);

        pos += del.size();

        old_pos = pos;
        pos = src.find(del, pos);
    }

    R.push_back(src.substr(old_pos));

    return R;
}

static map<string, ClassInfo> classes;
static map<string, EnumInfo> enums;

static ClassInfo *in_class = NULL;
static EnumInfo *in_enum = NULL;

static vector<MethodInfo> methods, all_methods;
static vector<GlobalVar> vars;

static stringstream custom_post;

string escape_class_method(const string &_class, const string &method)
{
    string r("WR_");

    r += _class;
    r += "_";
    r += method;

    return r;
}

string escape_class_property(const string &_class, const string &property)
{
    string r("P_");

    r += _class;
    r += "_";
    r += property;

    return r;
}

string escape_property_getter(const string &_class, const string &property)
{
    string r("WR__");

    r += _class;
    r += "_get_";
    r += property;

    return r;
}

string escape_property_setter(const string &_class, const string &property)
{
    string r("WR__");

    r += _class;
    r += "_set_";
    r += property;

    return r;
}

string escape_enum_element(const string &_enum, const string &element)
{
    string r("WR__E_");
    r += _enum;
    r += "_";
    r += element;

    return r;
}

std::ofstream oh;
std::ofstream oc;

namespace fs = boost::filesystem;

static void
out_methods(const ClassInfo *in_class, const vector<MethodInfo> &Methods)
{
    vector<MethodInfo>::const_iterator mit;
    string ClassName = in_class ? in_class->Name : "";

    for (mit = Methods.begin(); mit != Methods.end(); ++mit) {
        oc << "static E8_DECLARE_SUB(" << escape_class_method(ClassName, mit->Name)
            << ")" << std::endl
            << "{" << std::endl;
        if (in_class)
            oc << "\t" << in_class->Name << " *O = (" << ClassName << "*)obj;" << std::endl;

        oc << "\te8_property *margv = argv;" << std::endl;

        vector<ParamInfo>::const_iterator pit;
        int index = 0;
        for (pit = mit->Params.begin(); pit != mit->Params.end(); ++pit) {
            oc << "\tE8::Variant p" << index << "; " << std::endl;
            oc << "\t{" /*<< std::endl << "\t\te8_var v; e8_var_undefined(&v);"*/ << std::endl;
            oc << "\t\tif (argc) { argv->get(argv, &p" << index << ".plain()); --argc; ++argv; }" << std::endl;

            if (pit->expected) {
                oc << "\t\telse E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, "
                        "\"Wrong param " << (index + 1) << "\");" << std::endl;
            }
            oc /*<< "\t\tp" << index << " = E8::Variant(v);" << std::endl*/ << "\t}" << std::endl;
            ++index;
        }

        oc << "\ttry {" << std::endl;
        oc << "\t\t";
        if (mit->type != Void) {
            oc << "E8::Variant Result ( ";
        }
        if (in_class)
            oc << "O->";

        oc << mit->Name << "(";

        index = 0;
        for (pit = mit->Params.begin(); pit != mit->Params.end(); ++pit) {
            if (index) oc << ", ";
            oc << "p" << index;
            ++index;
        }

        if (mit->type != Void)
            oc << ")";

        oc << ");" << std::endl;


        if (mit->type == Void) {

            if (in_class)
                oc << "\t\te8_var_object(result, O);" << std::endl;
            else
                oc << "\t\te8_var_undefined(result);" << std::endl;

        } else {
            oc << "\t\te8_var_assign(result, &Result.plain());" << std::endl;
        }

        oc << "\t} catch(E8::Exception exc) {" << std::endl;
        oc << "\t\treturn exc.__e_raise();" << std::endl;

        oc << "\t} catch(...) {" << std::endl;
        oc << "\t\tE8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, \"Plugin exception in `" << escape_class_method(ClassName, mit->Name) << "` !\");" << std::endl;
        oc << "\t}" << std::endl;

        oc << "\targv = margv;" << std::endl;
        index = 0;
        for (pit = mit->Params.begin(); pit != mit->Params.end(); ++pit) {
            if (pit->in_out == ParamInfo::OUT) {
                oc  << "\targv[" << index << "].set(&argv[" << index << "], &p"
                    << index << ".plain());"
                    << std::endl
                ;
            }
            ++index;
        }

        oc << "\tE8_RETURN_OK;" << std::endl;
        oc << "}" << std::endl;
    }


}

void scan_params(const vector<string> &data, vector<ParamInfo> &params)
{
    vector<string>::const_iterator pit = data.begin();
    for (; pit != data.end(); ++pit) {

        ParamInfo p(ParamInfo::OUT, true);

        string ps(*pit);

        if (!ps.size())
            continue;

        if (ps[0] == '[') {
            p.expected = false;
            ps = ps.substr(1, ps.size() - 2);
        }

        vector<string> md = delimit(ps, ":");
        if (md.size() > 0) {
            if (md[0] == "IN")
                p.in_out = ParamInfo::IN;
            else if (md[0] == "OUT")
                p.in_out = ParamInfo::OUT;
            else
                throw codegen_exception("Wrong call param type: " + ps);

            params.push_back(p);
        }
    }

}

void go(const string &in_file)
{

    fs::path cwd(fs::current_path());
    fs::path fp (in_file);

    std::cout << "Parse " << in_file << "... ";

    if (!fs::exists(fp)) {
        std::cout << "Not Found!!!" << std::endl;
        return;
    }

    std::cout << "Found" << std::endl;

    fp = fs::absolute(fp);

    fs::current_path(fp.parent_path());

    ifstream in(fp, std::ios_base::in | std::ios_base::binary);

    if (!in)
        return;

    string line;

    do {
        std::getline(in, line);

        if (!line.size())
            continue;

        if (line[0] == '#')
            continue;

        string::size_type pos;
        string::size_type npos = string::npos;


        pos = line.find(E8_Custom);
        if (pos != npos) {
            line = line.substr(pos + E8_Custom.size() + 1);

            string data;
            do {
                std::getline(in, data);

                pos = data.find(E8_EndCustom);
                if (pos != npos) {
                    data = data.substr(pos + E8_EndCustom.size() + 1);
                    if (data == line)
                        break;
                }

                custom_post << data << std::endl;
            } while (!in.eof());

            continue;
        }

        pos = line.find(E8_Global);
        if (pos != npos) {

            line = line.substr(pos + E8_Global.size() + 1);

            vector<string> data = delimit(line, " ");
            vector<string> names = delimit(data[0], "|");
            GlobalVar var(names[0]);
            if (names.size() > 1) {
                names.erase(names.begin());
                assign(var.Aliases, names);
            }

            vector<string>::const_iterator it  = data.begin();
            for (++it; it != data.end(); ++it) {
                if (*it == "Read")
                    var.Read = "true";
                if (*it == "NoRead")
                    var.Read = "false";
                if (*it == "Write")
                    var.Write = "true";
                if (*it == "NoWrite")
                    var.Write = "false";
            }

            vars.push_back(var);

            continue;
        }

        pos = line.find(E8_Class);
        if (pos != npos) {

            if (in_class || in_enum)
                throw codegen_exception("[E8.Class] Already in class or enum!");

            ClassInfo _class;

            line = line.substr(pos + E8_Class.size() + 1);

            vector<string> data = delimit(line, " ");
            _class.Name = data[0];

            data = delimit(data[1], "|");
            assign(_class.Aliases, data);

            std::cout << "Class: " << _class.Name << std::endl;

            classes[_class.Name] = _class;
            in_class = &classes[_class.Name];

            continue;
        }

        pos = line.find(E8_Enum);
        if (pos != npos) {

            if (in_class || in_enum)
                throw codegen_exception("[E8.Enum] Already in class or enum!");

            EnumInfo _enum;

            line = line.substr(pos + E8_Enum.size() + 1);

            vector<string> data = delimit(line, " ");
            _enum.Name = data[0];

            if (data[2] == "AutoInit" && data.size() > 3)
                _enum.AutoInit = data[3];

            data = delimit(data[1], "|");
            assign(_enum.Aliases, data);

            std::cout << "Enum: " << _enum.Name << std::endl;

            enums[_enum.Name] = _enum;
            in_enum = &enums[_enum.Name];

            continue;
        }

        pos = line.find(E8_Cast);
        if (pos != npos) {

            if (in_enum == NULL)
                throw codegen_exception("[E8.Cast] Not in enum!");

            line = line.substr(pos + E8_Cast.size() + 1);
            in_enum->CastType = line;

            continue;
        }

        pos = line.find(E8_EndEnum);
        if (pos != npos) {

            if (in_enum == NULL)
                throw codegen_exception("[E8.EndEnum] Not in enum!");

            oc  << "static e8_vtable vmt_" << in_enum->Name << ";" << std::endl;
            oc  << "static E8_DECLARE_TO_STRING(to_string_" << in_enum->Name << ")" << std::endl;
            oc  << "{" << std::endl;
            oc  << "\tstatic const E8::string __name(\"" << in_enum->Name << "\");" << std::endl;
            oc  << "\treturn __name.c_str();" << std::endl;
            oc  << "}" << std::endl;

            oc  << "static e8_type_info type_info_" << in_enum->Name
                << " = {&to_string_" << in_enum->Name << ", ";
            oc  << "0"; // Constructor

            oc  << ", 0};" << std::endl;
            oc  << "E8::Variant V" << in_enum->Name << ";" << std::endl;
            oc  << "" << in_enum->Name << " E" << in_enum->Name << ";" << std::endl;
            oc  << "static e8_property PR_" << in_enum->Name << ";" << std::endl;

            if (in_enum->CastType.size() != 0) {
                oc  << "E8::Variant " << in_enum->Name << "::Cast(const " << in_enum->CastType << " index)" << std::endl;
                oc  << "{return E8::Variant(&E" << in_enum->Name << ", index); }" << std::endl;
            }

            if (in_enum->Elements.size()) {
                vector<EnumElement>::const_iterator pit;
                int index = 0;
                for (pit = in_enum->Elements.begin(); pit != in_enum->Elements.end(); ++pit) {
                    ++index;
                    oc  << "static const int P_"
                        << escape_enum_element(in_enum->Name, pit->Name) << " = " << index << ";" << std::endl
                    ;
                    oc  << "E8::Variant " << in_enum->Name << "::" << pit->Name << "(&E" << in_enum->Name;

                    if (in_enum->AutoInit.size() != 0)
                        oc  << ", " << in_enum->AutoInit << pit->Name;
                    else
                        oc  << ", P_" << escape_enum_element(in_enum->Name, pit->Name);
                    oc  << ");" << std::endl;
                }

                oc  << "static e8_property_element " << in_enum->Name << "_Properties[] = {" << std::endl;
                for (pit = in_enum->Elements.begin(); pit != in_enum->Elements.end(); ++pit) {
                    vector<string>::const_iterator ait;
                    for (ait = pit->Aliases.begin(); ait != pit->Aliases.end(); ++ait) {
                        oc  << "\t{\"" << pit->Name << "\", \"" << *ait << "\", {}, {}, P_" << escape_enum_element(in_enum->Name, pit->Name) << "}," << std::endl;
                    }
                }

                oc  << "\t{\"\", \"\", {}, {}, 0}};" << std::endl;

                for (pit = in_enum->Elements.begin(); pit != in_enum->Elements.end(); ++pit) {
                    oc  << "static E8_DECLARE_PROPERTY_GET(" << escape_enum_element(in_enum->Name, pit->Name) << "__get)" << std::endl;
                    oc  << "{" << std::endl;
                    oc  << "\t" << "try {" << std::endl;

                    oc  << "\t\tE8::Variant R(" << in_enum->Name<< "::" << pit->Name << ");" << std::endl;
                    oc  << "\t\te8_var_assign(value, &R.plain());" << std::endl;

                    oc  << "\t} catch (...) {" << std::endl;
                    oc  << "\t\tE8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, \"Plugin exception in `" << escape_enum_element(in_enum->Name, pit->Name) << "`!\");" << std::endl;
                    oc  << "\t}" << std::endl;
                    oc  << "\tE8_RETURN_OK;" << std::endl;
                    oc  << "}" << std::endl;
                }

                oc  << "static E8_DECLARE_GET_PROPERTY(" << in_enum->Name << "_GetProperty)" << std::endl;
                oc  << "{" << std::endl;
                oc  << "\tint index = e8_find_simple_property(name, " << in_enum->Name << "_Properties);" << std::endl;
                oc  << "\tswitch (index) {" << std::endl;

                for (pit = in_enum->Elements.begin(); pit != in_enum->Elements.end(); ++pit) {
                    oc  << "\t\tcase P_" << escape_enum_element(in_enum->Name, pit->Name) << ": { " << std::endl;

                    oc  << "\t\t\tproperty->get = " << escape_enum_element(in_enum->Name, pit->Name) << "__get;" << std::endl;
                    oc  << "\t\t\tproperty->can_get = true;" << std::endl;

                    oc  << "\t\t\tproperty->can_set = false;" << std::endl;
                    oc  << "\t\t\tbreak;" << std::endl;
                    oc  << "\t\t}" << std::endl;
                }

                oc  << "\t\tdefault: E8_THROW_DETAIL(E8_RT_PROPERTY_NOT_FOUND, e8_utf_strdup(name)); break;" << std::endl;
                oc  << "\t}" << std::endl;
                oc  << "\tproperty->data = obj;" << std::endl;
                oc  << "\tE8_RETURN_OK;" << std::endl;
                oc  << "}" << std::endl;
            }

            in_enum = NULL;
            continue;
        }

        pos = line.find(E8_Element);
        if (pos != npos) {

            line = line.substr(pos + E8_Element.size() + 1);

            vector<string> data = delimit(line, " ");
            vector<string> names = delimit(data[0], "|");

            EnumElement E;
            E.Name = names[0];
            E.Aliases.assign(names.begin() + 1, names.end());

            in_enum->Elements.push_back(E);

            continue;
        }



        pos = line.find(E8_Include);
        if (pos != npos) {
            line = line.substr(pos + E8_Include.size() + 1);
            oc << "#include \"" << line << "\"" << std::endl;

            continue;
        }


        pos = line.find(E8_Inherit);
        if (pos != npos) {

            if (!in_class)
                throw codegen_exception("Not in class!");

            line = line.substr(pos + E8_Include.size() + 1);
            if (classes.find(line) != classes.end()) {
                in_class->inherit(classes[line]);
            } else
                std::cout << "Warning: Class not found " << line << std::endl;

            continue;
        }

        pos = line.find(E8_EndClass);
        if (pos != npos) {

            out_methods(in_class, in_class->Methods);

            oc  << "static E8_METHOD_LIST_START(methods_"
                << in_class->Name << ")" << std::endl;

            vector<MethodInfo>::const_iterator mit;
            for (mit = in_class->Methods.begin(); mit != in_class->Methods.end(); ++mit) {

                if (mit->Aliases.size()) {

                    vector<string>::const_iterator cit = mit->Aliases.begin();
                    for (; cit != mit->Aliases.end(); ++cit) {
                        oc  << "\tE8_METHOD_LIST_ADD(\"" << mit->Name
                            << "\", \"" << *cit << "\", "
                            << escape_class_method(in_class->Name, mit->Name)
                            << ")"
                            << std::endl
                        ;
                    }

                } else {
                    oc  << "\tE8_METHOD_LIST_ADD(\"" << mit->Name
                        << "\", \"" << mit->Name << "\", "
                        << escape_class_method(in_class->Name, mit->Name)
                        << ")"
                        << std::endl;
                }
            }
            oc  << "E8_METHOD_LIST_END" << std::endl;

            oc  << "static E8_DECLARE_GET_METHOD(" << in_class->Name << "_GetMethod)" << std::endl;
            oc  << "{" << std::endl;
            oc  << "\te8_simple_get_method(name, methods_" << in_class->Name << ", fn);" << std::endl;
            oc  << "\tE8_RETURN_OK;" << std::endl;
            oc  << "}" << std::endl;

            oc  << "static E8_DECLARE_DESTRUCTOR(" << in_class->Name << "__Destroy)" << std::endl;
            oc  << "{" << std::endl;
            oc  << "\t" << in_class->Name << " *O = (" << in_class->Name << "*)obj;" << std::endl;
            oc  << "\tdelete O;" << std::endl;
            oc  << "}" << std::endl << std::endl;

            oc  << "static e8_vtable vmt_" << in_class->Name << ";" << std::endl;
            if (in_class->HasConstructor) {
                oc << "static E8_DECLARE_SUB(WR_Constructor_" << in_class->Name << ")" << std::endl;
                oc << "{" << std::endl;

                vector<ParamInfo>::const_iterator pit;
                int index = 0;
                for (pit = in_class->ConstructorParams.begin(); pit != in_class->ConstructorParams.end(); ++pit) {
                    oc << "\tE8::Variant p" << index << "; " << std::endl;
                    oc << "\t{" << std::endl << "\t\te8_var v; e8_var_undefined(&v);" << std::endl;
                    oc << "\t\tif (argc) { argv->get(argv, &v); --argc; }" << std::endl;

                    if (pit->expected) {
                        oc << "\t\telse E8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, "
                                "\"Wrong param " << (index + 1) << "\");" << std::endl;
                    }
                    oc << "\t\tp" << index << " = E8::Variant(v);" << std::endl << "\t}" << std::endl;
                    ++index;
                }

                oc << "\ttry {" << std::endl;
                oc << "\t\t" << in_class->Name << " *Result = ";

                oc << in_class->Name << "::Constructor" << "(";

                index = 0;
                for (pit = in_class->ConstructorParams.begin(); pit != in_class->ConstructorParams.end(); ++pit) {
                    if (index) oc << ", ";
                    oc << "p" << index;
                    ++index;
                }

                oc << ");" << std::endl;

                oc << "\t\te8_gc_register_object(Result, &vmt_" << in_class->Name << ");" << std::endl;
                oc << "\t\te8_var_object(result, Result);" << std::endl;

                oc << "\t} catch(...) {" << std::endl;
                oc << "\t\tE8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, \"Plugin exception in constructor of `" << in_class->Name << "`!\");" << std::endl;
                oc << "\t}" << std::endl;

                oc << "\tE8_RETURN_OK;" << std::endl;
                oc << "}" << std::endl;
            }

            oc  << "static E8_DECLARE_TO_STRING(to_string_" << in_class->Name << ")" << std::endl;
            oc  << "{" << std::endl;
            oc  << "\tstatic const E8::string __name(\"" << in_class->Name << "\");" << std::endl;
            oc  << "\treturn __name.c_str();" << std::endl;
            oc  << "}" << std::endl;

            oc  << "static e8_type_info type_info_" << in_class->Name
                << " = {&to_string_" << in_class->Name << ", ";
            if (in_class->HasConstructor)
                oc  << "&WR_Constructor_" << in_class->Name;
            else
                oc  << "0";

            oc  << ", 0};" << std::endl;

            if (in_class->Properties.size()) {
                vector<PropertyInfo>::const_iterator pit;
                int index = 0;
                for (pit = in_class->Properties.begin(); pit != in_class->Properties.end(); ++pit) {
                    ++index;
                    oc  << "static const int "
                        << escape_class_property(in_class->Name, pit->Name) << " = " << index << ";" << std::endl;
                }

                oc  << "static e8_property_element " << in_class->Name << "_Properties[] = {" << std::endl;
                for (pit = in_class->Properties.begin(); pit != in_class->Properties.end(); ++pit) {
                    vector<string>::const_iterator ait;
                    for (ait = pit->Aliases.begin(); ait != pit->Aliases.end(); ++ait) {
                        oc  << "\t{\"" << pit->Name << "\", \"" << *ait << "\", {}, {}, " << escape_class_property(in_class->Name, pit->Name) << "}," << std::endl;
                    }
                }

                oc  << "\t{\"\", \"\", {}, {}, 0}};" << std::endl;

                for (pit = in_class->Properties.begin(); pit != in_class->Properties.end(); ++pit) {
                    if (pit->Getter.size()) {
                        oc  << "static E8_DECLARE_PROPERTY_GET(" << escape_property_getter(in_class->Name, pit->Name) << ")" << std::endl;
                        oc  << "{" << std::endl;
                        oc  << "\t" << in_class->Name << " *O = (" << in_class->Name << "*)((e8_property *)property)->data;" << std::endl;
                        oc  << "\t" << "try {" << std::endl;

                        oc  << "\t\tE8::Variant R = O->" << pit->Getter << "();" << std::endl;
                        oc  << "\t\te8_var_assign(value, &R.plain());" << std::endl;

                        oc  << "\t} catch (...) {" << std::endl;
                        oc  << "\t\tE8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, \"Plugin exception in `" << escape_property_getter(in_class->Name, pit->Name) << "`!\");" << std::endl;
                        oc  << "\t}" << std::endl;
                        oc  << "\tE8_RETURN_OK;" << std::endl;
                        oc  << "}" << std::endl;
                    }

                    if (pit->Setter.size()) {
                        oc  << "static E8_DECLARE_PROPERTY_SET(" << escape_property_setter(in_class->Name, pit->Name) << ")" << std::endl;
                        oc  << "{" << std::endl;
                        oc  << "\t" << in_class->Name << " *O = (" << in_class->Name << "*)((e8_property *)property)->data;" << std::endl;
                        oc  << "\t" << "try {" << std::endl;

                        oc  << "\t\tE8::Variant R(*value);" << std::endl;
                        oc  << "\t\tO->" << pit->Setter << "(R);" << std::endl;

                        oc  << "\t} catch (...) {" << std::endl;
                        oc  << "\t\tE8_THROW_DETAIL(E8_RT_EXCEPTION_RAISED, \"Plugin exception in `" << escape_property_setter(in_class->Name, pit->Name) << "`!\");" << std::endl;
                        oc  << "\t}" << std::endl;
                        oc  << "\tE8_RETURN_OK;" << std::endl;
                        oc  << "}" << std::endl;
                    }
                }

                oc  << "static E8_DECLARE_GET_PROPERTY(" << in_class->Name << "_GetProperty)" << std::endl;
                oc  << "{" << std::endl;
                oc  << "\tint index = e8_find_simple_property(name, " << in_class->Name << "_Properties);" << std::endl;
                oc  << "\tswitch (index) {" << std::endl;

                for (pit = in_class->Properties.begin(); pit != in_class->Properties.end(); ++pit) {
                    oc  << "\t\tcase " << escape_class_property(in_class->Name, pit->Name) << ": { " << std::endl;

                    if (pit->Getter.size()) {
                        oc  << "\t\t\tproperty->get = " << escape_property_getter(in_class->Name, pit->Name) << ";" << std::endl;
                        oc  << "\t\t\tproperty->can_get = true;" << std::endl;
                    } else
                        oc  << "\t\t\tproperty->can_get = false;" << std::endl;

                    if (pit->Setter.size()) {
                        oc  << "\t\t\tproperty->set = " << escape_property_setter(in_class->Name, pit->Name) << ";" << std::endl;
                        oc  << "\t\t\tproperty->can_set = true;" << std::endl;
                    } else
                        oc  << "\t\t\tproperty->can_set = false;" << std::endl;
                    oc  << "\t\t\tbreak;" << std::endl;
                    oc  << "\t\t}" << std::endl;
                }

                oc  << "\t\tdefault: E8_THROW_DETAIL(E8_RT_PROPERTY_NOT_FOUND, e8_utf_strdup(name)); break;" << std::endl;
                oc  << "\t}" << std::endl;
                oc  << "\tproperty->data = obj;" << std::endl;
                oc  << "\tE8_RETURN_OK;" << std::endl;
                oc  << "}" << std::endl;
            }

            in_class = NULL;

            continue;
        }


        pos = line.find(E8_Identifier);
        if (pos != npos) {

            line = line.substr(pos + E8_Identifier.size() + 1);

            string protector("E8_PLUGIN_");
            protector += line;
            protector += "_H_INCLUDED";

            oh << "/* Autogenerated file! */" << std::endl;
            oh << "#ifndef " << protector << std::endl;
            oh << "#define " << protector << std::endl << std::endl;

            oh << "#include \"e8core/plugin/plugin.hpp\"" << std::endl;

            continue;
        }

        pos = line.find(E8_Attach);
        if (pos != npos) {

            line = line.substr(pos + E8_Attach.size() + 1);
            go(line);

            continue;
        }

        pos = line.find(E8_Main);
        if (pos != npos) {

            oc << "E8::Environment E8::Env;" << std::endl;
            oc << "extern \"C\" { " << std::endl;
            oc << "E8_PLUGIN_MAIN" << std::endl
                << "{" << std::endl
                << "\tE8::Env.assign(env);" << std::endl;
            ;

            map<string, ClassInfo>::const_iterator clit;
            for (clit = classes.begin(); clit != classes.end(); ++clit) {
                const ClassInfo &ci = clit->second;
                oc << "\te8_prepare_call_elements(methods_" << ci.Name << ", \"utf-8\");" << std::endl;
                oc << "\tE8::Env.VMT[\"" << ci.Name << "\"] = &vmt_" << ci.Name << ";" << std::endl;
                oc << "\te8_vtable_template(&vmt_" << ci.Name << ");" << std::endl;
                oc << "\tvmt_" << ci.Name << ".get_method = " << ci.Name << "_GetMethod;" << std::endl;
                oc << "\tvmt_" << ci.Name << ".dtor = " << ci.Name << "__Destroy;" << std::endl;
                oc << "\tvmt_" << ci.Name << ".to_string = to_string_" << ci.Name << ";" << std::endl;
                if (ci.Properties.size())
                    oc << "\tvmt_" << ci.Name << ".get_property = " << ci.Name << "_GetProperty;" << std::endl;

                if (ci.Properties.size())
                    oc << "\te8_prepare_property_elements(" << ci.Name << "_Properties, \"utf-8\");" << std::endl;

                vector<string>::const_iterator ait = ci.Aliases.begin();
                for (; ait != ci.Aliases.end(); ++ait) {
                    oc << "\tenv->vmt->register_type(env, \"" << *ait << "\", &type_info_" << ci.Name << ");" << std::endl;
                }
            }
            vector<MethodInfo>::const_iterator mit;
            for (mit = all_methods.begin(); mit != all_methods.end(); ++mit) {
                vector<string>::const_iterator ait = mit->Aliases.begin();
                for (; ait != mit->Aliases.end(); ++ait) {
                    oc << "\tenv->vmt->register_function(env, \"" << *ait << "\", &"
                        << escape_class_method("", mit->Name) << ");" << std::endl;
                }
            }

            map<string, EnumInfo>::const_iterator eit;
            for (eit = enums.begin(); eit != enums.end(); ++eit) {
                const EnumInfo &ei = eit->second;
                oc << "\te8_vtable_template(&vmt_" << ei.Name << ");" << std::endl;
                if (ei.Elements.size()) {
                    oc << "\tvmt_" << ei.Name << ".get_property = " << ei.Name << "_GetProperty;" << std::endl;
                    oc << "\te8_prepare_property_elements(" << ei.Name << "_Properties, \"utf-8\");" << std::endl;
                    /*
                    vector<EnumElement>::const_iterator ait;
                    for (ait = ei.Elements.begin(); ait != ei.Elements.end(); ++ait) {
                        oc << "\t" << ei.Name << "::" << ait->Name
                           << " = E8::Variant(&E" << ei.Name << ", P_" << escape_enum_element(ei.Name, ait->Name)
                           << ");" << std::endl;
                    }
                    */
                }

                oc << "\te8_gc_register_object(&E" << ei.Name << ", &vmt_" << ei.Name << ");" << std::endl;
                oc << "\t{e8_var __v; e8_var_object(&__v, &E" << ei.Name << "); V" << ei.Name << ".assign(__v); }"
                   << std::endl;
                oc << "\te8_create_variant_property(&PR_" << ei.Name << ", &V" << ei.Name << ".plain());"
                   << std::endl
                ;

                vector<string>::const_iterator ait = ei.Aliases.begin();
                for (; ait != ei.Aliases.end(); ++ait) {
                    oc << "\tenv->vmt->register_type(env, \"" << *ait << "\", &type_info_" << ei.Name << ");" << std::endl;
                    oc << "\tenv->vmt->register_property(env, \"" << *ait << "\", &PR_" << ei.Name << ");" << std::endl;
                }
            }

            vector<GlobalVar>::const_iterator vit;
            for (vit = vars.begin(); vit != vars.end(); ++vit) {
                oc  << "\tE8::Env.RegisterGlobalValue(\"" << vit->Name << "\", "
                    << vit->Name << ", " << vit->Read << ", " << vit->Write << ");"
                    << std::endl;

                vector<string>::const_iterator ait = vit->Aliases.begin();
                for (; ait != vit->Aliases.end(); ++ait) {
                    oc  << "\tE8::Env.RegisterGlobalValue(\"" << *ait << "\", "
                        << vit->Name << ", " << vit->Read << ", " << vit->Write << ");"
                        << std::endl;
                }
            }

            oc  << custom_post.str();

            oc  << "\treturn 0;" << std::endl
                << "}" << std::endl;
            oc  << "} // extern C " << std::endl;

            continue;
        }

        pos = line.find(E8_Constructor);
        if (pos != npos) {

            if (!in_class)
                throw codegen_exception("[E8.Constructor] Not in class!");

            string::size_type lpos = pos + E8_Constructor.size() + 1;
            if (lpos < line.size()) {
                line = line.substr(lpos);
                vector<string> data = delimit(line, " ");

                scan_params(data, in_class->ConstructorParams);
            }
            in_class->HasConstructor = true;
        }

        pos = line.find(E8_Method);
        if (pos != npos) {

            line = line.substr(pos + E8_Method.size() + 1);

            vector<string> data = delimit(line, " ");
            vector<string> names = delimit(data[0], "|");

            MethodInfo M;
            M.Name = names[0];

            M.Aliases.assign(names.begin() + 1, names.end());

            M.type = types[data[1]];

            data.erase(data.begin());
            data.erase(data.begin());
            scan_params(data, M.Params);

            if (in_class)
                in_class->Methods.push_back(M);
            else {
                methods.push_back(M);
                all_methods.push_back(M);
            }

            continue;
        }

        pos = line.find(E8_Property);
        if (pos != npos) {

            line = line.substr(pos + E8_Property.size() + 1);

            vector<string> data = delimit(line, " ");
            vector<string> names = delimit(data[0], "|");

            PropertyInfo P;
            P.Name = names[0];

            P.Aliases.assign(names.begin() + 1, names.end());

            vector<string>::const_iterator it = data.begin() + 1;
            for (; it != data.end(); ++it) {

                string::size_type pos;

                pos = it->find("get:");
                if (pos != npos) {
                    P.Getter = it->substr(4);
                    continue;
                }

                pos = it->find("set:");
                if (pos != npos) {
                    P.Setter = it->substr(4);
                    continue;
                }

            }

            in_class->Properties.push_back(P);

            continue;
        }

    } while (!in.eof());

    in.close();

    out_methods(NULL, methods);
    methods.clear();

    fs::current_path(cwd);
}

void usage()
{
    std::cout
        << "usage: "
        << "codegen --in <input-file> --header <output header> --cxx <output cxx>"
    ;
}

int main(int argc, char **argv)
{
    int i = 0;
    int flag = 0;

    const int FLAG_HAS_IN = 1;
    const int FLAG_HAS_H = 2;
    const int FLAG_HAS_CXX = 4;
    const int FLAG_HAS_ALL = 7;

    while (i < argc) {
        if (strcmp(argv[i], "--in") == 0) {
            ++i;
            in_file = argv[i];
            flag |= FLAG_HAS_IN;
        }
        if (strcmp(argv[i], "--header") == 0) {
            ++i;
            out_h = argv[i];
            flag |= FLAG_HAS_H;
        }
        if (strcmp(argv[i], "--cxx") == 0) {
            ++i;
            out_cxx = argv[i];
            flag |= FLAG_HAS_CXX;
        }
        ++i;
    }

    if (flag != FLAG_HAS_ALL) {
        usage();
        return 0;
    }

    types["void"] = Void;
    types["bool"] = Bool;
    types["object"] = Object;
    types["int"] = Int;
    types["double"] = Double;
    types["string"] = String;
    types["variant"] = Variant;


    oh.open(out_h.c_str(), std::ios_base::out | std::ios_base::binary);
    oc.open(out_cxx.c_str(), std::ios_base::out | std::ios_base::binary);

    oc << "/* Autogenerated file! */" << std::endl;
    oc << "#include \"" << out_h << "\"" << std::endl;

    int result = 0;

    try {
        go(in_file);
    } catch (codegen_exception x) {
        std::cout << "Error: ";
        x.tell();
        result = 1;
    }

    oh << std::endl << "#endif" << std::endl;

    oh.close();
    oc.close();

    return result;
}
