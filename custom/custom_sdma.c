/*
 * custom_sdma
 *
 * Copyright 2013 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * Author: Vincent Stehlé <vincent.stehle@freescale.com>,
 *         based on 'scull' from the book "Linux Device Drivers" by Alessandro
 *         Rubini and Jonathan Corbet, published by O'Reilly & Associates.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/fs.h>		/* everything... */
#include <linux/cdev.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <asm/uaccess.h>	/* copy_*_user */
#include <asm/io.h>		/* ioremap */
#include <mach/dma.h>

#define SRAM_ADDR	0x900000
#define SRAM_SIZE	0x40000

#ifndef CUSTOM_SDMA_MAJOR
#define CUSTOM_SDMA_MAJOR 0   /* dynamic major by default */
#endif

struct custom_sdma_dev {
	struct semaphore sem;		/* mutual exclusion semaphore */
	struct cdev cdev;		/* Char device structure */
	bool chrdev_registered;		/* For cleanup */
	bool cdev_added;		/* For cleanup */
	void *sram_base;
	struct dma_chan *dma_chan;
};

/*
 * Ioctl definitions
 */

/* Use 'F' as magic number */
#define CUSTOM_SDMA_IOC_MAGIC  'F'
/* Please use a different 8-bit number in your code */

#define CUSTOM_SDMA_IOCRESET    _IO(CUSTOM_SDMA_IOC_MAGIC, 0)

#define CUSTOM_SDMA_IOC_MAXNR 0

/*
 * Our parameters which can be set at load time.
 */

int custom_sdma_major =   CUSTOM_SDMA_MAJOR;
int custom_sdma_minor =   0;

module_param(custom_sdma_major, int, S_IRUGO);
module_param(custom_sdma_minor, int, S_IRUGO);

MODULE_AUTHOR("Vincent Stehlé <vincent.stehle@freescale.com>");
MODULE_LICENSE("GPL v2");

struct custom_sdma_dev custom_sdma_device;

/*
 * Open and close
 */

int custom_sdma_open(struct inode *inode, struct file *filp)
{
	struct custom_sdma_dev *dev; /* device information */

	dev = container_of(inode->i_cdev, struct custom_sdma_dev, cdev);
	filp->private_data = dev; /* for other methods */
	return 0;          /* success */
}

int custom_sdma_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/*
 * Data management: read and write
 */

ssize_t custom_sdma_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
	struct custom_sdma_dev *dev = filp->private_data; 
	ssize_t retval = 0;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	if (*f_pos >= SRAM_SIZE)
		goto out;

	if (*f_pos + count > SRAM_SIZE)
		count = SRAM_SIZE - *f_pos;

	// Copy SRAM contents.
	if (copy_to_user(buf, dev->sram_base + *f_pos, count)) {
		retval = -EFAULT;
		goto out;
	}

	*f_pos += count;
	retval = count;

  out:
	up(&dev->sem);
	return retval;
}

static void custom_sdma_dma_callback(void *data)
{

	// TODO!

//	struct asrc_pair_params *params;
//	unsigned long lock_flags;
//
//	params = data;
//	dma_unmap_sg(NULL, params->input_sg,
//		params->input_sg_nodes, DMA_MEM_TO_DEV);
//	spin_lock_irqsave(&input_int_lock, lock_flags);
//	params->input_counter++;
//	wake_up_interruptible(&params->input_wait_queue);
//	spin_unlock_irqrestore(&input_int_lock, lock_flags);
//	schedule_work(&params->task_output_work);

	printk("custom_sdma: callback\n");
}

static void print_sram(void)
{
	volatile char *p = (volatile char *)custom_sdma_device.sram_base;
	printk("custom_sdma: sram: %02x %02x %02x %02x\n", p[0], p[1], p[2], p[3]);
}

