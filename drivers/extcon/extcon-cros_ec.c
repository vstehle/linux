/**
 * drivers/extcon/extcon-cros_ec - ChromeOS Embedded Controller extcon
 *
 * Copyright (C) 2015 Google, Inc
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/extcon.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/mfd/cros_ec.h>
#include <linux/mfd/cros_ec_commands.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/usb/class-dual-role.h>

#define CROS_EC_USB_POLLING_DELAY msecs_to_jiffies(1000)

/*
 * Timeout for a USB PD power swap execution
 * 1000 ms for tSwapRecovery : maximum time after Hard Reset to settle
 *  275 ms for tSrcTurnOn (VBUS going from 0V to 5V)
 *  650 ms for tSafe0V (VBUS going to 0V)
 *  500 ms of extra margin
 */
#define POWER_SWAP_TIMEOUT msecs_to_jiffies(2425)
/*
 * Timeout for USB PD data swap execution
 *   30 ms for tSenderResponse
 * 2x 1 ms for tReceive
 *   some margin for events and AP/EC communication
 */
#define DATA_SWAP_TIMEOUT msecs_to_jiffies(150)

struct cros_ec_extcon_info {
	struct device *dev;
	struct extcon_dev *edev;

	int port_id;

	struct cros_ec_device *ec;

	struct notifier_block notifier;

	unsigned int dr; /* data role */
	unsigned int pr; /* power role */
	bool dp; /* DisplayPort enabled */
	bool mux; /* SuperSpeed (usb3) enabled */
	unsigned int power_type;
	unsigned int writeable;
	wait_queue_head_t role_wait;

	struct dual_role_phy_instance *drp_inst;
};

static const unsigned int usb_type_c_cable[] = {
	EXTCON_USB,
	EXTCON_USB_HOST,
	EXTCON_DISP_DP,
	EXTCON_NONE,
};

/**
 * cros_ec_pd_command() - Send a command to the EC.
 * @info: pointer to struct cros_ec_extcon_info
 * @command: EC command
 * @version: EC command version
 * @outdata: EC command output data
 * @outsize: Size of outdata
 * @indata: EC command input data
 * @insize: Size of indata
 *
 * Return: 0 on success, <0 on failure.
 */
static int cros_ec_pd_command(struct cros_ec_extcon_info *info,
			      unsigned int command,
			      unsigned int version,
			      void *outdata,
			      unsigned int outsize,
			      void *indata,
			      unsigned int insize)
{
	struct cros_ec_command *msg;
	int ret;

	msg = kzalloc(sizeof(*msg) + max(outsize, insize), GFP_KERNEL);

	msg->version = version;
	msg->command = command;
	msg->outsize = outsize;
	msg->insize = insize;

	if (outsize)
		memcpy(msg->data, outdata, outsize);

	ret = cros_ec_cmd_xfer_status(info->ec, msg);
	if (ret >= 0 && insize)
		memcpy(indata, msg->data, insize);

	kfree(msg);
	return ret;
}

/**
 * cros_ec_usb_get_power_type() - Get power type info about PD device attached to
 * given port.
 * @info: pointer to struct cros_ec_extcon_info
 *
 * Return: power type on success, <0 on failure.
 */
static int cros_ec_usb_get_power_type(struct cros_ec_extcon_info *info)
{
	struct ec_params_usb_pd_power_info req;
	struct ec_response_usb_pd_power_info resp;
	int ret;

	req.port = info->port_id;
	ret = cros_ec_pd_command(info, EC_CMD_USB_PD_POWER_INFO, 0,
				 &req, sizeof(req), &resp, sizeof(resp));
	if (ret < 0)
		return ret;

	return resp.type;
}



/**
 * cros_ec_usb_get_pd_mux_state() - Get PD mux state for given port.
 * @info: pointer to struct cros_ec_extcon_info
 *
 * Return: PD mux state on success, <0 on failure.
 */
static int cros_ec_usb_get_pd_mux_state(struct cros_ec_extcon_info *info)
{
	struct ec_params_usb_pd_mux_info req;
	struct ec_response_usb_pd_mux_info resp;
	int ret;

	req.port = info->port_id;
	ret = cros_ec_pd_command(info, EC_CMD_USB_PD_MUX_INFO, 0,
				 &req, sizeof(req),
				 &resp, sizeof(resp));
	if (ret < 0)
		return ret;

	return resp.flags;
}

