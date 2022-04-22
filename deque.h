#include <iostream>
#include <vector>
#include <exception>

const size_t kSize = 16;
const int kIntSize = static_cast<int>(kSize);

template<typename T>
class Deque {
private:
    size_t mBegin;
    size_t mBeginIndex;
    size_t mEnd;
    size_t mEndIndex;
    size_t mCapacity;
    std::vector<T*> mArray;

public:
    template<bool Const>
    class Iterator;
    using const_iterator = Iterator<true>;
    using iterator = Iterator<false>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using reverse_const_iterator = std::reverse_iterator<const_iterator>;

    template<bool Const>
    class Iterator{
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = typename std::conditional<Const, const T*, T*>::type;
        using reference = typename std::conditional<Const, const T&, T&>::type;
        using iterator_category = std::random_access_iterator_tag;

    private:
        int mInternalIndex;
        int mExternalIndex;
        T** mPtr;

        friend void Deque<T>::insert(iterator it, const T& value);

    public:
        Iterator(T** newPtr, size_t index, size_t start) : mInternalIndex(static_cast<int>(index)),
            mExternalIndex(static_cast<int>(start)), mPtr(newPtr) {}
        Iterator() noexcept : mInternalIndex(0), mExternalIndex(0), mPtr(nullptr) {}
        Iterator(const Iterator& other) noexcept : mInternalIndex(other.mInternalIndex),
            mExternalIndex(other.mExternalIndex), mPtr(other.mPtr) {}
        operator Iterator<true>() const {
            return Iterator<true>(*this);
        }
        operator Iterator<false>() const {
            static_assert(!Const);
            return Iterator<false>(*this);
        }
        Iterator& operator=(const Iterator& other) noexcept {
            if (this == &other) {
                return *this;
            }
            mInternalIndex = other.mInternalIndex;
            mExternalIndex = other.mExternalIndex;
            mPtr = other.mPtr;
            return *this;
        }
        Iterator& operator++() noexcept {
            checkIndexPlus();
            return *this;
        }
        Iterator operator++(int) noexcept {
            Deque<T>::Iterator newIt(*this);
            checkIndexPlus();
            return newIt;
        }
        Iterator& operator--() noexcept {
            checkIndexMinus();
            return *this;
        }
        Iterator operator--(int) noexcept {
            Deque<T>::Iterator newIt(*this);
            checkIndexMinus();
            return newIt;
        }
        Iterator& operator+=(int shift) noexcept {
            mInternalIndex += shift;
            mPtr += mInternalIndex / kIntSize;
            mExternalIndex += mInternalIndex / kIntSize;
            mInternalIndex %= kIntSize;
            checkIndexShift();
            return *this;
        }
        Iterator operator+(int shift) const noexcept {
            Deque<T>::Iterator newIt(*this);
            newIt += shift;
            return newIt;
        }
        Iterator& operator-=(int shift) noexcept {
            mInternalIndex -= shift;
            mPtr += mInternalIndex / kIntSize;
            mExternalIndex += mInternalIndex / kIntSize;
            mInternalIndex %= kIntSize;
            checkIndexShift();
            return *this;
        }
        Iterator operator-(int shift) const noexcept {
            Deque<T>::Iterator newIt(*this);
            newIt -= shift;
            return newIt;
        }
        bool operator<(const Iterator& other) const noexcept {
            return (other.mExternalIndex - mExternalIndex > 0
                    || (other.mExternalIndex - mExternalIndex == 0
                        && mInternalIndex < other.mInternalIndex));
        }
        bool operator<=(const Iterator& other) const noexcept {
            return !(other < *this);
        }
        bool operator>(const Iterator& other) const noexcept {
            return (other < *this);
        }
        bool operator>=(const Iterator& other) const noexcept {
            return !(*this < other);
        }
        bool operator==(const Iterator& other) const noexcept {
            return !(other < *this || *this < other);
        }
        bool operator!=(const Iterator& other) const noexcept {
            return (other < *this || *this < other);
        }
        difference_type operator-(const Iterator& other) const noexcept {
            return static_cast<difference_type>((mExternalIndex - other.mExternalIndex) * kIntSize + mInternalIndex - other.mInternalIndex);
        }
        reference operator*() const {
            return *(*mPtr + mInternalIndex);
        }
        pointer operator->() const {
            return (*mPtr + mInternalIndex);
        }
    private:
        void checkIndexPlus() {
            if (mInternalIndex == kIntSize - 1) {
                mInternalIndex = 0;
                ++mPtr;
                ++mExternalIndex;
            } else {
                ++mInternalIndex;
            }
        }
        void checkIndexMinus() {
            if (mInternalIndex == 0) {
                mInternalIndex = kIntSize - 1;
                --mPtr;
                --mExternalIndex;
            } else {
                --mInternalIndex;
            }
        }
        void checkIndexShift() {
            if (mInternalIndex < 0) {
                mInternalIndex += kIntSize;
                --mPtr;
                --mExternalIndex;
            }
        }
    };

public:
    ~Deque();
    Deque();
    Deque(const Deque<T>& copy);
    explicit Deque(int newSize);
    Deque(int newSize, const T& value);

