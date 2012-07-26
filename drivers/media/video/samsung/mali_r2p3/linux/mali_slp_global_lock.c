#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/hash.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <asm/current.h>
#include <linux/sched.h>

#include "mali_slp_global_lock.h"

#define SGL_WARN(x, y...)	printk(KERN_INFO "[SGL_WARN(%d,%d)](%s):%d " x " \n" , current->tgid, current->pid, __FUNCTION__, __LINE__, ##y);

#if 0 /* LOG */

#define SGL_LOG(x, y...)	printk(KERN_INFO "[SGL_LOG(%d,%d)](%s):%d " x " \n" , current->tgid, current->pid, __FUNCTION__, __LINE__, ##y);
#define SGL_DEBUG(x, y...)	printk(KERN_INFO "[SGL_DEBUG(%d,%d)](%s):%d " x " \n" , current->tgid, current->pid, __FUNCTION__, __LINE__, ##y);

#else

#define SGL_LOG(x, y...)
#define SGL_DEBUG(x, y...)

#endif

static struct sgl_global {
	int major;
	struct class		*class;
	struct device		*device;
	void				*locks;			/* global lock table */
	int					refcnt;			/* ref count of sgl_global */
	struct mutex		mutex;
} sgl_global;

struct sgl_session_data {

	void				*inited_locks;	/* per session initialized lock list */
	struct mutex		inited_mutex;
	void				*locked_locks;	/* per session locked lock list */
	struct mutex		locked_mutex;
};

struct sgl_lock {
	unsigned int		key;			/* key of this lock */
	unsigned int		timeout_ms;		/* timeout in ms */
	unsigned int		refcnt;			/* ref count of initialization */
	wait_queue_head_t	waiting_queue;	/* waiting queue */
	struct list_head	waiting_list;	/* waiting list */
	struct mutex		waiting_list_mutex;
	unsigned int		locked;			/* flag if this lock is locked */
	unsigned int		owner;			/* session data */
};

struct sgl_list {
	struct list_head	list;
	struct sgl_lock		*lock;
};

/**************** hash code start ***************/
#define SGL_HASH_BITS		4
#define SGL_HASH_ENTRIES	(1 << SGL_HASH_BITS)

struct sgl_hash_head {
	struct hlist_head	head;			/* hash_head */
	struct mutex		mutex;
};

struct sgl_hash_node {
	unsigned int		key;			/* key for lock. must be same as lock->key */
	struct sgl_lock		*lock;			/* lock object */
	struct hlist_node	node;			/* hash node */
};

/* find the sgl_lock object with key in the hash table */
static struct sgl_hash_node *sgl_hash_get_node(struct sgl_hash_head *hash, unsigned int key)
{
	struct sgl_hash_head *hash_head = &hash[hash_32(key, SGL_HASH_BITS)];
	struct sgl_hash_node *hash_node = NULL;
	struct sgl_hash_node *found = NULL;

	struct hlist_head *head = &hash_head->head;
	struct hlist_node *pos;

	SGL_LOG("key %d", key);

	mutex_lock(&hash_head->mutex);
	hlist_for_each_entry(hash_node, pos, head, node) {
		if (hash_node->key == key) {
			found = hash_node;
			break;
		}
	}
	mutex_unlock(&hash_head->mutex);

	SGL_LOG("hash_node: %p", hash_node);

	return found;
}

/* insert the hash entry */
static struct sgl_hash_node *sgl_hash_insert_node(struct sgl_hash_head *hash, unsigned int key)
{
	struct sgl_hash_head *hash_head = &hash[hash_32(key, SGL_HASH_BITS)];
	struct sgl_hash_node *hash_node;

	struct hlist_head *head = &hash_head->head;

	SGL_LOG("key %d", key);

	hash_node = kzalloc(sizeof(struct sgl_hash_node), GFP_KERNEL);
	if (hash_node == NULL)
		return NULL;

	INIT_HLIST_NODE(&hash_node->node);
	mutex_lock(&hash_head->mutex);
	hlist_add_head(&hash_node->node, head);
	mutex_unlock(&hash_head->mutex);

	hash_node->key = key;

	SGL_LOG();

	return hash_node;
}

