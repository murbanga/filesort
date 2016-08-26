#include <vector>

using namespace std;

template <class T>
class ReadOnlyAllocator: public allocator<T>
{
    template <class U> friend class ReadOnlyAllocator;
public:
    typedef T value_type;
    typedef size_t size_type;
    typedef T *pointer;
    typedef const T *const_pointer;

    template<typename T2>
    struct rebind
    {
        typedef ReadOnlyAllocator<T2> other;
    };
    
    ReadOnlyAllocator(void * p, size_t size):p(p), size(size){}

    template <class U>
    ReadOnlyAllocator(const ReadOnlyAllocator<U> &a):p(a.p), size(a.size){}

    pointer allocate(size_type n)
    {
        if(n*sizeof(T) != size){
            //throw bad_alloc();
            pointer p = (pointer)malloc(n*sizeof(T));
            if(!p)throw bad_alloc();
            return p;
        }
        return (pointer)p;
    }

    void deallocate(pointer q, size_type n)
    {
        if(p != q)free(q);
    }

private:
    void * p;
    size_t size;
};

int _tmain(int argc, _TCHAR* argv[])
{
    size_t size = 256;
    float *p = new float[size];

    {
        vector<float, ReadOnlyAllocator<float> > v(size, 0, ReadOnlyAllocator<float>(p, size*sizeof(float)));
    }
    delete[] p;
    return 0;
}
