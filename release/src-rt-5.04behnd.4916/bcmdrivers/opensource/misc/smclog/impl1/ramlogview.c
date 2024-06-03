/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2022 Broadcom Ltd.
 */
/*
   <:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
   All Rights Reserved

   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.

   :>
   */

/****************************************************************************
 *
 * Filename:    ramlogview.c
 *
 ****************************************************************************
 *
 * Description: ramlog view kernel driver
 *
 ****************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/fsnotify.h>
#include <asm/errno.h>
#include <linux/version.h>

#include <linux/of_device.h>
#include <linux/platform_device.h>

#include "ramlogrpc.h"

#define	MAX_FORMATTED_LENGTH		1024
#define	MAX_TOKEN_LENGTH		63
#define	MAX_RAMLOG_LINE_LENGTH		256
#define	RAMLOG_PAGE_SIZE		4096
#define	DEF_RAMLOG_PARAMETER_LENGTH	1024
#define	RAMLOG_DEVICE_NAME		"smclog"
#define	RAMLOG_PROCFS_DIR_NAME		"driver/smc"
#define	RAMLOG_PROCFS_NAME		"log"

#define	BYTES_PER_LINE			32

#define	GET_TIMESTAMP_DIFF(left, right)	\
    (((int32_t) (left)) - ((int32_t) (right)))

struct ramlog_file_buffer_t {
    struct file *filep;
    struct device *dev;
    struct ramlog_rpc_data *ramlog;
    uint32_t buf_size;
    uint8_t *buf_ptr;
    dma_addr_t dma_addr;
    uint32_t fifo_index, fifo_count;
    uint32_t text_length;
    char text_buffer[MAX_FORMATTED_LENGTH + 1];
    struct semaphore sem;
};

static int ramlog_open(struct inode *inodep, struct file *filep);
static int ramlog_release(struct inode *inodep, struct file *filep);
static ssize_t ramlog_read(struct file *filep, char *buffer, size_t count,
    loff_t *offp);
static ssize_t ramlog_write(struct file *filep, const char *buffer,
    size_t count, loff_t *offp);
static unsigned int ramlog_poll(struct file *filep,
    struct poll_table_struct *wait);

static int ramlog_probe(struct platform_device *pdev);
static int ramlog_remove(struct platform_device *pdev);

static uint8_t *alloc_ramlog_buffer (struct ramlog_file_buffer_t *ramlog_view,
    unsigned int buf_size);
static void execute_ramlog_command(struct ramlog_file_buffer_t *ramlog_view,
    const char *command);
static const char *get_ramlog_token(char *token, unsigned int max_token_size,
    const char *str_to_parse);

static const struct of_device_id ramlog_of_match[] = {
    { .compatible = "brcm,smcramlog-rpc", },
    { /* end of the list */ }
};

static struct platform_driver ramlog_platform_driver = {
    .driver = {
        .name           = RAMLOG_DEVICE_NAME,
        .owner          = THIS_MODULE,
        .of_match_table = ramlog_of_match,
    },
    .probe  = ramlog_probe,
    .remove = ramlog_remove,
};

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5,15,0))
static const struct proc_ops ramlog_file_fops = {
    .proc_open    = ramlog_open,
    .proc_release = ramlog_release,
    .proc_read    = ramlog_read,
    .proc_write   = ramlog_write,
    .proc_poll    = ramlog_poll,
};
#else
static const struct file_operations ramlog_file_fops = {
    .owner   = THIS_MODULE,
    .open    = ramlog_open,
    .release = ramlog_release,
    .read    = ramlog_read,
    .write   = ramlog_write,
    .poll    = ramlog_poll,
};
#endif

struct ramlog_rpc_data *smc_ramlog;
EXPORT_SYMBOL(smc_ramlog);

static struct proc_dir_entry *ramlog_proc_entry;
static struct proc_dir_entry *ramlog_proc_parent;
static DECLARE_WAIT_QUEUE_HEAD(ramlog_wait);

