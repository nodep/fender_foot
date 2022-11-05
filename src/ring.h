#pragma once

template <class T, uint8_t Capacity>
class ring
{
private:
    T store[Capacity];
    uint8_t head = 0;
    uint8_t tail = 0;

public:

    uint8_t capacity()
    {
        return Capacity - 1;
    }

    uint8_t size()
    {
        if (head < tail)
            return Capacity + head - tail;

        return head - tail;
    }

    void push(T c)
    {
        store[head] = c;
        head = (head + 1) % Capacity;
    }

    T pop()
    {
        T ret_val = store[tail];
        tail = (tail + 1) % Capacity;

        return ret_val;
    }

    T peek()
    {
        return store[tail];
    }

    bool is_full()
    {
        return ((head + 1) % Capacity) == tail;
    }

    bool empty()
    {
        return head == tail;
    }

    void clear()
    {
        head = tail = 0;
    }
};
