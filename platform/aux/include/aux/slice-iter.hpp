
// According to Stroustroup book

#ifndef SLICE_ITER_H_
#define SLICE_ITER_H_

#include <valarray>

namespace aux
{

template<typename T>
class slice_iter
{
    protected:
        std::valarray<T> *v;
        std::slice s;
        size_t curr;    //index of the current element

        T& ref(size_t i) const { return v->operator[](s.start()+i*s.stride()); }

    public:
        slice_iter(std::valarray<T> *vv, std::slice ss) : v(vv), s(ss), curr(0) {};
        slice_iter<T>& operator++() { curr++; return *this; }

        T& operator[](size_t i) { return ref(curr=i); }  //C's style indexing
        T& operator()(size_t i) { return ref(curr=i); }  //Fortran's style indexing
        T& operator*() { return ref(curr); }             //Current element

};

}

#endif /*SLICE_ITER_H_*/
