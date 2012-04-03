/* exynos_drm_gem.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 * Author: Inki Dae <inki.dae@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "drmP.h"
#include "drm.h"

#include <drm/exynos_drm.h>
#include <linux/dma-buf.h>

#include "exynos_drm_drv.h"
#include "exynos_drm_gem.h"
#include "exynos_drm_buf.h"

static struct exynos_drm_private_cb *private_cb;

void exynos_drm_priv_cb_register(struct exynos_drm_private_cb *cb)
{
	if (cb)
		private_cb = cb;
}

int register_buf_to_priv_mgr(dma_addr_t dma_addr, unsigned long size,
		unsigned int *priv_handle, unsigned int *priv_id)
{
	int ret = -1;

	/* register buffer information to private buffer manager. */
	if (private_cb && private_cb->add_buffer) {
		ret = private_cb->add_buffer(dma_addr, size, priv_handle,
						priv_id);
		if (ret < 0)
			return ret;
	}

	return ret;
}

static unsigned int convert_to_vm_err_msg(int msg)
{
	unsigned int out_msg;

	switch (msg) {
	case 0:
	case -ERESTARTSYS:
	case -EINTR:
		out_msg = VM_FAULT_NOPAGE;
		break;

	case -ENOMEM:
		out_msg = VM_FAULT_OOM;
		break;

	default:
		out_msg = VM_FAULT_SIGBUS;
		break;
	}

	return out_msg;
}

static int exynos_drm_gem_handle_create(struct drm_gem_object *obj,
					struct drm_file *file_priv,
					unsigned int *handle)
{
	int ret;

	/*
	 * allocate a id of idr table where the obj is registered
	 * and handle has the id what user can see.
	 */
	ret = drm_gem_handle_create(file_priv, obj, handle);
	if (ret)
		return ret;

	DRM_DEBUG_KMS("gem handle = 0x%x\n", *handle);

	/* drop reference from allocate - handle holds it now. */
	drm_gem_object_unreference_unlocked(obj);

	return 0;
}

void exynos_drm_gem_destroy(struct exynos_drm_gem_obj *exynos_gem_obj)
{
	struct drm_gem_object *obj;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (!exynos_gem_obj)
		return;

	obj = &exynos_gem_obj->base;

	DRM_DEBUG_KMS("handle count = %d\n", atomic_read(&obj->handle_count));

	/* release private buffer from private buffer manager. */
	if (private_cb->release_buffer)
		private_cb->release_buffer(exynos_gem_obj->priv_handle);

	exynos_drm_buf_destroy(obj->dev, exynos_gem_obj->buffer);

	if (obj->map_list.map)
		drm_gem_free_mmap_offset(obj);

	/* release file pointer to gem object. */
	drm_gem_object_release(obj);

	kfree(exynos_gem_obj);
}

struct exynos_drm_gem_obj *exynos_drm_gem_init(struct drm_device *dev,
						      unsigned long size)
{
	struct exynos_drm_gem_obj *exynos_gem_obj;
	struct drm_gem_object *obj;
	int ret;

	exynos_gem_obj = kzalloc(sizeof(*exynos_gem_obj), GFP_KERNEL);
	if (!exynos_gem_obj) {
		DRM_ERROR("failed to allocate exynos gem object\n");
		return NULL;
	}

	obj = &exynos_gem_obj->base;

	ret = drm_gem_object_init(dev, obj, size);
	if (ret < 0) {
		DRM_ERROR("failed to initialize gem object\n");
		kfree(exynos_gem_obj);
		return NULL;
	}

	DRM_DEBUG_KMS("created file object = 0x%x\n", (unsigned int)obj->filp);

	return exynos_gem_obj;
}

