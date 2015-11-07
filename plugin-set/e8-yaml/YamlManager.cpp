#include "YamlManager.hpp"
#include <yaml-cpp/yaml.h>

YamlDocument::YamlDocument(const YAML::Node &node)
    : node(node)
{
    //ctor
}

YamlDocument::~YamlDocument()
{
    //dtor
}

YamlDocument *YamlDocument::Constructor(E8_IN Text)
{
    if (Text.of_type(varString)) {

        std::string cText = Text.to_string().to_string();

        return new YamlDocument(YAML::Load(cText));

    } else {

        E8::Variant FullFileName = Text.__get("FullName");
        std::string cText = FullFileName.to_string().to_string();

        return new YamlDocument(YAML::LoadFile(cText));

    }
}

bool YamlDocument::IsScalar() const
{
    return node.IsScalar();
}

bool YamlDocument::IsArray() const
{
    return node.IsSequence();
}

bool YamlDocument::IsMap() const
{
    return node.IsMap();
}

bool YamlDocument::IsNull() const
{
    return node.IsNull();
}

bool YamlDocument::IsDefined() const
{
    return node.IsDefined();
}

int YamlDocument::Count() const
{
    return node.size();
}

bool is_double(const YAML::Node &node)
{
    if (
        (node.as<double>(1.0) == 1.0)
        && (node.as<double>(2.0) == 2.0)
        )
        return false;

    return true;
}

bool is_integer(const YAML::Node &node)
{
    if (
        (node.as<long>(1l) == 1l)
        && (node.as<long>(2l) == 2l)
        )
        return false;

    return true;
}

E8::Variant YamlDocument::Get(E8_IN Index)
{

    YAML::Node element;

    if (Index.of_type(varString)) {

        std::string s = Index.to_string().to_string();
        element = node[s];

    } else {

        long l_index = Index.plain().num.l_value;

        if (l_index < 0 || (unsigned long)l_index >= node.size())
            throw E8::Exception(E8_RT_INDEX_OUT_OF_BOUNDS, E8::string("Index out of bounds!"));

        element = node[l_index];

    }

    if (element.IsSequence() || element.IsMap())
        return E8::Variant::Object( new YamlDocument(element), E8::Env.VMT["YamlDocument"] );

    if (element.IsNull())
        return E8::Variant::Null();


    if (!element.IsDefined())
        return E8::Undefined();

    if (element.IsScalar()) {

        if (is_integer(element))
            return E8::Variant(element.as<long>());

        if (is_double(element))
            return E8::Variant(element.as<double>());

        return E8::Variant(E8::string(element.as<std::string>().c_str()));

    }

    /* Тип значения не поддерживается */
    throw E8::Exception(E8::string("Yaml-type not supported!"));
}

YamlNode::YamlNode(const YAML::Node &node)
    : node(node)
{

}
YamlNode::~YamlNode()
{

}

static
E8::Variant
__scan_node(const YAML::Node &node)
{
    if (!node.IsDefined())
        return E8::Variant();

    if (node.IsNull())
        return E8::Variant::Null();

    if (node.IsSequence()) {

        E8::Variant R = E8::NewObject(E8::string("Array"));

        YAML::Node::const_iterator it = node.begin();
        for (; it != node.end(); ++it) {
            E8::Variant Value = __scan_node(*it);
            R.__Call("Add", Value);
        }

        return R;
    }

    if (node.IsMap()) {
        E8::Variant R = E8::NewObject(E8::string("Map"));

        YAML::Node::const_iterator it = node.begin();
        for (; it != node.end(); ++it) {
            E8::Variant First = __scan_node(it->first), Second = __scan_node(it->second);
            R.__Call("Insert", First, Second);
        }

        return R;
    }

    if (node.IsScalar()) {

        if (is_integer(node))
            return E8::Variant(node.as<long>());

        if (is_double(node))
            return E8::Variant(node.as<double>());

        return E8::Variant(E8::string(node.as<std::string>().c_str()));

    }

    throw E8::Exception(E8::string("Yaml-type not supported!"));
}

E8::Variant ReadYaml(E8_IN Text)
{
    YAML::Node doc;
    if (Text.of_type(varString)) {

        std::string cText = Text.to_string().to_string();

        doc = YAML::Load(cText);

    } else {

        E8::Variant FullFileName = Text.__get("FullName");
        std::string cText = FullFileName.to_string().to_string();

        doc = YAML::LoadFile(cText);

    }

    return __scan_node(doc);
}