/* remove the hash entry */
static int sgl_hash_remove_node(struct sgl_hash_head *hash, unsigned int key)
{
	struct sgl_hash_head *hash_head = &hash[hash_32(key, SGL_HASH_BITS)];
	struct sgl_hash_node *hash_node;

	struct hlist_head *head = &hash_head->head;
	struct hlist_node *pos;

	int err = -ENOENT;

	SGL_LOG("key %d", key);

	mutex_lock(&hash_head->mutex);
	hlist_for_each_entry(hash_node, pos, head, node) {
		if (hash_node->key == key) {
			hlist_del(&hash_node->node);
			kfree(hash_node);
			err = 0;
			break;
		}
	}
	mutex_unlock(&hash_head->mutex);

	SGL_LOG();

	return err;
}

/* allocate the hash table */
static struct sgl_hash_head *sgl_hash_create_table(void)
{
	struct sgl_hash_head *hash;

	int i;

	SGL_LOG();

	hash = kzalloc(sizeof(struct sgl_hash_head) * SGL_HASH_ENTRIES, GFP_KERNEL);
	if (hash == NULL)
		return NULL;

	for (i = 0; i < SGL_HASH_ENTRIES; i++) {
		INIT_HLIST_HEAD(&hash[i].head);
		mutex_init(&hash[i].mutex);
	}

	SGL_LOG();

	return hash;
}

/* release the hash table */
static void sgl_hash_destroy_table(struct sgl_hash_head *hash)
{
	SGL_LOG();

	kfree(hash);

	SGL_LOG();

	return;
}

/**************** hash code end ***************/

static struct sgl_lock *sgl_get_lock(void *locks, unsigned int key)
{
	struct sgl_hash_node *hash_node;

	hash_node = sgl_hash_get_node((struct sgl_hash_head *)locks, key);
	if (hash_node == NULL) {
		return NULL;
	}

	return hash_node->lock;
}

static int sgl_insert_lock(void *locks, struct sgl_lock *lock)
{
	struct sgl_hash_node *hash_node;

	hash_node = sgl_hash_insert_node((struct sgl_hash_head *)locks, lock->key);
	if (hash_node == NULL)
		return -ENOMEM;
	hash_node->lock = lock;

	return 0;
}

static int sgl_remove_lock(void *locks, struct sgl_lock *lock)
{
	int err;

	err = sgl_hash_remove_node((struct sgl_hash_head *)locks, lock->key);

	return err;
}

static int sgl_lock_lock(struct sgl_session_data *session_data, unsigned int key)
{
	struct sgl_lock *lock;
	struct sgl_list *list;

	struct list_head waiting_entry;

	unsigned long jiffies;
	int wait_count = 0;
	long ret = 0;

	SGL_LOG("key: %d", key);

	lock = sgl_get_lock(sgl_global.locks, key);
	if (lock == NULL) {
		SGL_LOG("lock is not in the global locks");
		return -ENOENT;
	}

	SGL_DEBUG("lock: %p", lock);
	SGL_DEBUG("key: %d", key);
	INIT_LIST_HEAD(&waiting_entry);
	mutex_lock(&lock->waiting_list_mutex);
	list_add_tail(&waiting_entry, &lock->waiting_list);
	mutex_unlock(&lock->waiting_list_mutex);

	jiffies = msecs_to_jiffies(lock->timeout_ms);

	while (1) {
		SGL_LOG();

		SGL_DEBUG("%d", jiffies);
		SGL_DEBUG("%d", lock->locked);
		SGL_DEBUG("%d", (lock->waiting_list.next == &waiting_entry));
		ret = wait_event_interruptible_timeout(lock->waiting_queue, ((lock->locked == 0) && lock->waiting_list.next == &waiting_entry), jiffies);
		if (ret == 0) {
			SGL_LOG("timed out");
		} else if (ret == -ERESTARTSYS) {
			SGL_LOG("interrupted by signal");
		} else {
			SGL_LOG();
			break;
		}

		if (wait_count++ >= 60) {
			mutex_lock(&lock->waiting_list_mutex);
			list_del(&waiting_entry);
			mutex_unlock(&lock->waiting_list_mutex);
			return -ETIMEDOUT;
		}

		SGL_LOG();
	}

	SGL_LOG();

	lock->owner = (unsigned int)session_data;
	lock->locked = 1;
	lock->refcnt++;

	mutex_lock(&lock->waiting_list_mutex);
	list_del(&waiting_entry);
	mutex_unlock(&lock->waiting_list_mutex);

	/* add to the locked lock list */
	list = kzalloc(sizeof(struct sgl_list), GFP_KERNEL);
	list->lock = lock;
	mutex_lock(&session_data->locked_mutex);
	list_add_tail(&list->list, (struct list_head *)session_data->locked_locks);
	mutex_unlock(&session_data->locked_mutex);

	SGL_LOG();
	SGL_DEBUG();

	return 0;
}

