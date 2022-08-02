#pragma once

template <typename T>
class BotMutable
{
    T* m_value;
public:
    BotMutable(T* value)
        : m_value(value)
    {}

    void set(T value)
    {
        *m_value = value;
    }

    T get()
    {
        return *m_value;
    }
};