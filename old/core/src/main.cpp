#include <entt/entt.hpp>
#include <iostream>

using namespace std;

struct position {
    float x;
    float y;
};

struct test
{
    test()
    {
        cout << "constructed" << endl;
    }
    test(const test&)
    {
        cout << "copy constructed" << endl;
    }
    test(test&&)
    {
        cout << "move constructed" << endl;
    }

    test& operator=(const test&)
    {
        cout << "copy assigned" << endl;
        return *this;
    }
    test& operator=(test&&)
    {
        cout << "move assigned" << endl;
        return *this;
    }

    ~test()
    {
        cout << "destructed" << endl;
    }
};

struct velocity {
    test dx;
    float dy;
};

void update(entt::registry &registry) {
    auto view = registry.view<const position, velocity>();


    // use a callback
    view.each([](const auto &pos, auto &vel) { /* ... */ });

    // use an extended callback
    view.each([](const auto entity, const auto &pos, auto &vel) { /* ... */ });

    // use a range-for
    for(auto [entity, pos, vel]: view.each()) {
        std::cout << pos.x << std::endl;
    }
    std::cout << std::endl;
    // use forward iterators and get only the components of interest
    for(auto entity: view) {
        auto &pos = view.get<position>(entity);
        std::cout << pos.x << std::endl;
    }
}

#include "core/type_manager.hpp"

namespace tt::tt
{
template<typename T>
auto testa()
{
    std::string_view pretty_function{ENTT_PRETTY_FUNCTION};
    auto first = pretty_function.find_first_not_of(' ', pretty_function.find_first_of(ENTT_PRETTY_FUNCTION_PREFIX) + 1);
    auto value = pretty_function.substr(first, pretty_function.find_last_of(ENTT_PRETTY_FUNCTION_SUFFIX) - first);
    return value;
}
}

template<typename A, typename B> struct C{};

void onCreate()
{
    cout << "Created" << endl;
}

void onUpdate()
{
    cout << "Updated" << endl;
}

void onDestroy()
{
    cout << "Destroyed" << endl;
}

int main() {
    entt::registry registry;

    registry.on_construct<position>().connect<&onCreate>();
    registry.on_update<position>().connect<&onUpdate>();
    registry.on_destroy<position>().connect<&onDestroy>();

    for(auto i = 0u; i < 2u; ++i) {
        const auto entity = registry.create();
        registry.emplace<position>(entity, i * 1.f, i * 1.f);
        if(i % 2 == 0) { registry.emplace<velocity>(entity, test{}, i * .1f); }
        registry.patch<position>(entity);
    }

    update(registry);

    auto tm = type_manager();
    auto tnr = type_name_resolver();

    cout << tnr.resolve_name<int>() << endl;
    cout << tnr.resolve_name<C<string, string_view>>() << endl;

    return 0;
}
