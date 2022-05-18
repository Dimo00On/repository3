#include <iostream>
#include <exception>
#include <cassert>

template<size_t N>
class StackStorage{
public:
    char mArray[N];
    size_t mSize = 0;

public:
    StackStorage() = default;
    StackStorage(const StackStorage& other) = delete;
    void operator=(const StackStorage& other) = delete;
    ~StackStorage() = default;

};

template<typename T, size_t N>
class StackAllocator {
public:
    StackStorage<N>* mPool;
    using value_type = T;

public:
    StackAllocator() = delete;
    explicit StackAllocator(StackStorage<N>& stack) noexcept;
    StackAllocator(const StackAllocator& other) noexcept;
    template<typename U>
    StackAllocator(const StackAllocator<U, N>& other) noexcept;
    ~StackAllocator();
    T* allocate(size_t amount);
    void deallocate(T* ptr, size_t amount);
    StackAllocator select_on_container_copy_construction();
    StackAllocator& operator=(const StackAllocator& other);

    template <typename U>
    struct rebind {
        typedef StackAllocator<U, N> other;
    };

    template<typename U, size_t M>
    bool operator==(const StackAllocator<U, M>& other) noexcept;

    template<typename U, size_t M>
    bool operator!=(const StackAllocator<U, M>& other) noexcept;
};

template<typename T, size_t N>
StackAllocator<T, N>::StackAllocator(StackStorage<N>& stack) noexcept : mPool(&stack) {}

template<typename T, size_t N>
StackAllocator<T, N>::StackAllocator(const StackAllocator& other) noexcept : mPool(other.mPool) {}


template<typename T, size_t N>
template<typename U>
StackAllocator<T, N>::StackAllocator(const StackAllocator<U, N>& other) noexcept : mPool(other.mPool) {}

template<typename T, size_t N>
template<typename U, size_t M>
bool StackAllocator<T, N>::operator==(const StackAllocator<U, M>& other) noexcept {
    if (std::is_same<T, U>::value && N == M && mPool->mArray == other.mPool->mArray) {
        return true;
    }
    return false;
}

template<typename T, size_t N>
template<typename U, size_t M>
bool StackAllocator<T, N>::operator!=(const StackAllocator<U, M>& other) noexcept {
    return !(*this == other);
}

template<typename T, size_t N>
StackAllocator<T, N>& StackAllocator<T, N>::operator=(const StackAllocator<T, N>& other) {
    if (this == &other) {
        return *this;
    }
    mPool = other.mPool;
    return *this;
}

template<typename T, size_t N>
StackAllocator<T, N>::~StackAllocator() {}

template<typename T, size_t N>
T* StackAllocator<T, N>::allocate(size_t amount) {
    if (mPool->mSize + sizeof(T) * amount > N) {
        throw std::bad_alloc();
    }
    int residue = static_cast<int>(mPool->mSize) % sizeof(T);
    if (residue != 0) {
        mPool->mSize += sizeof(T);
        mPool->mSize -= residue;
    }
    T* answer = reinterpret_cast<T*>(mPool->mArray + mPool->mSize);
    mPool->mSize += sizeof(T) * amount;
    return answer;
}

template<typename T, size_t N>
void StackAllocator<T, N>::deallocate(T*, size_t) {}

template<typename T, size_t N>
StackAllocator<T, N> StackAllocator<T, N>::select_on_container_copy_construction() {
    return *this;
}

template<typename T, typename Alloc = std::allocator<T>>
class List {
private:
    class BaseNode {
    public:
        BaseNode* next;
        BaseNode* prev;

        BaseNode() noexcept = default;
        virtual ~BaseNode() noexcept {};
        void create(BaseNode* ptr) noexcept {
            next = ptr;
            prev = ptr;
        }
    };

    class Node : public BaseNode {
    public:
        T mValue;

        Node() : mValue() {};
        explicit Node(const T& value) : mValue(value) {};
        ~Node() noexcept override {};
    };

