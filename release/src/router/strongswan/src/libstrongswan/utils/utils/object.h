/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup object_i object
 * @{ @ingroup utils_i
 */

#ifndef OBJECT_H_
#define OBJECT_H_

/**
 * Call destructor of an object, if object != NULL
 */
#define DESTROY_IF(obj) if (obj) (obj)->destroy(obj)

/**
 * Call offset destructor of an object, if object != NULL
 */
#define DESTROY_OFFSET_IF(obj, offset) if (obj) obj->destroy_offset(obj, offset);

/**
 * Call function destructor of an object, if object != NULL
 */
#define DESTROY_FUNCTION_IF(obj, fn) if (obj) obj->destroy_function(obj, fn);

/**
 * Object allocation/initialization macro, using designated initializer.
 */
#define INIT(this, ...) { (this) = malloc(sizeof(*(this))); \
						   *(this) = (typeof(*(this))){ __VA_ARGS__ }; }

/**
 * Aligning version of INIT().
 *
 * The returned pointer must be freed using free_align(), not free().
 *
 * @param this		object to allocate/initialize
 * @param align		alignment for allocation, in bytes
 * @param ...		initializer
 */
#define INIT_ALIGN(this, align, ...) { \
						(this) = malloc_align(sizeof(*(this)), align); \
						*(this) = (typeof(*(this))){ __VA_ARGS__ }; }

/**
 * Object allocation/initialization macro, with extra allocated bytes at tail.
 *
 * The extra space gets zero-initialized.
 *
 * @param this		pointer to object to allocate memory for
 * @param extra		number of bytes to allocate at end of this
 * @param ...		initializer
 */
#define INIT_EXTRA(this, extra, ...) { \
						typeof(extra) _extra = (extra); \
						(this) = malloc(sizeof(*(this)) + _extra); \
						*(this) = (typeof(*(this))){ __VA_ARGS__ }; \
						memset((this) + 1, 0, _extra); }

/**
 * Aligning version of INIT_EXTRA().
 *
 * The returned pointer must be freed using free_align(), not free().
 *
 * @param this		object to allocate/initialize
 * @param extra		number of bytes to allocate at end of this
 * @param align		alignment for allocation, in bytes
 * @param ...		initializer
 */
#define INIT_EXTRA_ALIGN(this, extra, align, ...) { \
						typeof(extra) _extra = (extra); \
						(this) = malloc_align(sizeof(*(this)) + _extra, align); \
						*(this) = (typeof(*(this))){ __VA_ARGS__ }; \
						memset((this) + 1, 0, _extra); }

/**
 * Method declaration/definition macro, providing private and public interface.
 *
 * Defines a method name with this as first parameter and a return value ret,
 * and an alias for this method with a _ prefix, having the this argument
 * safely casted to the public interface iface.
 * _name is provided a function pointer, but will get optimized out by GCC.
 */
#define METHOD(iface, name, ret, this, ...) \
	static ret name(union {iface *_public; this;} \
	__attribute__((transparent_union)), ##__VA_ARGS__); \
	static typeof(name) *_##name = (typeof(name)*)name; \
	static ret name(this, ##__VA_ARGS__)

/**
 * Same as METHOD(), but is defined for two public interfaces.
 */
#define METHOD2(iface1, iface2, name, ret, this, ...) \
	static ret name(union {iface1 *_public1; iface2 *_public2; this;} \
	__attribute__((transparent_union)), ##__VA_ARGS__); \
	static typeof(name) *_##name = (typeof(name)*)name; \
	static ret name(this, ##__VA_ARGS__)

/**
 * Callback declaration/definition macro, allowing casted first parameter.
 *
 * This is very similar to METHOD, but instead of casting the first parameter
 * to a public interface, it uses a void*. This allows type safe definition
 * of a callback function, while using the real type for the first parameter.
 */
#define CALLBACK(name, ret, param1, ...) \
	static ret _cb_##name(union {void *_generic; param1;} \
	__attribute__((transparent_union)), ##__VA_ARGS__); \
	static typeof(_cb_##name) *name = (typeof(_cb_##name)*)_cb_##name; \
	static ret _cb_##name(param1, ##__VA_ARGS__)

#endif /** OBJECT_H_ @} */