/**
 * cros_ec_usb_get_role() - Get role info about possible PD device attached to a
 * given port.
 * @info: pointer to struct cros_ec_extcon_info
 * @polarity: pointer to cable polarity (return value)
 *
 * Return: role info on success, -ENOTCONN if no cable is connected, <0 on failure.
 */
static int cros_ec_usb_get_role(struct cros_ec_extcon_info *info,
				bool *polarity)
{
	struct ec_params_usb_pd_control pd_control;
	struct ec_response_usb_pd_control_v1 resp;
	int ret;

	pd_control.port = info->port_id;
	pd_control.role = USB_PD_CTRL_ROLE_NO_CHANGE;
	pd_control.mux = USB_PD_CTRL_MUX_NO_CHANGE;
	pd_control.swap = USB_PD_CTRL_SWAP_NONE;
	ret = cros_ec_pd_command(info, EC_CMD_USB_PD_CONTROL, 1,
				 &pd_control, sizeof(pd_control),
				 &resp, sizeof(resp));
	if (ret < 0)
		return ret;

	if (!(resp.enabled & PD_CTRL_RESP_ENABLED_CONNECTED))
		return -ENOTCONN;

	*polarity = resp.polarity;

	return resp.role;
}

/**
 * cros_ec_pd_get_num_ports() - Get number of EC charge ports.
 * @info: pointer to struct cros_ec_extcon_info
 *
 * Return: number of ports on success, <0 on failure.
 */
static int cros_ec_pd_get_num_ports(struct cros_ec_extcon_info *info)
{
	struct ec_response_usb_pd_ports resp;
	int ret;

	ret = cros_ec_pd_command(info, EC_CMD_USB_PD_PORTS,
				 0, NULL, 0, &resp, sizeof(resp));
	if (ret < 0)
		return ret;

	return resp.num_ports;
}

static const char *cros_ec_usb_role_string(unsigned int role)
{
	return DUAL_ROLE_PROP_DR_NONE == role ? "DISCONNECTED" :
		(DUAL_ROLE_PROP_DR_HOST == role ? "DFP" : "UFP");
}

static const char *cros_ec_usb_power_type_string(unsigned int type)
{
	switch (type) {
	case USB_CHG_TYPE_NONE:
		return "USB_CHG_TYPE_NONE";
	case USB_CHG_TYPE_PD:
		return "USB_CHG_TYPE_PD";
	case USB_CHG_TYPE_PROPRIETARY:
		return "USB_CHG_TYPE_PROPRIETARY";
	case USB_CHG_TYPE_C:
		return "USB_CHG_TYPE_C";
	case USB_CHG_TYPE_BC12_DCP:
		return "USB_CHG_TYPE_BC12_DCP";
	case USB_CHG_TYPE_BC12_CDP:
		return "USB_CHG_TYPE_BC12_CDP";
	case USB_CHG_TYPE_BC12_SDP:
		return "USB_CHG_TYPE_BC12_SDP";
	case USB_CHG_TYPE_OTHER:
		return "USB_CHG_TYPE_OTHER";
	case USB_CHG_TYPE_VBUS:
		return "USB_CHG_TYPE_VBUS";
	case USB_CHG_TYPE_UNKNOWN:
		return "USB_CHG_TYPE_UNKNOWN";
	default:
		return "USB_CHG_TYPE_UNKNOWN";
	}
}

static bool cros_ec_usb_power_type_is_wall_wart(unsigned int type,
						unsigned int role)
{
	switch (type) {
	/* FIXME : Guppy, Donnettes, and other chargers will be miscategorized
	 * because they identify with USB_CHG_TYPE_C, but we can't return true
	 * here from that code because that breaks Suzy-Q and other kinds of
	 * USB Type-C cables and peripherals.
	 */
	case USB_CHG_TYPE_PROPRIETARY:
	case USB_CHG_TYPE_BC12_DCP:
		return true;
	case USB_CHG_TYPE_PD:
#if 0		/* TODO(crosbug.com/p/45871) use USB comm bit when available */
		return !(role & PD_CTRL_RESP_ROLE_USB_COMM);
#else
		return false;
#endif
	case USB_CHG_TYPE_C:
	case USB_CHG_TYPE_BC12_CDP:
	case USB_CHG_TYPE_BC12_SDP:
	case USB_CHG_TYPE_OTHER:
	case USB_CHG_TYPE_VBUS:
	case USB_CHG_TYPE_UNKNOWN:
	case USB_CHG_TYPE_NONE:
	default:
		return false;
	}
}

