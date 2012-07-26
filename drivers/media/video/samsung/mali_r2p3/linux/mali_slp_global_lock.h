#ifndef __SLP_GLOBAL_LOCK_H__
#define __SLP_GLOBAL_LOCK_H__

#include <linux/ioctl.h>

static char sgl_dev_name[] = "slp_global_lock";
static char sgl_devfile[] = "/dev/slp_global_lock";

#define SGL_IOC_BASE				0x32

struct sgl_attribute {
	unsigned int key;
	unsigned int timeout_ms;
};

typedef enum {
	_SGL_INIT_LOCK = 1,
	_SGL_DESTROY_LOCK,
	_SGL_LOCK_LOCK,
	_SGL_UNLOCK_LOCK,
} _sgl_ioctls;

#define SGL_IOC_INIT_LOCK			_IOW(SGL_IOC_BASE, _SGL_INIT_LOCK, struct sgl_attribute *)
#define SGL_IOC_DESTROY_LOCK		_IOW(SGL_IOC_BASE, _SGL_DESTROY_LOCK, unsigned int)
#define SGL_IOC_LOCK_LOCK			_IOW(SGL_IOC_BASE, _SGL_LOCK_LOCK, unsigned int)
#define SGL_IOC_UNLOCK_LOCK			_IOW(SGL_IOC_BASE, _SGL_UNLOCK_LOCK, unsigned int)

#endif /* __SLP_GLOBAL_LOCK_H__ */