static const char *const help_msgs[] = {
    "\r\nTo dump encrypted SMC ramlog:\r\n",
    "    cat /proc/driver/smc/log\r\n",
    "To clear ramlog:\r\n",
    "    echo \"clear\" >/proc/driver/smc/log\r\n",
    "To add a message to ramlog:\r\n",
    "    echo \"<message>\" >/proc/driver/smc/log\r\n",
    "To add a message to ramlog at a specific level:\r\n",
    "    echo \"(error|warning|notice|message|debug|deep):\r\n",
    "        <message>\" >/proc/driver/smc/log\r\n",
    "To change level:\r\n",
    "    echo \"level=(error|warning|notice|msg|debug|deep)\"\r\n",
    "        >/proc/driver/smc/log\r\n",
    "To change level for a specific source:\r\n",
    "    echo \"level=(error|warning|notice|msg|debug|deep)\r\n",
    "        source=<source name>\" >/proc/driver/smc/log\r\n",
    "To clear all custom levels:\r\n",
    "    echo \"level=(error|warning|notice|msg|debug|deep)\r\n",
    "        source=*\" >/proc/driver/smc/log\r\n\r\n"
};

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hadula, Tomasz");
MODULE_DESCRIPTION("SMC ramlog viewer");
MODULE_VERSION("1.0");

static int ramlog_open(struct inode *inodep, struct file *filep)
{
    struct ramlog_file_buffer_t *ramlog_view;

    (void) inodep;

    if (!filep)
        return -EBADF;

    ramlog_view = (struct ramlog_file_buffer_t *)
        kmalloc(sizeof(struct ramlog_file_buffer_t), GFP_KERNEL);
    if (ramlog_view) {
        memset(ramlog_view, 0, sizeof(struct ramlog_file_buffer_t));
        ramlog_view->fifo_index = RAMLOG_HEAD_START_INDEX;
        sema_init(&ramlog_view->sem, 1);
        ramlog_view->filep = filep;
        filep->private_data = (void *) ramlog_view;
        ramlog_view->ramlog = smc_ramlog;
        ramlog_view->dev = smc_ramlog->dev;
    } else
        return -EINVAL;
    return 0;
}

static int ramlog_release(struct inode *inodep, struct file *filep)
{
    struct ramlog_file_buffer_t *ramlog_view;

    (void) inodep;

    if (!filep)
        return -EBADF;

    if (filep->private_data) {
        ramlog_view = (struct ramlog_file_buffer_t *)
            filep->private_data;
        if (ramlog_view->buf_size) {
            dma_unmap_single(ramlog_view->dev,
                ramlog_view->dma_addr,
                ramlog_view->buf_size, DMA_BIDIRECTIONAL);
            kfree(ramlog_view->buf_ptr);
        }
        ramlog_view->buf_size = 0;
        ramlog_view->buf_ptr = 0;
        ramlog_view->dma_addr = 0;
        ramlog_view->filep = 0;
        kfree(filep->private_data);
        filep->private_data = NULL;
    }
    return 0;
}

