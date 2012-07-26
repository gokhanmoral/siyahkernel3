
/* Copyright (c) 2010-2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/log2.h>
#include <linux/spinlock.h>
#include <linux/hrtimer.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/switch.h>
#include <linux/workqueue.h>
#include <linux/irqdesc.h>
#include <linux/fs.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/irqdesc.h>


#include <linux/wakelock.h>

/////////////////////////////////////////////////////////////////////
//#define PM8058_IRQ_BASE				(NR_MSM_IRQS + NR_GPIO_IRQS)
//#define FLIP_DET_PIN		36
#define FLIP_NOTINIT		-1
#define FLIP_OPEN			1
#define FLIP_CLOSE		0	

#define FLIP_SCAN_INTERVAL	(50)	/* ms */
#define FLIP_STABLE_COUNT	(1)
#define GPIO_FLIP  GPIO_HALL_SW

#define __TSP_DEBUG 1

#if 1 // DEBUG
#define dbg_printk(fmt, ...) \
	printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#else
#define dbg_printk(fmt, ...) \
	({ if (0) printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__); 0; })
#endif

/////////////////////////////////////////////////////////////////////
struct sec_flip_pdata {
	int wakeup; 
}; 

struct sec_flip {
	struct input_dev *input;
	struct sec_flip_pdata *pdata;
	//spinlock_t lock;
	struct wake_lock wlock; 

	struct workqueue_struct *wq;	/* The actuak work queue */
	struct work_struct flip_id_det;	/* The work being queued */

	struct timer_list flip_timer;

	int			gpio; 
	int    		irq;
};


/////////////////////////////////////////////////////////////////////
extern void samsung_switching_lcd_suspend(int init, int flip);
extern void samsung_switching_lcd_resume(int init, int flip);
extern void samsung_switching_tsp(int init, int flip);
extern void samsung_switching_tkey(int init, int flip);
extern void samsung_switching_tsp_pre(int init, int flip);
#ifdef CONFIG_BATTERY_SEC
extern unsigned int is_lpcharging_state(void);
#endif


/////////////////////////////////////////////////////////////////////
static int flip_status=FLIP_NOTINIT;  // first state
//struct timer_list flip_timer;

/* 0 : open, 1: close */
int Is_folder_state(void)
{
	printk("%s: flip_status = %d\n", __func__,flip_status); 
	return !flip_status;
}
EXPORT_SYMBOL(Is_folder_state);

int Is_folder_state_sensor(void)
{
	return !flip_status;
}
EXPORT_SYMBOL(Is_folder_state_sensor);

struct switch_dev switch_flip = {
		.name = "flip",
};

/////////////////////////////////////////////////////////////////////
//	spin_lock_irqsave(&flip->lock, flags);
//	spin_unlock_irqrestore(&flip->lock, flags);

static void sec_flip_work_func(struct work_struct *work)
{
	int init = 0;
	struct sec_flip* flip = container_of( work, struct sec_flip, flip_id_det); 

	//enable_irq(flip->irq);

	printk("%s: %s\n", __func__, (flip_status) ? "OPEN":"CLOSE" ); 

	if ( flip_status==FLIP_NOTINIT) {
		flip_status = gpio_get_value_cansleep( flip->gpio );
		init = 1; 		
	}

//	switch_set_state(&switch_flip, !flip_status);
	
//################ first  #################
	// if flip status was changed and lcd is on, switch device. 
#ifdef CONFIG_BATTERY_SEC
		if ( !is_lpcharging_state() )
#endif
		{
			if ( flip_status ){ 
				#ifdef __TSP_DEBUG
				printk ("[FLIP] flip_status- open\n");
				#endif
				samsung_switching_tkey(init, flip_status);
				//lp8720_set_folder_open();
			}
			else {
				#ifdef __TSP_DEBUG
				printk ("[FLIP] flip_status- open\n");
				#endif
				//lp8720_set_folder_close();
				samsung_switching_tkey(init, flip_status);
			}
		}

		samsung_switching_lcd_suspend(init, flip_status);
		gpio_set_value(GPIO_LCD_SEL, !flip_status);
		samsung_switching_lcd_resume(init, flip_status);
	
//################ send event  #################

	if( flip_status )	{
		input_report_key(flip->input, KEY_FOLDER_OPEN, 1);
		input_report_key(flip->input, KEY_FOLDER_OPEN, 0);
		input_sync(flip->input);
		#ifdef __TSP_DEBUG
		printk("[FLIP] %s: input flip key :  open\n", __FUNCTION__);
		#endif
	} else	{
		input_report_key(flip->input, KEY_FOLDER_CLOSE, 1);
		input_report_key(flip->input, KEY_FOLDER_CLOSE, 0);
		input_sync(flip->input);
		#ifdef __TSP_DEBUG
		printk ("[FLIP] %s: input flip key : close\n", __FUNCTION__);
		#endif

	}	

//################ second  #################

#ifdef CONFIG_BATTERY_SEC
		if ( !is_lpcharging_state() )
#endif
		{
			samsung_switching_tsp(init, flip_status);
//			if (!flip_status){
//				lp8720_set_folder_close();
//				samsung_switching_tkey(init, flip_status);		// after close
//			}
		}
}