struct exynos_drm_gem_obj *exynos_drm_gem_create(struct drm_device *dev,
						 unsigned long size)
{
	struct exynos_drm_gem_buf *buffer;
	struct exynos_drm_gem_obj *exynos_gem_obj;
	int ret;

	size = roundup(size, PAGE_SIZE);
	DRM_DEBUG_KMS("%s: size = 0x%lx\n", __FILE__, size);

	buffer = exynos_drm_buf_create(dev, size);
	if (!buffer)
		return ERR_PTR(-ENOMEM);

	exynos_gem_obj = exynos_drm_gem_init(dev, size);
	if (!exynos_gem_obj) {
		exynos_drm_buf_destroy(dev, buffer);
		return ERR_PTR(-ENOMEM);
	}

	exynos_gem_obj->buffer = buffer;

	/* register buffer information to private buffer manager. */
	ret = register_buf_to_priv_mgr((dma_addr_t)buffer->dma_addr, size,
				(unsigned int *)&exynos_gem_obj->priv_handle,
				(unsigned int *)&exynos_gem_obj->priv_id);
	if (ret < 0)
		goto err;

	return exynos_gem_obj;
err:
	exynos_drm_buf_destroy(dev, buffer);
	drm_gem_object_release(&exynos_gem_obj->base);
	kfree(exynos_gem_obj);
	return ERR_PTR(ret);
}

int exynos_drm_gem_create_ioctl(struct drm_device *dev, void *data,
				struct drm_file *file_priv)
{
	struct drm_exynos_gem_create *args = data;
	struct exynos_drm_gem_obj *exynos_gem_obj;
	int ret;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	exynos_gem_obj = exynos_drm_gem_create(dev, args->size);
	if (IS_ERR(exynos_gem_obj))
		return PTR_ERR(exynos_gem_obj);

	ret = exynos_drm_gem_handle_create(&exynos_gem_obj->base, file_priv,
			&args->handle);
	if (ret) {
		exynos_drm_gem_destroy(exynos_gem_obj);
		return ret;
	}

	return 0;
}

int exynos_drm_gem_map_offset_ioctl(struct drm_device *dev, void *data,
				    struct drm_file *file_priv)
{
	struct drm_exynos_gem_map_off *args = data;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	DRM_DEBUG_KMS("handle = 0x%x, offset = 0x%lx\n",
			args->handle, (unsigned long)args->offset);

	if (!(dev->driver->driver_features & DRIVER_GEM)) {
		DRM_ERROR("does not support GEM.\n");
		return -ENODEV;
	}

	return exynos_drm_gem_dumb_map_offset(file_priv, dev, args->handle,
			&args->offset);
}

static int exynos_drm_gem_mmap_buffer(struct file *filp,
				      struct vm_area_struct *vma)
{
	struct drm_gem_object *obj = filp->private_data;
	struct exynos_drm_gem_obj *exynos_gem_obj = to_exynos_gem_obj(obj);
	struct exynos_drm_gem_buf *buffer;
	unsigned long pfn, vm_size;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	vma->vm_flags |= (VM_IO | VM_RESERVED);

	/* in case of direct mapping, always having non-cachable attribute */
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	vma->vm_file = filp;

	vm_size = vma->vm_end - vma->vm_start;
	/*
	 * a buffer contains information to physically continuous memory
	 * allocated by user request or at framebuffer creation.
	 */
	buffer = exynos_gem_obj->buffer;

	/* check if user-requested size is valid. */
	if (vm_size > buffer->size)
		return -EINVAL;

	/*
	 * get page frame number to physical memory to be mapped
	 * to user space.
	 */
	pfn = ((unsigned long)exynos_gem_obj->buffer->dma_addr) >> PAGE_SHIFT;

	DRM_DEBUG_KMS("pfn = 0x%lx\n", pfn);

	if (remap_pfn_range(vma, vma->vm_start, pfn, vm_size,
				vma->vm_page_prot)) {
		DRM_ERROR("failed to remap pfn range.\n");
		return -EAGAIN;
	}

	return 0;
}

static const struct file_operations exynos_drm_gem_fops = {
	.mmap = exynos_drm_gem_mmap_buffer,
};