ssize_t custom_sdma_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	struct custom_sdma_dev *dev = filp->private_data;
	ssize_t retval = -ENOMEM; /* value used in "goto out" statements */
	struct dma_slave_config slave_config;
	int ret;
	struct dma_async_tx_descriptor *desc;
	struct scatterlist sgl;
	dma_cookie_t cookie;
	enum dma_status status;

	// TODO! Handle non-multiples of 2x4 bytes

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	// Hack! No more than SRAM size for now.
	// TODO! true scatterlist
	if (count > SRAM_SIZE)
		count = SRAM_SIZE;

	// Check range
	if (!access_ok(VERIFY_READ, buf, count)) {
		retval = -EFAULT;
		goto out;
	}

	print_sram();

	// We need a scatterlist
	// TODO! true scatterlist
	sg_init_one(&sgl, buf, count);

//	sg_init_table(sg, sg_nent);
//	switch (sg_nent) {
//	case 1:
//		sg_init_one(sg, buf_addr, buf_len);
//		break;
//	case 2:
//	case 3:
//	case 4:
//		for (sg_index = 0; sg_index < (sg_nent - 1); sg_index++) {
//			sg_set_buf(&sg[sg_index],
//				buf_addr + sg_index * ASRC_MAX_BUFFER_SIZE,
//				ASRC_MAX_BUFFER_SIZE);
//		}
//		sg_set_buf(&sg[sg_index],
//			buf_addr + sg_index * ASRC_MAX_BUFFER_SIZE,
//			buf_len - ASRC_MAX_BUFFER_SIZE * sg_index);
//		break;
//	default:
//		pr_err("Error Input DMA nodes number[%d]!", sg_nent);
//		return -EINVAL;
//	}

	ret = dma_map_sg(NULL, &sgl, 1/*TODO! nents*/, DMA_TO_DEVICE);
	if (ret == 0) {
		pr_err("custom_sdma: write: dma_map_sg error!\n");
		retval = -EBUSY;
		goto out;
	}

	// Configure the slave DMA
// TODO!
//	slave_config.direction = DMA_MEM_TO_DEV;
	slave_config.direction = DMA_MEM_TO_MEM;
	slave_config.dst_addr = SRAM_ADDR;
	slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	slave_config.dst_maxburst = 2;	// 2 words burst.

	ret = dmaengine_slave_config(dev->dma_chan, &slave_config);
	if (ret) {
		pr_err("custom_sdma: write: dmaengine_slave_config error %d!\n", ret);
		retval = -EBUSY;
		goto out;
	}

	// Get transfer descriptor
	desc = dev->dma_chan->device->device_prep_slave_sg(
			dev->dma_chan,
			&sgl,
			1,		// TODO! sg_len
// TODO!
//			DMA_MEM_TO_DEV,
			DMA_MEM_TO_MEM,
			0		// flags
			);

	if (!desc) {
		pr_err("custom_sdma: write: device_prep_slave_sg error!\n");
		retval = -EBUSY;
		goto out;
	}

	// Do the DMA transfer
	desc->callback = custom_sdma_dma_callback;
	cookie = dmaengine_submit(desc);

	// TODO! Better: sleep and wake up with callback.
	status = dma_sync_wait(dev->dma_chan, cookie);
	if (status != DMA_SUCCESS) {
		pr_err("custom_sdma: write: dma_sync_wait error %d!\n", status);
		dmaengine_terminate_all(dev->dma_chan);
		retval = -EBUSY;
	}

	dma_unmap_sg(NULL, &sgl, 1/* TODO! nents */, DMA_TO_DEVICE);

out:
	up(&dev->sem);

	print_sram();

	return retval;
}

/*
 * The ioctl() implementation
 */

int custom_sdma_ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{

	int err = 0;
	int retval = 0;
    
	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != CUSTOM_SDMA_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > CUSTOM_SDMA_IOC_MAXNR) return -ENOTTY;

	/*
	 * the direction is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. `Type' is user-oriented, while
	 * access_ok is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;

	switch(cmd) {

	  case CUSTOM_SDMA_IOCRESET:
		// TODO!
		break;
        
	  default:  /* redundant, as cmd was checked against MAXNR */
		return -ENOTTY;
	}

	return retval;
}

static struct file_operations custom_sdma_fops = {
	.owner =    THIS_MODULE,
	.read =     custom_sdma_read,
	.write =    custom_sdma_write,
#ifdef CONFIG_COMPAT
	.compat_ioctl =    custom_sdma_ioctl,
#endif
	.open =     custom_sdma_open,
	.release =  custom_sdma_release,
};