static ssize_t ramlog_read(struct file *filep, char *buffer, size_t count,
    loff_t *offp)
{
    struct ramlog_file_buffer_t *ramlog_view;
    struct smc_ramlog_hdr *header;
    uint32_t i, j, k, buf_size, data_index, record_count;
    uint32_t line_index, column_index, line_length, total_length;
    uint8_t value, digit;
    loff_t offset;
    int result;

    if (!filep)
        return -EBADF;

    ramlog_view = (struct ramlog_file_buffer_t *) filep->private_data;
    if (!ramlog_view) {
        pr_err("ramlog read buffer not allocated\n");
        return -EBADF;
    }
    result = down_interruptible(&ramlog_view->sem);
    if (result < 0) {
        dev_err(ramlog_view->dev, "Cannot enter mutex, result=%d\n", result);
        return result;
    }

    if (offp) {
        offset = *offp;
        if (offset < 0)
            offset = 0;
    } else
        offset = 0;

    if (!ramlog_view->text_length) {
        record_count = get_ramlog_count(ramlog_view->ramlog);
        buf_size = ALIGN((MAX_RAMLOG_LINE_LENGTH * record_count +
            RAMLOG_PAGE_SIZE), RAMLOG_PAGE_SIZE);
#ifdef	SMC_RAMLOG_VERBOSE
        dev_info(ramlog_view->dev, "ramlog count=%u\n",
            (unsigned int) record_count);
#endif	/* SMC_RAMLOG_VERBOSE */
        if (buf_size > RAMLOG_MSG_BUFFER_SIZE)
            buf_size = RAMLOG_MSG_BUFFER_SIZE;
        header = (struct smc_ramlog_hdr *)
            alloc_ramlog_buffer(ramlog_view, buf_size);
        if (!header)
            return -ENOMEM;
#ifdef	SMC_RAMLOG_VERBOSE
        dev_info(ramlog_view->dev, "buffer=%#llx\n",
            (unsigned long long) ramlog_view->dma_addr);
#endif	/* SMC_RAMLOG_VERBOSE */
        memset(header, 0, sizeof(struct smc_ramlog_hdr));
        ramlog_view->fifo_index = get_encrypted_ramlog_entries(
            ramlog_view->dma_addr, ramlog_view->buf_size,
            ramlog_view->ramlog, &ramlog_view->fifo_count, NULL,
            ramlog_view->fifo_index);
        total_length = header->length + sizeof(struct smc_ramlog_hdr);
        ramlog_view->text_length = ((total_length << 1) +
            ((total_length + BYTES_PER_LINE - 1) /
            BYTES_PER_LINE));
#ifdef	SMC_RAMLOG_VERBOSE
        dev_info(ramlog_view->dev, "text length=%u header.length=%u\n",
            (unsigned int) ramlog_view->text_length,
            (unsigned int) header->length);
#endif	/* SMC_RAMLOG_VERBOSE */
    }
#ifdef	SMC_RAMLOG_VERBOSE
    dev_info(ramlog_view->dev, "offset=%lu count=%u\n", (unsigned long) offset,
        (unsigned int) count);
#endif	/* SMC_RAMLOG_VERBOSE */
    for (i = 0, j = 0, k = 0;
        i < count && offset < (loff_t) ramlog_view->text_length;) {
        line_index = (uint32_t) (offset / (2 * BYTES_PER_LINE + 1));
        column_index = (uint32_t) (offset % (2 * BYTES_PER_LINE + 1));
        if ((line_index + 1) * (2 * BYTES_PER_LINE + 1) >=
            ramlog_view->text_length)
            line_length = ramlog_view->text_length -
                line_index * (2 * BYTES_PER_LINE + 1);
        else
            line_length = (2 * BYTES_PER_LINE + 1);
#ifdef	SMC_RAMLOG_VERBOSE
        dev_info(ramlog_view->dev,
            "i=%u j=%u k=%u line=%u column=%u line-length=%u\n",
            (unsigned int) i, (unsigned int) j,
            (unsigned int) k, (unsigned int) line_index,
            (unsigned int) column_index,
            (unsigned int) line_length);
#endif	/* SMC_RAMLOG_VERBOSE */
        if (column_index >= line_length - 1) {
            ramlog_view->text_buffer[j++] = '\n';
            if (j >= sizeof(ramlog_view->text_buffer) - 1) {
                result = copy_to_user(&buffer[k],
                    ramlog_view->text_buffer, j);
                if (result < 0) {
                    dev_err(ramlog_view->dev, "Ramlog not copied\n");
                    return result;
                }
#ifdef	SMC_RAMLOG_VERBOSE
                ramlog_view->text_buffer[j] = '\0';
                dev_info(ramlog_view->dev, "k=%u buffer=\"%s\"\n",
                    (unsigned int) k,
                    ramlog_view->text_buffer);
#endif	/* SMC_RAMLOG_VERBOSE */
                k += j;
                j = 0;
            }
            ++offset;
            ++i;
        } else {
            data_index = ((line_index * (2 * BYTES_PER_LINE) +
                column_index) >> 1);
            value = ramlog_view->buf_ptr[data_index];
#ifdef	SMC_RAMLOG_VERBOSE
            dev_info(ramlog_view->dev, "index=%u data=%#x\n",
                (unsigned int) data_index,
                (unsigned int) value);
#endif	/* SMC_RAMLOG_VERBOSE */
            if ((column_index & 1) == 0) {
                digit = (uint8_t) ((value & 0xf0) >> 4);
                if (digit < 10)
                    ramlog_view->text_buffer[j++] =
                        (char) (digit + '0');
                else
                    ramlog_view->text_buffer[j++] =
                        (char) (digit - 10 + 'a');
                if (j >= sizeof(ramlog_view->text_buffer)-1) {
                    result = copy_to_user(&buffer[k],
                        ramlog_view->text_buffer, j);
                    if (result < 0) {
                        dev_err(ramlog_view->dev, "Ramlog not copied\n");
                        return result;
                    }
#ifdef	SMC_RAMLOG_VERBOSE
                    ramlog_view->text_buffer[j] = '\0';
                    dev_info(ramlog_view->dev, "k=%u buffer=\"%s\"\n",
                        (unsigned int) k,
                        ramlog_view->text_buffer);
#endif	/* SMC_RAMLOG_VERBOSE */
                    k += j;
                    j = 0;
                }
                ++i;
                ++offset;
            }
            if (i >= count || offset >= (loff_t)
                ramlog_view->text_length)
                break;
            digit = (uint8_t) (value & 0xf);
            if (digit < 10)
                ramlog_view->text_buffer[j++] =
                    (char) (digit + '0');
            else
                ramlog_view->text_buffer[j++] =
                    (char) (digit - 10 + 'a');
            if (j >= sizeof(ramlog_view->text_buffer) - 1) {
                result = copy_to_user(&buffer[k],
                    ramlog_view->text_buffer, j);
                if (result < 0) {
                    dev_err(ramlog_view->dev, "Ramlog not copied\n");
                    return result;
                }
#ifdef	SMC_RAMLOG_VERBOSE
                ramlog_view->text_buffer[j] = '\0';
                dev_info(ramlog_view->dev, "k=%u buffer=\"%s\"\n",
                    (unsigned int) k,
                    ramlog_view->text_buffer);
#endif	/* SMC_RAMLOG_VERBOSE */
                k += j;
                j = 0;
            }
            ++i;
            ++offset;
        }
    }
    if (j > 0) {
        result = copy_to_user(&buffer[k],
            ramlog_view->text_buffer, j);
        if (result < 0) {
            dev_err(ramlog_view->dev, "Ramlog not copied\n");
            return result;
        }
#ifdef	SMC_RAMLOG_VERBOSE
        ramlog_view->text_buffer[j] = '\0';
        dev_info(ramlog_view->dev, "k=%u buffer=\"%s\"\n",
            (unsigned int) k,
            ramlog_view->text_buffer);
#endif	/* SMC_RAMLOG_VERBOSE */
        k += j;
        j = 0;
    }
#ifdef	SMC_RAMLOG_VERBOSE
    dev_info(ramlog_view->dev, "done looping offset=%lu\n",
        (unsigned long) offset);
#endif	/* SMC_RAMLOG_VERBOSE */
    if (offp)
        *offp = offset;

    up(&ramlog_view->sem);

    return (ssize_t) i;
}