static int _sgl_unlock_lock(struct sgl_lock *lock)
{
	SGL_LOG();

	if (lock == NULL) {
		SGL_LOG("lock == NULL");
		return -EBADRQC;
	}

	if (lock->locked == 0) {
		SGL_LOG("tried to unlock not-locked lock");
		return -EBADRQC;
	}

	lock->locked = 0;
	lock->owner = 0;
	lock->refcnt--;

	if (waitqueue_active(&lock->waiting_queue)) {
		wake_up(&lock->waiting_queue);
	}

	SGL_LOG();

	return 0;
}

static int sgl_unlock_lock(struct sgl_session_data *session_data, unsigned int key)
{
	struct sgl_lock *lock;
	struct sgl_list *pos;

	int err = -ENOENT;

	SGL_LOG("key: %d", key);

	lock = sgl_get_lock(sgl_global.locks, key);
	if (lock == NULL) {
		SGL_LOG("lock is not in the global locks(could be cleaned up)");
		return -ENOENT;
	}

	SGL_DEBUG("lock: %p", lock);
	SGL_DEBUG("key: %d", key);
	if (lock->owner != (unsigned int)session_data) {
		SGL_LOG("tried to unlock the lock not-owned by calling thread(could be cleaned up)");
		return -EBADRQC;
	}

	err = _sgl_unlock_lock(lock);
	if (err < 0)
		SGL_LOG();

	mutex_lock(&session_data->locked_mutex);
	list_for_each_entry(pos, (struct list_head *)session_data->locked_locks, list) {
		if (pos->lock == lock) {
			list_del(&pos->list);
			kfree(pos);
			err = 0;
			break;
		}
	}
	mutex_unlock(&session_data->locked_mutex);

	SGL_LOG();
	SGL_DEBUG();

	return err;
}

static int sgl_init_lock(struct sgl_session_data *session_data, struct sgl_attribute *attr)
{
	struct sgl_lock *lock;
	struct sgl_list *pos;
	struct sgl_list *list;

	int err = 0;

	SGL_LOG("key: %d", attr->key);

	mutex_lock(&sgl_global.mutex);

	/* check the inited lock list */
	mutex_lock(&session_data->inited_mutex);
	list_for_each_entry(pos, (struct list_head *)session_data->inited_locks, list) {
		if (pos->lock->key == attr->key) {
			SGL_LOG("lock with key = %d was already inited(refcnt++)", attr->key);
			pos->lock->refcnt++;
			mutex_unlock(&session_data->inited_mutex);
			goto out_unlock;
		}
	}
	mutex_unlock(&session_data->inited_mutex);

	lock = sgl_get_lock(sgl_global.locks, attr->key);
	if (lock == NULL) {
		/* allocate and add to the global table if this is the first initialization */
		lock = kzalloc(sizeof(struct sgl_lock), GFP_KERNEL);
		if (lock == NULL) {
			err = -ENOMEM;
			goto out_unlock;
		}

		lock->key = attr->key;

		err = sgl_insert_lock((struct sgl_hash_head *)sgl_global.locks, lock);
		if (err < 0)
			goto out_unlock;

		/* default timeout value is 16ms */
		lock->timeout_ms = attr->timeout_ms ? attr->timeout_ms : 16;

		init_waitqueue_head(&lock->waiting_queue);
		INIT_LIST_HEAD(&lock->waiting_list);
		mutex_init(&lock->waiting_list_mutex);
	}

	lock->refcnt++;

	/* add to the inited lock list */
	list = kzalloc(sizeof(struct sgl_list), GFP_KERNEL);
	list->lock = lock;
	mutex_lock(&session_data->inited_mutex);
	list_add_tail(&list->list, (struct list_head *)session_data->inited_locks);
	mutex_unlock(&session_data->inited_mutex);

out_unlock:

	mutex_unlock(&sgl_global.mutex);

	SGL_LOG("lock->refcnt: %d", lock->refcnt);

	return err;
}

