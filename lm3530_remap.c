/* lm3530_remap.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/backlight.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <mach/board.h>
#include <linux/earlysuspend.h>

int bl_set_intensity(struct backlight_device *bd);
int bl_get_intensity(struct backlight_device *bd);
void lm3530_lcd_backlight_set_level_remap(int level);

struct lm3530_device {
    struct i2c_client *client;
    struct backlight_device *bl_dev;
    int gpio;
    int max_current;
    int min_brightness;
    int max_brightness;
    int default_brightness;
    int factory_brightness;
    struct mutex bl_mutex;
};

extern struct i2c_client *lm3530_i2c_client;
extern struct lm3530_device *main_lm3530_dev;

static struct backlight_ops lm3530_bl_ops = {
    .update_status = bl_set_intensity,
    .get_brightness = bl_get_intensity,
};

static const struct backlight_ops* old_lm3530_bl_ops = NULL;
static void* old_mipi_lgit_callback = NULL;

static int backlight_remap_callback(struct device *d, void *data)
{
    struct i2c_client* ic;
    struct lm3530_device * dev;
    unsigned char init;

    init = *((unsigned char*) data);

    if (!d->driver) {
        return 0;
    }

    // looking for lm3530 driver
    if (strcmp(d->driver->name, "lm3530")) {
        return 0;
    }

    printk("Found lm3530 driver, remap now");
    ic = to_i2c_client(d);
    dev = (struct lm3530_device *)i2c_get_clientdata(ic);
    lm3530_i2c_client = ic;
    main_lm3530_dev = dev;

    // change backlight ops to our remap functions
    mutex_lock(&dev->bl_dev->ops_lock);

    if (init) {
        old_lm3530_bl_ops = dev->bl_dev->ops;
        dev->bl_dev->ops = &lm3530_bl_ops;

    } else {
        if (old_lm3530_bl_ops) {
            dev->bl_dev->ops = old_lm3530_bl_ops;
        }
    }

    printk("Adjust screen backlight...\n");
    dev->bl_dev->ops->update_status(dev->bl_dev);

    mutex_unlock(&dev->bl_dev->ops_lock);

    return 1;
}

static int mipi_lgit_backlight_level(int level, int max, int min)
{
    lm3530_lcd_backlight_set_level_remap(level);
    return 0;
}

static int mipi_lgit_remap_callback(struct device *d, void *data)
{
    struct platform_device* pd;
    struct msm_panel_common_pdata* panel_data;
    unsigned char init;

    init = *((unsigned char*) data);
    if (!d->driver) {
        return 0;
    }

    if (strcmp(d->driver->name, "mipi_lgit")) {
        return 0;
    }

    pd = to_platform_device(d);
    panel_data = (struct msm_panel_common_pdata*)pd->dev.platform_data;

    if (!panel_data->backlight_level) {
        return 0;
    }

    printk("Found mipi_lgit driver, remap now\n");
    if (init) {

        old_mipi_lgit_callback = panel_data->backlight_level;
        printk("mipi-lgit: 0x%x->0x%x\n",
                (unsigned int) panel_data->backlight_level,
                (unsigned int) mipi_lgit_backlight_level);
        panel_data->backlight_level = mipi_lgit_backlight_level;
    } else {
        if (old_mipi_lgit_callback) {
            panel_data->backlight_level = old_mipi_lgit_callback;
        }
    }
    return 0;
}

static DEFINE_MUTEX(core_lock);

static int remap_helper(unsigned char init)
{
    mutex_lock(&core_lock);
    i2c_for_each_dev(&init, backlight_remap_callback);
    bus_for_each_dev(&platform_bus_type, NULL, &init, mipi_lgit_remap_callback);
    mutex_unlock(&core_lock);
    return 0;
}
static int __init lcd_backlight_init(void)
{
    return remap_helper(1);
}

static void lcd_backlight_remove(void)
{
    remap_helper(0);
}

module_init(lcd_backlight_init);
module_exit(lcd_backlight_remove);

MODULE_DESCRIPTION("LM3530 Backlight remap");
MODULE_AUTHOR("Binary <hyo1411@gmail.com>");
MODULE_LICENSE("GPL");

