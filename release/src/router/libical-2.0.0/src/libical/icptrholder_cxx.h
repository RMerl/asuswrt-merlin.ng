/**
 * @file    icptrholder_cxx.h
 * @author  wyau (08/29/02)
 * @brief   C++ template classes for managing C++ pointers returned by
 *          VComponent::get_..._component, VComponent::get_..._property,
 *          ICalProperty::get_..._value.
 *
 * @remarks VComponent::get... functions returns a C++ oject that wraps the
 * libical implementation. It is important to note that the wrapped
 * implementation still belongs to the original component. To stop memory leak,
 * caller must delete the pointer. However, the destructor will call the
 * appropriate free function. eg. ~VComponent calls icalcomponent_free(imp).
 *
 * As stated previously, imp stil belongs to the original component. To avoid
 * freeing the wrapped "imp", caller must set the "imp" to null before deleting
 * the pointer.
 *
 * The template class relieves the burden of memory management when used as a
 * stack based object.  The class holds a pointer to the C++ Wrapper.
 * The destructor set the imp to null before deleting the pointer.
 *
 * Each C++ Wrapper instantiates a template class in it's corresponding .h file.
 *
 * Usage example:
 *   VComponentTmpPtr p;// VComponentTmpPtr is an instantiation of this template
 *   for (p=component.get_first_component; p!= 0; p=component.get_next_component) {
 *
 * (C) COPYRIGHT 2001, Critical Path

 This program is free software; you can redistribute it and/or modify
 it under the terms of either:

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/
 */

#ifndef ICPTRHOLDER_CXX_H
#define ICPTRHOLDER_CXX_H

#include <cassert>

template < class T > class ICPointerHolder {
  public:
    ICPointerHolder()
      : ptr(0)
    {
    }

    ICPointerHolder(T *p)
      : ptr(p)
    {
    }

    // copy constructor to support assignment
    ICPointerHolder(const ICPointerHolder &ip)
      : ptr(ip.ptr)
    {
        // We need to transfer ownership of ptr to this object by setting
        // ip's ptr to null. Otherwise, ptr will de deleted twice.
        // const ugliness requires us to do the const_cast.
        ICPointerHolder *ipp = const_cast < ICPointerHolder * >(&ip);

        ipp->ptr = 0;
    };

    ~ICPointerHolder()
    {
        release();
    }

    ICPointerHolder & operator=(T *p)
    {
        this->release();
        ptr = p;
        return *this;
    }

    ICPointerHolder &operator=(ICPointerHolder &p)
    {
        this->release();
        ptr = p.ptr;    // this transfer ownership of the pointer
        p.ptr = 0;      // set it to null so the pointer won't get delete twice.
        return *this;
    }

    int operator!=(T *p)
    {
        return (ptr != p);
    }

    int operator==(T *p)
    {
        return (ptr == p);
    }

    operator  T *() const
    {
        return ptr;
    }

    T *operator->() const
    {
        assert(ptr);
        return ptr;
    }

    T &operator*()
    {
        assert(ptr);
        return *ptr;
    }

  private:
    void release()
    {
        if (ptr != 0) {
            ptr->detach();
            delete ptr;

            ptr = 0;
        }
    }

    T *ptr;
};

#endif
