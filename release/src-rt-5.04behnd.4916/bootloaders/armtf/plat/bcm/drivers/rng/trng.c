// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2019 Broadcom Ltd.
 */
/*
 * 
 */
// BCMFORMAT: notabs reindent:uncrustify:bcm_minimal_i4.cfg

#include <trng.h>
#include <bcm_rng.h>
#include <delay_timer.h>

#define EBUSY	1
#define EFAULT	2

/**
 * @brief Supported command types.
 */
typedef enum
{
    TRNG_CMD_NONE = 0,          /**< No command currently submitted.            */
    TRNG_CMD_INSTANTIATE,   /**< Instantiate the block ready to generate.   */
    TRNG_CMD_RESEED,        /**< Force the generation of a new seed.        */
    TRNG_CMD_GENERATE,      /**< Generate random bytes.                     */
    TRNG_CMD_TEST,          /**< Run self tests of DRBG and entropy source. */
    TRNG_CMD_UNINSTANTIATE, /**< Erase internal state and flush TRNG fifo.  */
    TRNG_CMD_STANDBY,       /**< Toggle standby mode.                       */
    TRNG_CMD_RESET          /**< Reset the block and execute startup tests. */
} TRNG_COMMAND;

#define TRNG_MAX_RDY_WAIT_US        500
#define TRNG_MAX_RESETS_PER_READ    1
#define TRNG_DEBUG                  0

static int trng_waitrdy(void)
{
    int wait_us = 0;
    uint32_t val = 0;

    /* Check if TRNG is ready */
    val = TRNG->sts;
    while (!(val & TRNG_STATUS_RDY) && !(val & TRNG_STATUS_ERR)) {
        /* Check wait limit */
        if (wait_us > TRNG_MAX_RDY_WAIT_US)
        {
#if TRNG_DEBUG
            printf("%s: Error TRNG not responding! status: 0x%08x\n", __FUNCTION__, val);
#endif
            return -EBUSY;
        }

        /* wait */
        mdelay(10000);
        wait_us += 10;

        /* read status */
        val = TRNG->sts;
    }

#if TRNG_DEBUG
    printf("%s: status: 0x%08x wait: %d\n", __FUNCTION__, val, wait_us);
#endif

    return (val & TRNG_STATUS_ERR);
}

static int trng_sendcmd(TRNG_COMMAND cmd)
{
    int ret = 0;

    /* wait for ready status for certain commands */
    if( (cmd != TRNG_CMD_TEST) && (cmd != TRNG_CMD_RESET) )
        ret = trng_waitrdy();

    if( ret == 0 ) {
        /* Send command */
        TRNG->cmd = cmd;
    }
    return ret;
}

static int trng_reset( bool enable)
{
    uint32_t val;
    int ret = 0;

    /* Reset the TRNG */
    ret = trng_sendcmd(TRNG_CMD_RESET);
    if (ret) {
#if TRNG_DEBUG
        printf( "%s: Error sending TRNG_CMD_RESET\n", __FUNCTION__);
#endif
        return -EFAULT;
    }

    if( enable ) {  
        /* Enable use of internal entropy source */
        val = TRNG->ctrl;
        val |= TRNG_CTRL_USE_RO;
        TRNG->ctrl = val;

        /* Disable user personalization string */
        TRNG->user_in_len = 0;
        
        /* Instantiate initial seed */
        ret = trng_sendcmd( TRNG_CMD_INSTANTIATE);
        if (ret) {
#if TRNG_DEBUG
            printf("%s: Error sending TRNG_CMD_INSTANTIATE\n", __FUNCTION__);
#endif
            return -EFAULT;
        }
    }

    /* Return after waiting for entire reset process to complete */
    return trng_waitrdy();
}

int rng_get_rand(uint8_t *data, int len)
{
    uint32_t status;
    char *buf = (char *)data;
    uint32_t num_remaining = len;
    uint32_t ret = 0;
    uint32_t num_words_avail = 0;

    uint32_t num_resets = 0;

    while (num_remaining > 0) {
        /* Get TRNG status */
        status = TRNG->sts;

        /* Check TRNG status */
        if (status & TRNG_STATUS_ERR) {
            /* TRNG is in error condition, reset it */
            if (num_resets >= TRNG_MAX_RESETS_PER_READ)
                return len - num_remaining;

            ret = trng_reset(true);
            if (ret) {
#if TRNG_DEBUG
                printf("%s: Error resetting TRNG!\n", __FUNCTION__);
#endif
                return -EFAULT;
            }

            /* Log number of resets */
            num_resets++;
        } else if (status & TRNG_STATUS_DATA_CNT) {
            /* Are there any random numbers available? */
            num_words_avail = ((status & TRNG_STATUS_DATA_CNT) >> TRNG_STATUS_DATA_CNT_SHIFT);  

            /* Use the words available */
            while( num_words_avail && num_remaining )
            {
                if (num_remaining >= sizeof(uint32_t)) {
                    /* Buffer has room to store entire word */
                    *(uint32_t *)buf = TRNG->dout;
                    buf += sizeof(uint32_t);
                    num_remaining -= sizeof(uint32_t);
                } else {
                    /* Buffer can only store partial word */
                    uint32_t rnd_number = TRNG->dout;
                    memcpy(buf, &rnd_number, num_remaining);
                    buf += num_remaining;
                    num_remaining = 0;
                }
                num_words_avail--;
            }
        } else if (status & TRNG_STATUS_RDY) {
            /* No random number available AND num_remaining !=0 ---> generate new random numbers */
            TRNG->req_len = TRNG_REQ_LEN_MAX_VAL;
            ret = trng_sendcmd(TRNG_CMD_GENERATE);
            if (ret) {
#if TRNG_DEBUG
                printf("%s: Error sending TRNG_CMD_GENERATE\n", __FUNCTION__);
#endif
                return -EFAULT;
            }

            /* Wait for generation */
            udelay(10);
        } else {
            /* Should never come here */
#if TRNG_DEBUG
            printf("%s: Unknown Error\n", __FUNCTION__);
#endif
            return -EFAULT;
        }

    }

    return 0;
}

void rng_init(void)
{
    trng_reset(true);
}


void rng_lock(void)
{
#ifndef CONFIG_BRCM_SMC_BASED
    TRNG->perm = TRNG_PERM_ALLOW_SECURE_ACCESS;
#endif
}