static ssize_t ramlog_write(struct file *filep, const char *buffer,
    size_t count, loff_t *offp)
{
    struct ramlog_file_buffer_t *ramlog_view;
    char *ptr;
    const char *source;
    unsigned int length;
    int result;

    if (!filep)
        return -EBADF;

    ramlog_view = (struct ramlog_file_buffer_t *) filep->private_data;
    if (!ramlog_view) {
        pr_err("ramlog write buffer not allocated\n");
        return -EBADF;
    }

    result = down_interruptible(&ramlog_view->sem);
    if (result < 0) {
        dev_err(ramlog_view->dev, "Cannot enter mutex, result=%d\n", result);
        return result;
    }

    ramlog_view->text_buffer[0] = '\0';
    ramlog_view->text_buffer[sizeof(ramlog_view->text_buffer) - 1] = '\0';
    length = (unsigned int) count;
    if (length > sizeof(ramlog_view->text_buffer) - 1)
        length = sizeof(ramlog_view->text_buffer) - 1;
    if (buffer && length) {
        result = copy_from_user(ramlog_view->text_buffer,
            buffer, length);
        barrier();
        if (result < 0) {
            dev_err(ramlog_view->dev, "Cannot copy from user, result=%d\n",
                result);
            return result;
        }
    }
    ramlog_view->text_buffer[length] = '\0';
    if (!length)
        return count;

    ptr = ramlog_view->text_buffer + length - 1;
    while (ptr >= ramlog_view->text_buffer)
        if (*ptr == '\r' || *ptr == '\n') {
            *ptr-- = '\0';
            --length;
        } else
            break;
    ramlog_view->text_buffer[length] = '\0';
    source = ramlog_view->text_buffer;
    while (*source && isspace(*source)) {
        ++source;
        --length;
    }
    if (!length)
        return count;
    execute_ramlog_command(ramlog_view, source);
    up(&ramlog_view->sem);
    return (ssize_t) count;
}