    using Allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
    using AllocTraits = typename std::allocator_traits<Alloc>::template rebind_traits<Node>;
    using BaseAllocator = typename std::allocator_traits<Alloc>::template rebind_alloc<BaseNode>;
    using BaseAllocTraits = typename std::allocator_traits<Alloc>::template rebind_traits<BaseNode>;
    using AssignSelectAlloc = typename std::allocator_traits<Alloc>::propagate_on_container_copy_assignment;

private:
    Alloc mAllocForGetAllocator;
    Allocator mAlloc;
    size_t mSize;
    BaseNode* mFakeNode;

public:
    template<bool Const>
    class Iterator;
    using const_iterator = Iterator<true>;
    using iterator = Iterator<false>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    explicit List(const Alloc& alloc  = Alloc());
    List(int amount, const Alloc& alloc, const T* valuePtr, bool isDefaultValue);
    List(int amount, const Alloc& alloc = Alloc());
    List(int amount, const T& value, const Alloc& alloc = Alloc());
    Alloc get_allocator() noexcept;
    List(const List<T, Alloc>& other);
    List<T, Alloc>& operator=(const List<T, Alloc>& other);
    ~List() noexcept;
    size_t size() const noexcept;
    void push_back(const T& value);
    void push_front(const T& value);
    void pop_back() noexcept;
    void pop_front() noexcept;
    iterator begin() noexcept;
    iterator end() noexcept;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;
    reverse_iterator rbegin() noexcept;
    reverse_iterator rend() noexcept;
    const_reverse_iterator rbegin() const noexcept;
    const_reverse_iterator rend() const noexcept;
    const_reverse_iterator crbegin() const noexcept;
    const_reverse_iterator crend() const noexcept;
    template<bool Const>
    void insert(Iterator<Const> it, const T& value);
    template<bool Const>
    void erase(Iterator<Const> it) noexcept;

    template<bool Const>
    class Iterator{
    private:
        BaseNode* mNode;
        BaseNode* mEnd;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = typename std::conditional<Const, const T*, T*>::type;
        using reference = typename std::conditional<Const, const T&, T&>::type;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit Iterator(BaseNode* newNode, BaseNode* newEnd) noexcept : mNode(newNode), mEnd(newEnd) {}
        Iterator(const Iterator& other) noexcept : mNode(other.getNode()), mEnd(other.mEnd) {}
        ~Iterator() noexcept {};
        operator Iterator<true>() const {
            const_iterator newIt(mNode, mEnd);
            return newIt;
        }

        Iterator& operator=(const Iterator& other) noexcept {
            if (this != &other) {
                mNode = other.getNode();
                mEnd = other.mEnd;
            }
            return *this;
        }

        Iterator& operator++() noexcept {
            mNode = mNode->next;
            return *this;
        }

        Iterator operator++(int) noexcept {
            List<T, Alloc>::Iterator newIt(*this);
            mNode = mNode->next;
            return newIt;
        }

        Iterator& operator--() noexcept {
            mNode = mNode->prev;
            return *this;
        }

        Iterator operator--(int) noexcept {
            List<T, Alloc>::Iterator newIt(*this);
            mNode = mNode->prev;
            return newIt;
        }

        Iterator& operator+=(int shift) noexcept {
            if (shift > 0) {
                for (int i = 0; i < shift; ++i) {
                    mNode = mNode->next;
                }
            } else {
                for (int i = 0; i < -shift; ++i) {
                    mNode = mNode->prev;
                }
            }
            return *this;
        }

        Iterator operator+(int shift) const noexcept {
            List<T, Alloc>::Iterator newIt(*this);
            newIt += shift;
            return newIt;
        }

        Iterator& operator-=(int shift) noexcept {
            *this += -shift;
            return *this;
        }

        Iterator operator-(int shift) const noexcept {
            List<T, Alloc>::Iterator newIt(*this);
            newIt -= shift;
            return newIt;
        }

        bool operator<(const Iterator& other) const noexcept {
            if (mEnd != other.mEnd) {
                return false;
            }
            List<T, Alloc>::Iterator newIt(*this);
            while (newIt.getNode() != newIt.mEnd) {
                ++newIt;
                if (newIt.getNode() == other.getNode()) {
                    return true;
                }
            }
            return false;
        }

        bool operator<=(const Iterator& other) const noexcept {
            return (*this == other || *this < other);
        }

        bool operator>(const Iterator& other) const noexcept {
            return other < *this;
        }

        bool operator>=(const Iterator& other) const noexcept {
            return (other == *this || other < *this);
        }

        bool operator==(const Iterator& other) const noexcept {
            return (mNode == other.getNode());
        }

        bool operator!=(const Iterator& other) const noexcept {
            return !(*this == other);
        }

        difference_type operator-(const Iterator& other) const noexcept {
            assert(other.mEnd != mEnd);
            if (*this == other) {
                return static_cast<difference_type>(0);
            }
            int firstLength = 0;
            int secondLength = 0;
            List<T, Alloc>::Iterator first(*this);
            List<T, Alloc>::Iterator second(other);
            while (first.getNode() != first.mEnd) {
                ++first;
                ++firstLength;
                if (first == other) {
                    return static_cast<difference_type>(firstLength);
                }
            }
            while (second.getNode() != second.mEnd) {
                ++second;
                ++secondLength;
                if (second == *this) {
                    return static_cast<difference_type>(secondLength);
                }
            }
            return static_cast<difference_type>(secondLength - firstLength);
        }

