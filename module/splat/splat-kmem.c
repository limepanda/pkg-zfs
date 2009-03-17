/*
 *  This file is part of the SPL: Solaris Porting Layer.
 *
 *  Copyright (c) 2008 Lawrence Livermore National Security, LLC.
 *  Produced at Lawrence Livermore National Laboratory
 *  Written by:
 *	  Brian Behlendorf <behlendorf1@llnl.gov>,
 *	  Herb Wartens <wartens2@llnl.gov>,
 *	  Jim Garlick <garlick@llnl.gov>
 *  UCRL-CODE-235197
 *
 *  This is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 */

#include "splat-internal.h"

#define SPLAT_SUBSYSTEM_KMEM		0x0100
#define SPLAT_KMEM_NAME			"kmem"
#define SPLAT_KMEM_DESC			"Kernel Malloc/Slab Tests"

#define SPLAT_KMEM_TEST1_ID		0x0101
#define SPLAT_KMEM_TEST1_NAME		"kmem_alloc"
#define SPLAT_KMEM_TEST1_DESC		"Memory allocation test (kmem_alloc)"

#define SPLAT_KMEM_TEST2_ID		0x0102
#define SPLAT_KMEM_TEST2_NAME		"kmem_zalloc"
#define SPLAT_KMEM_TEST2_DESC		"Memory allocation test (kmem_zalloc)"

#define SPLAT_KMEM_TEST3_ID		0x0103
#define SPLAT_KMEM_TEST3_NAME		"vmem_alloc"
#define SPLAT_KMEM_TEST3_DESC		"Memory allocation test (vmem_alloc)"

#define SPLAT_KMEM_TEST4_ID		0x0104
#define SPLAT_KMEM_TEST4_NAME		"vmem_zalloc"
#define SPLAT_KMEM_TEST4_DESC		"Memory allocation test (vmem_zalloc)"

#define SPLAT_KMEM_TEST5_ID		0x0105
#define SPLAT_KMEM_TEST5_NAME		"slab_small"
#define SPLAT_KMEM_TEST5_DESC		"Slab ctor/dtor test (small)"

#define SPLAT_KMEM_TEST6_ID		0x0106
#define SPLAT_KMEM_TEST6_NAME		"slab_large"
#define SPLAT_KMEM_TEST6_DESC		"Slab ctor/dtor test (large)"

#define SPLAT_KMEM_TEST7_ID		0x0107
#define SPLAT_KMEM_TEST7_NAME		"slab_align"
#define SPLAT_KMEM_TEST7_DESC		"Slab alignment test"

#define SPLAT_KMEM_TEST8_ID		0x0108
#define SPLAT_KMEM_TEST8_NAME		"slab_reap"
#define SPLAT_KMEM_TEST8_DESC		"Slab reaping test"

#define SPLAT_KMEM_TEST9_ID		0x0109
#define SPLAT_KMEM_TEST9_NAME		"slab_age"
#define SPLAT_KMEM_TEST9_DESC		"Slab aging test"

#define SPLAT_KMEM_TEST10_ID		0x010a
#define SPLAT_KMEM_TEST10_NAME		"slab_lock"
#define SPLAT_KMEM_TEST10_DESC		"Slab locking test"

#define SPLAT_KMEM_TEST11_ID		0x010b
#define SPLAT_KMEM_TEST11_NAME		"slab_overcommit"
#define SPLAT_KMEM_TEST11_DESC		"Slab memory overcommit test"

#define SPLAT_KMEM_TEST12_ID		0x010c
#define SPLAT_KMEM_TEST12_NAME		"vmem_size"
#define SPLAT_KMEM_TEST12_DESC		"Memory zone test"

#define SPLAT_KMEM_ALLOC_COUNT		10
#define SPLAT_VMEM_ALLOC_COUNT		10


static int
splat_kmem_test1(struct file *file, void *arg)
{
	void *ptr[SPLAT_KMEM_ALLOC_COUNT];
	int size = PAGE_SIZE;
	int i, count, rc = 0;

	/* We are intentionally going to push kmem_alloc to its max
	 * allocation size, so suppress the console warnings for now */
	kmem_set_warning(0);

	while ((!rc) && (size <= (PAGE_SIZE * 32))) {
		count = 0;

		for (i = 0; i < SPLAT_KMEM_ALLOC_COUNT; i++) {
			ptr[i] = kmem_alloc(size, KM_SLEEP);
			if (ptr[i])
				count++;
		}

		for (i = 0; i < SPLAT_KMEM_ALLOC_COUNT; i++)
			if (ptr[i])
				kmem_free(ptr[i], size);

		splat_vprint(file, SPLAT_KMEM_TEST1_NAME,
			   "%d byte allocations, %d/%d successful\n",
			   size, count, SPLAT_KMEM_ALLOC_COUNT);
		if (count != SPLAT_KMEM_ALLOC_COUNT)
			rc = -ENOMEM;

		size *= 2;
	}

	kmem_set_warning(1);

	return rc;
}

static int
splat_kmem_test2(struct file *file, void *arg)
{
	void *ptr[SPLAT_KMEM_ALLOC_COUNT];
	int size = PAGE_SIZE;
	int i, j, count, rc = 0;

	/* We are intentionally going to push kmem_alloc to its max
	 * allocation size, so suppress the console warnings for now */
	kmem_set_warning(0);

	while ((!rc) && (size <= (PAGE_SIZE * 32))) {
		count = 0;

		for (i = 0; i < SPLAT_KMEM_ALLOC_COUNT; i++) {
			ptr[i] = kmem_zalloc(size, KM_SLEEP);
			if (ptr[i])
				count++;
		}

		/* Ensure buffer has been zero filled */
		for (i = 0; i < SPLAT_KMEM_ALLOC_COUNT; i++) {
			for (j = 0; j < size; j++) {
				if (((char *)ptr[i])[j] != '\0') {
					splat_vprint(file, SPLAT_KMEM_TEST2_NAME,
						  "%d-byte allocation was "
						  "not zeroed\n", size);
					rc = -EFAULT;
				}
			}
		}

		for (i = 0; i < SPLAT_KMEM_ALLOC_COUNT; i++)
			if (ptr[i])
				kmem_free(ptr[i], size);

		splat_vprint(file, SPLAT_KMEM_TEST2_NAME,
			   "%d byte allocations, %d/%d successful\n",
			   size, count, SPLAT_KMEM_ALLOC_COUNT);
		if (count != SPLAT_KMEM_ALLOC_COUNT)
			rc = -ENOMEM;

		size *= 2;
	}

	kmem_set_warning(1);

	return rc;
}

