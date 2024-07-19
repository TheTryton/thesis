#pragma once

#include <type_traits>
#include <concepts>
#include <atomic>
#include <variant>
#include <tuple>

template<typename ControlBlockType>
concept scoped_lock_control_block = requires(ControlBlockType controlBlock)
{
    { controlBlock.acquire() } -> std::same_as<bool>;
    { controlBlock.release() } -> std::same_as<std::tuple<bool, bool>>;
    { controlBlock.acquireWeak() } -> std::same_as<void>;
    { controlBlock.releaseWeak() } -> std::same_as<bool>;
    { controlBlock.useCount() } -> std::same_as<size_t>;
} && std::is_default_constructible_v<ControlBlockType>;

class synchronized_scoped_ptr_control_block
{
private:
    std::atomic<size_t> _countStrong{1};
    std::atomic<size_t> _countWeak{0};
public:
    inline bool acquire() noexcept
    {
        size_t countStrong = _countStrong.load(std::memory_order_relaxed);

        while (countStrong != 0 && !_countStrong.compare_exchange_weak(
            countStrong,
            countStrong + 1,
            std::memory_order_relaxed,
            std::memory_order_relaxed
            )
        );

        return countStrong != 0;
    }
    inline std::tuple<bool, bool> release() noexcept
    {
        size_t countStrongBeforeRelease = _countStrong.fetch_sub(1, std::memory_order_relaxed);
        size_t countWeak = _countWeak.load(std::memory_order_relaxed);
        return std::make_tuple(countStrongBeforeRelease == 1, countWeak == 0);
    }
    inline void acquireWeak() noexcept
    {
        size_t countStrong = _countStrong.load(std::memory_order_relaxed);
        if(countStrong == 0)
            return;

        _countWeak.fetch_add(1, std::memory_order_relaxed);
    }
    inline bool releaseWeak() noexcept
    {
        size_t countWeakBeforeRelease = _countWeak.fetch_sub(1, std::memory_order_relaxed);
        size_t countStrong = _countStrong.load(std::memory_order_relaxed);
        return countStrong == 0 && countWeakBeforeRelease == 1;
    }
public:
    inline size_t useCount() noexcept
    {
        return _countStrong.load(std::memory_order_relaxed);
    }
};

namespace internal
{
    template<typename ElementType, typename ControlBlockType>
    requires scoped_lock_control_block<ControlBlockType>
    inline void release_strong_and_destroy(ControlBlockType* controlBlock, ElementType* element)
    {
        if(controlBlock == nullptr)
            return;

        const auto [deleteElement, deleteAll] = controlBlock->release();

        if(deleteElement)
        {
            delete element;
            if(deleteAll)
                delete controlBlock;
        }
    }

    template<typename ElementType, typename ControlBlockType>
    requires scoped_lock_control_block<ControlBlockType>
    inline void release_weak_and_destroy(ControlBlockType* controlBlock, ElementType* element)
    {
        if(controlBlock == nullptr)
            return;

        if(!controlBlock->releaseWeak())
            return;

        delete controlBlock;
    }
}

template<typename ElementType, typename ControlBlockType = synchronized_scoped_ptr_control_block>
requires scoped_lock_control_block<ControlBlockType>
class scoped_source_ptr;
template<typename ElementType, typename ControlBlockType = synchronized_scoped_ptr_control_block>
requires scoped_lock_control_block<ControlBlockType>
class scoped_weak_ptr;
template<typename ElementType, typename ControlBlockType = synchronized_scoped_ptr_control_block>
requires scoped_lock_control_block<ControlBlockType>
class scoped_ptr;

template<typename ElementType, typename ControlBlockType>
requires scoped_lock_control_block<ControlBlockType>
class scoped_source_ptr
{
public:
    using control_block_type = ControlBlockType;
    using element_type = ElementType;
    using weak_type = scoped_weak_ptr<element_type, control_block_type>;
    using strong_type = scoped_ptr<element_type, control_block_type>;
private:
    element_type* _element{};
    control_block_type* _controlBlock{};
public:
    constexpr scoped_source_ptr() noexcept = default;
    constexpr scoped_source_ptr(std::nullptr_t) noexcept
        : scoped_source_ptr()
    {}
    explicit scoped_source_ptr(element_type* element)
        : _element(element)
        , _controlBlock(element != nullptr ? new control_block_type{} : nullptr)
    {}
    scoped_source_ptr(const scoped_source_ptr& other) = delete;
    constexpr scoped_source_ptr(scoped_source_ptr&& other) noexcept = default;
public:
    scoped_source_ptr& operator=(const scoped_source_ptr& other) = delete;
    scoped_source_ptr& operator=(scoped_source_ptr&& other) noexcept = default;
public:
    ~scoped_source_ptr()
    {
        internal::release_strong_and_destroy(_controlBlock, _element);
    }
public:
    inline size_t useCount() noexcept
    {
        if(_controlBlock != nullptr)
            return _controlBlock->useCount();
        return 0;
    }
    inline void reset(element_type* element = nullptr) noexcept
    {
        internal::release_strong_and_destroy(_controlBlock, _element);
        _controlBlock = nullptr;

        _element = element;
        _controlBlock = element != nullptr ? new control_block_type{} : nullptr;
    }
public:
    inline element_type* get() const noexcept
    {
        if(_controlBlock != nullptr)
            return _element;
        else
            return nullptr;
    }
    inline element_type* operator->() noexcept
    {
        return get();
    }
    inline element_type& operator*() noexcept
    {
        return *get();
    }
public:
    inline explicit operator bool() const noexcept
    {
        return _element != nullptr;
    }
    inline operator weak_type() const noexcept;
    inline operator strong_type() const noexcept;
};