static unsigned int cros_ec_usb_role_is_writeable(unsigned int role)
{
	unsigned int write_mask = 0;

	if (role & PD_CTRL_RESP_ROLE_DR_POWER)
		write_mask |= (1 << DUAL_ROLE_PROP_PR);
	if ((role & PD_CTRL_RESP_ROLE_DR_DATA) &&
	    (role & PD_CTRL_RESP_ROLE_USB_COMM))
		write_mask |= (1 << DUAL_ROLE_PROP_DR);

	return write_mask;
}

static int extcon_cros_ec_detect_cable(struct cros_ec_extcon_info *info,
				       bool force)
{
	struct device *dev = info->dev;
	int role, power_type;
	unsigned int dr, pr;
	bool polarity, dp, mux, hpd;

	power_type = cros_ec_usb_get_power_type(info);
	if (power_type < 0) {
		dev_err(dev, "failed getting power type err = %d\n",
			power_type);
		return power_type;
	}

	role = cros_ec_usb_get_role(info, &polarity);
	if (role < 0) {
		if (role != -ENOTCONN) {
			dev_err(dev, "failed getting role err = %d\n", role);
			return role;
		}
		dr = DUAL_ROLE_PROP_DR_NONE;
		pr = DUAL_ROLE_PROP_PR_NONE;
		polarity = false;
		dp = false;
		mux = false;
		hpd = false;
		dev_dbg(dev, "disconnected\n");
	} else {
		int pd_mux_state;

		dr = (role & PD_CTRL_RESP_ROLE_DATA) ?
			DUAL_ROLE_PROP_DR_HOST : DUAL_ROLE_PROP_DR_DEVICE;
		pr = (role & PD_CTRL_RESP_ROLE_POWER) ?
			DUAL_ROLE_PROP_PR_SRC : DUAL_ROLE_PROP_PR_SNK;
		pd_mux_state = cros_ec_usb_get_pd_mux_state(info);
		if (pd_mux_state < 0)
			pd_mux_state = USB_PD_MUX_USB_ENABLED;
		dp = pd_mux_state & USB_PD_MUX_DP_ENABLED;
		mux = pd_mux_state & USB_PD_MUX_USB_ENABLED;
		hpd = pd_mux_state & USB_PD_MUX_HPD_IRQ;

		dev_dbg(dev,
			"connected role 0x%x pwr type %d dr %d pr %d pol %d mux %d dp %d hpd %d\n",
			role, power_type, dr, pr, polarity, mux, dp, hpd);
	}

	/*
	 * When there is no USB host (e.g. USB PD charger),
	 * we are not really a UFP for the AP.
	 */
	if (dr == DUAL_ROLE_PROP_DR_DEVICE &&
	    cros_ec_usb_power_type_is_wall_wart(power_type, role))
		dr = DUAL_ROLE_PROP_DR_NONE;

	if (force || info->dr != dr || info->pr != pr || info->dp != dp ||
	    info->mux != mux || info->power_type != power_type) {
		bool host_connected = false, device_connected = false;

		dev_dbg(dev, "Type/Role switch! type = %s role = %s\n",
			cros_ec_usb_power_type_string(power_type),
			cros_ec_usb_role_string(dr));
		info->dr = dr;
		info->pr = pr;
		info->dp = dp;
		info->mux = mux;
		info->power_type = power_type;
		info->writeable = cros_ec_usb_role_is_writeable(role);

		if (dr == DUAL_ROLE_PROP_DR_DEVICE)
			device_connected = true;
		else if (dr == DUAL_ROLE_PROP_DR_HOST)
			host_connected = true;

		extcon_set_state(info->edev, EXTCON_USB, device_connected);
		extcon_set_state(info->edev, EXTCON_USB_HOST, host_connected);
		extcon_set_state(info->edev, EXTCON_DISP_DP, dp);
		extcon_set_property(info->edev, EXTCON_USB,
				    EXTCON_PROP_USB_VBUS,
				    (union extcon_property_value)(int)pr);
		extcon_set_property(info->edev, EXTCON_USB_HOST,
				    EXTCON_PROP_USB_VBUS,
				    (union extcon_property_value)(int)pr);
		extcon_set_property(info->edev, EXTCON_USB,
				    EXTCON_PROP_USB_TYPEC_POLARITY,
				    (union extcon_property_value)(int)polarity);
		extcon_set_property(info->edev, EXTCON_USB_HOST,
				    EXTCON_PROP_USB_TYPEC_POLARITY,
				    (union extcon_property_value)(int)polarity);
		extcon_set_property(info->edev, EXTCON_DISP_DP,
				    EXTCON_PROP_USB_TYPEC_POLARITY,
				    (union extcon_property_value)(int)polarity);
		extcon_set_property(info->edev, EXTCON_USB,
				    EXTCON_PROP_USB_SS,
				    (union extcon_property_value)(int)mux);
		extcon_set_property(info->edev, EXTCON_USB_HOST,
				    EXTCON_PROP_USB_SS,
				    (union extcon_property_value)(int)mux);
		extcon_set_property(info->edev, EXTCON_DISP_DP,
				    EXTCON_PROP_USB_SS,
				    (union extcon_property_value)(int)mux);

		extcon_sync(info->edev, EXTCON_USB);
		extcon_sync(info->edev, EXTCON_USB_HOST);
		extcon_sync(info->edev, EXTCON_DISP_DP);

		wake_up_all(&info->role_wait);
		dual_role_instance_changed(info->drp_inst);
	} else if (hpd) {
		extcon_sync(info->edev, EXTCON_DISP_DP);
	}
	return 0;
}