        reference operator*() const {
            return (dynamic_cast<Node*>(mNode)->mValue);
        }

        pointer operator->() const {
            return &(dynamic_cast<Node*>(mNode)->mValue);
        }

        BaseNode* getNode() const noexcept {
            return mNode;
        }
    };
};

template<typename T, typename Alloc>
Alloc List<T, Alloc>::get_allocator() noexcept {
    return mAllocForGetAllocator;
}

template<typename T, typename Alloc>
typename List<T, Alloc>::iterator List<T, Alloc>::begin() noexcept {
    return iterator(mFakeNode->next, mFakeNode);
}

template<typename T, typename Alloc>
typename List<T, Alloc>::iterator List<T, Alloc>::end() noexcept {
    return iterator(mFakeNode, mFakeNode);
}

template<typename T, typename Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::begin() const noexcept {
    return const_iterator(mFakeNode->next, mFakeNode);
}

template<typename T, typename Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::end() const noexcept {
    return const_iterator(mFakeNode, mFakeNode);
}

template<typename T, typename Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::cbegin() const noexcept {
    return const_iterator(mFakeNode->next, mFakeNode);
}

template<typename T, typename Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::cend() const noexcept {
    return const_iterator(mFakeNode, mFakeNode);
}

template<typename T, typename Alloc>
typename List<T, Alloc>::reverse_iterator List<T, Alloc>::rbegin() noexcept {
    return std::reverse_iterator(end());
}

template<typename T, typename Alloc>
typename List<T, Alloc>::reverse_iterator List<T, Alloc>::rend() noexcept {
    return std::reverse_iterator(begin());
}

template<typename T, typename Alloc>
typename List<T, Alloc>::const_reverse_iterator List<T, Alloc>::rbegin() const noexcept {
    return std::reverse_iterator(end());
}

template<typename T, typename Alloc>
typename List<T, Alloc>::const_reverse_iterator List<T, Alloc>::rend() const noexcept {
    return std::reverse_iterator(begin());
}

template<typename T, typename Alloc>
typename List<T, Alloc>::const_reverse_iterator List<T, Alloc>::crbegin() const noexcept {
    return std::reverse_iterator(cend());
}

template<typename T, typename Alloc>
typename List<T, Alloc>::const_reverse_iterator List<T, Alloc>::crend() const noexcept {
    return std::reverse_iterator(cbegin());
}

template<typename T, typename Alloc>
List<T, Alloc>::List(const Alloc& alloc): mAllocForGetAllocator(alloc), mAlloc(Allocator(alloc)), mSize(0) {
    BaseAllocator baseAllocator(mAlloc);
    mFakeNode = BaseAllocTraits::allocate(baseAllocator, 1);
    try {
        BaseAllocTraits::construct(baseAllocator, mFakeNode);
        mFakeNode->create(mFakeNode);
    }
    catch(...) {
        BaseAllocTraits::deallocate(baseAllocator, mFakeNode, 1);
        throw;
    }
}
template<typename T, typename Alloc>
List<T, Alloc>::List(int amount, const Alloc& alloc, const T* valuePtr, bool isDefaultValue) : List(alloc) {
    int count;
    BaseNode* previous = mFakeNode;
    bool isAllocated = false;
    Node* newNode = nullptr;
    try {
        for (count = 0; count < amount; ++count) {
            newNode = AllocTraits::allocate(mAlloc, 1);
            isAllocated = true;
            if (isDefaultValue) {
                AllocTraits::construct(mAlloc, newNode);
            }
            else {
                AllocTraits::construct(mAlloc, newNode, *valuePtr);
            }
            previous->next = newNode;
            newNode->prev = previous;
            previous = newNode;
            isAllocated = false;
        }
        newNode->next = mFakeNode;
        mFakeNode->prev = newNode;
    }
    catch(...) {
        if (isAllocated) {
            AllocTraits::deallocate(mAlloc, newNode, 1);
        }
        for (int i = 0; i < count; ++i) {
            newNode = dynamic_cast<Node*>(previous);
            previous = newNode->prev;
            AllocTraits::destroy(mAlloc, newNode);
            AllocTraits::deallocate(mAlloc, newNode, 1);
        }
        throw;
    }
    mSize = amount;
}

template<typename T, typename Alloc>
List<T, Alloc>::List(int amount, const Alloc& alloc): List(amount, alloc, nullptr, true) {}