static int
splat_kmem_test3(struct file *file, void *arg)
{
	void *ptr[SPLAT_VMEM_ALLOC_COUNT];
	int size = PAGE_SIZE;
	int i, count, rc = 0;

	while ((!rc) && (size <= (PAGE_SIZE * 1024))) {
		count = 0;

		for (i = 0; i < SPLAT_VMEM_ALLOC_COUNT; i++) {
			ptr[i] = vmem_alloc(size, KM_SLEEP);
			if (ptr[i])
				count++;
		}

		for (i = 0; i < SPLAT_VMEM_ALLOC_COUNT; i++)
			if (ptr[i])
				vmem_free(ptr[i], size);

		splat_vprint(file, SPLAT_KMEM_TEST3_NAME,
			   "%d byte allocations, %d/%d successful\n",
			   size, count, SPLAT_VMEM_ALLOC_COUNT);
		if (count != SPLAT_VMEM_ALLOC_COUNT)
			rc = -ENOMEM;

		size *= 2;
	}

	return rc;
}

static int
splat_kmem_test4(struct file *file, void *arg)
{
	void *ptr[SPLAT_VMEM_ALLOC_COUNT];
	int size = PAGE_SIZE;
	int i, j, count, rc = 0;

	while ((!rc) && (size <= (PAGE_SIZE * 1024))) {
		count = 0;

		for (i = 0; i < SPLAT_VMEM_ALLOC_COUNT; i++) {
			ptr[i] = vmem_zalloc(size, KM_SLEEP);
			if (ptr[i])
				count++;
		}

		/* Ensure buffer has been zero filled */
		for (i = 0; i < SPLAT_VMEM_ALLOC_COUNT; i++) {
			for (j = 0; j < size; j++) {
				if (((char *)ptr[i])[j] != '\0') {
					splat_vprint(file, SPLAT_KMEM_TEST4_NAME,
						  "%d-byte allocation was "
						  "not zeroed\n", size);
					rc = -EFAULT;
				}
			}
		}

		for (i = 0; i < SPLAT_VMEM_ALLOC_COUNT; i++)
			if (ptr[i])
				vmem_free(ptr[i], size);

		splat_vprint(file, SPLAT_KMEM_TEST4_NAME,
			   "%d byte allocations, %d/%d successful\n",
			   size, count, SPLAT_VMEM_ALLOC_COUNT);
		if (count != SPLAT_VMEM_ALLOC_COUNT)
			rc = -ENOMEM;

		size *= 2;
	}

	return rc;
}

#define SPLAT_KMEM_TEST_MAGIC		0x004488CCUL
#define SPLAT_KMEM_CACHE_NAME		"kmem_test"
#define SPLAT_KMEM_OBJ_COUNT		1024
#define SPLAT_KMEM_OBJ_RECLAIM		20 /* percent */
#define SPLAT_KMEM_THREADS		32

#define KCP_FLAG_READY			0x01

typedef struct kmem_cache_data {
	unsigned long kcd_magic;
	int kcd_flag;
	char kcd_buf[0];
} kmem_cache_data_t;

typedef struct kmem_cache_thread {
	kmem_cache_t *kct_cache;
	spinlock_t kct_lock;
	int kct_id;
	int kct_kcd_count;
	kmem_cache_data_t *kct_kcd[0];
} kmem_cache_thread_t;

typedef struct kmem_cache_priv {
	unsigned long kcp_magic;
	struct file *kcp_file;
	kmem_cache_t *kcp_cache;
	spinlock_t kcp_lock;
	wait_queue_head_t kcp_ctl_waitq;
	wait_queue_head_t kcp_thr_waitq;
	int kcp_flags;
	int kcp_kct_count;
	kmem_cache_thread_t *kcp_kct[SPLAT_KMEM_THREADS];
	int kcp_size;
	int kcp_align;
	int kcp_count;
	int kcp_alloc;
	int kcp_rc;
	int kcp_kcd_count;
	kmem_cache_data_t *kcp_kcd[0];
} kmem_cache_priv_t;

static kmem_cache_priv_t *
splat_kmem_cache_test_kcp_alloc(struct file *file, char *name,
				int size, int align, int alloc, int count)
{
	kmem_cache_priv_t *kcp;

	kcp = vmem_zalloc(sizeof(kmem_cache_priv_t) +
			  count * sizeof(kmem_cache_data_t *), KM_SLEEP);
	if (!kcp)
		return NULL;

	kcp->kcp_magic = SPLAT_KMEM_TEST_MAGIC;
	kcp->kcp_file = file;
	kcp->kcp_cache = NULL;
	spin_lock_init(&kcp->kcp_lock);
	init_waitqueue_head(&kcp->kcp_ctl_waitq);
	init_waitqueue_head(&kcp->kcp_thr_waitq);
	kcp->kcp_flags = 0;
	kcp->kcp_kct_count = -1;
	kcp->kcp_size = size;
	kcp->kcp_align = align;
	kcp->kcp_count = 0;
	kcp->kcp_alloc = alloc;
	kcp->kcp_rc = 0;
	kcp->kcp_kcd_count = count;

	return kcp;
}

