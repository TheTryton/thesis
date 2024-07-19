#include <print>
#include <concepts>
#include <array>
#include <algorithm>
#include <vector>
#include <format>
#include <string>
#include <chrono>
#include <random>
#include <cassert>
#include <execution>

namespace std
{
template <typename ElementType, typename Allocator>
struct formatter<std::vector<ElementType, Allocator>> {
    template<class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    FormatContext::iterator format(const std::vector<ElementType, Allocator>& v, FormatContext& ctx) const
    {
        auto it = ctx.out();
        if(std::size(v) == 0)
            return it;

        auto current = std::begin(v);
        auto preLast = --std::end(v);

        it = std::format_to(it, "{{");

        while(current != preLast)
        {
            it = std::format_to(it, "{}, ", *current++);
        }
        it = std::format_to(it, "{}", *current++);

        it = std::format_to(it, "}}");

        return it;
    }
};
}

namespace internal
{
    template<size_t K, typename InIt, typename OutIt, typename CompareF>
    requires
        (K >= 2) &&
        std::input_iterator<InIt> &&
        std::output_iterator<OutIt, std::iter_value_t<InIt>> &&
        std::indirect_binary_predicate<CompareF, InIt, InIt>
    OutIt k_way_merge(std::array<std::pair<InIt, InIt>, K> inIts, OutIt outIt, CompareF compareF)
    {
        using inItIt = std::array<std::pair<InIt, InIt>, K>::iterator;
        using inItItV = std::iter_value_t<inItIt>;
        using inItV = std::iter_value_t<InIt>;

        auto find_min = [&](){
            return std::min_element(std::begin(inIts), std::end(inIts), [&](const inItItV& current, const inItItV& smallest)
            {
                if(smallest.first == smallest.second)
                    return current.first != current.second;

                if(current.first == current.second)
                    return false;

                inItV value = *current.first;
                inItV smallestValue = *smallest.first;

                return compareF(value, smallestValue);
            });
        };

        for(auto currentIt = find_min(); currentIt->first != currentIt->second; currentIt = find_min())
        {
            *outIt = *currentIt->first;
            ++outIt;
            ++currentIt->first;
        }

        return outIt;
    }

    template<size_t BufferSize, typename InIt, typename OutIt, typename CompareF>
    requires
        (BufferSize >= 1) &&
        std::random_access_iterator<InIt> &&
        std::random_access_iterator<OutIt> &&
        std::indirect_binary_predicate<CompareF, InIt, InIt>
    void k_way_merge_initial_sort(InIt inItFirst, InIt inItLast, OutIt outIt, CompareF compareF)
    {
        const auto totalSize = static_cast<size_t>(std::distance(inItFirst, inItLast));
        const auto bufferCount = (totalSize + BufferSize - 1) / BufferSize;


#pragma omp parallel for if(bufferCount > 1024)
        for(size_t bufferIndex = 0; bufferIndex < bufferCount; ++bufferIndex)
        {
            const auto bufferFirst = bufferIndex * BufferSize;
            const auto bufferSize = std::min(totalSize - bufferFirst, BufferSize);
            const auto bufferLast = bufferFirst + bufferSize;

            const auto bufferInBegin = inItFirst + bufferFirst;
            const auto bufferInEnd = inItFirst + bufferLast;

            const auto bufferOutBegin = outIt + bufferFirst;
            const auto bufferOutEnd = outIt + bufferLast;

            std::copy(bufferInBegin, bufferInEnd, bufferOutBegin);
            std::stable_sort(bufferOutBegin, bufferOutEnd, compareF);
        }
    }
}

template<size_t BufferSize, size_t K, typename InIt, typename OutIt, typename CompareF>
requires
    (BufferSize >= 1) &&
    (K >= 2) &&
    std::random_access_iterator<InIt> &&
    std::random_access_iterator<OutIt> &&
    std::indirect_binary_predicate<CompareF, InIt, InIt>
