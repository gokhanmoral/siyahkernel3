#ifndef __MDM_HSIC_PM_H__
#define __MDM_HSIC_PM_H__

enum pwr_stat {
	POWER_OFF,
	POWER_ON,
};

void request_active_lock_set(const char *name);
void request_active_lock_release(const char *name);
void set_host_stat(const char *name, enum pwr_stat status);
int wait_dev_pwr_stat(const char *name, enum pwr_stat status);
int check_udev_suspend_allowed(const char *name);

/**
 * register_udev_to_pm_dev - called at interface driver probe function
 *
 * @name:		name of pm device to register usb device
 * @udev:		pointer of udev to register
 */
int register_udev_to_pm_dev(const char *name, struct usb_device *udev);

/**
 * unregister_udev_from_pm_dev - called at interface driver disconnect function
 *
 * @name:		name of pm device to unregister usb device
 * @udev:		pointer of udev to unregister
 */
void unregister_udev_from_pm_dev(const char *name, struct usb_device *udev);

#endif /* __MDM_HSIC_PM_H__ */