static void
splat_kmem_cache_test_kcp_free(kmem_cache_priv_t *kcp)
{
	vmem_free(kcp, sizeof(kmem_cache_priv_t) +
		  kcp->kcp_kcd_count * sizeof(kmem_cache_data_t *));
}

static kmem_cache_thread_t *
splat_kmem_cache_test_kct_alloc(int id, int count)
{
	kmem_cache_thread_t *kct;

	ASSERTF(id < SPLAT_KMEM_THREADS, "id=%d\n", id);
	kct = vmem_zalloc(sizeof(kmem_cache_thread_t) +
			  count * sizeof(kmem_cache_data_t *), KM_SLEEP);
	if (!kct)
		return NULL;

	spin_lock_init(&kct->kct_lock);
	kct->kct_cache = NULL;
	kct->kct_id = id;
	kct->kct_kcd_count = count;

	return kct;
}

static void
splat_kmem_cache_test_kct_free(kmem_cache_thread_t *kct)
{
	vmem_free(kct, sizeof(kmem_cache_thread_t) +
		  kct->kct_kcd_count * sizeof(kmem_cache_data_t *));
}

static int
splat_kmem_cache_test_constructor(void *ptr, void *priv, int flags)
{
	kmem_cache_priv_t *kcp = (kmem_cache_priv_t *)priv;
	kmem_cache_data_t *kcd = (kmem_cache_data_t *)ptr;

	if (kcd && kcp) {
		kcd->kcd_magic = kcp->kcp_magic;
		kcd->kcd_flag = 1;
		memset(kcd->kcd_buf, 0xaa, kcp->kcp_size - (sizeof *kcd));
		kcp->kcp_count++;
	}

	return 0;
}

static void
splat_kmem_cache_test_destructor(void *ptr, void *priv)
{
	kmem_cache_priv_t *kcp = (kmem_cache_priv_t *)priv;
	kmem_cache_data_t *kcd = (kmem_cache_data_t *)ptr;

	if (kcd && kcp) {
		kcd->kcd_magic = 0;
		kcd->kcd_flag = 0;
		memset(kcd->kcd_buf, 0xbb, kcp->kcp_size - (sizeof *kcd));
		kcp->kcp_count--;
	}

	return;
}

/*
 * Generic reclaim function which assumes that all objects may
 * be reclaimed at any time.  We free a small  percentage of the
 * objects linked off the kcp or kct[] every time we are called.
 */
static void
splat_kmem_cache_test_reclaim(void *priv)
{
	kmem_cache_priv_t *kcp = (kmem_cache_priv_t *)priv;
	kmem_cache_thread_t *kct;
	int i, j, count;

	ASSERT(kcp->kcp_magic == SPLAT_KMEM_TEST_MAGIC);
	count = kcp->kcp_kcd_count * SPLAT_KMEM_OBJ_RECLAIM / 100;

	/* Objects directly attached to the kcp */
	spin_lock(&kcp->kcp_lock);
	for (i = 0; i < kcp->kcp_kcd_count; i++) {
		if (kcp->kcp_kcd[i]) {
			kmem_cache_free(kcp->kcp_cache, kcp->kcp_kcd[i]);
			kcp->kcp_kcd[i] = NULL;

			if ((--count) == 0)
				break;
		}
	}
	spin_unlock(&kcp->kcp_lock);

	/* No threads containing objects to consider */
	if (kcp->kcp_kct_count == -1)
		return;

	/* Objects attached to a kct thread */
	for (i = 0; i < kcp->kcp_kct_count; i++) {
		spin_lock(&kcp->kcp_lock);
		kct = kcp->kcp_kct[i];
		spin_unlock(&kcp->kcp_lock);
		if (!kct)
			continue;

		spin_lock(&kct->kct_lock);
		count = kct->kct_kcd_count * SPLAT_KMEM_OBJ_RECLAIM / 100;

		for (j = 0; j < kct->kct_kcd_count; j++) {
			if (kct->kct_kcd[j]) {
				kmem_cache_free(kcp->kcp_cache,kct->kct_kcd[j]);
				kct->kct_kcd[j] = NULL;

				if ((--count) == 0)
					break;
			}
		}
		spin_unlock(&kct->kct_lock);
	}

	return;
}

static int
splat_kmem_cache_test_threads(kmem_cache_priv_t *kcp, int threads)
{
	int rc;

	spin_lock(&kcp->kcp_lock);
	rc = (kcp->kcp_kct_count == threads);
	spin_unlock(&kcp->kcp_lock);

	return rc;
}

static int
splat_kmem_cache_test_flags(kmem_cache_priv_t *kcp, int flags)
{
	int rc;

	spin_lock(&kcp->kcp_lock);
	rc = (kcp->kcp_flags & flags);
	spin_unlock(&kcp->kcp_lock);

	return rc;
}