static void execute_ramlog_command(struct ramlog_file_buffer_t *ramlog_view,
    const char *command)
{
    const char *parsing, *module;
    unsigned int severity;
    char token[MAX_TOKEN_LENGTH + 1];
    unsigned int i, length;

    parsing = get_ramlog_token(token, sizeof(token), command);
    severity = RAMLOGFLAG_SEVERITY_MESSAGE;
    if (!strcmp(token, "help") || !strcmp(token, "HELP")) {
        if (!parsing) {
            for (i = 0; i < sizeof(help_msgs) / sizeof(*help_msgs);
                i++)
                pr_info("%s", help_msgs[i]);
            return;
        }
    } else if (!strcmp(token, "clear") || !strcmp(token, "CLEAR") ||
        !strcmp(token, "erase") || !strcmp(token, "ERASE")) {
        if (!parsing) {
            clear_ramlog(ramlog_view->ramlog);
            dev_info(ramlog_view->dev, "ramlog cleared\n");
            return;
        }
    } else if (!strcmp(token, "level") || !strcmp(token, "LEVEL")) {
        parsing = get_ramlog_token(token, sizeof(token), parsing);
        if (strcmp(token, "=")) {
            if (!command || !*command)
                return;
            if (!alloc_ramlog_buffer(ramlog_view,
                DEF_RAMLOG_PARAMETER_LENGTH))
                return;
            length = strlen(command);
            if (length >= DEF_RAMLOG_PARAMETER_LENGTH)
                length = DEF_RAMLOG_PARAMETER_LENGTH - 1;
            memcpy(ramlog_view->buf_ptr, command, length);
            ramlog_view->buf_ptr[length] = (uint8_t) 0;
            add_to_ramlog(ramlog_view->ramlog, severity,
                length + 1, ramlog_view->dma_addr);
            return;
        }
        severity = RAMLOGFLAG_SEVERITY_NONE;
        parsing = get_ramlog_token(token,
            sizeof(token), parsing);
        if (!strcmp(token, "error") ||
            !strcmp(token, "ERROR") ||
            !strcmp(token, "err") || !strcmp(token, "ERR")) {
            severity = RAMLOGFLAG_SEVERITY_ERROR;
        } else if (!strcmp(token, "warning") ||
            !strcmp(token, "WARNING") ||
            !strcmp(token, "warn") ||
            !strcmp(token, "WARN")) {
            severity = RAMLOGFLAG_SEVERITY_WARNING;
        } else if (!strcmp(token, "notice") ||
            !strcmp(token, "NOTICE")) {
            severity = RAMLOGFLAG_SEVERITY_NOTICE;
        } else if (!strcmp(token, "message") ||
            !strcmp(token, "MESSAGE") ||
            !strcmp(token, "msg") ||
            !strcmp(token, "MSG")) {
            severity = RAMLOGFLAG_SEVERITY_MESSAGE;
        } else if (!strcmp(token, "debug") ||
            !strcmp(token, "DEBUG") ||
            !strcmp(token, "dbg") ||
            !strcmp(token, "DBG")) {
            severity = RAMLOGFLAG_SEVERITY_DEBUG;
        } else if (!strcmp(token, "debug2") ||
            !strcmp(token, "DEBUG2") ||
            !strcmp(token, "dbg2") ||
            !strcmp(token, "DBG2") ||
            !strcmp(token, "deep") ||
            !strcmp(token, "DEEP") ||
            !strcmp(token, "deepdbg") ||
            !strcmp(token, "DEEPDBG")) {
            severity = RAMLOGFLAG_SEVERITY_DEEP_DEBUG;
        }
        if (severity == RAMLOGFLAG_SEVERITY_NONE) {
            severity = RAMLOGFLAG_SEVERITY_MESSAGE;
            if (!command || !*command)
                return;
            if (!alloc_ramlog_buffer(ramlog_view,
                DEF_RAMLOG_PARAMETER_LENGTH))
                return;
            length = strlen(command);
            if (length >= DEF_RAMLOG_PARAMETER_LENGTH)
                length = DEF_RAMLOG_PARAMETER_LENGTH - 1;
            memcpy(ramlog_view->buf_ptr, command, length);
            ramlog_view->buf_ptr[length] = (uint8_t) 0;
            add_to_ramlog(ramlog_view->ramlog, severity,
                length + 1, ramlog_view->dma_addr);
            return;
        }
        if (!parsing) {
            set_ramlog_level(ramlog_view->ramlog, severity, 0,
                (dma_addr_t) NULL);
            dev_info(ramlog_view->dev, "ramlog level=%u\n", severity);
            return;
        }
        parsing = get_ramlog_token(token, sizeof(token), parsing);
        module = NULL;
        if (!strcmp(token, "module") ||
            !strcmp(token, "MODULE") ||
            !strcmp(token, "source") ||
            !strcmp(token, "SOURCE")) {
            if (!alloc_ramlog_buffer(ramlog_view,
                DEF_RAMLOG_PARAMETER_LENGTH))
                return;
            parsing = get_ramlog_token(token,
                sizeof(token), parsing);
            if (strcmp(token, "=")) {
                severity = RAMLOGFLAG_SEVERITY_MESSAGE;
                if (!command || !*command)
                    return;
                length = strlen(command);
                if (length >= DEF_RAMLOG_PARAMETER_LENGTH)
                    length = DEF_RAMLOG_PARAMETER_LENGTH
                        - 1;
                memcpy(ramlog_view->buf_ptr, command, length);
                ramlog_view->buf_ptr[length] = (uint8_t) 0;
                add_to_ramlog(ramlog_view->ramlog, severity,
                    length + 1, ramlog_view->dma_addr);
                return;
            }
            token[0] = '\0';
            parsing = get_ramlog_token(token, sizeof(token),
                parsing);
            if (!strcmp(token, "!") ||
                !strcmp(token, "*"))
                module = "*";
            else if (token[0])
                module = token;
            if (module) {
                length = strlen(module);
                if (length >= DEF_RAMLOG_PARAMETER_LENGTH)
                    length = DEF_RAMLOG_PARAMETER_LENGTH
                        - 1;
                memcpy(ramlog_view->buf_ptr, module, length);
                ramlog_view->buf_ptr[length] = (uint8_t) 0;
                set_ramlog_level(ramlog_view->ramlog,
                    severity, length + 1,
                    ramlog_view->dma_addr);
                dev_info(ramlog_view->dev, "ramlog level=%u source=%s\n",
                    severity, module);
                return;
            }
        }
        severity = RAMLOGFLAG_SEVERITY_MESSAGE;
    } else if (!strcmp(token, "error") || !strcmp(token, "ERROR") ||
        !strcmp(token, "err") || !strcmp(token, "ERR")) {
        parsing = get_ramlog_token(token, sizeof(token), parsing);
        if (!strcmp(token, ":")) {
            severity = RAMLOGFLAG_SEVERITY_ERROR;
            command = parsing;
        }
    } else if (!strcmp(token, "warning") ||
        !strcmp(token, "WARNING") ||
        !strcmp(token, "warn") || !strcmp(token, "WARN")) {
        parsing = get_ramlog_token(token, sizeof(token), parsing);
        if (!strcmp(token, ":")) {
            severity = RAMLOGFLAG_SEVERITY_WARNING;
            command = parsing;
        }
    } else if (!strcmp(token, "notice") || !strcmp(token, "NOTICE")) {
        parsing = get_ramlog_token(token, sizeof(token), parsing);
        if (!strcmp(token, ":")) {
            severity = RAMLOGFLAG_SEVERITY_NOTICE;
            command = parsing;
        }
    } else if (!strcmp(token, "message") ||
        !strcmp(token, "MESSAGE") ||
        !strcmp(token, "msg") || !strcmp(token, "MSG")) {
        parsing = get_ramlog_token(token, sizeof(token), parsing);
        if (!strcmp(token, ":")) {
            severity = RAMLOGFLAG_SEVERITY_MESSAGE;
            command = parsing;
        }
    } else if (!strcmp(token, "debug") || !strcmp(token, "DEBUG") ||
        !strcmp(token, "dbg") || !strcmp(token, "DBG")) {
        parsing = get_ramlog_token(token, sizeof(token), parsing);
        if (!strcmp(token, ":")) {
            severity = RAMLOGFLAG_SEVERITY_DEBUG;
            command = parsing;
        }
    } else if (!strcmp(token, "debug2") || !strcmp(token, "DEBUG2") ||
        !strcmp(token, "dbg2") || !strcmp(token, "DBG2") ||
        !strcmp(token, "deep") || !strcmp(token, "DEEP") ||
        !strcmp(token, "deepdbg") || !strcmp(token, "DEEPDBG")) {
        parsing = get_ramlog_token(token, sizeof(token), parsing);
        if (!strcmp(token, ":")) {
            severity = RAMLOGFLAG_SEVERITY_DEEP_DEBUG;
            command = parsing;
        }
    }

    if (!command || !*command)
        return;
    if (!alloc_ramlog_buffer(ramlog_view, DEF_RAMLOG_PARAMETER_LENGTH))
        return;
    length = strlen(command);
    if (length >= DEF_RAMLOG_PARAMETER_LENGTH)
        length = DEF_RAMLOG_PARAMETER_LENGTH - 1;
    memcpy(ramlog_view->buf_ptr, command, length);
    ramlog_view->buf_ptr[length] = (uint8_t) 0;
    add_to_ramlog(ramlog_view->ramlog, severity,
        length + 1, ramlog_view->dma_addr);
}