template<typename T, typename Alloc>
List<T, Alloc>::List(int amount, const T& value, const Alloc& alloc): List(amount, alloc, &value, false) {}

template<typename T, typename Alloc>
List<T, Alloc>::List(const List<T, Alloc>& other):
        List(std::allocator_traits<Alloc>::select_on_container_copy_construction(other.mAllocForGetAllocator)) {
    size_t count;
    BaseNode* previous = mFakeNode;
    BaseNode* otherNode = other.mFakeNode->next;
    bool isAllocated = false;
    Node* newNode = nullptr;
    try {
        for (count = 0; count < other.mSize; ++count) {
            newNode = AllocTraits::allocate(mAlloc, 1);
            isAllocated = true;
            AllocTraits::construct(mAlloc, newNode, dynamic_cast<Node*>(otherNode)->mValue);
            previous->next = newNode;
            newNode->prev = previous;
            previous = newNode;
            otherNode = otherNode->next;
            isAllocated = false;
        }
        newNode->next = mFakeNode;
        mFakeNode->prev = newNode;
    }
    catch(...) {
        if (isAllocated) {
            AllocTraits::deallocate(mAlloc, newNode, 1);
        }
        for (size_t i = 0; i < count; ++i) {
            newNode = dynamic_cast<Node*>(previous);
            previous = newNode->prev;
            AllocTraits::destroy(mAlloc, newNode);
            AllocTraits::deallocate(mAlloc, newNode, 1);
        }
        throw;
    }
    mSize = other.mSize;
}

template<typename T, typename Alloc>
List<T, Alloc>& List<T, Alloc>::operator=(const List<T, Alloc>& other) {
    Alloc copyGetAlloc = mAllocForGetAllocator;
    Allocator copyAlloc = mAlloc;
    if (AssignSelectAlloc::value) {
        mAlloc = other.mAlloc;
        mAllocForGetAllocator = other.mAllocForGetAllocator;
    } else {
        if (mAllocForGetAllocator != other.mAllocForGetAllocator) {
            mAlloc = std::allocator_traits<Allocator>::select_on_container_copy_construction(other.mAlloc);
            mAllocForGetAllocator = std::allocator_traits<Alloc>::select_on_container_copy_construction(other.mAllocForGetAllocator);
        }
    }
    if (this == &other) {
        return *this;
    }
    BaseNode* copy = mFakeNode;
    BaseAllocator baseAllocator(mAlloc);
    BaseNode* newFake = BaseAllocTraits::allocate(baseAllocator, 1);
    try {
        BaseAllocTraits::construct(baseAllocator, newFake, BaseNode());
        newFake->create(newFake);
    }
    catch(...) {
        mAlloc = copyAlloc;
        mAllocForGetAllocator = copyGetAlloc;
        BaseAllocTraits::deallocate(baseAllocator, newFake, 1);
        throw;
    }
    mFakeNode = newFake;
    size_t count;
    bool isAllocated = false;
    Node* newNode = nullptr;
    BaseNode* otherNode = other.mFakeNode->next;
    BaseNode* previous = mFakeNode;
    try {
        for (count = 0; count < other.mSize; ++count) {
            newNode = AllocTraits::allocate(mAlloc, 1);
            isAllocated = true;
            AllocTraits::construct(mAlloc, newNode, dynamic_cast<Node*>(otherNode)->mValue);
            previous->next = newNode;
            newNode->prev = previous;
            previous = newNode;
            otherNode = otherNode->next;
            isAllocated = false;
        }
        newNode->next = mFakeNode;
        mFakeNode->prev = newNode;
    }
    catch(...) {
        if (isAllocated) {
            AllocTraits::deallocate(mAlloc, newNode, 1);
        }
        for (size_t i = 0; i < count; ++i) {
            newNode = dynamic_cast<Node*>(previous);
            previous = newNode->prev;
            AllocTraits::destroy(mAlloc, newNode);
            AllocTraits::deallocate(mAlloc, newNode, 1);
        }
        BaseAllocTraits::destroy(baseAllocator, mFakeNode);
        BaseAllocTraits::deallocate(baseAllocator, mFakeNode, 1);
        mAlloc = copyAlloc;
        mAllocForGetAllocator = copyGetAlloc;
        mFakeNode = copy;
        throw;
    }
    BaseNode* start = copy->prev;
    for (size_t i = 0; i < mSize; ++i) {
        BaseNode* next = start->prev;
        Node* deleting = dynamic_cast<Node*>(start);
        AllocTraits::destroy(copyAlloc, deleting);
        AllocTraits::deallocate(copyAlloc, deleting, 1);
        start = next;
    }
    BaseAllocator baseAlloc(copyAlloc);
    BaseAllocTraits::destroy(baseAlloc, copy);
    BaseAllocTraits::deallocate(baseAlloc, copy, 1);
    mSize = other.mSize;
    return *this;
}

