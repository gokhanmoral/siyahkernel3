/* exynos_drm_dmabuf.c
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
#include "exynos_drm_drv.h"
#include "exynos_drm_gem.h"

#include <linux/dma-buf.h>

static int exynos_dmabuf_attach(struct dma_buf *dmabuf, struct device *dev,
			struct dma_buf_attachment *attach)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);

	/* TODO */

	return 0;
}


static void exynos_dmabuf_detach(struct dma_buf *dmabuf,
					struct dma_buf_attachment *attach)
{
	DRM_DEBUG_KMS("%s\n", __FILE__);

	/* TODO */

	/*
	 * when drm_prime_handle_to_fd() is called, file->f_count of this
	 * dmabuf will be increased by dma_buf_get() so drop the reference
	 * here.
	 */
	dma_buf_put(dmabuf);
}

static struct sg_table *exynos_map_dmabuf(struct dma_buf_attachment *attach,
					enum dma_data_direction direction)
{
	struct drm_gem_object *obj = attach->dmabuf->priv;
	struct exynos_drm_gem_obj *exynos_gem_obj;
	struct exynos_drm_gem_buf *buffer;
	struct sg_table *sgt;
	int ret;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	exynos_gem_obj = to_exynos_gem_obj(obj);

	buffer = exynos_gem_obj->buffer;

	/* TODO. consider physically non-continuous memory with IOMMU. */

	sgt = kzalloc(sizeof(*sgt), GFP_KERNEL);
	if (!sgt) {
		DRM_DEBUG_KMS("failed to allocate sg table.\n");
		return ERR_PTR(-ENOMEM);
	}

	ret = sg_alloc_table(sgt, 1, GFP_KERNEL);
	if (ret < 0) {
		DRM_DEBUG_KMS("failed to allocate scatter list.\n");
		kfree(sgt);
		sgt = NULL;
		return ERR_PTR(-ENOMEM);
	}

	sg_init_table(sgt->sgl, 1);
	sg_dma_len(sgt->sgl) = buffer->size;
	sg_set_page(sgt->sgl, pfn_to_page(PFN_DOWN(buffer->dma_addr)),
			buffer->size, 0);
	sg_dma_address(sgt->sgl) = buffer->dma_addr;

	/*
	 * increase reference count of this buffer.
	 *
	 * Note:
	 * allocated physical memory region is being shared with others
	 * so this region shouldn't be released until all references of
	 * this region will be dropped by exynos_unmap_dmabuf().
	 */
	atomic_inc(&buffer->shared_refcount);

	return sgt;
}

static void exynos_unmap_dmabuf(struct dma_buf_attachment *attach,
						struct sg_table *sgt)
{
	struct drm_gem_object *obj = attach->dmabuf->priv;
	struct exynos_drm_gem_obj *exynos_gem_obj;
	struct exynos_drm_gem_buf *buffer;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	exynos_gem_obj = to_exynos_gem_obj(obj);

	buffer = exynos_gem_obj->buffer;

	sg_free_table(sgt);
	kfree(sgt);
	sgt = NULL;

	if (atomic_read(&buffer->shared_refcount) <= 0)
		BUG();

	atomic_dec(&buffer->shared_refcount);
}

static void exynos_dmabuf_release(struct dma_buf *dmabuf)
{
	struct drm_gem_object *obj = dmabuf->priv;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	/*
	 * exynos_dmabuf_release() call means that file object's
	 * f_count is 0 and it calls drm_gem_object_handle_unreference()
	 * to drop the references that these values had been increased
	 * at drm_prime_handle_to_fd()
	 */
	if (obj->export_dma_buf == dmabuf) {
		obj->prime_fd = -1;
		obj->export_dma_buf = NULL;

		/*
		 * drop this gem object refcount to release allocated buffer
		 * and resources.
		 */
		drm_gem_object_unreference_unlocked(obj);
	}
}

static struct dma_buf_ops exynos_dmabuf_ops = {
	.attach			= exynos_dmabuf_attach,
	.detach			= exynos_dmabuf_detach,
	.map_dma_buf		= exynos_map_dmabuf,
	.unmap_dma_buf		= exynos_unmap_dmabuf,
	.release		= exynos_dmabuf_release,
};

int exynos_dmabuf_prime_handle_to_fd(struct drm_device *drm_dev,
					struct drm_file *file,
					unsigned int handle, int *prime_fd)
{
	struct drm_gem_object *obj;
	struct exynos_drm_gem_obj *exynos_gem_obj;
	int ret = 0;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	ret = mutex_lock_interruptible(&drm_dev->struct_mutex);
	if (ret < 0)
		return ret;

	obj = drm_gem_object_lookup(drm_dev, file, handle);
	if (!obj) {
		DRM_DEBUG_KMS("failed to lookup gem object.\n");
		ret = -EINVAL;
		goto err1;
	}

	exynos_gem_obj = to_exynos_gem_obj(obj);

	if (obj->prime_fd != -1) {
		/* we have a prime fd already referencing the object. */
		goto have_fd;
	}

	/*
	 * get the dmabuf object for a gem object after registering
	 * the gem object to allocated dmabuf.
	 *
	 * P.S. dma_buf_export function performs the followings:
	 *	- create a new dmabuf object.
	 *	- dmabuf->priv = gem object.
	 *	- file->private_data = dmabuf.
	 */
	obj->export_dma_buf = dma_buf_export(obj, &exynos_dmabuf_ops,
						obj->size, 0600);
	if (!obj->export_dma_buf) {
		ret = PTR_ERR(obj->export_dma_buf);
		goto err2;
	}

	/* get file descriptor for a given dmabuf object. */
	obj->prime_fd = dma_buf_fd(obj->export_dma_buf);
	if (obj->prime_fd < 0) {
		DRM_DEBUG_KMS("failed to get fd from dmabuf.\n");
		dma_buf_put(obj->export_dma_buf);
		ret = obj->prime_fd;
		goto err2;
	}

	/*
	 * this gem object is referenced by the fd so
	 * the object refcount should be increased.
	 * after that when dmabuf_ops->release() is called,
	 * it will be decreased again.
	 */
	drm_gem_object_reference(obj);

have_fd:
	*prime_fd = obj->prime_fd;
err2:
	drm_gem_object_unreference(obj);
err1:
	mutex_unlock(&drm_dev->struct_mutex);
	return ret;
}

