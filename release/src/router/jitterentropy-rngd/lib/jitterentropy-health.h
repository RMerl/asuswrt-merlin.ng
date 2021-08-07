/*
 * Copyright (C) 2021, Stephan Mueller <smueller@chronox.de>
 *
 * License: see LICENSE file in root directory
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ALL OF
 * WHICH ARE HEREBY DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#ifndef JITTERENTROPY_HEALTH_H
#define JITTERENTROPY_HEALTH_H

#ifdef __cplusplus
extern "C"
{
#endif

static inline uint64_t jent_delta(uint64_t prev, uint64_t next)
{
	return (next - prev);
}

#ifdef JENT_HEALTH_LAG_PREDICTOR
void jent_lag_init(struct rand_data *ec, unsigned int osr);
#else /* JENT_HEALTH_LAG_PREDICTOR */
static inline void jent_lag_init(struct rand_data *ec, unsigned int osr)
{
	(void)ec;
	(void)osr;
}
#endif /* JENT_HEALTH_LAG_PREDICTOR */

void jent_apt_init(struct rand_data *ec, unsigned int osr);
unsigned int jent_stuck(struct rand_data *ec, uint64_t current_delta);
unsigned int jent_health_failure(struct rand_data *ec);

#ifdef __cplusplus
}
#endif

#endif /* JITTERENTROPY_HEALTH_H */