static uint8_t *alloc_ramlog_buffer (struct ramlog_file_buffer_t *ramlog_view,
    unsigned int buf_size)
{
    if (ramlog_view->buf_size < buf_size) {
        buf_size = ALIGN(buf_size, RAMLOG_PAGE_SIZE);
        if (ramlog_view->buf_size) {
            dma_unmap_single(ramlog_view->dev,
                ramlog_view->dma_addr, ramlog_view->buf_size,
                DMA_BIDIRECTIONAL);
            kfree(ramlog_view->buf_ptr);
        }
        ramlog_view->buf_ptr = (uint8_t *)
            kmalloc(buf_size, GFP_KERNEL);
        if (!ramlog_view->buf_ptr)
            return 0;
        ramlog_view->dma_addr = dma_map_single(ramlog_view->dev,
            ramlog_view->buf_ptr, buf_size, DMA_BIDIRECTIONAL);
        if (dma_mapping_error(ramlog_view->dev,
            ramlog_view->dma_addr)) {
            dev_err(ramlog_view->dev, "DMA address not mapped to %#lx\n",
                (unsigned long) ramlog_view->buf_ptr);
            return 0;
        }
        ramlog_view->buf_size = buf_size;
    }
    return ramlog_view->buf_ptr;
}

static const char *get_ramlog_token(char *token, unsigned int max_token_size,
    const char *str_to_parse)
{
    const char *processing;
    char *target;

    if (token && max_token_size) {
        token[0] = '\0';
        if (max_token_size > 1)
            token[max_token_size - 1] = '\0';
    }
    if (!str_to_parse || !*str_to_parse)
        return 0;
    processing = str_to_parse;
    while (*processing && isspace(*processing))
        ++processing;
    if (!*processing)
        return 0;
    switch (*processing) {
    case ':':
    case '=':
        if (max_token_size > 0) {
            token[0] = *processing;
            if (max_token_size > 1)
                token[1] = '\0';
        }
        ++processing;
        while (*processing && isspace(*processing))
            ++processing;
        if (!*processing)
            processing = 0;
        return processing;
    default:
        break;
    }
    target = token;
    while (*processing) {
        if (isspace(*processing) || *processing == ':' ||
            *processing == '=')
            break;
        if (target && max_token_size > 1) {
            *target++ = *processing;
            --max_token_size;
        }
        ++processing;
    }
    if (target && max_token_size > 0)
        *target++ = '\0';
    while (*processing && isspace(*processing))
        ++processing;
    if (!*processing)
        processing = 0;
    return processing;
}