static int _sgl_destroy_lock(struct sgl_lock *lock)
{
	int err = 0;

	SGL_LOG();

	if (lock == NULL) {
		SGL_LOG("lock == NULL");
		return -EBADRQC;
	}

	lock->refcnt--;
	if (lock->refcnt == 0) {
		err = sgl_remove_lock((struct sgl_hash_head *)sgl_global.locks, lock);
		if (err < 0)
			return err;

		kfree(lock);
	}

	SGL_LOG();

	return err;
}

static int sgl_destroy_lock(struct sgl_session_data *session_data, unsigned int key)
{
	struct sgl_lock *lock;
	struct sgl_list *pos;
	struct sgl_list *found;

	int err = 0;

	SGL_LOG();

	mutex_lock(&sgl_global.mutex);

	lock = sgl_get_lock(sgl_global.locks, key);
	if (lock == NULL) {
		SGL_LOG("lock is not in the global locks");
		err = -ENOENT;
		goto out_unlock;
	}

	/* check if lock is still locked */
	mutex_lock(&session_data->locked_mutex);
	list_for_each_entry(pos, (struct list_head *)session_data->locked_locks, list) {
		if (pos->lock == lock) {
			SGL_LOG("destroy failed. lock is still locked");
			err = -EBUSY;
			goto out_unlock;
		}
	}
	mutex_unlock(&session_data->locked_mutex);

	/* remove from the inited lock list */
	mutex_lock(&session_data->inited_mutex);
	list_for_each_entry(pos, (struct list_head *)session_data->inited_locks, list) {
		if (pos->lock == lock) {
			found = pos;
			break;
		}
	}
	mutex_unlock(&session_data->inited_mutex);

	err = _sgl_destroy_lock(lock);
	if (err < 0)
		goto out_unlock;

	if (found) {
		list_del(&found->list);
		kfree(found);
	}

out_unlock:

	mutex_unlock(&sgl_global.mutex);

	SGL_LOG();

	return err;
}

#ifdef HAVE_UNLOCKED_IOCTL
static long sgl_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int sgl_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
	struct sgl_session_data *session_data = (struct sgl_session_data *)file->private_data;

	int err = 0;

	SGL_LOG();

	switch (cmd) {
	case SGL_IOC_INIT_LOCK:
		/* destroy lock with attribute */
		err = sgl_init_lock(session_data, (struct sgl_attribute *)arg);
		break;
	case SGL_IOC_DESTROY_LOCK:
		/* destroy lock with id(=arg) */
		err = sgl_destroy_lock(session_data, (unsigned int)arg);
		break;
	case SGL_IOC_LOCK_LOCK:
		/* lock lock with id(=arg) */
		err = sgl_lock_lock(session_data, (unsigned int)arg);
		break;
	case SGL_IOC_UNLOCK_LOCK:
		/* unlock lock with id(=arg) */
		err = sgl_unlock_lock(session_data, (unsigned int)arg);
		break;
	default:
		SGL_LOG();
		break;
	}

	SGL_LOG();

    return err;
}

static void *sgl_create_locks(void)
{
	return sgl_hash_create_table();
}

static void sgl_destroy_locks(void *locks)
{
	sgl_hash_destroy_table((struct sgl_hash_head *)locks);
	return;
}

static int sgl_open(struct inode *inode, struct file *file)
{
	struct sgl_session_data *session_data;

	SGL_LOG();

	/* init per thread data using file->private_data*/
	session_data = kzalloc(sizeof(struct sgl_session_data), GFP_KERNEL);
	if (session_data == NULL)
		goto err_session_data;

	session_data->inited_locks = kzalloc(sizeof(struct list_head), GFP_KERNEL);
	if (session_data->inited_locks == NULL) {
		goto err_inited_locks;
	}
	INIT_LIST_HEAD(session_data->inited_locks);
	mutex_init(&session_data->inited_mutex);

	session_data->locked_locks = kzalloc(sizeof(struct list_head), GFP_KERNEL);
	if (session_data->locked_locks == NULL) {
		goto err_locked_locks;
	}
	INIT_LIST_HEAD(session_data->locked_locks);
	mutex_init(&session_data->locked_mutex);

	file->private_data = (void *)session_data;

	sgl_global.refcnt++;

	SGL_LOG();

    return 0;

err_locked_locks:
	kfree(session_data->inited_locks);
err_inited_locks:
	kfree(session_data);
err_session_data:
	return -ENOMEM;
}