    Deque<T>& operator=(const Deque<T>& other);
    T& operator[](size_t index);
    const T& operator[](size_t index) const;

    T& at(size_t index);
    const T& at(size_t index) const;
    void clear();
    size_t size() const noexcept;
    void push_back(const T& value);
    void pop_back();
    void push_front(const T& value);
    void pop_front();
    iterator begin() noexcept;
    iterator end() noexcept;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;
    reverse_iterator rbegin() noexcept;
    reverse_iterator rend() noexcept;
    reverse_const_iterator rbegin() const noexcept;
    reverse_const_iterator rend() const noexcept;
    reverse_const_iterator crbegin() const noexcept;
    reverse_const_iterator crend() const noexcept;
    void erase(iterator it);
    void insert(iterator it, const T& value);

private:
    void reserveFromClear(size_t capacity);
    void expand(size_t capacity);
    void checkEndMinus();
    void checkEndPlus();
    void checkBeginMinus();
    void checkBeginPlus();
};

template<typename T, bool Const>
typename Deque<T>::template Iterator<Const> operator+(int shift, const typename Deque<T>::template Iterator<Const>& it) noexcept {
    return it + shift;
}

template<typename T, bool Const>
typename Deque<T>::template Iterator<Const> operator-(int shift, const typename Deque<T>::template Iterator<Const>& it) noexcept {
    return it - shift;
}

template<typename T>
Deque<T>::~Deque() {
    clear();
}

template<typename T>
Deque<T>::Deque() : mBegin(kSize / 2 - 1), mBeginIndex(kSize - 1), mEnd(kSize / 2), mEndIndex(0) {
    reserveFromClear(kSize);
}

template<typename T>
Deque<T>::Deque(const Deque<T>& copy) : mBegin(copy.mBegin), mBeginIndex(copy.mBeginIndex),
    mEnd(copy.mBegin), mEndIndex(copy.mBeginIndex + 1) {
    try {
        reserveFromClear(copy.mCapacity);
        for (size_t j = mBeginIndex + 1; j < kSize; ++j, ++mEndIndex) {
            new(mArray[mBegin] + j) T(copy.mArray[mBegin][j]);
        }
        ++mEnd;
        mEndIndex = 0;
        for (size_t i = mBegin + 1; i < copy.mEnd; ++i, ++mEnd) {
            for (size_t j = 0; j < kSize; ++j, ++mEndIndex) {
                new(mArray[i]+ j) T(copy.mArray[i][j]);
            }
            mEndIndex = 0;
        }
        for (size_t j = 0; j < copy.mEndIndex; ++j, ++mEndIndex) {
            new(mArray[mEnd] + j) T(copy.mArray[mEnd][j]);
        }
    }
    catch (...) {
        clear();
        throw;
    }
}

template<typename T>
Deque<T>::Deque(int newSize, const T& value) {
    if (newSize < 0) {
        throw std::runtime_error("bad size");
    } else {
        mCapacity = static_cast<size_t>(2 * std::max((newSize + kIntSize - 1) / kIntSize, kIntSize / 2));
        reserveFromClear(mCapacity);
    }
    mBegin = (mCapacity - static_cast<size_t>(newSize) / kSize) / 2;
    mBeginIndex = kSize - 1;
    mEnd = mBegin + 1;
    mEndIndex = 0;
    try {
        for (int i = 1; i <= newSize; ++i) {
            new(mArray[mEnd] + mEndIndex)  T(value);
            checkEndPlus();
        }
    }
    catch (...) {
        clear();
        throw;
    }
}

template<typename T>
Deque<T>::Deque(int newSize) : Deque(newSize, T()) {};