static void
splat_kmem_cache_test_thread(void *arg)
{
	kmem_cache_priv_t *kcp = (kmem_cache_priv_t *)arg;
	kmem_cache_thread_t *kct;
	int rc = 0, id, i;
	void *obj;

	ASSERT(kcp->kcp_magic == SPLAT_KMEM_TEST_MAGIC);

	/* Assign thread ids */
	spin_lock(&kcp->kcp_lock);
	if (kcp->kcp_kct_count == -1)
		kcp->kcp_kct_count = 0;

	id = kcp->kcp_kct_count;
	kcp->kcp_kct_count++;
	spin_unlock(&kcp->kcp_lock);

	kct = splat_kmem_cache_test_kct_alloc(id, kcp->kcp_alloc);
	if (!kct) {
		rc = -ENOMEM;
		goto out;
	}

	spin_lock(&kcp->kcp_lock);
	kcp->kcp_kct[id] = kct;
	spin_unlock(&kcp->kcp_lock);

	/* Wait for all threads to have started and report they are ready */
	if (kcp->kcp_kct_count == SPLAT_KMEM_THREADS)
		wake_up(&kcp->kcp_ctl_waitq);

	wait_event(kcp->kcp_thr_waitq,
		splat_kmem_cache_test_flags(kcp, KCP_FLAG_READY));

	/*
	 * Updates to kct->kct_kcd[] are performed under a spin_lock so
	 * they may safely run concurrent with the reclaim function.  If
	 * we are not in a low memory situation we have one lock per-
	 * thread so they are not expected to be contended.
	 */
	for (i = 0; i < kct->kct_kcd_count; i++) {
		obj = kmem_cache_alloc(kcp->kcp_cache, KM_SLEEP);
		spin_lock(&kct->kct_lock);
		kct->kct_kcd[i] = obj;
		spin_unlock(&kct->kct_lock);
	}

	for (i = 0; i < kct->kct_kcd_count; i++) {
		spin_lock(&kct->kct_lock);
		if (kct->kct_kcd[i]) {
			kmem_cache_free(kcp->kcp_cache, kct->kct_kcd[i]);
			kct->kct_kcd[i] = NULL;
		}
		spin_unlock(&kct->kct_lock);
	}
out:
	spin_lock(&kcp->kcp_lock);
	if (kct) {
		splat_kmem_cache_test_kct_free(kct);
		kcp->kcp_kct[id] = kct = NULL;
	}

	if (!kcp->kcp_rc)
		kcp->kcp_rc = rc;

	if ((--kcp->kcp_kct_count) == 0)
		wake_up(&kcp->kcp_ctl_waitq);

	spin_unlock(&kcp->kcp_lock);

	thread_exit();
}

static int
splat_kmem_cache_test(struct file *file, void *arg, char *name,
		      int size, int align, int flags)
{
	kmem_cache_priv_t *kcp;
	kmem_cache_data_t *kcd;
	int rc = 0, max;

	kcp = splat_kmem_cache_test_kcp_alloc(file, name, size, align, 0, 1);
	if (!kcp) {
		splat_vprint(file, name, "Unable to create '%s'\n", "kcp");
		return -ENOMEM;
	}

	kcp->kcp_kcd[0] = NULL;
	kcp->kcp_cache =
		kmem_cache_create(SPLAT_KMEM_CACHE_NAME,
				  kcp->kcp_size, kcp->kcp_align,
				  splat_kmem_cache_test_constructor,
				  splat_kmem_cache_test_destructor,
				  NULL, kcp, NULL, flags);
	if (!kcp->kcp_cache) {
		splat_vprint(file, name,
			     "Unable to create '%s'\n",
			     SPLAT_KMEM_CACHE_NAME);
		rc = -ENOMEM;
		goto out_free;
	}

	kcd = kmem_cache_alloc(kcp->kcp_cache, KM_SLEEP);
	if (!kcd) {
		splat_vprint(file, name,
			     "Unable to allocate from '%s'\n",
			     SPLAT_KMEM_CACHE_NAME);
		rc = -EINVAL;
		goto out_free;
	}
	spin_lock(&kcp->kcp_lock);
	kcp->kcp_kcd[0] = kcd;
	spin_unlock(&kcp->kcp_lock);

	if (!kcp->kcp_kcd[0]->kcd_flag) {
		splat_vprint(file, name,
			     "Failed to run contructor for '%s'\n",
			     SPLAT_KMEM_CACHE_NAME);
		rc = -EINVAL;
		goto out_free;
	}

	if (kcp->kcp_kcd[0]->kcd_magic != kcp->kcp_magic) {
		splat_vprint(file, name,
			     "Failed to pass private data to constructor "
			     "for '%s'\n", SPLAT_KMEM_CACHE_NAME);
		rc = -EINVAL;
		goto out_free;
	}

	max = kcp->kcp_count;
	spin_lock(&kcp->kcp_lock);
	kmem_cache_free(kcp->kcp_cache, kcp->kcp_kcd[0]);
	kcp->kcp_kcd[0] = NULL;
	spin_unlock(&kcp->kcp_lock);

	/* Destroy the entire cache which will force destructors to
	 * run and we can verify one was called for every object */
	kmem_cache_destroy(kcp->kcp_cache);
	if (kcp->kcp_count) {
		splat_vprint(file, name,
			     "Failed to run destructor on all slab objects "
			     "for '%s'\n", SPLAT_KMEM_CACHE_NAME);
		rc = -EINVAL;
	}

	splat_vprint(file, name,
		     "Successfully ran ctors/dtors for %d elements in '%s'\n",
		     max, SPLAT_KMEM_CACHE_NAME);

	return rc;

out_free:
	if (kcp->kcp_kcd[0]) {
		spin_lock(&kcp->kcp_lock);
		kmem_cache_free(kcp->kcp_cache, kcp->kcp_kcd[0]);
		kcp->kcp_kcd[0] = NULL;
		spin_unlock(&kcp->kcp_lock);
	}

	if (kcp->kcp_cache)
		kmem_cache_destroy(kcp->kcp_cache);

	splat_kmem_cache_test_kcp_free(kcp);

	return rc;
}