static int extcon_cros_ec_event(struct notifier_block *nb,
	unsigned long queued_during_suspend, void *_notify)
{
	struct cros_ec_extcon_info *info;
	struct cros_ec_device *ec;
	u32 host_event;

	info = container_of(nb, struct cros_ec_extcon_info, notifier);
	ec = info->ec;

	host_event = cros_ec_get_host_event(ec);
	if (host_event & (EC_HOST_EVENT_MASK(EC_HOST_EVENT_PD_MCU) |
			  EC_HOST_EVENT_MASK(EC_HOST_EVENT_USB_MUX))) {
		extcon_cros_ec_detect_cable(info, false);
		return NOTIFY_OK;
	}
	return NOTIFY_DONE;
}

static bool extcon_cros_ec_has_vconn(struct cros_ec_extcon_info *info)
{
	bool polarity;
	int role;

	role = cros_ec_usb_get_role(info, &polarity);

	return (role >= 0) && (role & PD_CTRL_RESP_ROLE_VCONN);
}

static int extcon_cros_ec_force_data_role(struct cros_ec_extcon_info *info,
					   unsigned int new_dr)
{
	struct device *dev = info->dev;
	struct ec_params_usb_pd_control pd_control;
	struct ec_response_usb_pd_control_v1 resp;
	int ret;

	dev_info(dev, "Force Data Role to %d (from %d)\n", new_dr, info->dr);

	if ((new_dr != DUAL_ROLE_PROP_DR_HOST) &&
	    (new_dr != DUAL_ROLE_PROP_DR_DEVICE))
		return -EINVAL;

	if (new_dr == info->dr)
		return 0;

	pd_control.port = info->port_id;
	pd_control.role = USB_PD_CTRL_ROLE_NO_CHANGE;
	pd_control.mux = USB_PD_CTRL_MUX_NO_CHANGE;
	pd_control.swap = USB_PD_CTRL_SWAP_DATA;
	ret = cros_ec_pd_command(info, EC_CMD_USB_PD_CONTROL, 1,
				 &pd_control, sizeof(pd_control),
				 &resp, sizeof(resp));
	dev_dbg(dev, "EC data swap to %s = %d\n",
		new_dr == DUAL_ROLE_PROP_DR_HOST ? "dfp" : "ufp", ret);
	if (ret < 0)
		return ret;

	/* wait for the swap to happen or timeout */
	ret = wait_event_timeout(info->role_wait, new_dr == info->dr,
				 DATA_SWAP_TIMEOUT);
	dev_dbg(dev, "data swap %s role %s\n",
		ret == 0 ? "timed out" : "succeeded", info->dr ? "UFP" : "DFP");

	return ret == 0 ? -ETIMEDOUT : ret;
}

