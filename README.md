# 🏴‍☠️ The Aligned Memory Battle: std::aligned_alloc() vs aligned_malloc()
⚡ **Memory alignment benchmark in C++** ⚡  

## 🚀 Introduction  
In low-level C++, **memory alignment** is crucial for optimizing performance and avoiding CPU cache penalties.  

💡 **Is std::aligned_alloc() (C++17) really the best choice, or can a manually implemented aligned_malloc() be faster?**  
💀 **What happens when we take memory allocation into our own hands using pointers and bitwise alignment?**  

To answer these questions, we ran **a large-scale benchmark**, comparing both strategies across different memory sizes.  

---

## 🔥 **Benchmark Results: Which One is Faster?**
| Size | std::aligned_alloc() | aligned_malloc() | 🏆 Winner |
|------|------------------------|--------------------|------------|
| 64B  | **1.5 ms** | **3.0 ms** | std::aligned_alloc() ✅ |
| 128B | **1.4 ms** | **2.6 ms** | std::aligned_alloc() ✅ |
| 1KB  | **6.1 ms** | **2.6 ms** | aligned_malloc() ✅ |
| 1MB  | **14.5 ms** | **5.9 ms** | aligned_malloc() ✅ |
| <64B | **129 us** | **30 us** | aligned_malloc() ✅ |

📌 **Key Takeaways:**  
- For **small allocations (~64B - 256B)**, std::aligned_alloc() is consistently faster.  
- For **large allocations (~1MB+)**, aligned_malloc() is significantly more efficient, avoiding mmap() overhead.  
- For **ultra-small allocations**, aligned_malloc() is up to **5 times faster** due to lower internal overhead.  

---

## ⚙️ **Code: How Each Method Works**

### 🏆 **Method 1: std::aligned_alloc() (C++17)**
```cpp
void* ptr = std::aligned_alloc(64, 1024); // 64-byte alignment, 1024-byte size
std::free(ptr);
✅ Pros: Simpler, safer, and standard-compliant.
❌ Cons: May be slower for very large or very small allocations.

### 🏴‍☠️ Method 2: aligned_malloc() (Manual malloc() Implementation)
```cpp
void* aligned_malloc(size_t size, size_t alignment) {
    void* raw_mem = malloc(size + alignment + sizeof(void*));
    if (!raw_mem) return nullptr;
    
    uintptr_t aligned_addr = (reinterpret_cast<uintptr_t>(raw_mem) + alignment + sizeof(void*)) & ~(alignment - 1);
    reinterpret_cast<void**>(aligned_addr)[-1] = raw_mem;
    
    return reinterpret_cast<void*>(aligned_addr);
}

void aligned_free(void* aligned_mem) {
    if (aligned_mem) {
        free(reinterpret_cast<void**>(aligned_mem)[-1]);
    }
}

✅ Pros: Faster for large and ultra-small allocations.
❌ Cons: Requires manual memory management (potential for segfault if misused).

📢 How to Run the Benchmark

🔹 Compile and Execute

g++ -std=c++17 -O2 benchmark.cpp -o benchmark
./benchmark
This will run 100,000 allocations to compare both methods.

🔹 Testing Different Sizes
Modify the code to test different memory sizes:

benchmark(64, 16, 100000);
benchmark(1024, 64, 100000);
benchmark(1024 * 1024, 64, 1000);  // 1MB allocation
🎯 Final Thoughts
1️⃣ If you are using modern C++ and want safety and portability, std::aligned_alloc() is your best bet.
2️⃣ If you need extreme performance optimizations, aligned_malloc() can be more efficient in specific scenarios.
3️⃣ Always benchmark before deciding which allocation strategy to use.

🚀 Want to test it yourself? Run the code && execute the benchmarks!