static int
splat_kmem_cache_thread_test(struct file *file, void *arg, char *name,
			     int size, int alloc, int max_time)
{
	kmem_cache_priv_t *kcp;
	kthread_t *thr;
	struct timespec start, stop, delta;
	char cache_name[32];
	int i, rc = 0;

	kcp = splat_kmem_cache_test_kcp_alloc(file, name, size, 0, alloc, 0);
	if (!kcp) {
		splat_vprint(file, name, "Unable to create '%s'\n", "kcp");
		return -ENOMEM;
	}

	(void)snprintf(cache_name, 32, "%s-%d-%d",
		       SPLAT_KMEM_CACHE_NAME, size, alloc);
	kcp->kcp_cache =
		kmem_cache_create(cache_name, kcp->kcp_size, 0,
				  splat_kmem_cache_test_constructor,
				  splat_kmem_cache_test_destructor,
				  splat_kmem_cache_test_reclaim,
				  kcp, NULL, KMC_KMEM);
	if (!kcp->kcp_cache) {
		splat_vprint(file, name, "Unable to create '%s'\n", cache_name);
		rc = -ENOMEM;
		goto out_kcp;
	}

	start = current_kernel_time();

	for (i = 0; i < SPLAT_KMEM_THREADS; i++) {
		thr = thread_create(NULL, 0,
				    splat_kmem_cache_test_thread,
				    kcp, 0, &p0, TS_RUN, minclsyspri);
		if (thr == NULL) {
			rc = -ESRCH;
			goto out_cache;
		}
	}

	/* Sleep until all threads have started, then set the ready
	 * flag and wake them all up for maximum concurrency. */
	wait_event(kcp->kcp_ctl_waitq,
		   splat_kmem_cache_test_threads(kcp, SPLAT_KMEM_THREADS));

	spin_lock(&kcp->kcp_lock);
	kcp->kcp_flags |= KCP_FLAG_READY;
	spin_unlock(&kcp->kcp_lock);
	wake_up_all(&kcp->kcp_thr_waitq);

	/* Sleep until all thread have finished */
	wait_event(kcp->kcp_ctl_waitq, splat_kmem_cache_test_threads(kcp, 0));

	stop = current_kernel_time();
	delta = timespec_sub(stop, start);

	splat_vprint(file, name,
		     "%-22s %2ld.%09ld\t"
		     "%lu/%lu/%lu\t%lu/%lu/%lu\n",
		     kcp->kcp_cache->skc_name,
		     delta.tv_sec, delta.tv_nsec,
		     (unsigned long)kcp->kcp_cache->skc_slab_total,
		     (unsigned long)kcp->kcp_cache->skc_slab_max,
		     (unsigned long)(kcp->kcp_alloc *
				    SPLAT_KMEM_THREADS /
				    SPL_KMEM_CACHE_OBJ_PER_SLAB),
		     (unsigned long)kcp->kcp_cache->skc_obj_total,
		     (unsigned long)kcp->kcp_cache->skc_obj_max,
		     (unsigned long)(kcp->kcp_alloc *
				     SPLAT_KMEM_THREADS));

	if (delta.tv_sec >= max_time)
		rc = -ETIME;

	if (!rc && kcp->kcp_rc)
		rc = kcp->kcp_rc;

out_cache:
	kmem_cache_destroy(kcp->kcp_cache);
out_kcp:
	splat_kmem_cache_test_kcp_free(kcp);
	return rc;
}

/* Validate small object cache behavior for dynamic/kmem/vmem caches */
static int
splat_kmem_test5(struct file *file, void *arg)
{
	char *name = SPLAT_KMEM_TEST5_NAME;
	int rc;

	rc = splat_kmem_cache_test(file, arg, name, 128, 0, 0);
	if (rc)
		return rc;

	rc = splat_kmem_cache_test(file, arg, name, 128, 0, KMC_KMEM);
	if (rc)
		return rc;

	return splat_kmem_cache_test(file, arg, name, 128, 0, KMC_VMEM);
}

/* Validate large object cache behavior for dynamic/kmem/vmem caches */
static int
splat_kmem_test6(struct file *file, void *arg)
{
	char *name = SPLAT_KMEM_TEST6_NAME;
	int rc;

	rc = splat_kmem_cache_test(file, arg, name, 128*1024, 0, 0);
	if (rc)
		return rc;

	rc = splat_kmem_cache_test(file, arg, name, 128*1024, 0, KMC_KMEM);
	if (rc)
		return rc;

	return splat_kmem_cache_test(file, arg, name, 128*1028, 0, KMC_VMEM);
}

/* Validate object alignment cache behavior for caches */
static int
splat_kmem_test7(struct file *file, void *arg)
{
	char *name = SPLAT_KMEM_TEST7_NAME;
	int i, rc;

	for (i = 8; i <= PAGE_SIZE; i *= 2) {
		rc = splat_kmem_cache_test(file, arg, name, 157, i, 0);
		if (rc)
			return rc;
	}

	return rc;
}