static irqreturn_t sec_flip_irq_handler(int irq, void *_flip)
{
	struct sec_flip *flip = _flip;
	int val = 0;

	val = gpio_get_value_cansleep(flip->gpio);  

	if(val){		// OPEN
		flip_status = 1;		
	} else{			// CLOSE
		flip_status = 0;
		samsung_switching_tsp_pre(0, flip_status);
	}
	printk("[FLIP] %s: val=%d (1:open, 0:close)\n", __func__, val);

	wake_lock_timeout(&flip->wlock, 1 * HZ);
#if 0 // it has timer
	//disable_irq_nosync(flip->irq);
dbg_printk("jgb 1111\n"); 
//	if ( flip->irq != -1 ) 
	{
dbg_printk("jgb 222\n"); 
		if (!timer_pending(&flip->flip_timer)) {
dbg_printk("jgb 333\n"); 
			//wake_lock(&flip->wlock);
			mod_timer(&flip->flip_timer, jiffies + msecs_to_jiffies(FLIP_SCAN_INTERVAL));
		}
	}
#else // do not have timer, report right now
		queue_work(flip->wq, &flip->flip_id_det );
#endif
	return IRQ_HANDLED;
}


void sec_flip_timer(unsigned long data)
{
	int val = 0; 
	struct sec_flip* flip = (struct sec_flip*)data; 
	static int wait_flip_count = 0;
	static int wait_flip_status = 0;

	printk("%s: %s\n", __func__, (flip_status) ? "OPEN":"CLOSE" ); 

	if ( flip_status==FLIP_NOTINIT) {
		queue_work(flip->wq, &flip->flip_id_det ); 
		return ; 
	}

	val = gpio_get_value_cansleep( flip->gpio );  

	if (val != wait_flip_status) {
		wait_flip_count = 0;
		wait_flip_status = val;
		printk("%s: flip changed (%s)\n", __func__, (val) ? "OPEN":"CLOSE"); 
	} else if (wait_flip_count < FLIP_STABLE_COUNT) {
		wait_flip_count++;
		printk("%s: flip count:%d (%s)\n", __func__, wait_flip_count, (val) ? "OPEN":"CLOSE"); 
	}
	
	if (wait_flip_count >= FLIP_STABLE_COUNT) {
		if(val){		// OPEN
			flip_status = 1;		
		} 
		else{			// CLOSE
			flip_status = 0;
		}
		
		queue_work(flip->wq, &flip->flip_id_det );

	} 
	else {
		mod_timer(&flip->flip_timer, jiffies + msecs_to_jiffies(FLIP_SCAN_INTERVAL));
	}

}

#if 0// CONFIG_PM
static int sec_flip_suspend(struct device *dev)
{
	struct sec_flip *flip = dev_get_drvdata(dev);

	printk("%s:\n", __func__); 

	if (device_may_wakeup(dev)) {
		enable_irq_wake(flip->irq);
	}

	return 0;
}

static int sec_flip_resume(struct device *dev)
{
	struct sec_flip *flip = dev_get_drvdata(dev);

	printk("%s:\n", __func__); 

	if (device_may_wakeup(dev)) {
		disable_irq_wake(flip->irq);
	}

	return 0;
}

static struct dev_pm_ops pm8058_flip_pm_ops = {
	.suspend	= sec_flip_suspend,
	.resume		= sec_flip_resume,
};
#endif

/*zjh added flip state check file*/
#if 1 //zjh
static ssize_t status_check(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("flip status check :  %d\n", flip_status);
	return sprintf(buf, "%d\n", flip_status);
}

static DEVICE_ATTR(flipStatus, S_IRUGO | S_IWUSR | S_IWGRP , status_check, NULL);
#endif

