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

**Size Restriction:** `std::aligned_alloc()` requires that the allocation size (the first parameter) be an exact multiple of the alignment (the second parameter). This is because the function internally assumes that it can divide the block of memory into equal parts of size "alignment".

**Example:**  
Requesting an allocation with alignment of 32 bytes and size 100 bytes is invalid since 100 is not a multiple of 32. This could result in undefined behavior or memory corruption.

**Note:**  
May be slower for very large or very small allocations if the size restriction forces unnecessary adjustments.


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

**No Size Restriction:** Unlike `std::aligned_alloc()`, this implementation does not require that the size be an exact multiple of the alignment. It allocates extra space and computes an aligned address, making it more flexible in production environments where the allocation size might not be a neat multiple of the alignment requirement.


---

## 👌 **Final Thoughts**
1️⃣ **For ultra-small allocations (64B), `std::aligned_alloc()` is better.**  
2️⃣ **For anything larger than 64B, `aligned_malloc()` is consistently faster.**  
3️⃣ **For frequent allocations & deallocations, `aligned_malloc()` is more efficient.**  
4️⃣ **For very large memory (1MB+), `aligned_malloc()` dominates due to `mmap()` overhead in `std::aligned_alloc()`.**  

🚀 **Want to test it yourself? Run the code, execute the benchmarks, and share your results!**  

---

# 🏴‍☠️ **Dark Magic in Progress...** ⚡🔮

> **"There are secrets in the memory... waiting to be unlocked."** 🧙‍♂️  

This repository is **under construction**, but the journey into **memory alignment, cache optimization, and raw performance wizardry** has just begun. 🚀  

## ⚔️ **What Lies Ahead?**
- 🛠️ **Deep dives** into memory allocators, heap fragmentation, and low-level optimizations.  
- ⚡ **More benchmarks** pushing `std::aligned_alloc()` and `aligned_malloc()` to their limits.  
- 🔥 **Dark magic tricks** to optimize performance like a true low-level sorcerer.  

🔻 **Stay tuned... the real battle for memory efficiency is just getting started.**  

⚡ **May your pointers stay aligned, and your cache lines stay hot.** 💀🔥  

# 🧐 Explanation


# 🏴‍☠️ mmap() vs. malloc(): The Dark Magic of Memory Allocation ⚡

## 1️⃣ Why is `mmap()` Slower in Allocation?
🔹 **Because `mmap()` makes a system call to the kernel.**  

When you use `mmap()` to allocate memory, it requests memory **directly from the operating system**. This means it has to go through the **kernel**, which involves a **context switch**, adding overhead.  

### **How `mmap()` Works in Allocation:**
1. The request is sent to the OS via a syscall.  
2. The OS checks the page table and assigns virtual memory pages.  
3. The memory is mapped to the process, but it’s **not actually allocated in RAM yet**.  
4. On the first access, the OS assigns a **physical page** (page fault).  

### **Why `mmap()` is slower:**
✅ **Syscalls add latency** (switching between user-space and kernel-space).  
✅ **Virtual memory pages are lazy-loaded** (not fully allocated until first access).  

---

### **How `malloc()` Works in Allocation:**
1. `malloc()` typically gets memory **from the heap**, which is already reserved by the process.  
2. If the heap needs more memory, `malloc()` **might call `sbrk()`** (which expands the heap).  
3. If the request is very large, `malloc()` **might use `mmap()` internally** (but only for large allocations).  

Since **`malloc()` just hands out memory from the heap (which is already managed by the process)**, it’s **faster** than calling `mmap()` directly.

---

## 2️⃣ Why is `mmap()` Better for Long-Term Use?
🔹 **Because it avoids heap fragmentation and allows fine-grained memory control.**  

When you allocate memory with `malloc()`, that memory comes from the **heap**. The problem is that **`malloc()` cannot release memory back to the OS until the entire heap segment is freed**.

### **Heap Fragmentation Problem:**
- If you frequently allocate and free different-sized blocks, **holes appear in the heap**.  
- These holes cause **fragmentation**, making future allocations inefficient.  
- Even if memory is freed, the OS **cannot reclaim it** if it’s inside an active heap segment.  

### **Why `mmap()` Avoids This Problem:**
- Each `mmap()` call **allocates a separate memory region**.  
- When you free an `mmap()` allocation, the OS **immediately reclaims the memory**.  
- This is **great for large, long-lived allocations** (like buffers that last the whole program).  

✅ **If you have memory that will live for a long time, `mmap()` prevents heap fragmentation.**  
✅ **If you allocate and free memory frequently, `malloc()` might cause fragmentation, while `mmap()` can avoid it.**  

---

## 3️⃣ When to Use `mmap()` vs. `malloc()`?

| **Scenario**                  | **Use `malloc()` ✅** | **Use `mmap()` ✅** |
|--------------------------------|----------------------|----------------------|
| **Small allocations (< 1MB)**  | ✅ Faster & efficient | ❌ Overhead too high |
| **Frequent allocations/deallocations** | ✅ Avoids syscall overhead | ❌ Can be slow due to syscalls |
| **Large allocations (> 1MB)**  | ❌ May cause heap fragmentation | ✅ OS handles memory better |
| **Memory that lives for a long time** | ❌ Might fragment the heap | ✅ OS can reclaim it easily |

---

## 🔥 **Summary**
- **Use `malloc()` for small, short-lived allocations** (faster, avoids syscall overhead).  
- **Use `mmap()` for large, long-lived allocations** (avoids fragmentation, gives fine control).  
- **If you allocate and free large memory chunks frequently, consider `mmap()` to reduce fragmentation.**  

---

## 🚀 **The Real Battle: `mmap()` vs. `malloc()`**
- **If your program needs ultra-fast memory allocation/deallocation, use `malloc()`.**  
- **If you need efficient, long-term memory usage with minimal fragmentation, use `mmap()`.**  

🔥 **Now you know the dark magic behind memory allocation!** 🏴‍☠️⚡  


