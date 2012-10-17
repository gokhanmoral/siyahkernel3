#include <linux/kallsyms.h>
#include <linux/switch.h>

#define DEBUG

//codec_list pointer to the one in soc_core
struct list_head *codec_list_ptr;
static char *devicename = NULL;
static struct miscdevice voodoo_sound_device;

//these two structures will hold the pointers to the original...
//...wm8994 read and write functions. the names are chosen...
//...as they are used in the original voodoo implementation.
static int (*tegrak_wm8994_write)(struct snd_soc_codec *codec,
				unsigned int reg, unsigned int val);
static unsigned int (*tegrak_wm8994_read)(struct snd_soc_codec *codec,
				unsigned int reg);

static void * (*gm_dev_get_drvdata)(const struct device *dev);
static int (*gm_misc_register)(struct miscdevice * misc);
static int (*gm_misc_deregister)(struct miscdevice *misc);

#define misc_register (*gm_misc_register)
#define misc_deregister (*gm_misc_deregister)
#define snd_soc_codec_get_drvdata(codec) (*gm_dev_get_drvdata)(codec->dev)

static struct snd_soc_codec *pwm8994hijack = NULL;


static int wm8994_write_new(struct snd_soc_codec *codec,
			   unsigned int reg, unsigned int value)
{
	//modify the value in voodoo logic and pass it to the original function
	value = voodoo_hook_wm8994_write(codec, reg, value);
	return  (*tegrak_wm8994_write)(codec,reg,value);
}

static unsigned int wm8994_read_new(struct snd_soc_codec *codec,
			   unsigned int reg)
{
	//for now there is nothing to do here, so just call the original function
	return  (*tegrak_wm8994_read)(codec,reg);
}

static struct switch_dev *gm_android_switch;
static inline int detect_headphone(void)
{
	return switch_get_state(gm_android_switch)>0;
}

static int __init wm8994_voodoo_start(void)
{
	unsigned long soc_addr = kallsyms_lookup_name("codec_list");
	struct snd_soc_codec *codec;

	if(soc_addr > 0)
	{
		pr_info("Found codec_list, trying to hijack wm8994 read and write\n");
		codec_list_ptr = (struct list_head *)soc_addr;
		list_for_each_entry(codec, codec_list_ptr, list) {
			pr_info("Codec found: %s\n", codec->name);
			if(!strcmp(codec->name, "wm8994-codec"))
			{
				pr_info("Hijacking...\n");
				pwm8994hijack = codec;
				break;
			}
		}
		if( pwm8994hijack != NULL )
		{
			gm_android_switch = (struct switch_dev *)
								kallsyms_lookup_name("android_switch");
			gm_dev_get_drvdata = (void * (*)(const struct device *))
								kallsyms_lookup_name("dev_get_drvdata");
			gm_misc_deregister = (int (*)(struct miscdevice *))
								kallsyms_lookup_name("misc_deregister");
			gm_misc_register = (int (*)(struct miscdevice *))
								kallsyms_lookup_name("misc_register");
			tegrak_wm8994_write = (*pwm8994hijack).write;
			(*pwm8994hijack).write = wm8994_write_new;
			tegrak_wm8994_read = (*pwm8994hijack).read;
			(*pwm8994hijack).read = wm8994_read_new;
			if(devicename != NULL)
			{
				voodoo_sound_device.name = devicename;
			}
			pr_info("Using '%s' as device name.\n", voodoo_sound_device.name);
			voodoo_hook_wm8994_pcm_probe(codec);
		}
	}
	return 0;
}

static void __exit wm8994_voodoo_stop(void)
{
	if(pwm8994hijack != NULL)
	{
		pr_info("Releasing wm8994_write\n");
		(*pwm8994hijack).write = tegrak_wm8994_write;
		(*pwm8994hijack).read = tegrak_wm8994_read;
		voodoo_hook_wm8994_pcm_remove();
	}
}

module_init( wm8994_voodoo_start );
module_exit( wm8994_voodoo_stop );

module_param(devicename, charp, 0);
MODULE_PARM_DESC(devicename, "Optional: Name of the devicefile.");

MODULE_AUTHOR("Gokhan Moral <gm@alumni.bilkent.edu.tr>");
MODULE_DESCRIPTION("a primitive (but effective) implementation for "
	"the missing code in original Voodoo sound to use "
	"it as a module");
MODULE_LICENSE("GPL");
