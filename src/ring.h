#pragma once

// simple ring buffer for POD types
template <class T, uint8_t Capacity>
class ring
{
private:
	static_assert(Capacity < 0x100);

    T store[Capacity];
    uint8_t head = 0;
    uint8_t tail = 0;

public:

    uint8_t capacity() const
    {
        return Capacity - 1;
    }

    uint8_t size() const
    {
        if (head < tail)
            return Capacity + head - tail;

        return head - tail;
    }

    void push(T c)
    {
        store[head] = c;
        head = static_cast<uint8_t>((head + 1) % Capacity);
    }

    bool safe_push(T c)
    {
        if (full())
            return false;

        push(c);
        return true;
    }

    T pop()
    {
        T ret_val = store[tail];
        tail = static_cast<uint8_t>((tail + 1) % Capacity);

        return ret_val;
    }

    bool safe_pop(T& c)
    {
        if (empty())
            return false;

        c = pop();
        return true;
    }

    T peek() const
    {
        return store[tail];
    }

    bool full() const
    {
        return ((head + 1) % Capacity) == tail;
    }

    bool empty() const
    {
        return head == tail;
    }

    void clear()
    {
        head = tail = 0;
    }
};
