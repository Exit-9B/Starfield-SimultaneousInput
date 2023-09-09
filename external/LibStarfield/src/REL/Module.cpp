#include "REL/Module.h"

#define NOMINMAX
#include <Windows.h>

#include <algorithm>
#include <cstring>
#include <iterator>
#include <type_traits>

namespace REL
{
    void Module::load_segments()
    {
        auto dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(_base);
        auto ntHeader  = stl::adjust_pointer<IMAGE_NT_HEADERS64>(
            dosHeader,
            dosHeader->e_lfanew);
        const auto* sections = IMAGE_FIRST_SECTION(ntHeader);
        const auto  size     = std::min<std::size_t>(
            ntHeader->FileHeader.NumberOfSections,
            _segments.size());
        for (std::size_t i = 0; i < size; ++i) {
            const auto& section = sections[i];
            const auto  it      = std::find_if(
                SEGMENTS.begin(),
                SEGMENTS.end(),
                [&](auto&& a_elem)
                {
                    constexpr auto size = std::extent_v<decltype(section.Name)>;
                    const auto     len  = std::min(a_elem.first.size(), size);
                    return std::memcmp(a_elem.first.data(), section.Name, len) == 0 &&
                        (section.Characteristics & a_elem.second) == a_elem.second;
                });
            if (it != SEGMENTS.end()) {
                const auto idx = static_cast<std::size_t>(
                    std::distance(SEGMENTS.begin(), it));
                _segments[idx] = Segment{
                    _base,
                    _base + section.VirtualAddress,
                    section.Misc.VirtualSize
                };
            }
        }
    }
}