static int extcon_cros_ec_force_power_role(struct cros_ec_extcon_info *info,
					   unsigned int new_pr)
{
	struct device *dev = info->dev;
	struct ec_params_charge_port_override p;
	int ret;

	dev_info(dev, "Force Power Role to %d (from %d)\n", new_pr, info->pr);

	if (new_pr == info->pr)
		return 0;

	switch (new_pr) {
	case DUAL_ROLE_PROP_PR_SRC:
		p.override_port = OVERRIDE_DONT_CHARGE;
		break;
	case DUAL_ROLE_PROP_PR_SNK:
		p.override_port = 0;
		break;
	case DUAL_ROLE_PROP_PR_NONE:
	default:
		return -EINVAL;
	}

	ret = cros_ec_pd_command(info, EC_CMD_PD_CHARGE_PORT_OVERRIDE, 0,
				 &p, sizeof(p), NULL, 0);
	dev_dbg(dev, "EC charge port override to %d = %d\n",
		p.override_port, ret);
	if (ret < 0)
		return ret;

	/* wait for the swap to happen or timeout */
	ret = wait_event_timeout(info->role_wait, new_pr == info->pr,
				 POWER_SWAP_TIMEOUT);
	dev_dbg(dev, "power swap %s role %s\n",
		ret == 0 ? "timed out" : "succeed", info->pr ? "SNK" : "SRC");

	return ret == 0 ? -ETIMEDOUT : ret;
}