static int sgl_release(struct inode *inode, struct file *file)
{
	struct sgl_session_data *session_data = file->private_data;
	struct sgl_list *pos;

	int err = 0;

	SGL_LOG();

	/* clean up the locked lock list */
	mutex_lock(&session_data->locked_mutex);
	list_for_each_entry(pos, (struct list_head *)session_data->locked_locks, list) {
		SGL_LOG();

		err = _sgl_unlock_lock((struct sgl_lock *)pos->lock);
		if (err < 0)
			SGL_LOG();

		list_del(&pos->list);
		kfree(pos);

		pos = list_entry((struct list_head *)session_data->locked_locks, struct sgl_list, list);

		SGL_LOG();
	}
	mutex_unlock(&session_data->locked_mutex);


	/* clean up the inited lock list */
	mutex_lock(&session_data->inited_mutex);
	list_for_each_entry(pos, (struct list_head *)session_data->inited_locks, list) {
		SGL_LOG();

		err = _sgl_destroy_lock(pos->lock);
		if (err < 0)
			SGL_LOG();

		list_del(&pos->list);
		kfree(pos);

		pos = list_entry((struct list_head *)session_data->inited_locks, struct sgl_list, list);

		SGL_LOG();
	}
	mutex_unlock(&session_data->inited_mutex);

	/* clean up per thread data */
	file->private_data = NULL;

	kfree(session_data->locked_locks);
	kfree(session_data->inited_locks);

	kfree(session_data);

	sgl_global.refcnt--;
	if (sgl_global.refcnt == 0) {
		/* destroy global lock table */
		sgl_destroy_locks(sgl_global.locks);

	    device_destroy(sgl_global.class, MKDEV(sgl_global.major, 0));
	    class_destroy(sgl_global.class);
	    unregister_chrdev(sgl_global.major, sgl_dev_name);
	}

	SGL_LOG();

    return err;
}

static struct file_operations sgl_ops = {
	.owner = THIS_MODULE,
    .open = sgl_open,
    .release = sgl_release,
#ifdef HAVE_UNLOCKED_IOCTL
	.unlocked_ioctl = sgl_ioctl,
#else
	.ioctl = sgl_ioctl,
#endif
};

int sgl_init(void)
{
	int err = 0;

	SGL_LOG();

	memset(&sgl_global, 0, sizeof(struct sgl_global));

    sgl_global.major = register_chrdev(UNNAMED_MAJOR, sgl_dev_name, &sgl_ops);
	if (sgl_global.major < 0) {
		err = sgl_global.major;
		goto err_register_chrdev;
	}
	SGL_LOG("%d", sgl_global.major);

    sgl_global.class = class_create(THIS_MODULE, sgl_dev_name);
	if (IS_ERR(sgl_global.class)) {
		err = PTR_ERR(sgl_global.class);
		goto err_class_create;
	}

    sgl_global.device = device_create(sgl_global.class, NULL, MKDEV(sgl_global.major, 0), NULL, sgl_dev_name);
	if (IS_ERR(sgl_global.device)) {
		err = PTR_ERR(sgl_global.device);
		goto err_device_create;
	}

	/* create the global lock table */
	sgl_global.locks = sgl_create_locks();
	if (sgl_global.locks == NULL) {
		err = -ENOMEM;
		goto err_create_locks;
	}

	mutex_init(&sgl_global.mutex);

	sgl_global.refcnt++;

	SGL_LOG();

    return 0;

err_create_locks:
err_device_create:
    class_unregister(sgl_global.class);
err_class_create:
    unregister_chrdev(sgl_global.major, sgl_dev_name);
err_register_chrdev:
	SGL_LOG();
	return err;
}

void sgl_exit(void)
{
	SGL_LOG();

	sgl_global.refcnt--;
	if (sgl_global.refcnt == 0) {
		mutex_destroy(&sgl_global.mutex);

		/* destroy global lock table */
		sgl_destroy_locks(sgl_global.locks);

	    device_destroy(sgl_global.class, MKDEV(sgl_global.major, 0));
	    class_destroy(sgl_global.class);
	    unregister_chrdev(sgl_global.major, sgl_dev_name);
	}

	SGL_LOG();

    return;
}

module_init(sgl_init);
module_exit(sgl_exit);

MODULE_LICENSE("GPL");
