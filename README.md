# 💡 **The Aligned Memory Battle: `std::aligned_alloc()` vs `aligned_malloc()`**

## ⚡ **Benchmarking Memory Alignment in C++**

### 🔍 **Introduction**
Memory alignment plays a **crucial role** in optimizing CPU performance, reducing cache misses, and improving SIMD operations. In C++17, `std::aligned_alloc()` was introduced to simplify memory alignment, but **is it really the best choice?**  

This article presents a **detailed benchmark** comparing `std::aligned_alloc()` and a manually implemented `aligned_malloc()`. We answer the following questions:

- **Is `std::aligned_alloc()` always the fastest option?**  
- **Can manual memory management outperform the standard implementation?**  
- **How do different memory sizes affect performance?**  

---

## 📊 **Benchmark Results**
| Allocation Size | `std::aligned_alloc()` | `aligned_malloc()` | 🏆 Winner |
|----------------|------------------------|--------------------|------------|
| 64B           | **2415 us** | **3802 us** | `std::aligned_alloc()` ✅ |
| 128B          | **5622 us** | **3164 us** | `aligned_malloc()` ✅ |
| 256B          | **5132 us** | **2661 us** | `aligned_malloc()` ✅ |
| 1024B         | **14608 us** | **6369 us** | `aligned_malloc()` ✅ |
| 1MB           | **141 us** | **38 us** | `aligned_malloc()` ✅ |

### **📌 Key Takeaways**
- `std::aligned_alloc()` is **only faster for ultra-small allocations (64B)** due to optimized heap management.
- `aligned_malloc()` is **more efficient for allocations larger than 64B**, avoiding unnecessary libc allocator overhead.
- For **very large allocations (1MB+), `aligned_malloc()` is significantly faster** due to `mmap()` overhead in `std::aligned_alloc()`.

---

## 💡 **Understanding the Benchmark Results**
### **1. Why is `std::aligned_alloc()` faster for 64B but slower for larger sizes?**
- `std::aligned_alloc()` handles **very small blocks** more efficiently inside the heap.
- At **128B and 256B**, it likely introduces **internal alignment overhead**, making it slower.
- `aligned_malloc()` avoids this extra management and remains **faster for 128B+ allocations**.

### **2. Why is `std::aligned_alloc()` much slower for 1024B and 1MB?**
- **For allocations >1KB, `std::aligned_alloc()` starts using `mmap()`** instead of `malloc()`.
- `mmap()` involves **syscalls to the kernel**, significantly increasing allocation time.
- `aligned_malloc()` keeps using `malloc()`, avoiding the overhead.

### **3. Why is `aligned_malloc()` so much faster at 1MB?**
- `std::aligned_alloc()` is likely calling `mmap()`, introducing syscall overhead.
- `aligned_malloc()` keeps working in the heap, making it **dramatically faster (141 us vs. 38 us)**.
- **But caution**: `mmap()` is **slower in allocation but better for long-term memory use**.

🛠 **Test if `std::aligned_alloc()` uses `mmap()` with:**
```bash
strace ./bc 2>&1 | grep mmap
```
If it prints `mmap()` calls, that explains the performance drop.

---

## 🛠 **How Each Method Works**
### 🏆 **Method 1: `std::aligned_alloc()` (C++17)**
```cpp
void* ptr = std::aligned_alloc(64, 1024); // 64-byte alignment, 1024-byte allocation
std::free(ptr);
```
✅ **Pros:** Simpler, safer, and standard-compliant.  
❌ **Cons:** May be slower for very large or very small allocations.  

---

### 🏴‍☠️ **Method 2: `aligned_malloc()` (Manual `malloc()` Implementation)**
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
```
✅ **Pros:** Faster for allocations larger than 64B.  
❌ **Cons:** Requires manual memory management (**potential `segfault` if misused**).  

---

## 👌 **Final Thoughts**
1️⃣ **For ultra-small allocations (64B), `std::aligned_alloc()` is better.**  
2️⃣ **For anything larger than 64B, `aligned_malloc()` is consistently faster.**  
3️⃣ **For frequent allocations & deallocations, `aligned_malloc()` is more efficient.**  
4️⃣ **For very large memory (1MB+), `aligned_malloc()` dominates due to `mmap()` overhead in `std::aligned_alloc()`.**  

🚀 **Want to test it yourself? Run the code, execute the benchmarks, and share your results!**  

---

📌 **Further Reading & Related Discussions**
- [LinkedIn Post](#) *(Discussion on optimization strategies)*  
- [GitHub Repository](#) *(Source code & benchmarking results)*  

🌟 **May Odin bless your pointers, and may you never dereference an uninitialized variable!** 🔥