static int
splat_kmem_test8(struct file *file, void *arg)
{
	kmem_cache_priv_t *kcp;
	kmem_cache_data_t *kcd;
	int i, j, rc = 0;

	kcp = splat_kmem_cache_test_kcp_alloc(file, SPLAT_KMEM_TEST8_NAME,
					      256, 0, 0, SPLAT_KMEM_OBJ_COUNT);
	if (!kcp) {
		splat_vprint(file, SPLAT_KMEM_TEST8_NAME,
			     "Unable to create '%s'\n", "kcp");
		return -ENOMEM;
	}

	kcp->kcp_cache =
		kmem_cache_create(SPLAT_KMEM_CACHE_NAME, kcp->kcp_size, 0,
				  splat_kmem_cache_test_constructor,
				  splat_kmem_cache_test_destructor,
				  splat_kmem_cache_test_reclaim,
				  kcp, NULL, 0);
	if (!kcp->kcp_cache) {
		splat_kmem_cache_test_kcp_free(kcp);
		splat_vprint(file, SPLAT_KMEM_TEST8_NAME,
			   "Unable to create '%s'\n", SPLAT_KMEM_CACHE_NAME);
		return -ENOMEM;
	}

	for (i = 0; i < SPLAT_KMEM_OBJ_COUNT; i++) {
		kcd = kmem_cache_alloc(kcp->kcp_cache, KM_SLEEP);
		spin_lock(&kcp->kcp_lock);
		kcp->kcp_kcd[i] = kcd;
		spin_unlock(&kcp->kcp_lock);
		if (!kcd) {
			splat_vprint(file, SPLAT_KMEM_TEST8_NAME,
				   "Unable to allocate from '%s'\n",
				   SPLAT_KMEM_CACHE_NAME);
		}
	}

	/* Request the slab cache free any objects it can.  For a few reasons
	 * this may not immediately result in more free memory even if objects
	 * are freed.  First off, due to fragmentation we may not be able to
	 * reclaim any slabs.  Secondly, even if we do we fully clear some
	 * slabs we will not want to immedately reclaim all of them because
	 * we may contend with cache allocs and thrash.  What we want to see
	 * is the slab size decrease more gradually as it becomes clear they
	 * will not be needed.  This should be acheivable in less than minute
	 * if it takes longer than this something has gone wrong.
	 */
	for (i = 0; i < 60; i++) {
		kmem_cache_reap_now(kcp->kcp_cache);
		splat_vprint(file, SPLAT_KMEM_TEST8_NAME,
			     "%s cache objects %d, slabs %u/%u objs %u/%u mags ",
			     SPLAT_KMEM_CACHE_NAME, kcp->kcp_count,
			    (unsigned)kcp->kcp_cache->skc_slab_alloc,
			    (unsigned)kcp->kcp_cache->skc_slab_total,
			    (unsigned)kcp->kcp_cache->skc_obj_alloc,
			    (unsigned)kcp->kcp_cache->skc_obj_total);

		for_each_online_cpu(j)
			splat_print(file, "%u/%u ",
				     kcp->kcp_cache->skc_mag[j]->skm_avail,
				     kcp->kcp_cache->skc_mag[j]->skm_size);

		splat_print(file, "%s\n", "");

		if (kcp->kcp_cache->skc_obj_total == 0)
			break;

		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(HZ);
	}

	if (kcp->kcp_cache->skc_obj_total == 0) {
		splat_vprint(file, SPLAT_KMEM_TEST8_NAME,
			"Successfully created %d objects "
			"in cache %s and reclaimed them\n",
			SPLAT_KMEM_OBJ_COUNT, SPLAT_KMEM_CACHE_NAME);
	} else {
		splat_vprint(file, SPLAT_KMEM_TEST8_NAME,
			"Failed to reclaim %u/%d objects from cache %s\n",
			(unsigned)kcp->kcp_cache->skc_obj_total,
			SPLAT_KMEM_OBJ_COUNT, SPLAT_KMEM_CACHE_NAME);
		rc = -ENOMEM;
	}

	/* Cleanup our mess (for failure case of time expiring) */
	spin_lock(&kcp->kcp_lock);
	for (i = 0; i < SPLAT_KMEM_OBJ_COUNT; i++)
		if (kcp->kcp_kcd[i])
			kmem_cache_free(kcp->kcp_cache, kcp->kcp_kcd[i]);
	spin_unlock(&kcp->kcp_lock);

	kmem_cache_destroy(kcp->kcp_cache);
	splat_kmem_cache_test_kcp_free(kcp);

	return rc;
}

static int
splat_kmem_test9(struct file *file, void *arg)
{
	kmem_cache_priv_t *kcp;
	kmem_cache_data_t *kcd;
	int i, j, rc = 0, count = SPLAT_KMEM_OBJ_COUNT * 128;

	kcp = splat_kmem_cache_test_kcp_alloc(file, SPLAT_KMEM_TEST9_NAME,
					      256, 0, 0, count);
	if (!kcp) {
		splat_vprint(file, SPLAT_KMEM_TEST9_NAME,
			     "Unable to create '%s'\n", "kcp");
		return -ENOMEM;
	}

	kcp->kcp_cache =
		kmem_cache_create(SPLAT_KMEM_CACHE_NAME, kcp->kcp_size, 0,
				  splat_kmem_cache_test_constructor,
				  splat_kmem_cache_test_destructor,
				  NULL, kcp, NULL, 0);
	if (!kcp->kcp_cache) {
		splat_kmem_cache_test_kcp_free(kcp);
		splat_vprint(file, SPLAT_KMEM_TEST9_NAME,
			   "Unable to create '%s'\n", SPLAT_KMEM_CACHE_NAME);
		return -ENOMEM;
	}

	for (i = 0; i < count; i++) {
		kcd = kmem_cache_alloc(kcp->kcp_cache, KM_SLEEP);
		spin_lock(&kcp->kcp_lock);
		kcp->kcp_kcd[i] = kcd;
		spin_unlock(&kcp->kcp_lock);
		if (!kcd) {
			splat_vprint(file, SPLAT_KMEM_TEST9_NAME,
				   "Unable to allocate from '%s'\n",
				   SPLAT_KMEM_CACHE_NAME);
		}
	}

	spin_lock(&kcp->kcp_lock);
	for (i = 0; i < count; i++)
		if (kcp->kcp_kcd[i])
			kmem_cache_free(kcp->kcp_cache, kcp->kcp_kcd[i]);
	spin_unlock(&kcp->kcp_lock);

	/* We have allocated a large number of objects thus creating a
	 * large number of slabs and then free'd them all.  However since
	 * there should be little memory pressure at the moment those
	 * slabs have not been freed.  What we want to see is the slab
	 * size decrease gradually as it becomes clear they will not be
	 * be needed.  This should be acheivable in less than minute
	 * if it takes longer than this something has gone wrong.
	 */
	for (i = 0; i < 60; i++) {
		splat_vprint(file, SPLAT_KMEM_TEST9_NAME,
			     "%s cache objects %d, slabs %u/%u objs %u/%u mags ",
			     SPLAT_KMEM_CACHE_NAME, kcp->kcp_count,
			    (unsigned)kcp->kcp_cache->skc_slab_alloc,
			    (unsigned)kcp->kcp_cache->skc_slab_total,
			    (unsigned)kcp->kcp_cache->skc_obj_alloc,
			    (unsigned)kcp->kcp_cache->skc_obj_total);

		for_each_online_cpu(j)
			splat_print(file, "%u/%u ",
				     kcp->kcp_cache->skc_mag[j]->skm_avail,
				     kcp->kcp_cache->skc_mag[j]->skm_size);

		splat_print(file, "%s\n", "");

		if (kcp->kcp_cache->skc_obj_total == 0)
			break;

		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(HZ);
	}

	if (kcp->kcp_cache->skc_obj_total == 0) {
		splat_vprint(file, SPLAT_KMEM_TEST9_NAME,
			"Successfully created %d objects "
			"in cache %s and reclaimed them\n",
			count, SPLAT_KMEM_CACHE_NAME);
	} else {
		splat_vprint(file, SPLAT_KMEM_TEST9_NAME,
			"Failed to reclaim %u/%d objects from cache %s\n",
			(unsigned)kcp->kcp_cache->skc_obj_total, count,
			SPLAT_KMEM_CACHE_NAME);
		rc = -ENOMEM;
	}

	kmem_cache_destroy(kcp->kcp_cache);
	splat_kmem_cache_test_kcp_free(kcp);

	return rc;
}