/*
 * Finally, the module stuff
 */

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
void custom_sdma_cleanup_module(void)
{
	struct custom_sdma_dev *dev = &custom_sdma_device;

	/* unmap sram */
	if (dev->sram_base)
		iounmap(dev->sram_base);

	/* free dma */
	if (dev->dma_chan) {
		dmaengine_terminate_all(dev->dma_chan);
		dma_release_channel(dev->dma_chan);
	}

	/* Get rid of our char dev entries */
	if (dev->chrdev_registered) {
		dev_t devno = MKDEV(custom_sdma_major, custom_sdma_minor);
		unregister_chrdev_region(devno, 1);
	}

	if (dev->cdev_added)
		cdev_del(&dev->cdev);

	printk("custom_sdma: cleaned up\n");
}

/*
 * Set up the char_dev structure for this device.
 */
static int custom_sdma_setup_cdev(struct cdev *cdev)
{
	int devno = MKDEV(custom_sdma_major, custom_sdma_minor);
	cdev_init(cdev, &custom_sdma_fops);
	cdev->owner = THIS_MODULE;
	cdev->ops = &custom_sdma_fops;
	return cdev_add(cdev, devno, 1);
}

static bool filter_dma_chan(struct dma_chan *chan, void *param)
{
	if (!strcmp(dev_name(chan->device->dev), "imx-sdma")) {
		chan->private = param;
		return true;
	}

	return false;
}

static struct dma_chan *allocate_dma(void)
{
	dma_cap_mask_t dma_mask;
	struct imx_dma_data dma_data = {0};

	dma_data.peripheral_type = IMX_DMATYPE_EXT;	/* External peripheral */
	dma_data.priority = DMA_PRIO_MEDIUM;
/*	dma_data.dma_request = dma_req;*/	// TODO!

	dma_cap_zero(dma_mask);
	dma_cap_set(DMA_SLAVE, dma_mask);
	return dma_request_channel(dma_mask, filter_dma_chan, &dma_data);
}

int custom_sdma_init_module(void)
{
	int result;
	dev_t devno = 0;
	struct custom_sdma_dev *dev = &custom_sdma_device;

	// Allocate a DMA channel.
	dev->dma_chan = allocate_dma();

	if (!dev->dma_chan) {
		pr_err("custom_sdma: failed to allocate dma channel!\n");
		result = -EBUSY;
		goto err;
	}

	printk("custom_sdma: allocated dma chan_id %i\n", dev->dma_chan->chan_id);

	// Remap SRAM.
	dev->sram_base = ioremap(SRAM_ADDR, SRAM_SIZE);

	if (!dev->sram_base) {
		pr_err("custom_sdma: ioremap error!\n");
		result = -ENOMEM;
		goto err;
	}

	printk("custom_sdma: remapped sram at %p\n", dev->sram_base);
	print_sram();

	/*
	 * Get a range of minor numbers to work with, asking for a dynamic
	 * major unless directed otherwise at load time.
	 */
	if (custom_sdma_major) {
		devno = MKDEV(custom_sdma_major, custom_sdma_minor);
		result = register_chrdev_region(devno, 1, "custom_sdma");

	} else {
		result = alloc_chrdev_region(&devno, custom_sdma_minor, 1, "custom_sdma");
		custom_sdma_major = MAJOR(devno);
		printk("custom_sdma: got major %i\n", custom_sdma_major);
	}

	if (result < 0) {
		pr_err("custom_sdma: register chrdev error %d (major: %d)\n",
			result, custom_sdma_major);
		goto err;
	}

	dev->chrdev_registered = true;

        /* Initialize the device. */
	sema_init(&dev->sem, 1);
	result = custom_sdma_setup_cdev(&dev->cdev);

	if (result < 0) {
		pr_err("custom_sdma: error %d adding custom_sdma cdev\n", result);
		goto err;
	}

	dev->cdev_added = true;

	printk("custom_sdma: initialized\n");
	return 0; /* succeed */

err:
	custom_sdma_cleanup_module();
	return result;
}

module_init(custom_sdma_init_module);
module_exit(custom_sdma_cleanup_module);