static unsigned int ramlog_poll(struct file *filep,
    struct poll_table_struct *wait)
{
    poll_wait(filep, &ramlog_wait, wait);
    return 0;
}

static int ramlog_probe(struct platform_device *pdev)
{
    int status;

    /* allocate resources */
    smc_ramlog = (struct ramlog_rpc_data *)
        kzalloc(sizeof(struct ramlog_rpc_data), GFP_KERNEL);
    if (!smc_ramlog)
        return -ENOMEM;
    status = register_ramlog_rpc_from_platform_device(smc_ramlog, pdev);
    if (status)
        return status;

    /* create /proc pseudo file */
    ramlog_proc_parent = proc_mkdir(RAMLOG_PROCFS_DIR_NAME, NULL);
    if (!ramlog_proc_parent)
        dev_err(&pdev->dev, "/proc/%s cannot be created.\n",
            RAMLOG_PROCFS_DIR_NAME);
    else {
        ramlog_proc_entry = proc_create(RAMLOG_PROCFS_NAME, 0644,
            ramlog_proc_parent, &ramlog_file_fops);
        if (!ramlog_proc_entry)
            dev_err(&pdev->dev, "/proc/%s/%s cannot be created.\n",
                RAMLOG_PROCFS_DIR_NAME, RAMLOG_PROCFS_NAME);
        else
            dev_info(&pdev->dev, "/proc/%s/%s created.\n",
                RAMLOG_PROCFS_DIR_NAME, RAMLOG_PROCFS_NAME);
    }

    dev_info(&pdev->dev,"ramlog viewer probe succeeded\n");
    return 0;
}