int exynos_drm_gem_mmap_ioctl(struct drm_device *dev, void *data,
			      struct drm_file *file_priv)
{
	struct drm_exynos_gem_mmap *args = data;
	struct drm_gem_object *obj;
	unsigned int addr;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	if (!(dev->driver->driver_features & DRIVER_GEM)) {
		DRM_ERROR("does not support GEM.\n");
		return -ENODEV;
	}

	obj = drm_gem_object_lookup(dev, file_priv, args->handle);
	if (!obj) {
		DRM_ERROR("failed to lookup gem object.\n");
		return -EINVAL;
	}

	obj->filp->f_op = &exynos_drm_gem_fops;
	obj->filp->private_data = obj;

	down_write(&current->mm->mmap_sem);
	addr = do_mmap(obj->filp, 0, args->size,
			PROT_READ | PROT_WRITE, MAP_SHARED, 0);
	up_write(&current->mm->mmap_sem);

	drm_gem_object_unreference_unlocked(obj);

	if (IS_ERR((void *)addr))
		return PTR_ERR((void *)addr);

	args->mapped = addr;

	DRM_DEBUG_KMS("mapped = 0x%lx\n", (unsigned long)args->mapped);

	return 0;
}

int exynos_drm_gem_export_ump_ioctl(struct drm_device *dev, void *data,
		struct drm_file *file)
{
	struct exynos_drm_gem_obj *exynos_gem_obj;
	struct drm_gem_object *obj;
	struct drm_exynos_gem_ump *ump = data;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	mutex_lock(&dev->struct_mutex);

	obj = drm_gem_object_lookup(dev, file, ump->gem_handle);
	if (!obj) {
		DRM_ERROR("failed to lookup gem object.\n");
		mutex_unlock(&dev->struct_mutex);
		return -EINVAL;
	}

	exynos_gem_obj = to_exynos_gem_obj(obj);

	ump->secure_id = exynos_gem_obj->priv_id;

	drm_gem_object_unreference(obj);

	mutex_unlock(&dev->struct_mutex);

	DRM_DEBUG_KMS("got secure id = %d\n", ump->secure_id);

	return 0;
}

static void exynos_gem_flush_cache_all(void *info)
{
	flush_cache_all();
}

int exynos_drm_gem_cache_op_ioctl(struct drm_device *drm_dev, void *data,
		struct drm_file *file_priv)
{
	struct drm_exynos_gem_cache_op *cache_obj = data;
	unsigned int cache_sel = cache_obj->flags & EXYNOS_DRM_ALL_CACHE;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	switch (cache_sel) {
	case EXYNOS_DRM_ALL_CACHE:
		smp_call_function(exynos_gem_flush_cache_all, NULL, 1);
		outer_flush_all();
		break;
	case EXYNOS_DRM_L1_CACHE:
		smp_call_function(exynos_gem_flush_cache_all, NULL, 1);
		break;
	case EXYNOS_DRM_L2_CACHE:
		outer_flush_all();
		break;
	default:
		DRM_DEBUG_KMS("invalid cache unit.\n");
		return -EINVAL;
	}

	return 0;
}

/* temporary functions. */
int exynos_drm_gem_get_phy_ioctl(struct drm_device *drm_dev, void *data,
		struct drm_file *file_priv)
{
	struct drm_exynos_gem_get_phy *get_phy = data;
	struct exynos_drm_gem_obj *exynos_gem_obj;
	struct drm_gem_object *obj;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	mutex_lock(&drm_dev->struct_mutex);

	obj = drm_gem_object_lookup(drm_dev, file_priv, get_phy->gem_handle);
	if (!obj) {
		DRM_ERROR("failed to lookup gem object.\n");
		mutex_unlock(&drm_dev->struct_mutex);
		return -EINVAL;
	}

	exynos_gem_obj = to_exynos_gem_obj(obj);

