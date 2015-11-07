#ifndef COMMON_HPP_INCLUDED
#define COMMON_HPP_INCLUDED

#include <e8core/plugin/plugin.hpp>

/*E8.Include common.hpp
*/

/**E8.Method IsDate|ЭтоДата bool IN:Значение
 */
bool IsDate(E8_IN Value);

/**E8.Method IsString|ЭтоСтрока bool IN:Значение
 */
bool IsString(E8_IN Value);

/**E8.Method IsBool|ЭтоБулевоЗначение bool IN:Значение
 */
bool IsBool(E8_IN Value);

/**E8.Method IsNumber|ЭтоЧисло bool IN:Значение
 */
bool IsNumber(E8_IN Value);

/**E8.Method IsScalar|ЭтоЗначение bool IN:Значение
 */
bool IsScalar(E8_IN Value);

/**E8.Method IsObject|ЭтоОбъект bool IN:Значение
 */
bool IsObject(E8_IN Value);

#endif // COMMON_HPP_INCLUDED
