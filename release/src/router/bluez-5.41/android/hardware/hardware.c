/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <hardware/hardware.h>

#include <dlfcn.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>

#define LOG_TAG "HAL"

#define LOG_INFO " I"
#define LOG_WARN " W"
#define LOG_ERROR " E"
#define LOG_DEBUG " D"
#define ALOG(pri, tag, fmt, arg...) fprintf(stderr, tag pri": " fmt"\n", ##arg)

#define info(fmt, arg...) ALOG(LOG_INFO, LOG_TAG, fmt, ##arg)
#define warn(fmt, arg...) ALOG(LOG_WARN, LOG_TAG, fmt, ##arg)
#define error(fmt, arg...) ALOG(LOG_ERROR, LOG_TAG, fmt, ##arg)

/**
 * Load the file defined by the variant and if successful
 * return the dlopen handle and the hmi.
 * @return 0 = success, !0 = failure.
 */
static int load(const char *id,
        const char *path,
        const struct hw_module_t **pHmi)
{
    int status;
    void *handle;
    struct hw_module_t *hmi;
    const char *sym = HAL_MODULE_INFO_SYM_AS_STR;

    /*
     * load the symbols resolving undefined symbols before
     * dlopen returns. Since RTLD_GLOBAL is not or'd in with
     * RTLD_NOW the external symbols will not be global
     */
    handle = dlopen(path, RTLD_NOW);
    if (handle == NULL) {
        char const *err_str = dlerror();
        error("load: module=%s\n%s", path, err_str?err_str:"unknown");
        status = -EINVAL;
        goto done;
    }

    /* Get the address of the struct hal_module_info. */
    hmi = (struct hw_module_t *)dlsym(handle, sym);
    if (hmi == NULL) {
        error("load: couldn't find symbol %s", sym);
        status = -EINVAL;
        goto done;
    }

    /* Check that the id matches */
    if (strcmp(id, hmi->id) != 0) {
        error("load: id=%s != hmi->id=%s", id, hmi->id);
        status = -EINVAL;
        goto done;
    }

    hmi->dso = handle;

    *pHmi = hmi;

    info("loaded HAL id=%s path=%s hmi=%p handle=%p",
                id, path, *pHmi, handle);

    return 0;

done:
    hmi = NULL;
    if (handle != NULL) {
        dlclose(handle);
        handle = NULL;
    }

    return status;
}

int hw_get_module_by_class(const char *class_id, const char *inst,
                           const struct hw_module_t **module)
{
    char path[PATH_MAX];
    char name[PATH_MAX];

    if (inst)
        snprintf(name, PATH_MAX, "%s.%s", class_id, inst);
    else
        snprintf(name, PATH_MAX, "%s", class_id);

    /*
     * Here we rely on the fact that calling dlopen multiple times on
     * the same .so will simply increment a refcount (and not load
     * a new copy of the library).
     * We also assume that dlopen() is thread-safe.
     */
    snprintf(path, sizeof(path), "%s/%s.default.so", PLUGINDIR, name);

    return load(class_id, path, module);
}

int hw_get_module(const char *id, const struct hw_module_t **module)
{
    return hw_get_module_by_class(id, NULL, module);
}