template<typename T>
void Deque<T>::reserveFromClear(size_t capacity) {
    size_t count = 0;
    try {
        mCapacity = capacity;
        mArray.resize(capacity);
        for (size_t i = 0; i < capacity; ++i) {
            mArray[i] = reinterpret_cast<T*>(new uint8_t[sizeof(T) * kSize]);
            ++count;
        }
    }
    catch(...) {
        for (size_t i = 0; i < count; ++i) {
            delete[] reinterpret_cast<uint8_t*>(mArray[i]);
        }
        mCapacity = 0;
        mArray.clear();
        throw;
    }
}

template<typename T>
void Deque<T>::clear() {
    if (mCapacity > 0) {
        if (mBegin == mEnd) {
            for (size_t j = mBeginIndex + 1; j < mEndIndex; ++j) {
                (mArray[mBegin] + j)->~T();
            }
            delete[] reinterpret_cast<uint8_t *>(mArray[mBegin]);
        } else {
            for (size_t j = mBeginIndex + 1; j < kSize; ++j) {
                (mArray[mBegin] + j)->~T();
            }
            delete[] reinterpret_cast<uint8_t *>(mArray[mBegin]);
            for (size_t j = 0; j < mEndIndex; ++j) {
                (mArray[mEnd] + j)->~T();
            }
            delete[] reinterpret_cast<uint8_t *>(mArray[mEnd]);
            for (size_t i = mBegin + 1; i < mEnd; ++i) {
                for (size_t j = 0; j < kSize; ++j) {
                    (mArray[i] + j)->~T();
                }
                delete[] reinterpret_cast<uint8_t *>(mArray[i]);
            }
        }
        for (size_t i = 0; i < mBegin; ++i) {
            delete[] reinterpret_cast<uint8_t *>(mArray[i]);
        }
        for (size_t i = mEnd + 1; i < mCapacity; ++i) {
            delete[] reinterpret_cast<uint8_t *>(mArray[i]);
        }
        mArray.clear();
        mCapacity = 0;
    }
}

template<typename T>
Deque<T>& Deque<T>::operator=(const Deque<T>& other) {
    if (this == &other) {
        return *this;
    }
    std::vector<T*> backup(mArray);
    size_t count = 0;
    try {
        mArray.clear();
        reserveFromClear(other.mCapacity);
        for (size_t i = other.mBeginIndex + 1; i < kSize; ++i, ++count) {
            new(mArray[other.mBegin] + i) T(*(other.mArray[other.mBegin] + i));
        }
        for (size_t i = other.mBegin + 1; i < other.mEnd; ++i) {
            for (size_t j = 0; j < kSize; ++j, ++count) {
                new(mArray[i] + j) T(*(other.mArray[i] + j));
            }
        }
        for (size_t i = 0; i < other.mEndIndex; ++i, ++count) {
            new(mArray[other.mEnd] + i) T(*(other.mArray[other.mEnd] + i));
        }
        mBegin = other.mBegin;
        mEnd = other.mEnd;
        mBeginIndex = other.mBeginIndex;
        mEndIndex = other.mEndIndex;
    } catch (...) {
        for (size_t i = other.mBeginIndex + 1; i < kSize; ++i, --count) {
            if (count == 0) {
                break;
            }
            (mArray[other.mBegin] + i)->~T();
        }
        for (size_t i = other.mBegin + 1; i < other.mEnd; ++i) {
            if (count == 0) {
                break;
            }
            for (size_t j = 0; j < kSize; ++j, --count) {
                if (count == 0) {
                    break;
                }
                (mArray[i] + j)->~T();
            }
        }
        for (size_t i = 0; i < other.mEndIndex; ++i, --count) {
            if (count == 0) {
                break;
            }
            (mArray[other.mEnd] + i)->~T();
        }
        mArray = backup;
        throw;
    }
    return *this;
}

template<typename T>
size_t Deque<T>::size() const noexcept {
    return (mEnd - mBegin) * kSize + mEndIndex - mBeginIndex - 1;
}

template<typename T>
T& Deque<T>::operator[](size_t index) {
    index += (mBeginIndex + 1);
    return mArray[index / kSize + mBegin][index % kSize];
}

template<typename T>
const T& Deque<T>::operator[](size_t index) const {
    index += (mBeginIndex + 1);
    return mArray[index / kSize + mBegin][index % kSize];
}