template<typename ElementType, typename ControlBlockType>
requires scoped_lock_control_block<ControlBlockType>
class scoped_weak_ptr
{
    friend class scoped_source_ptr<ElementType, ControlBlockType>;
public:
    using control_block_type = ControlBlockType;
    using element_type = ElementType;
    using weak_type = scoped_weak_ptr<element_type, control_block_type>;
    using strong_type = scoped_ptr<element_type, control_block_type>;
private:
    element_type* _element{};
    control_block_type* _controlBlock{};
public:
    constexpr scoped_weak_ptr() noexcept = default;
    constexpr scoped_weak_ptr(const scoped_weak_ptr& other) noexcept
    {
        if(other._controlBlock != nullptr && other._controlBlock.acquire_weak())
        {
            _controlBlock = other._controlBlock;
            _element = other._element;
        }
    }
    constexpr scoped_weak_ptr(scoped_weak_ptr&& other) noexcept = default;
private:
    scoped_weak_ptr(element_type* element, control_block_type* control_block)
        : _element(element)
          , _controlBlock(control_block)
    {}
public:
    scoped_weak_ptr& operator=(const scoped_weak_ptr& other) noexcept
    {
        if(&other == this)
            return *this;

        reset();
        if(other._controlBlock != nullptr && other._controlBlock.acquire_weak())
        {
            _controlBlock = other._controlBlock;
            _element = other._element;
        }

        return *this;
    }
    scoped_weak_ptr& operator=(scoped_weak_ptr&& other) noexcept = default;
public:
    ~scoped_weak_ptr()
    {
        internal::release_weak_and_destroy(_controlBlock, _element);
    }
public:
    inline size_t useCount() noexcept
    {
        if(_controlBlock != nullptr)
            return _controlBlock->useCount();
        return 0;
    }
    inline void reset() noexcept
    {
        internal::release_weak_and_destroy(_controlBlock, _element);
        _controlBlock = nullptr;
    }
    inline strong_type lock() noexcept;
};

template<typename ElementType, typename ControlBlockType>
requires scoped_lock_control_block<ControlBlockType>
class scoped_ptr
{
    friend class scoped_source_ptr<ElementType, ControlBlockType>;
    friend class scoped_weak_ptr<ElementType, ControlBlockType>;
public:
    using control_block_type = ControlBlockType;
    using element_type = ElementType;
    using weak_type = scoped_weak_ptr<element_type, control_block_type>;
    using strong_type = scoped_ptr<element_type, control_block_type>;
private:
    element_type* _element{};
    control_block_type* _controlBlock{};
public:
    constexpr scoped_ptr() noexcept = default;
    constexpr scoped_ptr(const scoped_ptr& other) = delete;
    constexpr scoped_ptr(scoped_ptr&& other) noexcept = default;
private:
    scoped_ptr(element_type* element, control_block_type* control_block)
        : _element(element)
          , _controlBlock(control_block)
    {}
public:
    scoped_ptr& operator=(const scoped_ptr& other) = delete;
    scoped_ptr& operator=(scoped_ptr&& other) noexcept = default;
public:
    ~scoped_ptr()
    {
        internal::release_strong_and_destroy(_controlBlock, _element);
    }
public:
    inline size_t useCount() noexcept
    {
        if(_controlBlock != nullptr)
            return _controlBlock->useCount();
        return 0;
    }
    inline void reset() noexcept
    {
        internal::release_strong_and_destroy(_controlBlock, _element);
        _controlBlock = nullptr;
    }
    constexpr element_type* get() const noexcept
    {
        if(_controlBlock != nullptr)
            return _element;
        else
            return nullptr;
    }
    constexpr element_type* operator->() noexcept
    {
        return get();
    }
    constexpr element_type& operator*() noexcept
    {
        return *get();
    }
public:
    constexpr explicit operator bool() const noexcept
    {
        return  _controlBlock != nullptr;
    }
};

template<typename ElementType, typename ControlBlockType>
requires scoped_lock_control_block<ControlBlockType>
scoped_source_ptr<ElementType, ControlBlockType>::operator weak_type() const noexcept
{
    if(_controlBlock == nullptr)
        return {};

    _controlBlock->acquireWeak();
    return {_element, _controlBlock};
}

template<typename ElementType, typename ControlBlockType>
requires scoped_lock_control_block<ControlBlockType>
scoped_source_ptr<ElementType, ControlBlockType>::operator strong_type() const noexcept
{
    if(_controlBlock == nullptr)
        return {};

    if(!_controlBlock->acquire())
        return {};

    return {_element, _controlBlock};
}

template<typename ElementType, typename ControlBlockType>
requires scoped_lock_control_block<ControlBlockType>
scoped_weak_ptr<ElementType, ControlBlockType>::strong_type
scoped_weak_ptr<ElementType, ControlBlockType>::lock() noexcept
{
    if(_controlBlock == nullptr)
        return {};

    if(!_controlBlock->acquire())
        return {};

    return {_element, _controlBlock};
}
