#ifndef GUI_ARR_H
#define GUI_ARR_H
#include <cstddef>

template <typename T>
struct Arr {
    T* data = nullptr;
    std::size_t count = 0;
    std::size_t cap = 0;

    Arr() = default;
    Arr(const Arr&) = delete;
    Arr& operator=(const Arr&) = delete;
    ~Arr() { delete[] data; }

    void reserve(std::size_t need) {
        if (need <= cap) return;
        std::size_t nc = cap ? cap * 2 : 16;
        while (nc < need) nc *= 2;
        T* nd = new T[nc];
        for (std::size_t i = 0; i < count; ++i) nd[i] = data[i];
        delete[] data;
        data = nd;
        cap = nc;
    }

    void push_back(const T& v) {
        reserve(count + 1);
        data[count++] = v;
    }

    void clear() {
        delete[] data;
        data = nullptr;
        count = 0;
        cap = 0;
    }

    void erase_at(std::size_t i) {
        data[i] = data[--count];
    }

    std::size_t size() const { return count; }
    bool empty() const { return count == 0; }

    T& operator[](std::size_t i) { return data[i]; }
    const T& operator[](std::size_t i) const { return data[i]; }

    T* begin() { return data; }
    T* end() { return data + count; }
    const T* begin() const { return data; }
    const T* end() const { return data + count; }
};

#endif
