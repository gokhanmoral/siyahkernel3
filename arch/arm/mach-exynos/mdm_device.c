#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <mach/gpio-exynos4.h>
#include <plat/gpio-cfg.h>
#include <plat/devs.h>
#include <plat/ehci.h>
#include <linux/msm_charm.h>
#include <mach/mdm2.h>
#include "mdm_private.h"

#ifdef CONFIG_MACH_M3
static struct resource mdm_resources[] = {
	{
		.start	= MDM2AP_ERRFATAL,
		.end	= MDM2AP_ERRFATAL,
		.name	= "MDM2AP_ERRFATAL",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= AP2MDM_ERRFATAL,
		.end	= AP2MDM_ERRFATAL,
		.name	= "AP2MDM_ERRFATAL",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= MDM2AP_STATUS,
		.end	= MDM2AP_STATUS,
		.name	= "MDM2AP_STATUS",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= AP2MDM_STATUS,
		.end	= AP2MDM_STATUS,
		.name	= "AP2MDM_STATUS",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= AP2MDM_PMIC_RESET_N,
		.end	= AP2MDM_PMIC_RESET_N,
		.name	= "AP2MDM_PMIC_RESET_N",
		.flags	= IORESOURCE_IO,
	},
};
#else
static struct resource mdm_resources[] = {
	{
		.start	= GPIO_MDM2AP_ERR_FATAL,
		.end	= GPIO_MDM2AP_ERR_FATAL,
		.name	= "MDM2AP_ERRFATAL",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO_AP2MDM_ERR_FATAL,
		.end	= GPIO_AP2MDM_ERR_FATAL,
		.name	= "AP2MDM_ERRFATAL",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO_MDM2AP_STATUS,
		.end	= GPIO_MDM2AP_STATUS,
		.name	= "MDM2AP_STATUS",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO_AP2MDM_STATUS,
		.end	= GPIO_AP2MDM_STATUS,
		.name	= "AP2MDM_STATUS",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO_AP2MDM_PON_RESET_N,
		.end	= GPIO_AP2MDM_PON_RESET_N,
		.name	= "AP2MDM_PMIC_RESET_N",
		.flags	= IORESOURCE_IO,
	},
};
#endif

#ifdef CONFIG_MDM_HSIC_PM
static struct resource mdm_pm_resource[] = {
	{
#ifdef CONFIG_MACH_M3
		.start	= EXYNOS4_GPF2(2),
		.end	= EXYNOS4_GPF2(2),
#else
		.start	= GPIO_AP2MDM_HSIC_PORT_ACTIVE,
		.end	= GPIO_AP2MDM_HSIC_PORT_ACTIVE,
#endif
		.name	= "AP2MDM_HSIC_ACTIVE",
		.flags	= IORESOURCE_IO,
	},
	{
#ifdef CONFIG_MACH_M3
		.start	= EXYNOS4_GPX1(2),
		.end	= EXYNOS4_GPX1(2),
#else
		.start	= GPIO_MDM2AP_HSIC_PWR_ACTIVE,
		.end	= GPIO_MDM2AP_HSIC_PWR_ACTIVE,
#endif
		.name	= "MDM2AP_DEVICE_PWR_ACTIVE",
		.flags	= IORESOURCE_IO,
	},
	{
#ifdef CONFIG_MACH_M3
		.start	= EXYNOS4_GPX0(5),
		.end	= EXYNOS4_GPX0(5),
#else
		.start	= GPIO_MDM2AP_HSIC_RESUME_REQ,
		.end	= GPIO_MDM2AP_HSIC_RESUME_REQ,
#endif
		.name	= "MDM2AP_RESUME_REQ",
		.flags	= IORESOURCE_IO,
	},
};

struct platform_device mdm_pm_device = {
	.name		= "mdm_hsic_pm0",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(mdm_pm_resource),
	.resource	= mdm_pm_resource,
};
#endif

static struct mdm_platform_data mdm_platform_data = {
	.mdm_version = "3.0",
	.ramdump_delay_ms = 2000,
	.peripheral_platform_device_ehci = &s5p_device_ehci,
	.peripheral_platform_device_ohci = &s5p_device_ohci,
};

struct platform_device mdm_device = {
	.name		= "mdm2_modem",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(mdm_resources),
	.resource	= mdm_resources,
};

static int __init init_mdm_modem(void)
{
	int ret;
	pr_info("%s: registering modem dev, pm dev\n", __func__);

#ifdef CONFIG_MDM_HSIC_PM
	ret = platform_device_register(&mdm_pm_device);
	if (ret < 0) {
		pr_err("%s: fail to register mdm hsic pm dev(err:%d)\n",
								__func__, ret);
		return ret;
	}
#endif
	mdm_device.dev.platform_data = &mdm_platform_data;
	ret = platform_device_register(&mdm_device);
	if (ret < 0) {
		pr_err("%s: fail to register mdm modem dev(err:%d)\n",
								__func__, ret);
		return ret;
	}
	return 0;
}
module_init(init_mdm_modem);