	get_phy->phy_addr = exynos_gem_obj->buffer->dma_addr;
	get_phy->size = exynos_gem_obj->buffer->size;

	drm_gem_object_unreference(obj);

	mutex_unlock(&drm_dev->struct_mutex);

	return 0;
}

int exynos_drm_gem_phy_imp_ioctl(struct drm_device *drm_dev, void *data,
				 struct drm_file *file_priv)
{
	struct drm_exynos_gem_phy_imp *args = data;
	struct exynos_drm_gem_obj *exynos_gem_obj;
	struct exynos_drm_gem_buf *buffer;
	int ret = 0;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	exynos_gem_obj = exynos_drm_gem_init(drm_dev, args->size);
	if (!exynos_gem_obj)
		return -ENOMEM;

	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer) {
		DRM_DEBUG_KMS("failed to allocate buffer\n");
		ret = -ENOMEM;
		goto err;
	}

	buffer->dma_addr = (dma_addr_t)args->phy_addr;
	buffer->size = args->size;

	/*
	 * if shared is true, this bufer wouldn't be released.
	 * this buffer was allocated by other so don't release it.
	 */
	buffer->shared = true;

	exynos_gem_obj->buffer = buffer;

	/* register buffer information to private buffer manager. */
	ret = register_buf_to_priv_mgr(buffer->dma_addr, buffer->size,
				(unsigned int *)&exynos_gem_obj->priv_handle,
				(unsigned int *)&exynos_gem_obj->priv_id);
	if (ret < 0)
		goto err_kfree_buffer;

	ret = exynos_drm_gem_handle_create(&exynos_gem_obj->base, file_priv,
			&args->gem_handle);
	if (ret)
		goto err_release_buffer;

	DRM_DEBUG_KMS("got gem handle = 0x%x\n", args->gem_handle);

	return 0;

err_release_buffer:
	if (private_cb->release_buffer)
		private_cb->release_buffer(exynos_gem_obj->priv_handle);
err_kfree_buffer:
	kfree(buffer);
err:
	drm_gem_object_release(&exynos_gem_obj->base);
	kfree(exynos_gem_obj);
	return ret;
}

int exynos_drm_gem_init_object(struct drm_gem_object *obj)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);

	return 0;
}

void exynos_drm_gem_free_object(struct drm_gem_object *obj)
{
	struct exynos_drm_gem_obj *exynos_gem_obj;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	exynos_gem_obj = to_exynos_gem_obj(obj);

	/*
	 * now this gem object could be released so if there is the dmabuf
	 * imported to this gem object then release it and this opration
	 * will release import_attach object and also drop file->f_count
	 * of this dmabuf.
	 */
	if (obj->import_attach)
		drm_prime_gem_destroy(obj, exynos_gem_obj->sgt);

	exynos_drm_gem_destroy(to_exynos_gem_obj(obj));
}

int exynos_drm_gem_dumb_create(struct drm_file *file_priv,
			       struct drm_device *dev,
			       struct drm_mode_create_dumb *args)
{
	struct exynos_drm_gem_obj *exynos_gem_obj;
	int ret;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	/*
	 * alocate memory to be used for framebuffer.
	 * - this callback would be called by user application
	 *	with DRM_IOCTL_MODE_CREATE_DUMB command.
	 */

	args->pitch = args->width * args->bpp >> 3;
	args->size = args->pitch * args->height;

	exynos_gem_obj = exynos_drm_gem_create(dev, args->size);
	if (IS_ERR(exynos_gem_obj))
		return PTR_ERR(exynos_gem_obj);

	ret = exynos_drm_gem_handle_create(&exynos_gem_obj->base, file_priv,
			&args->handle);
	if (ret) {
		exynos_drm_gem_destroy(exynos_gem_obj);
		return ret;
	}

	return 0;
}