static int __devinit sec_flip_probe(struct platform_device *pdev)
{
	struct input_dev *input;
	int err;
	struct sec_flip *flip;
	struct sec_flip_pdata *pdata = pdev->dev.platform_data;

	dev_info(&pdev->dev, "probe\n");

	if (!pdata) {
		dev_err(&pdev->dev, "power key platform data not supplied\n");
		return -EINVAL;
	}

	flip = kzalloc(sizeof(*flip), GFP_KERNEL);
	if (!flip)
		return -ENOMEM;

	flip->pdata   = pdata;

	/* Enable runtime PM ops, start in ACTIVE mode */
//	err = pm_runtime_set_active(&pdev->dev);
//	if (err < 0)
//		dev_err(&pdev->dev, "unable to set runtime pm state\n");
//	pm_runtime_enable(&pdev->dev);

/* INPUT DEVICE */
	input = input_allocate_device();
	if (!input) {
		dev_err(&pdev->dev, "Can't allocate power button\n");
		err = -ENOMEM;
		goto free_flip;
	}

	//input_set_capability(input, EV_KEY, KEY_POWER);
	set_bit(EV_KEY, input->evbit);
	set_bit(KEY_FOLDER_OPEN & KEY_MAX, input->keybit);
	set_bit(KEY_FOLDER_CLOSE & KEY_MAX, input->keybit);	
	
	input->name = "sec_flip_key";
	input->phys = "sec_flip/input0";
	input->dev.parent = &pdev->dev;

	err = input_register_device(input);
	if (err) {
		dev_err(&pdev->dev, "Can't register power key: %d\n", err);
		goto free_input_dev;
	}
	flip->input = input;

	platform_set_drvdata(pdev, flip);

//	err = switch_dev_register(&switch_flip);
//	if (err < 0) {
//		printk("[FLIP]: Failed to register switch device, err:%d\n", err);
//		goto err_flip;
//	}

/* INTERRUPT */ 
 	flip->gpio = GPIO_FLIP; 
/*	err = gpio_request(flip->gpio, "flip_det_pin") ; 
	if (err < 0) {
		printk(KERN_ERR "[FLIP]: Failed to register switch device\n");
		goto err_flip;
	}
*/
	//gpio_direction_input(flip->gpio);
	
	flip->irq = gpio_to_irq(GPIO_FLIP);

	flip->wq = create_singlethread_workqueue("sec_flip_wq");
	if (!flip->wq) {
		dev_err(&pdev->dev, "create_workqueue failed.\n"); 
	}
	INIT_WORK(&flip->flip_id_det, sec_flip_work_func);

	err = request_threaded_irq(flip->irq, NULL, sec_flip_irq_handler, (IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING), "flip_det_irq", flip);
	if (err < 0) {
		dev_err(&pdev->dev, "Can't get %d IRQ for flip: %d\n",
				 flip->irq, err);
		goto unreg_input_dev;
	}
	//disable_irq_nosync(flip->irq);
	
	device_init_wakeup(&pdev->dev, pdata->wakeup);

	wake_lock_init(&flip->wlock, WAKE_LOCK_SUSPEND, "sec_flip");
		
	printk("%s:   gpio=%d   irq=%d\n", __func__,  flip->gpio, flip->irq);

	/*zjh added flip state check file*/
#if 1 //zjh
	err = device_create_file(&pdev->dev, &dev_attr_flipStatus);
	if(err<0){
		printk("flip status check cannot create file :  %d\n", flip_status);
		goto free_flip;
	}
#endif

//	init_timer(&flip->flip_timer);
//	flip->flip_timer.function = sec_flip_timer;
//	flip->flip_timer.data = (unsigned long)flip;
	setup_timer(&flip->flip_timer, sec_flip_timer, (unsigned long)flip);
	mod_timer(&flip->flip_timer, jiffies + msecs_to_jiffies(5000));

	return 0;

err_flip:
	del_timer_sync(&flip->flip_timer);
//	switch_dev_unregister(&switch_flip);

unreg_input_dev:
	input_unregister_device(input);
	input = NULL;
free_input_dev:
	input_free_device(input);
free_flip:
	//pm_runtime_set_suspended(&pdev->dev);
	//pm_runtime_disable(&pdev->dev);
	kfree(flip);
	
	return err;
}

static int __devexit sec_flip_remove(struct platform_device *pdev)
{
	struct sec_flip *flip = platform_get_drvdata(pdev);

	printk("%s:\n", __func__); 

	if (flip!=NULL)
		del_timer_sync(&flip->flip_timer);
//	switch_dev_unregister(&switch_flip);

//	pm_runtime_set_suspended(&pdev->dev);
//	pm_runtime_disable(&pdev->dev);
	device_init_wakeup(&pdev->dev, 0);

	if (flip!=NULL) {
		free_irq(flip->irq, NULL);
		destroy_workqueue(flip->wq); 
		input_unregister_device(flip->input);
		kfree(flip);
	}

	return 0;
}

static struct platform_driver sec_flip_driver = {
	.probe		= sec_flip_probe,
	.remove		= __devexit_p(sec_flip_remove),
	.driver		= {
		.name	= "sec_flip",
		.owner	= THIS_MODULE,
#if 0 //CONFIG_PM
		.pm	= &pm8058_flip_pm_ops,
#endif
	},
};

static int __init sec_flip_init(void)
{
	return platform_driver_register(&sec_flip_driver);
}
module_init(sec_flip_init);

static void __exit sec_flip_exit(void)
{
	platform_driver_unregister(&sec_flip_driver);
}
module_exit(sec_flip_exit);

MODULE_ALIAS("platform:sec_flip");
MODULE_DESCRIPTION("PMIC8058 Flip Key");
MODULE_LICENSE("GPL v2");
