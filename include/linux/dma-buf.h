/*
 * Header file for dma buffer sharing framework.
 *
 * Copyright(C) 2011 Linaro Limited. All rights reserved.
 * Author: Sumit Semwal <sumit.semwal@ti.com>
 *
 * Many thanks to linaro-mm-sig list, and specially
 * Arnd Bergmann <arnd@arndb.de>, Rob Clark <rob@ti.com> and
 * Daniel Vetter <daniel@ffwll.ch> for their support in creation and
 * refining of this idea.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __DMA_BUF_H__
#define __DMA_BUF_H__

#include <linux/file.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/scatterlist.h>
#include <linux/list.h>
#include <linux/dma-mapping.h>

struct dma_buf;

/**
 * struct dma_buf_attachment - holds device-buffer attachment data
 * @dmabuf: buffer for this attachment.
 * @dev: device attached to the buffer.
 * @node: list_head to allow manipulation of list of dma_buf_attachment.
 * @priv: exporter-specific attachment data.
 */
struct dma_buf_attachment {
	struct dma_buf *dmabuf;
	struct device *dev;
	struct list_head node;
	void *priv;
};

/**
 * struct dma_buf_ops - operations possible on struct dma_buf
 * @attach: allows different devices to 'attach' themselves to the given
 *	    buffer. It might return -EBUSY to signal that backing storage
 *	    is already allocated and incompatible with the requirements
 *	    of requesting device. [optional]
 * @detach: detach a given device from this buffer. [optional]
 * @map_dma_buf: returns list of scatter pages allocated, increases usecount
 *		 of the buffer. Requires atleast one attach to be called
 *		 before. Returned sg list should already be mapped into
 *		 _device_ address space. This call may sleep. May also return
 *		 -EINTR.
 * @unmap_dma_buf: decreases usecount of buffer, might deallocate scatter
 *		   pages.
 * @release: release this buffer; to be called after the last dma_buf_put.
 * @sync_sg_for_cpu: sync the sg list for cpu.
 * @sync_sg_for_device: synch the sg list for device.
 */
struct dma_buf_ops {
	int (*attach)(struct dma_buf *, struct device *,
			struct dma_buf_attachment *);

	void (*detach)(struct dma_buf *, struct dma_buf_attachment *);

	/* For {map,unmap}_dma_buf below, any specific buffer attributes
	 * required should get added to device_dma_parameters accessible
	 * via dev->dma_params.
	 */
	struct sg_table * (*map_dma_buf)(struct dma_buf_attachment *,
						enum dma_data_direction);
	void (*unmap_dma_buf)(struct dma_buf_attachment *,
						struct sg_table *);
	/* TODO: Add try_map_dma_buf version, to return immed with -EBUSY
	 * if the call would block.
	 */

	/* after final dma_buf_put() */
	void (*release)(struct dma_buf *);

};

/**
 * struct dma_buf - shared buffer object
 * @file: file pointer used for sharing buffers across, and for refcounting.
 * @attachments: list of dma_buf_attachment that denotes all devices attached.
 * @ops: dma_buf_ops associated with this buffer object
 * @priv: user specific private data
 */
struct dma_buf {
	size_t size;
	struct file *file;
	struct list_head attachments;
	const struct dma_buf_ops *ops;
	/* mutex to serialize list manipulation and other ops */
	struct mutex lock;
	void *priv;
};

#ifdef CONFIG_DMA_SHARED_BUFFER
struct dma_buf_attachment *dma_buf_attach(struct dma_buf *dmabuf,
							struct device *dev);
void dma_buf_detach(struct dma_buf *dmabuf,
				struct dma_buf_attachment *dmabuf_attach);
struct dma_buf *dma_buf_export(void *priv, struct dma_buf_ops *ops,
			size_t size, int flags);
int dma_buf_fd(struct dma_buf *dmabuf);
struct dma_buf *dma_buf_get(int fd);
void dma_buf_put(struct dma_buf *dmabuf);

struct sg_table *dma_buf_map_attachment(struct dma_buf_attachment *,
					enum dma_data_direction);
void dma_buf_unmap_attachment(struct dma_buf_attachment *, struct sg_table *);

#else

static inline struct dma_buf_attachment *dma_buf_attach(struct dma_buf *dmabuf,
							struct device *dev)
{
	return ERR_PTR(-ENODEV);
}

static inline void dma_buf_detach(struct dma_buf *dmabuf,
				  struct dma_buf_attachment *dmabuf_attach)
{
	return;
}

static inline struct dma_buf *dma_buf_export(void *priv,
						struct dma_buf_ops *ops,
						size_t size, int flags)
{
	return ERR_PTR(-ENODEV);
}

static inline int dma_buf_fd(struct dma_buf *dmabuf)
{
	return -ENODEV;
}

static inline struct dma_buf *dma_buf_get(int fd)
{
	return ERR_PTR(-ENODEV);
}

static inline void dma_buf_put(struct dma_buf *dmabuf)
{
	return;
}

static inline struct sg_table *dma_buf_map_attachment(
	struct dma_buf_attachment *attach, enum dma_data_direction write)
{
	return ERR_PTR(-ENODEV);
}

static inline void dma_buf_unmap_attachment(struct dma_buf_attachment *attach,
						struct sg_table *sg)
{
	return;
}

#endif /* CONFIG_DMA_SHARED_BUFFER */

#endif /* __DMA_BUF_H__ */