/*
 * This test creates N threads with a shared kmem cache.  They then all
 * concurrently allocate and free from the cache to stress the locking and
 * concurrent cache performance.  If any one test takes longer than 5
 * seconds to complete it is treated as a failure and may indicate a
 * performance regression.  On my test system no one test takes more
 * than 1 second to complete so a 5x slowdown likely a problem.
 */
static int
splat_kmem_test10(struct file *file, void *arg)
{
	uint64_t size, alloc, rc = 0;

	for (size = 16; size <= 1024*1024; size *= 2) {

		splat_vprint(file, SPLAT_KMEM_TEST10_NAME, "%-22s  %s", "name",
			     "time (sec)\tslabs       \tobjs	\thash\n");
		splat_vprint(file, SPLAT_KMEM_TEST10_NAME, "%-22s  %s", "",
			     "	  \ttot/max/calc\ttot/max/calc\n");

		for (alloc = 1; alloc <= 1024; alloc *= 2) {

			/* Skip tests which exceed available memory.  We
			 * leverage availrmem here for some extra testing */
			if (size * alloc * SPLAT_KMEM_THREADS > availrmem / 2)
				continue;

			rc = splat_kmem_cache_thread_test(file, arg,
				SPLAT_KMEM_TEST10_NAME, size, alloc, 5);
			if (rc)
				break;
		}
	}

	return rc;
}

/*
 * This test creates N threads with a shared kmem cache which overcommits
 * memory by 4x.  This makes it impossible for the slab to satify the
 * thread requirements without having its reclaim hook run which will
 * free objects back for use.  This behavior is triggered by the linum VM
 * detecting a low memory condition on the node and invoking the shrinkers.
 * This should allow all the threads to complete while avoiding deadlock
 * and for the most part out of memory events.  This is very tough on the
 * system so it is possible the test app may get oom'ed.
 */
static int
splat_kmem_test11(struct file *file, void *arg)
{
	uint64_t size, alloc, rc;

	size = 256*1024;
	alloc = ((4 * physmem * PAGE_SIZE) / size) / SPLAT_KMEM_THREADS;

	splat_vprint(file, SPLAT_KMEM_TEST11_NAME, "%-22s  %s", "name",
		     "time (sec)\tslabs       \tobjs	\thash\n");
	splat_vprint(file, SPLAT_KMEM_TEST11_NAME, "%-22s  %s", "",
		     "	  \ttot/max/calc\ttot/max/calc\n");

	rc = splat_kmem_cache_thread_test(file, arg,
		SPLAT_KMEM_TEST11_NAME, size, alloc, 60);

	return rc;
}

/*
 * Check vmem_size() behavior by acquiring the alloc/free/total vmem
 * space, then allocate a known buffer size from vmem space.  We can
 * then check that vmem_size() values were updated properly with in
 * a fairly small tolerence.  The tolerance is important because we
 * are not the only vmem consumer on the system.  Other unrelated
 * allocations might occur during the small test window.  The vmem
 * allocation itself may also add in a little extra private space to
 * the buffer.  Finally, verify total space always remains unchanged.
 */
static int
splat_kmem_test12(struct file *file, void *arg)
{
	ssize_t alloc1, free1, total1;
	ssize_t alloc2, free2, total2;
	int size = 8*1024*1024;
	void *ptr;

	alloc1 = vmem_size(NULL, VMEM_ALLOC);
	free1  = vmem_size(NULL, VMEM_FREE);
	total1 = vmem_size(NULL, VMEM_ALLOC | VMEM_FREE);
	splat_vprint(file, SPLAT_KMEM_TEST12_NAME, "Vmem alloc=%d free=%d "
	             "total=%d\n", (int)alloc1, (int)free1, (int)total1);

	splat_vprint(file, SPLAT_KMEM_TEST12_NAME, "Alloc %d bytes\n", size);
	ptr = vmem_alloc(size, KM_SLEEP);
	if (!ptr) {
		splat_vprint(file, SPLAT_KMEM_TEST12_NAME,
		             "Failed to alloc %d bytes\n", size);
		return -ENOMEM;
	}

	alloc2 = vmem_size(NULL, VMEM_ALLOC);
	free2  = vmem_size(NULL, VMEM_FREE);
	total2 = vmem_size(NULL, VMEM_ALLOC | VMEM_FREE);
	splat_vprint(file, SPLAT_KMEM_TEST12_NAME, "Vmem alloc=%d free=%d "
	             "total=%d\n", (int)alloc2, (int)free2, (int)total2);

	splat_vprint(file, SPLAT_KMEM_TEST12_NAME, "Free %d bytes\n", size);
	vmem_free(ptr, size);
	if (alloc2 < (alloc1 + size - (size / 100)) ||
	    alloc2 > (alloc1 + size + (size / 100))) {
		splat_vprint(file, SPLAT_KMEM_TEST12_NAME,
		             "Failed VMEM_ALLOC size: %d != %d+%d (+/- 1%%)\n",
		             (int)alloc2, (int)alloc1, size);
		return -ERANGE;
	}

	if (free2 < (free1 - size - (size / 100)) ||
	    free2 > (free1 - size + (size / 100))) {
		splat_vprint(file, SPLAT_KMEM_TEST12_NAME,
		             "Failed VMEM_FREE size: %d != %d-%d (+/- 1%%)\n",
		             (int)free2, (int)free1, size);
		return -ERANGE;
	}

	if (total1 != total2) {
		splat_vprint(file, SPLAT_KMEM_TEST12_NAME,
		             "Failed VMEM_ALLOC | VMEM_FREE not constant: "
		             "%d != %d\n", (int)total2, (int)total1);
		return -ERANGE;
	}

	splat_vprint(file, SPLAT_KMEM_TEST12_NAME,
	             "VMEM_ALLOC within tolerance: ~%d%% (%d/%d)\n",
	             (int)(((alloc1 + size) - alloc2) * 100 / size),
	             (int)((alloc1 + size) - alloc2), size);
	splat_vprint(file, SPLAT_KMEM_TEST12_NAME,
	             "VMEM_FREE within tolerance:  ~%d%% (%d/%d)\n",
	             (int)(((free1 - size) - free2) * 100 / size),
	             (int)((free1 - size) - free2), size);

	return 0;
}