bool k_way_merge_sort(InIt inItFirst, InIt inItLast, OutIt outIt0, OutIt outIt1, CompareF compareF)
{
    internal::k_way_merge_initial_sort<BufferSize>(inItFirst, inItLast, outIt0, compareF);

    const auto totalSize = static_cast<size_t>(std::distance(inItFirst, inItLast));
    const auto totalBuffersCount = (totalSize + BufferSize - 1) / BufferSize;

    auto bufferSize = BufferSize;
    auto runsCount = (totalBuffersCount + K - 1) / K;

    auto currentIn = outIt0;
    auto currentOut = outIt1;
    auto sortedInOut0 = true;

    auto do_merge_run = [&]()
    {
#pragma omp parallel for if(runsCount > 256)
        for(size_t runIndex = 0; runIndex < runsCount; ++runIndex)
        {
            size_t runOffset = runIndex * bufferSize * K;

            std::array<std::pair<InIt, InIt>, K> inIts;
            for(size_t inSpanIndex = 0; inSpanIndex < K; ++inSpanIndex)
            {
                size_t firstOffset = std::min(runOffset + inSpanIndex * bufferSize, totalSize);
                size_t lastOffset = std::min(firstOffset + bufferSize, totalSize);
                OutIt first = currentIn + firstOffset;
                OutIt last = currentIn + lastOffset;
                inIts[inSpanIndex] = std::pair(first, last);
            }

            internal::k_way_merge(inIts, currentOut + runOffset, compareF);
        }
    };

    while(runsCount > 1)
    {
        do_merge_run();

        bufferSize = bufferSize * K;
        runsCount = (runsCount + K - 1) / K;
        std::swap(currentIn, currentOut);
        sortedInOut0 = !sortedInOut0;
    }

    do_merge_run();
    std::swap(currentIn, currentOut);
    sortedInOut0 = !sortedInOut0;

    return sortedInOut0;
}

int main()
{
    auto generate_random_data = []<typename OutIt>(OutIt first, std::iter_difference_t<OutIt> count)
    {
        return std::transform(std::execution::par_unseq, first, first + count, first, [&](auto&& _){
            static thread_local std::random_device randomDevice;
            static thread_local std::mt19937 generator(randomDevice());
            static thread_local std::uniform_int_distribution<size_t> distribution(std::numeric_limits<size_t>::min(), std::numeric_limits<size_t>::max());

            return distribution(generator);
        });
    };

    constexpr size_t runCount = 1;
    constexpr size_t runSize = 8ull * 256 * 1024 * 1024 / sizeof(size_t);
    constexpr size_t bufferSize = 1024;
    constexpr size_t K = 64;

    std::vector<size_t> data(runSize);
    std::vector<size_t> buffer0(runSize);
    std::vector<size_t> buffer1(runSize);

    using milliseconds = std::chrono::duration<double, std::milli>;

    milliseconds accumulatedTime{};

    for(size_t i = 0; i < runCount; ++i)
    {
        std::print("Generating data: ");
        generate_random_data(std::begin(data), runSize);
        std::print("DONE\n");

        std::print("Sorting (K={}, BufferSize={}): ", K, bufferSize);
        const auto start = std::chrono::high_resolution_clock::now();
        bool sortedInBuffer0 = k_way_merge_sort<bufferSize, K>(
            std::begin(data), std::end(data),
                std::begin(buffer0),
                std::begin(buffer1),
                std::less<>()
        );
        const auto end = std::chrono::high_resolution_clock::now();
        std::print("DONE\n", K, bufferSize);

        auto& sortedBuffer = sortedInBuffer0 ? buffer0 : buffer1;

        std::print("Checking if actually sorted: ");
        assert(std::is_sorted(std::begin(sortedBuffer), std::end(sortedBuffer)));
        std::print("OK\n");

        accumulatedTime += std::chrono::duration_cast<milliseconds>(end - start);
    }

    std::print("Average time to sort [k_way_merge_sort] (size={}MB): {}ms\n", runSize * sizeof(size_t) / 1024 / 1024, (accumulatedTime / runCount).count());

    {
        std::vector<size_t> dataTemp(runSize);
        std::copy(std::execution::par_unseq, std::begin(data), std::end(data), std::begin(dataTemp));

        const auto start = std::chrono::high_resolution_clock::now();
        std::stable_sort(std::execution::par_unseq, std::begin(dataTemp), std::end(dataTemp));
        const auto end = std::chrono::high_resolution_clock::now();

        const auto timeTaken = std::chrono::duration_cast<milliseconds>(end - start);
        std::print("Time to sort [std::stable_sort] (size={}MB): {}ms\n", runSize * sizeof(size_t) / 1024 / 1024, timeTaken.count());
    }

    {
        std::vector<size_t> dataTemp(runSize);
        std::copy(std::execution::par_unseq, std::begin(data), std::end(data), std::begin(dataTemp));

        const auto start = std::chrono::high_resolution_clock::now();
        std::sort(std::execution::par_unseq, std::begin(dataTemp), std::end(dataTemp));
        const auto end = std::chrono::high_resolution_clock::now();

        const auto timeTaken = std::chrono::duration_cast<milliseconds>(end - start);
        std::print("Time to sort [std::sort] (size={}MB): {}ms\n", runSize * sizeof(size_t) / 1024 / 1024, timeTaken.count());
    }

    return 0;
}