int exynos_drm_gem_dumb_map_offset(struct drm_file *file_priv,
				   struct drm_device *dev, uint32_t handle,
				   uint64_t *offset)
{
	struct exynos_drm_gem_obj *exynos_gem_obj;
	struct drm_gem_object *obj;
	int ret = 0;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	mutex_lock(&dev->struct_mutex);

	/*
	 * get offset of memory allocated for drm framebuffer.
	 * - this callback would be called by user application
	 *	with DRM_IOCTL_MODE_MAP_DUMB command.
	 */

	obj = drm_gem_object_lookup(dev, file_priv, handle);
	if (!obj) {
		DRM_ERROR("failed to lookup gem object.\n");
		ret = -EINVAL;
		goto unlock;
	}

	exynos_gem_obj = to_exynos_gem_obj(obj);

	if (!exynos_gem_obj->base.map_list.map) {
		ret = drm_gem_create_mmap_offset(&exynos_gem_obj->base);
		if (ret)
			goto out;
	}

	*offset = (u64)exynos_gem_obj->base.map_list.hash.key << PAGE_SHIFT;
	DRM_DEBUG_KMS("offset = 0x%lx\n", (unsigned long)*offset);

out:
	drm_gem_object_unreference(obj);
unlock:
	mutex_unlock(&dev->struct_mutex);
	return ret;
}

int exynos_drm_gem_dumb_destroy(struct drm_file *file_priv,
				struct drm_device *dev,
				unsigned int handle)
{
	int ret;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	/*
	 * obj->refcount and obj->handle_count are decreased and
	 * if both them are 0 then exynos_drm_gem_free_object()
	 * would be called by callback to release resources.
	 */
	ret = drm_gem_handle_delete(file_priv, handle);
	if (ret < 0) {
		DRM_ERROR("failed to delete drm_gem_handle.\n");
		return ret;
	}

	return 0;
}

void exynos_drm_gem_close_object(struct drm_gem_object *obj,
				struct drm_file *file)
{
	struct drm_exynos_file_private *file_priv = file->driver_priv;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	/*
	 * remove a prime member object from prime list
	 * only if obj->refcount has 1 and note that obj->refcount
	 * would be decreased by drm_gem_object_handle_unreference_unlock().
	 *
	 * Note:
	 * if this prime is released with that obj->refcount has more than 2
	 * then when user requested import, a new gem object would be created
	 * and return it instead of old one registed to prime list.
	 */
	if (obj->import_attach && atomic_read(&obj->refcount.refcount) == 1) {
		drm_prime_remove_fd_handle_mapping(&file_priv->prime,
					obj->import_attach->dmabuf);
	}
}

int exynos_drm_gem_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct drm_gem_object *obj = vma->vm_private_data;
	struct exynos_drm_gem_obj *exynos_gem_obj = to_exynos_gem_obj(obj);
	struct drm_device *dev = obj->dev;
	unsigned long pfn;
	pgoff_t page_offset;
	int ret;

	page_offset = ((unsigned long)vmf->virtual_address -
			vma->vm_start) >> PAGE_SHIFT;

	mutex_lock(&dev->struct_mutex);

	pfn = (((unsigned long)exynos_gem_obj->buffer->dma_addr) >>
			PAGE_SHIFT) + page_offset;

	ret = vm_insert_mixed(vma, (unsigned long)vmf->virtual_address, pfn);

	mutex_unlock(&dev->struct_mutex);

	return convert_to_vm_err_msg(ret);
}

int exynos_drm_gem_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	/* set vm_area_struct. */
	ret = drm_gem_mmap(filp, vma);
	if (ret < 0) {
		DRM_ERROR("failed to mmap.\n");
		return ret;
	}

	vma->vm_flags &= ~VM_PFNMAP;
	vma->vm_flags |= VM_MIXEDMAP;

	return ret;
}

MODULE_AUTHOR("Inki Dae <inki.dae@samsung.com>");
MODULE_DESCRIPTION("Samsung SoC DRM GEM Module");
MODULE_LICENSE("GPL");