template<typename T>
T& Deque<T>::at(size_t index) {
    if (index < 0 || index >= size()) {
        throw std::out_of_range("index out of range");
    } else {
        index += (mBeginIndex + 1);
        return mArray[index / kSize + mBegin][index % kSize];
    }
}

template<typename T>
const T& Deque<T>::at(size_t index) const {
    if (index < 0 || index >= size()) {
        throw std::out_of_range("index out of range");
    } else {
        index += (mBeginIndex + 1);
        return mArray[index / kSize + mBegin][index % kSize];
    }
}

template<typename T>
void Deque<T>::expand(size_t capacity) {
    size_t count = 0;
    size_t newBegin = (capacity - (mEnd - mBegin)) / 2;
    std::vector<T *> newArray(capacity);
    try {
        if (newBegin > mBegin) {
            for (size_t j = 0; j < newBegin - mBegin; ++j) {
                newArray[j] = reinterpret_cast<T *>(new uint8_t[sizeof(T) * kSize]);
                ++count;
            }
        }
        for (size_t j = newBegin - mBegin + mCapacity; j < capacity; ++j) {
            newArray[j] = reinterpret_cast<T *>(new uint8_t[sizeof(T) * kSize]);
            ++count;
        }
        for (size_t i = 0, j = newBegin - mBegin; i < mCapacity; ++i, ++j) {
            newArray[j] = mArray[i];
        }
        mArray.resize(capacity);
        for (size_t i = 0; i < capacity; ++i) {
            mArray[i] = newArray[i];
        }
        mCapacity = capacity;
        mEnd = newBegin + mEnd - mBegin;
        mBegin = newBegin;
    }
    catch(...) {
        if (count <= newBegin - mBegin) {
            for (size_t j = 0; j < count; ++j) {
                delete[] reinterpret_cast<uint8_t*>(newArray[j]);
            }
        } else {
            for (size_t j = 0; j < newBegin - mBegin; ++j) {
                delete[] reinterpret_cast<uint8_t*>(newArray[j]);
            }
            for (size_t j = newBegin - mBegin + mCapacity; j < count - newBegin + mBegin; ++j) {
                delete[] reinterpret_cast<uint8_t*>(newArray[j]);
            }
        }
        throw;
    }
}

template<typename T>
void Deque<T>::push_back(const T& value) {
    new(mArray[mEnd] + mEndIndex)  T(value);
    checkEndPlus();
    try {
        if (mEnd + 1 == mArray.size()) {
            expand(2 * mCapacity);
        }
    }
    catch(...) {
        checkEndMinus();
        (mArray[mEnd] + mEndIndex)->~T();
        throw;
    }
}

template<typename T>
void Deque<T>::pop_back() {
    if (size() == 0) {
        throw std::runtime_error("zero size");
    } else {
        checkEndMinus();
        (mArray[mEnd] + mEndIndex)->~T();
    }
}

template<typename T>
void Deque<T>::push_front(const T& value) {
    new(mArray[mBegin] + mBeginIndex) T(value);
    checkBeginMinus();
    if (mBegin <= 1) {
        try {
            expand(2 * mCapacity);
        }
        catch(...) {
            (mArray[mBegin] + mBeginIndex)->~T();
            checkBeginPlus();
            throw;
        }
    }
}

template<typename T>
void Deque<T>::pop_front() {
    if (size() == 0) {
        throw std::runtime_error("zero size");
    } else {
        checkBeginPlus();
        (mArray[mBegin] + mBeginIndex)->~T();
    }
}

template<typename T>
typename Deque<T>::iterator Deque<T>::begin() noexcept {
    if (mBeginIndex == kSize - 1) {
        return iterator(&mArray[mBegin + 1], 0, mBegin + 1);
    }
    return iterator(&mArray[mBegin], mBeginIndex + 1, mBegin);
}

template<typename T>
typename Deque<T>::iterator Deque<T>::end() noexcept {
    return iterator(&mArray[mEnd], mEndIndex, mEnd);
}

template<typename T>
typename Deque<T>::const_iterator Deque<T>::begin() const noexcept {
    if (mBeginIndex == kSize - 1) {
        return const_iterator(const_cast<T**>(&mArray[mBegin + 1]), 0, mBegin + 1);
    }
    return const_iterator(const_cast<T**>(&mArray[mBegin]), mBeginIndex + 1, mBegin);
}

