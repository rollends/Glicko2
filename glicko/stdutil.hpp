#ifndef GLICKO_STD_UTIL_HPP
#define GLICKO_STD_UTIL_HPP

#include <iterator>
#include <type_traits>

namespace Glicko
{
    namespace type_util
    {
        template< typename Type >
        using is_forward_iterable =
            typename std::is_base_of<
                std::forward_iterator_tag,
                typename std::iterator_traits< typename Type:: iterator >:: iterator_category
            >;
    }
}

#endif