static int ramlog_remove(struct platform_device *pdev)
{
    /* get rid of /proc entry */
    if (ramlog_proc_parent) {
        if (ramlog_proc_entry) {
            remove_proc_entry(RAMLOG_PROCFS_NAME,
                ramlog_proc_parent);
            ramlog_proc_entry = NULL;
            dev_info(&pdev->dev, "/proc/%s/%s deleted\n",
                RAMLOG_PROCFS_DIR_NAME, RAMLOG_PROCFS_NAME);
        }
        remove_proc_entry(RAMLOG_PROCFS_DIR_NAME, NULL);
        ramlog_proc_parent = NULL;
        dev_info(&pdev->dev, "/proc/%s deleted\n", RAMLOG_PROCFS_DIR_NAME);
    }

    /* release resources */
    if (smc_ramlog) {
        release_ramlog_rpc(smc_ramlog);
        kfree(smc_ramlog);
        smc_ramlog = NULL;
    }

    dev_info(&pdev->dev, "ramlog viewer removed\n");
    return 0;
}

static int __init ramlogview_init(void)
{
    return platform_driver_register(&ramlog_platform_driver);
}

/**
 * This routine unloads the ramlog viewer from kernel
 */
static void __exit ramlogview_exit(void)
{
    platform_driver_unregister(&ramlog_platform_driver);
}

module_init(ramlogview_init);
module_exit(ramlogview_exit);