template<typename T>
typename Deque<T>::const_iterator Deque<T>::end() const noexcept {
    return const_iterator(const_cast<T**>(&mArray[mEnd]), mEndIndex, mEnd);
}

template<typename T>
typename Deque<T>::const_iterator Deque<T>::cbegin() const noexcept {
    if (mBeginIndex == kSize - 1) {
        return const_iterator(const_cast<T**>(&mArray[mBegin + 1]), 0, mBegin + 1);
    }
    return const_iterator(const_cast<T**>(&mArray[mBegin]), mBeginIndex + 1, mBegin);
}

template<typename T>
typename Deque<T>::const_iterator Deque<T>::cend() const noexcept {
    return const_iterator(const_cast<T**>(&mArray[mEnd]), mEndIndex, mEnd);
}

template<typename T>
typename Deque<T>::reverse_iterator Deque<T>::rbegin() noexcept {
    return std::reverse_iterator(end());
}

template<typename T>
typename Deque<T>::reverse_iterator Deque<T>::rend() noexcept {
    return std::reverse_iterator(begin());
}

template<typename T>
typename Deque<T>::reverse_const_iterator Deque<T>::rbegin() const noexcept {
    return std::reverse_iterator(end());
}

template<typename T>
typename Deque<T>::reverse_const_iterator Deque<T>::rend() const noexcept {
    return std::reverse_iterator(begin());
}

template<typename T>
typename Deque<T>::reverse_const_iterator Deque<T>::crbegin() const noexcept {
    return std::reverse_iterator(cend());
}

template<typename T>
typename Deque<T>::reverse_const_iterator Deque<T>::crend() const noexcept {
    return std::reverse_iterator(cbegin());
}

template<typename T>
void Deque<T>::erase(iterator it) {
    if (it == begin()) {
        pop_front();
        return;
    }
    if (it + 1 == end()) {
        pop_back();
    } else {
        int count = 0;
        T backup = *it;
        try {
            for (; it + 1 < end(); ++it, ++count) {
                *it = *(it + 1);
            }
            it->~T();
            checkEndMinus();
        }
        catch(...) {
            if (count > 0) {
                --it;
                --count;
                for (; count > 0; --it, --count) {
                    *it = *(it - 1);
                }
                *it = backup;
            }
            throw;
        }
    }
}

template<typename T>
void Deque<T>::insert(iterator it, const T &value) {
    Deque<T> backup;
    bool isExpanded = false;
    bool isConstructedPoint = false;
    size_t count = 0;
    if (mEnd + 1 == mArray.size()) {
        backup = *this;
        isExpanded = true;
        int lastBegin = static_cast<int>(mBegin);
        size_t capacity = 2 * std::max(mCapacity, kSize);
        expand(capacity);
        it.mExternalIndex += static_cast<int>((capacity - (mEnd - mBegin)) / 2) - lastBegin;
    }
    iterator start(it);
    iterator endIt(end());
    T* point = *endIt.mPtr + endIt.mInternalIndex;
    T backStart(*start);
    T temp(*it);
    try {
        new(point) T(value);
        isConstructedPoint = true;
        if (it != endIt) {
            for (; it + 1 <= endIt; ++it, ++count) {
                std::swap(temp, *(it + 1));
            }
            *start = value;
        }
    }
    catch (...) {
        for (; count > 0; --it, --count) {
            std::swap(temp, *it);
        }
        if (isConstructedPoint) {
            point->~T();
        }
        if (isExpanded) {
            *this = backup;
        }
        throw;
    }
    checkEndPlus();
}

template<typename T>
void Deque<T>::checkEndMinus() {
    if (mEndIndex == 0) {
        mEndIndex = kSize - 1;
        --mEnd;
    } else {
        --mEndIndex;
    }
}

template<typename T>
void Deque<T>::checkEndPlus() {
    if (mEndIndex == kSize - 1) {
        mEndIndex = 0;
        ++mEnd;
    } else {
        ++mEndIndex;
    }
}

template<typename T>
void Deque<T>::checkBeginPlus() {
    if (mBeginIndex == kSize - 1) {
        mBeginIndex = 0;
        ++mBegin;
    } else {
        ++mBeginIndex;
    }
}

template<typename T>
void Deque<T>::checkBeginMinus() {
    if (mBeginIndex == 0) {
        --mBegin;
        mBeginIndex = kSize - 1;
    } else {
        --mBeginIndex;
    }
}