int exynos_dmabuf_prime_fd_to_handle(struct drm_device *drm_dev,
					struct drm_file *file,
					int prime_fd, unsigned int *handle)
{
	struct drm_exynos_file_private *file_priv = file->driver_priv;
	struct dma_buf_attachment *attach;
	struct dma_buf *dmabuf;
	struct sg_table *sgt;
	struct exynos_drm_gem_obj *exynos_gem_obj;
	struct exynos_drm_gem_buf *buffer;
	int ret;

	DRM_DEBUG_KMS("%s\n", __FILE__);

	ret = mutex_lock_interruptible(&drm_dev->struct_mutex);
	if (ret < 0)
		return ret;

	dmabuf = dma_buf_get(prime_fd);
	if (IS_ERR(dmabuf)) {
		ret = PTR_ERR(dmabuf);
		goto out;
	}

	/*
	 * if there is same dmabuf as the one to prime_fd
	 * in file_priv->prime list then return the handle.
	 *
	 * Note:
	 * but if the prime_fd from user belongs to another process
	 * then there couldn't be the dmabuf in file_priv->prime list
	 * because file_priv is unique to process.
	 */
	ret = drm_prime_lookup_fd_handle_mapping(&file_priv->prime,
							dmabuf, handle);
	if (!ret) {
		/* drop reference we got above. */
		dma_buf_put(dmabuf);
		goto out;
	}

	attach = dma_buf_attach(dmabuf, drm_dev->dev);
	if (IS_ERR(attach)) {
		ret = PTR_ERR(attach);
		goto fail_put;
	}

	sgt = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(sgt)) {
		ret = PTR_ERR(sgt);
		goto fail_detach;
	}

	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer) {
		DRM_ERROR("failed to allocate exynos_drm_gem_buf.\n");
		ret = -ENOMEM;
		goto fail_unmap;
	}

	exynos_gem_obj = exynos_drm_gem_init(drm_dev, dmabuf->size);
	if (!exynos_gem_obj) {
		ret = -ENOMEM;
		goto fail_unmap;
	}

	exynos_gem_obj->sgt = sgt;

	ret = drm_gem_handle_create(file, &exynos_gem_obj->base, handle);
	if (ret < 0)
		goto fail_handle;

	/* consider physically non-continuous memory with IOMMU. */

	buffer->dma_addr = sg_dma_address(sgt->sgl);
	buffer->size = sg_dma_len(sgt->sgl);

	/*
	 * import(fd to handle) means that the physical memory region
	 * from the sgt is being shared with others so shared_refcount
	 * should be 1.
	 */
	atomic_set(&buffer->shared_refcount, 1);

	exynos_gem_obj->base.import_attach = attach;

	ret = drm_prime_insert_fd_handle_mapping(&file_priv->prime,
							dmabuf, *handle);
	if (ret < 0)
		goto fail_handle;

	/* register buffer information to private buffer manager. */
	ret = register_buf_to_priv_mgr(buffer->dma_addr, buffer->size,
						&exynos_gem_obj->priv_handle,
						&exynos_gem_obj->priv_id);
	if (ret < 0) {
		drm_prime_remove_fd_handle_mapping(&file_priv->prime, dmabuf);
		goto fail_handle;
	}

	DRM_DEBUG_KMS("fd = %d, handle = %d, dma_addr = 0x%x, size = 0x%lx\n",
			prime_fd, *handle, buffer->dma_addr, buffer->size);

	drm_gem_object_unreference(&exynos_gem_obj->base);
	mutex_unlock(&drm_dev->struct_mutex);

	return 0;

fail_handle:
	drm_gem_object_unreference(&exynos_gem_obj->base);
	kfree(buffer);
	drm_gem_object_release(&exynos_gem_obj->base);
	kfree(exynos_gem_obj);
fail_unmap:
	dma_buf_unmap_attachment(attach, sgt);
fail_detach:
	dma_buf_detach(dmabuf, attach);
fail_put:
	dma_buf_put(dmabuf);
out:
	mutex_unlock(&drm_dev->struct_mutex);
	return ret;
}

MODULE_AUTHOR("Inki Dae <inki.dae@samsung.com>");
MODULE_DESCRIPTION("Samsung SoC DRM DMABUF Module");
MODULE_LICENSE("GPL");
