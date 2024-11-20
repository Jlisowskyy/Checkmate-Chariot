//
// Created by Jlisowskyy on 3/7/24.
//

#ifndef STACK_H
#define STACK_H

#include <cstdlib>

#include "../EngineUtils.h"

/*
 *              Important notes:
 *  Performs all operations without boundary checks nor any sanity checks
 *  should be used wisely especially in situation that we are sure no
 *  overflowing situation will happen
 *
 *  Its main use case is to provide reliable and fast Stack implementation, allocate once used consistently during
 *  a thread lifetime. Also allows simple thread cancelling mechanism to work without any worries about memory leaks.
 *  It is not thread safe, and it should not be.
 */

template <class ItemT, size_t StackSize> struct Stack
{
    // ------------------------------
    // Class inner types
    // ------------------------------

    /* This structure is used to provide simple interface allowing popping entire payload at once
     * and also simplifies pushing mechanism.
     * It does not provide stack partitioning mechanism (creating substacks in stack)
     * it simply pushed items to the main stack, saving its size and allowing to pop them all at once.
     * */

    struct StackPayload
    {
        ItemT *data;
        size_t size;

        void Push(Stack &s, ItemT item)
        {
            s.Push(item);
            ++size;
        }

        const ItemT &operator[](size_t ind) const { return data[ind]; }
        ItemT &operator[](size_t ind) { return data[ind]; }
    };

    // ------------------------------
    // Class creation
    // ------------------------------

    Stack() : _data(static_cast<ItemT *>(AlignedAlloc(sizeof(ItemT), sizeof(ItemT) * StackSize))) {}
    ~Stack() { AlignedFree(_data); }

    Stack(const Stack &) = delete;
    Stack(Stack &&)      = delete;

    Stack &operator=(const Stack &) = delete;
    Stack &operator=(Stack &&)      = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void Push(const ItemT item) { _data[_last++] = item; }

    StackPayload GetPayload() { return {_data + _last, 0}; }

    void PopAggregate(const StackPayload payload) { _last -= payload.size; }

    ItemT Pop() { return _data[--_last]; }

    ItemT Top() { return _data[_last - 1]; }

    void Clear() { _last = 0; }

    [[nodiscard]] size_t Size() const { return _last; }
    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    size_t _last{};
    ItemT *_data;
};

#endif // STACK_H
