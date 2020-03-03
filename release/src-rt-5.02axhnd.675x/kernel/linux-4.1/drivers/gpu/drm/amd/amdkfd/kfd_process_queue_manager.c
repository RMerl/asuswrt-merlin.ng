/*
 * Copyright 2014 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <linux/slab.h>
#include <linux/list.h>
#include "kfd_device_queue_manager.h"
#include "kfd_priv.h"
#include "kfd_kernel_queue.h"

static inline struct process_queue_node *get_queue_by_qid(
			struct process_queue_manager *pqm, unsigned int qid)
{
	struct process_queue_node *pqn;

	BUG_ON(!pqm);

	list_for_each_entry(pqn, &pqm->queues, process_queue_list) {
		if (pqn->q && pqn->q->properties.queue_id == qid)
			return pqn;
		if (pqn->kq && pqn->kq->queue->properties.queue_id == qid)
			return pqn;
	}

	return NULL;
}

static int find_available_queue_slot(struct process_queue_manager *pqm,
					unsigned int *qid)
{
	unsigned long found;

	BUG_ON(!pqm || !qid);

	pr_debug("kfd: in %s\n", __func__);

	found = find_first_zero_bit(pqm->queue_slot_bitmap,
			KFD_MAX_NUM_OF_QUEUES_PER_PROCESS);

	pr_debug("kfd: the new slot id %lu\n", found);

	if (found >= KFD_MAX_NUM_OF_QUEUES_PER_PROCESS) {
		pr_info("amdkfd: Can not open more queues for process with pasid %d\n",
				pqm->process->pasid);
		return -ENOMEM;
	}

	set_bit(found, pqm->queue_slot_bitmap);
	*qid = found;

	return 0;
}

int pqm_init(struct process_queue_manager *pqm, struct kfd_process *p)
{
	BUG_ON(!pqm);

	INIT_LIST_HEAD(&pqm->queues);
	pqm->queue_slot_bitmap =
			kzalloc(DIV_ROUND_UP(KFD_MAX_NUM_OF_QUEUES_PER_PROCESS,
					BITS_PER_BYTE), GFP_KERNEL);
	if (pqm->queue_slot_bitmap == NULL)
		return -ENOMEM;
	pqm->process = p;

	return 0;
}

void pqm_uninit(struct process_queue_manager *pqm)
{
	int retval;
	struct process_queue_node *pqn, *next;

	BUG_ON(!pqm);

	pr_debug("In func %s\n", __func__);

	list_for_each_entry_safe(pqn, next, &pqm->queues, process_queue_list) {
		retval = pqm_destroy_queue(
				pqm,
				(pqn->q != NULL) ?
					pqn->q->properties.queue_id :
					pqn->kq->queue->properties.queue_id);

		if (retval != 0) {
			pr_err("kfd: failed to destroy queue\n");
			return;
		}
	}
	kfree(pqm->queue_slot_bitmap);
	pqm->queue_slot_bitmap = NULL;
}

static int create_cp_queue(struct process_queue_manager *pqm,
				struct kfd_dev *dev, struct queue **q,
				struct queue_properties *q_properties,
				struct file *f, unsigned int qid)
{
	int retval;

	retval = 0;

	/* Doorbell initialized in user space*/
	q_properties->doorbell_ptr = NULL;

	q_properties->doorbell_off =
			kfd_queue_id_to_doorbell(dev, pqm->process, qid);

	/* let DQM handle it*/
	q_properties->vmid = 0;
	q_properties->queue_id = qid;

	retval = init_queue(q, *q_properties);
	if (retval != 0)
		goto err_init_queue;

	(*q)->device = dev;
	(*q)->process = pqm->process;

	pr_debug("kfd: PQM After init queue");

	return retval;

err_init_queue:
	return retval;
}

