#ifndef E8_LIB_EPFMODULE_H
#define E8_LIB_EPFMODULE_H

#include "e8core/plugin/plugin.hpp"

/*E8.Include Epf.hpp
*/

/**E8.Class EpfModule EpfModule|МодульEpf|ВнешняяОбработка1С
 */
class EpfModule
{
public:
    EpfModule();
    virtual ~EpfModule();

protected:
private:
};
/*E8.EndClass*/

/**E8.Class Epf Epf|ВнешниеОбработки1С
 */
class Epf {
public:

    Epf();
    ~Epf();

    /**E8.Method AttachUnit|ПодключитьМодуль variant IN:ИмяФайла
     */
    E8::Variant AttachUnit(E8_IN FileName);

};
/*E8.EndClass*/

/**E8.Global g_Epf|Epf|ВнешниеОбработки1С Read NoWrite
 */
extern E8::Variant g_Epf;
/*
E8.Custom InitEpf
g_Epf = E8::Variant::Object(new Epf(), E8::Env.VMT["Epf"]);
E8.EndCustom InitEpf
*/


#endif // E8_LIB_EPFMODULE_H
