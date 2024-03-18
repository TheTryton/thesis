#pragma once

#include <memory>

template<typename Elem, typename Alloc = std::allocator<Elem>>
class dynamic_array
{
public:
    using value_type = Elem;
    using allocator_type = Alloc;
    using size_type = std::allocator_traits<Alloc>::size_type;
    using difference_type = std::allocator_traits<Alloc>::difference_type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = std::allocator_traits<Alloc>::pointer;
    using const_pointer = std::allocator_traits<Alloc>::const_pointer;

    using iterator = void; // TODO
    using const_iterator = void; // TODO
    using reverse_iterator = void; // TODO
    using const_reverse_iterator = void; // TODO
private:
    std::unique_ptr<Elem[]> _dummy;
};