#ifndef _GL_SCRATCH_BUFFER_H
#define _GL_SCRATCH_BUFFER_H

#include <libc-config.h>

#define __libc_scratch_buffer_grow gl_scratch_buffer_grow
#define __libc_scratch_buffer_grow_preserve gl_scratch_buffer_grow_preserve
#define __libc_scratch_buffer_set_array_size gl_scratch_buffer_set_array_size
#include <malloc/scratch_buffer.h>

#endif /* _GL_SCRATCH_BUFFER_H */
