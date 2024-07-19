#pragma once

#include <cstdint>
#include <bit>
#include <string_view>
#include <array>
#include <tuple>

class type_index
{
    friend class type_manager;

    std::uintptr_t _index;

    constexpr explicit type_index(std::uintptr_t index)
        : _index(index)
    {}
};

class type_manager
{
public:
    template<typename T>
    [[nodiscard]] constexpr type_index type_id() const
    {
        constexpr static struct {} _marker;
        return type_index{std::bit_cast<std::uintptr_t>(&_marker)};
    }
};

class type_name_resolver
{
private:
public:
    template<typename T>
    constexpr auto resolve_name() const
    {
        using namespace std;

        constexpr string_view functionName = __FUNCSIG__;

        constexpr auto first = functionName.find_first_of('<') + 1;
        constexpr auto last = functionName.find_last_of('>') - 1;
        constexpr auto typeName = functionName.substr(first, last - first + 1);

        constexpr auto clearedTypeNameAndLength = [&](){
            using namespace std;
            constexpr auto typeNameLength = size(typeName);

            array<char, typeNameLength + 1> clearedTypeName{};

            size_t srcI = 0;
            size_t dstI = 0;
            while(srcI < typeNameLength)
            {
                const auto offsetTypeName = typeName.substr(srcI);
                if (offsetTypeName.starts_with("class ") || offsetTypeName.starts_with("struct "))
                {
                    srcI += 6;
                    continue;
                }

                if(typeName[srcI] == ' ')
                {
                    ++srcI;
                    continue;
                }

                clearedTypeName[dstI++] = typeName[srcI++];
            }
            clearedTypeName[dstI] = '\0';

            return std::make_tuple(clearedTypeName, dstI);
        }();
        constexpr static auto formattedTypeName = [&](){
            using namespace std;
            constexpr auto typeNameLength = size(typeName);

            array<char, typeNameLength * 2 + 1> formattedTypeName{};

            size_t srcI = 0;
            size_t dstI = 0;
            while(srcI < get<1>(clearedTypeNameAndLength))
            {
                const auto c = get<0>(clearedTypeNameAndLength)[srcI++];

                formattedTypeName[dstI++] = c;
                if(c == ',')
                    formattedTypeName[dstI++] = ' ';
            }
            formattedTypeName[dstI] = '\0';

            return formattedTypeName;
        }();

        return string_view {formattedTypeName.data()};
    }
};
