#include <vector>

using namespace std;

class Mul{ };
class Add{ };

template <class T, class U, class Op> struct OpProxy;

template <class T>
class myvector
{
    template <class X> friend class myvector;

public:
    // vector = scalar * vector
    template <class X, class Y> 
    myvector<T>& operator = (const OpProxy<X, myvector<Y>, Mul> &proxy){ return this->operator=(OpProxy<myvector<Y>,X,Mul>(proxy.b, proxy.a)); }

    // vector = vector * scalar
    template <class X, class Y>
    myvector<T>& operator = (const OpProxy<myvector<X>, Y, Mul> &proxy)
    {
        v.resize(proxy.a.v.size());
        for(size_t i = 0; i < v.size(); ++i)
            v[i] = proxy.a.v[i]*proxy.b;

        return *this;
    }

    // vector = vector + vector
    template <class X, class Y>
    myvector<T>& operator = (const OpProxy<myvector<X>, myvector<Y>, Add> &proxy)
    {
        v.resize(proxy.a.v.size());
        for(size_t i = 0; i < v.size(); ++i)
            v[i] = proxy.a.v[i] + proxy.b.v[i];

        return *this;
    }

    // vector = vector * scalar + vector
    // general implementation
    template <class A, class B, class C>
    myvector<T>& operator = (const OpProxy<myvector<C>, OpProxy<myvector<A>, B, Mul>, Add> &proxy);

        template <class A, class B, class C>
    myvector<T>& operator = (const OpProxy<myvector<C>, OpProxy<B, myvector<A>, Mul>, Add> &proxy);

    template <class A, class B, class C>
    myvector<T>& operator = (const OpProxy<OpProxy<B, myvector<A>, Mul>, myvector<C>, Add> &proxy);

    template <class A, class B, class C>
    myvector<T>& operator = (const OpProxy<OpProxy<myvector<A>, B, Mul>, myvector<C>, Add> &proxy)
    {
        v.resize(proxy.a.a.v.size());
        for(size_t i = 0; i < v.size(); ++i)
            v[i] = proxy.a.a.v[i]*proxy.a.b + proxy.b.v[i];
        return *this;
    }

private:
    vector<T> v;
};

template <class T, class U, class Op>
struct OpProxy
{
    OpProxy(const T &a, const U &b):a(a), b(b){}
    OpProxy(const U &b, const T &a):a(a), b(b){}

    const T &a;
    const U &b;
};

// example of specific fast implementation
extern void ippsAddProduct_32f(float *dst, float a, const float *src, size_t n);

template <>
template <>
myvector<float>& myvector<float>::operator = (const OpProxy<OpProxy<myvector<float>, float, Mul>, myvector<float>, Add> &proxy)
{
    size_t n = proxy.a.a.v.size();
    v = proxy.a.a.v;
    
    ippsAddProduct_32f(v.data(), proxy.a.b, proxy.b.v.data(), n);

    return *this;
}

template <class T, class U>
OpProxy<T, U, Mul> operator * (const T &a, const U &b){ return OpProxy<T, U, Mul>(a, b); }

template <class T, class U>
OpProxy<T, U, Add> operator + (const T &a, const U &b){ return OpProxy<T, U, Add>(a, b); }

template <class X, class Y>
class A
{
public:
    A();
    A(const X &x, const Y &y);
    X x;
    Y y;
};

template <class X, class Y>
void foo(A<X, Y> &a);

int _tmain(int argc, _TCHAR* argv[])
{
    myvector<int> a;
    myvector<float> b;
    myvector<float> c;
    b = 2.0f*a;
    c = b + a;
    myvector<int> x;
    x = c*5 + b;

    b = c*0.001f + b;
    c = b + c*0.002f;

    /*A<int, float> cat;
    A<float, int> dog;
    foo(cat);
    foo(dog);*/
    return 0;
}