splat_subsystem_t *
splat_kmem_init(void)
{
	splat_subsystem_t *sub;

	sub = kmalloc(sizeof(*sub), GFP_KERNEL);
	if (sub == NULL)
		return NULL;

	memset(sub, 0, sizeof(*sub));
	strncpy(sub->desc.name, SPLAT_KMEM_NAME, SPLAT_NAME_SIZE);
	strncpy(sub->desc.desc, SPLAT_KMEM_DESC, SPLAT_DESC_SIZE);
	INIT_LIST_HEAD(&sub->subsystem_list);
	INIT_LIST_HEAD(&sub->test_list);
	spin_lock_init(&sub->test_lock);
	sub->desc.id = SPLAT_SUBSYSTEM_KMEM;

	SPLAT_TEST_INIT(sub, SPLAT_KMEM_TEST1_NAME, SPLAT_KMEM_TEST1_DESC,
			SPLAT_KMEM_TEST1_ID, splat_kmem_test1);
	SPLAT_TEST_INIT(sub, SPLAT_KMEM_TEST2_NAME, SPLAT_KMEM_TEST2_DESC,
			SPLAT_KMEM_TEST2_ID, splat_kmem_test2);
	SPLAT_TEST_INIT(sub, SPLAT_KMEM_TEST3_NAME, SPLAT_KMEM_TEST3_DESC,
			SPLAT_KMEM_TEST3_ID, splat_kmem_test3);
	SPLAT_TEST_INIT(sub, SPLAT_KMEM_TEST4_NAME, SPLAT_KMEM_TEST4_DESC,
			SPLAT_KMEM_TEST4_ID, splat_kmem_test4);
	SPLAT_TEST_INIT(sub, SPLAT_KMEM_TEST5_NAME, SPLAT_KMEM_TEST5_DESC,
			SPLAT_KMEM_TEST5_ID, splat_kmem_test5);
	SPLAT_TEST_INIT(sub, SPLAT_KMEM_TEST6_NAME, SPLAT_KMEM_TEST6_DESC,
			SPLAT_KMEM_TEST6_ID, splat_kmem_test6);
	SPLAT_TEST_INIT(sub, SPLAT_KMEM_TEST7_NAME, SPLAT_KMEM_TEST7_DESC,
			SPLAT_KMEM_TEST7_ID, splat_kmem_test7);
	SPLAT_TEST_INIT(sub, SPLAT_KMEM_TEST8_NAME, SPLAT_KMEM_TEST8_DESC,
			SPLAT_KMEM_TEST8_ID, splat_kmem_test8);
	SPLAT_TEST_INIT(sub, SPLAT_KMEM_TEST9_NAME, SPLAT_KMEM_TEST9_DESC,
			SPLAT_KMEM_TEST9_ID, splat_kmem_test9);
	SPLAT_TEST_INIT(sub, SPLAT_KMEM_TEST10_NAME, SPLAT_KMEM_TEST10_DESC,
			SPLAT_KMEM_TEST10_ID, splat_kmem_test10);
	SPLAT_TEST_INIT(sub, SPLAT_KMEM_TEST11_NAME, SPLAT_KMEM_TEST11_DESC,
			SPLAT_KMEM_TEST11_ID, splat_kmem_test11);
	SPLAT_TEST_INIT(sub, SPLAT_KMEM_TEST12_NAME, SPLAT_KMEM_TEST12_DESC,
			SPLAT_KMEM_TEST12_ID, splat_kmem_test12);

	return sub;
}

void
splat_kmem_fini(splat_subsystem_t *sub)
{
	ASSERT(sub);
	SPLAT_TEST_FINI(sub, SPLAT_KMEM_TEST12_ID);
	SPLAT_TEST_FINI(sub, SPLAT_KMEM_TEST11_ID);
	SPLAT_TEST_FINI(sub, SPLAT_KMEM_TEST10_ID);
	SPLAT_TEST_FINI(sub, SPLAT_KMEM_TEST9_ID);
	SPLAT_TEST_FINI(sub, SPLAT_KMEM_TEST8_ID);
	SPLAT_TEST_FINI(sub, SPLAT_KMEM_TEST7_ID);
	SPLAT_TEST_FINI(sub, SPLAT_KMEM_TEST6_ID);
	SPLAT_TEST_FINI(sub, SPLAT_KMEM_TEST5_ID);
	SPLAT_TEST_FINI(sub, SPLAT_KMEM_TEST4_ID);
	SPLAT_TEST_FINI(sub, SPLAT_KMEM_TEST3_ID);
	SPLAT_TEST_FINI(sub, SPLAT_KMEM_TEST2_ID);
	SPLAT_TEST_FINI(sub, SPLAT_KMEM_TEST1_ID);

	kfree(sub);
}

int
splat_kmem_id(void) {
	return SPLAT_SUBSYSTEM_KMEM;
}