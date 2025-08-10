#pragma once

template <typename T>
constexpr bool IsSigned();

template <char> constexpr bool IsSigned() { return true; };
template <short> constexpr bool IsSigned() { return true; };
template <int> constexpr bool IsSigned() { return true; };
template <long> constexpr bool IsSigned() { return true; };
template <long long> constexpr bool IsSigned() { return true; };
template <unsigned char> constexpr bool IsSigned() { return false; };
template <unsigned short> constexpr bool IsSigned() { return false; };
template <unsigned int> constexpr bool IsSigned() { return false; };
template <unsigned long> constexpr bool IsSigned() { return false; };
template <unsigned long long> constexpr bool IsSigned() { return false; };


template<typename T>
class MakeUnsigned{
    public:
        typedef T type;
};

template<>
class MakeUnsigned<char>{
    public:
        typedef unsigned char type;
};

template<>
class MakeUnsigned<short>{
    public:
        typedef unsigned short type;
};

template<>
class MakeUnsigned<int>{
    public:
        typedef unsigned int type;
};

template<>
class MakeUnsigned<long>{
    public:
        typedef unsigned long type;
};

template<>
class MakeUnsigned<long long>{
    public:
        typedef unsigned long long type;
};