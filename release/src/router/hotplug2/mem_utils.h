/*****************************************************************************\
*  _  _       _          _              ___                                   *
* | || | ___ | |_  _ __ | | _  _  __ _ |_  )                                  *
* | __ |/ _ \|  _|| '_ \| || || |/ _` | / /                                   *
* |_||_|\___/ \__|| .__/|_| \_,_|\__, |/___|                                  *
*                 |_|            |___/                                        *
\*****************************************************************************/

#ifndef MEM_UTILS_H
#define MEM_UTILS_H 1


#ifdef HND_ROUTER
#define inline  
#elif __GNUC__ >= 5
#define inline
#endif

inline void *xmalloc(size_t);
inline void *xrealloc(void *, size_t);
#endif