int pqm_create_queue(struct process_queue_manager *pqm,
			    struct kfd_dev *dev,
			    struct file *f,
			    struct queue_properties *properties,
			    unsigned int flags,
			    enum kfd_queue_type type,
			    unsigned int *qid)
{
	int retval;
	struct kfd_process_device *pdd;
	struct queue_properties q_properties;
	struct queue *q;
	struct process_queue_node *pqn;
	struct kernel_queue *kq;

	BUG_ON(!pqm || !dev || !properties || !qid);

	memset(&q_properties, 0, sizeof(struct queue_properties));
	memcpy(&q_properties, properties, sizeof(struct queue_properties));
	q = NULL;
	kq = NULL;

	pdd = kfd_get_process_device_data(dev, pqm->process);
	if (!pdd) {
		pr_err("Process device data doesn't exist\n");
		return -1;
	}

	retval = find_available_queue_slot(pqm, qid);
	if (retval != 0)
		return retval;

	if (list_empty(&pqm->queues)) {
		pdd->qpd.pqm = pqm;
		dev->dqm->ops.register_process(dev->dqm, &pdd->qpd);
	}

	pqn = kzalloc(sizeof(struct process_queue_node), GFP_KERNEL);
	if (!pqn) {
		retval = -ENOMEM;
		goto err_allocate_pqn;
	}

	switch (type) {
	case KFD_QUEUE_TYPE_SDMA:
		if (dev->dqm->queue_count >=
			CIK_SDMA_QUEUES_PER_ENGINE * CIK_SDMA_ENGINE_NUM) {
			pr_err("Over-subscription is not allowed for SDMA.\n");
			retval = -EPERM;
			goto err_create_queue;
		}

		retval = create_cp_queue(pqm, dev, &q, properties, f, *qid);
		if (retval != 0)
			goto err_create_queue;
		pqn->q = q;
		pqn->kq = NULL;
		retval = dev->dqm->ops.create_queue(dev->dqm, q, &pdd->qpd,
						&q->properties.vmid);
		pr_debug("DQM returned %d for create_queue\n", retval);
		print_queue(q);
		break;

	case KFD_QUEUE_TYPE_COMPUTE:
		/* check if there is over subscription */
		if ((sched_policy == KFD_SCHED_POLICY_HWS_NO_OVERSUBSCRIPTION) &&
		((dev->dqm->processes_count >= VMID_PER_DEVICE) ||
		(dev->dqm->queue_count >= PIPE_PER_ME_CP_SCHEDULING * QUEUES_PER_PIPE))) {
			pr_err("kfd: over-subscription is not allowed in radeon_kfd.sched_policy == 1\n");
			retval = -EPERM;
			goto err_create_queue;
		}

		retval = create_cp_queue(pqm, dev, &q, &q_properties, f, *qid);
		if (retval != 0)
			goto err_create_queue;
		pqn->q = q;
		pqn->kq = NULL;
		retval = dev->dqm->ops.create_queue(dev->dqm, q, &pdd->qpd,
						&q->properties.vmid);
		pr_debug("DQM returned %d for create_queue\n", retval);
		print_queue(q);
		break;
	case KFD_QUEUE_TYPE_DIQ:
		kq = kernel_queue_init(dev, KFD_QUEUE_TYPE_DIQ);
		if (kq == NULL) {
			retval = -ENOMEM;
			goto err_create_queue;
		}
		kq->queue->properties.queue_id = *qid;
		pqn->kq = kq;
		pqn->q = NULL;
		retval = dev->dqm->ops.create_kernel_queue(dev->dqm,
							kq, &pdd->qpd);
		break;
	default:
		BUG();
		break;
	}

	if (retval != 0) {
		pr_debug("Error dqm create queue\n");
		goto err_create_queue;
	}

	pr_debug("kfd: PQM After DQM create queue\n");

	list_add(&pqn->process_queue_list, &pqm->queues);

	if (q) {
		*properties = q->properties;
		pr_debug("kfd: PQM done creating queue\n");
		print_queue_properties(properties);
	}

	return retval;

err_create_queue:
	kfree(pqn);
err_allocate_pqn:
	/* check if queues list is empty unregister process from device */
	clear_bit(*qid, pqm->queue_slot_bitmap);
	if (list_empty(&pqm->queues))
		dev->dqm->ops.unregister_process(dev->dqm, &pdd->qpd);
	return retval;
}

int pqm_destroy_queue(struct process_queue_manager *pqm, unsigned int qid)
{
	struct process_queue_node *pqn;
	struct kfd_process_device *pdd;
	struct device_queue_manager *dqm;
	struct kfd_dev *dev;
	int retval;

	dqm = NULL;

	BUG_ON(!pqm);
	retval = 0;

	pr_debug("kfd: In Func %s\n", __func__);

	pqn = get_queue_by_qid(pqm, qid);
	if (pqn == NULL) {
		pr_err("kfd: queue id does not match any known queue\n");
		return -EINVAL;
	}

	dev = NULL;
	if (pqn->kq)
		dev = pqn->kq->dev;
	if (pqn->q)
		dev = pqn->q->device;
	BUG_ON(!dev);

	pdd = kfd_get_process_device_data(dev, pqm->process);
	if (!pdd) {
		pr_err("Process device data doesn't exist\n");
		return -1;
	}

	if (pqn->kq) {
		/* destroy kernel queue (DIQ) */
		dqm = pqn->kq->dev->dqm;
		dqm->ops.destroy_kernel_queue(dqm, pqn->kq, &pdd->qpd);
		kernel_queue_uninit(pqn->kq);
	}

	if (pqn->q) {
		dqm = pqn->q->device->dqm;
		retval = dqm->ops.destroy_queue(dqm, &pdd->qpd, pqn->q);
		if (retval != 0)
			return retval;

		uninit_queue(pqn->q);
	}

	list_del(&pqn->process_queue_list);
	kfree(pqn);
	clear_bit(qid, pqm->queue_slot_bitmap);

	if (list_empty(&pqm->queues))
		dqm->ops.unregister_process(dqm, &pdd->qpd);

	return retval;
}

int pqm_update_queue(struct process_queue_manager *pqm, unsigned int qid,
			struct queue_properties *p)
{
	int retval;
	struct process_queue_node *pqn;

	BUG_ON(!pqm);

	pqn = get_queue_by_qid(pqm, qid);
	if (!pqn) {
		pr_debug("amdkfd: No queue %d exists for update operation\n",
				qid);
		return -EFAULT;
	}

	pqn->q->properties.queue_address = p->queue_address;
	pqn->q->properties.queue_size = p->queue_size;
	pqn->q->properties.queue_percent = p->queue_percent;
	pqn->q->properties.priority = p->priority;

	retval = pqn->q->device->dqm->ops.update_queue(pqn->q->device->dqm,
							pqn->q);
	if (retval != 0)
		return retval;

	return 0;
}

static __attribute__((unused)) struct kernel_queue *pqm_get_kernel_queue(
					struct process_queue_manager *pqm,
					unsigned int qid)
{
	struct process_queue_node *pqn;

	BUG_ON(!pqm);

	pqn = get_queue_by_qid(pqm, qid);
	if (pqn && pqn->kq)
		return pqn->kq;

	return NULL;
}