template<typename T, typename Alloc>
List<T, Alloc>::~List() noexcept {
    BaseNode* node = mFakeNode->prev;
    for (size_t i = 0; i < mSize; ++i) {
        BaseNode* nextNode = node->prev;
        AllocTraits::destroy(mAlloc, dynamic_cast<Node*>(node));
        AllocTraits::deallocate(mAlloc, dynamic_cast<Node*>(node), 1);
        node = nextNode;
    }
    BaseAllocator baseAllocator(mAlloc);
    BaseAllocTraits::destroy(baseAllocator, mFakeNode);
    BaseAllocTraits::deallocate(baseAllocator, mFakeNode, 1);
}

template<typename T, typename Alloc>
size_t List<T, Alloc>::size() const noexcept {
    return mSize;
}

template<typename T, typename Alloc>
void List<T, Alloc>::push_back(const T& value) {
    Node* newNode = AllocTraits::allocate(mAlloc, 1);
    try {
        AllocTraits::construct(mAlloc, newNode, value);
    }
    catch(...) {
        AllocTraits::deallocate(mAlloc, newNode, 1);
        throw;
    }
    newNode->prev = mFakeNode->prev;
    newNode->next = mFakeNode;
    mFakeNode->prev->next = newNode;
    mFakeNode->prev = newNode;
    ++mSize;
}

template<typename T, typename Alloc>
void List<T, Alloc>::push_front(const T& value) {
    Node* newNode = AllocTraits::allocate(mAlloc, 1);
    try {
        AllocTraits::construct(mAlloc, newNode, value);
    }
    catch(...) {
        AllocTraits::deallocate(mAlloc, newNode, 1);
        throw;
    }
    newNode->next = mFakeNode->next;
    newNode->prev = mFakeNode;
    mFakeNode->next->prev = newNode;
    mFakeNode->next = newNode;
    ++mSize;
}

template<typename T, typename Alloc>
void List<T, Alloc>::pop_back() noexcept {
    BaseNode* newPrev = mFakeNode->prev->prev;
    Node* deleting = dynamic_cast<Node*>(mFakeNode->prev);
    AllocTraits::destroy(mAlloc, deleting);
    AllocTraits::deallocate(mAlloc, deleting, 1);
    newPrev->next = mFakeNode;
    mFakeNode->prev = newPrev;
    --mSize;
}

template<typename T, typename Alloc>
void List<T, Alloc>::pop_front() noexcept {
    BaseNode* newNext = mFakeNode->next->next;
    AllocTraits::destroy(mAlloc, dynamic_cast<Node*>(mFakeNode->next));
    AllocTraits::deallocate(mAlloc, dynamic_cast<Node*>(mFakeNode->next), 1);
    newNext->prev = mFakeNode;
    mFakeNode->next = newNext;
    --mSize;
}

template<typename T, typename Alloc>
template<bool Const>
void List<T, Alloc>::insert(Iterator<Const> it, const T& value) {
    if (it == end()) {
        push_back(value);
        return;
    }
    if (it == begin()) {
        push_front(value);
        return;
    }
    Node* newNode = AllocTraits::allocate(mAlloc, 1);
    try {
        AllocTraits::construct(mAlloc, newNode, value);
    }
    catch(...) {
        AllocTraits::deallocate(mAlloc, newNode, 1);
        throw;
    }
    BaseNode* prev = it.getNode()->prev;
    newNode->next = it.getNode();
    newNode->prev = prev;
    prev->next = newNode;
    it.getNode()->prev = newNode;
    ++mSize;
}

template<typename T, typename Alloc>
template<bool Const>
void List<T, Alloc>::erase(Iterator<Const> it) noexcept {
    if (it == end()) {
        return;
    }
    if (it + 1 == end()) {
        pop_back();
        return;
    }
    if (it == begin()) {
        pop_front();
        return;
    }
    BaseNode *next = it.getNode()->next;
    BaseNode *prev = it.getNode()->prev;
    AllocTraits::destroy(mAlloc, dynamic_cast<Node*>(it.getNode()));
    AllocTraits::deallocate(mAlloc, dynamic_cast<Node*>(it.getNode()), 1);
    prev->next = next;
    next->prev = prev;
    --mSize;
}
