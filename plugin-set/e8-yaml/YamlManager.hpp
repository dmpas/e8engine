#ifndef YAMLMANAGER_H
#define YAMLMANAGER_H

#include <e8core/plugin/plugin.hpp>
#include <yaml-cpp/yaml.h>

/*E8.Include YamlManager.hpp
 */

/**E8.Class YamlDocument YamlDocument|ДокументЯмль|ДокументYaml
 */
class YamlDocument
{
public:
    YamlDocument(const YAML::Node &node);
    ~YamlDocument();


    /**E8.Constructor IN:Данные
     */
    static YamlDocument *Constructor(E8_IN Text);


    /**E8.Method IsScalar|ЭтоЗначение bool
     */
    bool IsScalar() const;

    /**E8.Method IsArray|ЭтоМассив bool
     */
    bool IsArray() const;

    /**E8.Method IsMap|ЭтоСоответствие bool
     */
    bool IsMap() const;

    /**E8.Method IsNull|ЭтоNull bool
     */
    bool IsNull() const;

    /**E8.Method IsDefined|ЗначениеОпределено bool
     */
    bool IsDefined() const;

    /**E8.Method Count|Количество int
     */
    int Count() const;

    /**E8.Method Get|Получить variant IN:НомерЭлемента
     */
    E8::Variant Get(E8_IN Index);

protected:
private:
    YAML::Node node;
};
/*E8.EndClass*/

/**E8.Class YamlNode YamlNode|УзелYaml|УзелЯмль
 */
class YamlNode {
public:
    YamlNode(const YAML::Node &node);
    ~YamlNode();
private:
    YAML::Node node;
};

/*E8.EndClass*/

/**E8.Method ReadYaml|ПрочитатьYaml|ПрочитатьЯмль variant IN:Данные
 */
E8::Variant ReadYaml(E8_IN Text);

#endif // YAMLMANAGER_H