static int extcon_drp_get_prop(struct dual_role_phy_instance *inst,
			enum dual_role_property prop,
			unsigned int *val)
{
	struct cros_ec_extcon_info *info = dual_role_get_drvdata(inst);
	int ret = 0;

	if (!info)
		return -EINVAL;

	switch (prop) {
	case DUAL_ROLE_PROP_MODE:
		*val = info->pr == DUAL_ROLE_PROP_PR_SRC ?
			DUAL_ROLE_PROP_MODE_DFP :
			(info->pr == DUAL_ROLE_PROP_PR_SNK ?
				DUAL_ROLE_PROP_MODE_UFP :
				DUAL_ROLE_PROP_MODE_NONE);
		break;
	case DUAL_ROLE_PROP_PR:
		*val = info->pr;
		break;
	case DUAL_ROLE_PROP_DR:
		*val = info->dr;
		break;
	case DUAL_ROLE_PROP_VCONN_SUPPLY:
		*val = extcon_cros_ec_has_vconn(info);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static int extcon_drp_is_writeable(struct dual_role_phy_instance *inst,
				enum dual_role_property prop)
{
	struct cros_ec_extcon_info *info = dual_role_get_drvdata(inst);

	if (info)
		return info->writeable & (1 << prop);
	else /* Not initialized yet */
		return (prop == DUAL_ROLE_PROP_PR) ||
		       (prop == DUAL_ROLE_PROP_DR);
}

static int extcon_drp_set_prop(struct dual_role_phy_instance *inst,
				enum dual_role_property prop,
				const unsigned int *val)
{
	struct cros_ec_extcon_info *info = dual_role_get_drvdata(inst);
	int ret = 0;

	if (!info)
		return -EINVAL;

	switch (prop) {
	case DUAL_ROLE_PROP_PR:
		ret = extcon_cros_ec_force_power_role(info, *val);
		break;
	case DUAL_ROLE_PROP_DR:
		ret = extcon_cros_ec_force_data_role(info, *val);
		break;
	case DUAL_ROLE_PROP_MODE:
	case DUAL_ROLE_PROP_VCONN_SUPPLY:
	default:
		ret = -EINVAL;
	}

	return ret;
}

static enum dual_role_property extcon_drp_properties[] = {
	DUAL_ROLE_PROP_MODE,
	DUAL_ROLE_PROP_PR,
	DUAL_ROLE_PROP_DR,
	DUAL_ROLE_PROP_VCONN_SUPPLY,
};

static struct dual_role_phy_desc extcon_drp_desc = {
		.name = "otg_default",
		.supported_modes = DUAL_ROLE_SUPPORTED_MODES_DFP_AND_UFP,
		.properties = extcon_drp_properties,
		.num_properties = ARRAY_SIZE(extcon_drp_properties),
		.get_property = extcon_drp_get_prop,
		.set_property = extcon_drp_set_prop,
		.property_is_writeable = extcon_drp_is_writeable,
};

static int extcon_cros_ec_probe(struct platform_device *pdev)
{
	struct cros_ec_extcon_info *info;
	struct cros_ec_device *ec = dev_get_drvdata(pdev->dev.parent);
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	int numports, ret;

	info = devm_kzalloc(dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	info->dev = dev;
	info->ec = ec;

	if (np) {
		u32 port;

		ret = of_property_read_u32(np, "google,usb-port-id", &port);
		if (ret < 0) {
			dev_err(dev, "Missing google,usb-port-id property\n");
			return ret;
		}
		info->port_id = port;
	} else {
		info->port_id = pdev->id;
	}

	numports = cros_ec_pd_get_num_ports(info);
	if (numports < 0) {
		dev_err(dev, "failed getting number of ports! ret = %d\n",
			numports);
		return numports;
	}

	if (info->port_id >= numports) {
		dev_err(dev, "This system only supports %d ports\n", numports);
		return -ENODEV;
	}

	info->edev = devm_extcon_dev_allocate(dev, usb_type_c_cable);
	if (IS_ERR(info->edev)) {
		dev_err(dev, "failed to allocate extcon device\n");
		return -ENOMEM;
	}

	ret = devm_extcon_dev_register(dev, info->edev);
	if (ret < 0) {
		dev_err(dev, "failed to register extcon device\n");
		return ret;
	}

	extcon_set_property_capability(info->edev, EXTCON_USB,
				       EXTCON_PROP_USB_VBUS);
	extcon_set_property_capability(info->edev, EXTCON_USB_HOST,
				       EXTCON_PROP_USB_VBUS);
	extcon_set_property_capability(info->edev, EXTCON_USB,
				       EXTCON_PROP_USB_TYPEC_POLARITY);
	extcon_set_property_capability(info->edev, EXTCON_USB_HOST,
				       EXTCON_PROP_USB_TYPEC_POLARITY);
	extcon_set_property_capability(info->edev, EXTCON_DISP_DP,
				       EXTCON_PROP_USB_TYPEC_POLARITY);
	extcon_set_property_capability(info->edev, EXTCON_USB,
				       EXTCON_PROP_USB_SS);
	extcon_set_property_capability(info->edev, EXTCON_USB_HOST,
				       EXTCON_PROP_USB_SS);
	extcon_set_property_capability(info->edev, EXTCON_DISP_DP,
				       EXTCON_PROP_USB_SS);

	info->dr = DUAL_ROLE_PROP_DR_NONE;
	info->pr = DUAL_ROLE_PROP_PR_NONE;
	init_waitqueue_head(&info->role_wait);

	platform_set_drvdata(pdev, info);

	if (IS_ENABLED(CONFIG_DUAL_ROLE_USB_INTF)) {
		struct dual_role_phy_instance *inst;

		inst = devm_dual_role_instance_register(dev, &extcon_drp_desc);
		inst->drv_data = info;
		info->drp_inst = inst;
	}

	/* Get PD events from the EC */
	info->notifier.notifier_call = extcon_cros_ec_event;
	ret = blocking_notifier_chain_register(&info->ec->event_notifier,
					       &info->notifier);
	if (ret < 0) {
		dev_err(dev, "failed to register notifier\n");
		return ret;
	}

	/* Perform initial detection */
	ret = extcon_cros_ec_detect_cable(info, true);
	if (ret < 0) {
		dev_err(dev, "failed to detect initial cable state\n");
		goto unregister_notifier;
	}

	return 0;

unregister_notifier:
	blocking_notifier_chain_unregister(&info->ec->event_notifier,
					   &info->notifier);
	return ret;
}

static int extcon_cros_ec_remove(struct platform_device *pdev)
{
	struct cros_ec_extcon_info *info = platform_get_drvdata(pdev);
	struct device *dev = &pdev->dev;

	if (IS_ENABLED(CONFIG_DUAL_ROLE_USB_INTF))
		devm_dual_role_instance_unregister(dev, info->drp_inst);

	blocking_notifier_chain_unregister(&info->ec->event_notifier,
					   &info->notifier);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id extcon_cros_ec_of_match[] = {
	{ .compatible = "google,extcon-cros-ec" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, extcon_cros_ec_of_match);
#endif /* CONFIG_OF */

static struct platform_driver extcon_cros_ec_driver = {
	.driver = {
		.name  = "extcon-cros-ec",
		.of_match_table = of_match_ptr(extcon_cros_ec_of_match),
	},
	.remove  = extcon_cros_ec_remove,
	.probe   = extcon_cros_ec_probe,
};

module_platform_driver(extcon_cros_ec_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ChromeOS Embedded Controller extcon driver");
