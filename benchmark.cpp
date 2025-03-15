#include <iostream>
#include <cstdlib>  // For malloc and free
#include <cstdint>  // For uintptr_t (needed for pointer arithmetic)

// 🏴‍☠️ The Dark Magic: A manually implemented aligned memory allocator
void* aligned_malloc(size_t size, size_t alignment) {
    // We allocate extra memory to ensure we can align it correctly
    void* raw_mem = malloc(size + alignment + sizeof(void*));
    if (!raw_mem) return nullptr;  // If malloc fails, we return nullptr (never trust the heap blindly)

    // Compute the aligned address using bitwise magic
    uintptr_t aligned_addr = (reinterpret_cast<uintptr_t>(raw_mem) + alignment + sizeof(void*)) & ~(alignment - 1);

    // Store the original memory address right before the aligned pointer
    reinterpret_cast<void**>(aligned_addr)[-1] = raw_mem;

    // Return the aligned memory block
    return reinterpret_cast<void*>(aligned_addr);
}

// 🏴‍☠️ The Counter-Spell: Freeing our aligned memory properly
void aligned_free(void* aligned_mem) {
    if (aligned_mem) {
        // Retrieve and free the original memory block
        free(reinterpret_cast<void**>(aligned_mem)[-1]);
    }
}

#include <chrono>  // For high-precision benchmarking

// ⚡ The Ultimate Memory Benchmarking Function
void benchmark(size_t size, size_t alignment, int iterations) {
    using namespace std::chrono;

    // Measure performance of std::aligned_alloc
    auto start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        void* ptr = std::aligned_alloc(alignment, size);
        std::free(ptr);
    }
    auto end = high_resolution_clock::now();
    std::cout << "std::aligned_alloc: " 
              << duration_cast<microseconds>(end - start).count() 
              << " us\n";

    // Measure performance of our dark magic `aligned_malloc`
    start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        void* ptr = aligned_malloc(size, alignment);
        aligned_free(ptr);
    }
    end = high_resolution_clock::now();
    std::cout << "aligned_malloc: " 
              << duration_cast<microseconds>(end - start).count() 
              << " us\n";
}

int main() {
    // 🏴‍☠️ Let the battle begin! Testing different allocation sizes and alignments
    benchmark(64, 16, 100000);  // Small allocations (typical cache line size)
    benchmark(128, 32, 100000); // Medium allocations (optimized for SIMD)
    benchmark(256, 64, 100000); // Larger blocks (potential heap fragmentation effects)
    benchmark(1024, 64, 100000); // 1KB allocations (starting to hit mmap territory)
    benchmark(1024 * 1024, 64, 1000); // 1MB allocations (massive, likely using mmap)

    return 0;
}
