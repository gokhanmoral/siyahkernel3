/**
 * Copyright (C) 2010-2012 ARM Limited. All rights reserved.
 * 
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 * 
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file mali_osk_pm.c
 * Implementation of the callback functions from common power management
 */

#include <linux/sched.h>

#ifdef CONFIG_PM_RUNTIME
#include <linux/pm_runtime.h>
#endif /* CONFIG_PM_RUNTIME */
#include <linux/platform_device.h>
#include "mali_platform.h"
#include "mali_osk.h"
#include "mali_uk_types.h"
#include "mali_kernel_common.h"
#include "mali_kernel_license.h"
#include "mali_linux_pm.h"


/* @@@@ todo: don't poll inn the device stuct like this hack! FIX! */
extern struct platform_device mali_gpu_device;



#ifdef CONFIG_PM_RUNTIME
static mali_bool have_runtime_reference = MALI_FALSE;
#endif


void _mali_osk_pm_dev_enable(void)
{
#ifdef CONFIG_PM_RUNTIME
#if 0
	/* Can only be used if we only support 2.6.37 and onwards */
	pm_runtime_set_autosuspend_delay(&(mali_gpu_device.dev), 2000); /* @@@@ todo: make it a configurable option, and align with the existing light/deep sleep timeouts */
	pm_runtime_use_autosuspend(&(mali_gpu_device.dev));
#endif
	pm_runtime_enable(&(mali_gpu_device.dev));
	/* @@@@ we should probably use pm_runtime_set_active() somewhere since default is off??? Or is it ok, since we will power on on first job anyway, and init seq isn't that big of a deal??? */
#endif
}

/* NB: Function is not thread safe */
_mali_osk_errcode_t _mali_osk_pm_dev_idle(void)
{
#ifdef CONFIG_PM_RUNTIME

	if (MALI_TRUE == have_runtime_reference)
	{
		int err;
		err = pm_runtime_put_sync(&(mali_gpu_device.dev));	
		if (0 > err)
		{
			MALI_PRINT_ERROR(("OSK PM: pm_runtime_put_sync() returned error code %d\n", err));	
			return _MALI_OSK_ERR_FAULT;
		}
		have_runtime_reference = MALI_FALSE;
	}
#endif
	return _MALI_OSK_ERR_OK;
}

/* NB: Function is not thread safe */
_mali_osk_errcode_t _mali_osk_pm_dev_activate(void)
{
#ifdef CONFIG_PM_RUNTIME
	if (MALI_TRUE != have_runtime_reference)
	{
		int err;
		err = pm_runtime_get_sync(&(mali_gpu_device.dev));
		if (0 > err)
		{
			MALI_PRINT_ERROR(("OSK PM: pm_runtime_get_sync() returned error code %d\n", err));	
			return _MALI_OSK_ERR_FAULT;
		}
		have_runtime_reference = MALI_TRUE;
	}
#endif
	return _MALI_OSK_ERR_OK;
}
