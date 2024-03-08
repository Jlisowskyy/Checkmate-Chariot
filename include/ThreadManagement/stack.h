//
// Created by Jlisowskyy on 3/7/24.
//

#ifndef STACK_H
#define STACK_H

#include <cstdlib>

/*          Important notes:
 *  Performs all operations without boundry checks nor any sanity checks
 *  should be use wisely especially in situation that we are sure no
 *  overflowwing situation will happen
 */

template<
    class ItemT,
    size_t StackSize
>struct stack{
    // ------------------------------
    // Class inner types
    // ------------------------------

    struct stackPayload {
        ItemT* data;
        size_t size;

        void Push(stack& s, ItemT item) {
            s.Push(item);
            ++size;
        }


        ItemT operator[](size_t ind) const
        {
            return data[ind];
        }
    };

    struct stackPayloadConst {
        stackPayloadConst(ItemT* d, size_t s): data(d), size(d) {}

        ItemT* data;
        const size_t size;

        ItemT operator[](size_t ind) const
        {
            return data[ind];
        }
    };

    // ------------------------------
    // Class creation
    // ------------------------------

    stack(): _data(static_cast<ItemT *>(malloc(sizeof(ItemT) * StackSize))) {}
    ~stack() { free(_data); }

    stack(const stack&) = delete;
    stack(stack&&) = delete;

    stack& operator=(const stack&) = delete;
    stack& operator=(stack&&) = delete;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void Push(const ItemT item) {
        _data[_last++] = item;
    }

    stackPayloadConst PushAggregate(const size_t size) {
        ItemT* ret = _data + _last;
        _last += size;

        return stackPayloadConst(ret, size);
    }

    stackPayload GetPayload() {
        return {_data + _last, 0};
    }

    void PopAggregate(const stackPayloadConst payload) {
        _last -= payload.size;
    }

    void PopAggregate(const stackPayload payload) {
        _last -= payload.size;
    }

    ItemT Pop() {
        return _data[--_last];
    }

    ItemT Top() {
        return _data[_last-1];
    }

    void Clear() {
        _last = 0;
    }

    // ------------------------------
    // Private class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------

private:
    size_t _last{};
    ItemT* _data;
};

#endif //STACK_H
