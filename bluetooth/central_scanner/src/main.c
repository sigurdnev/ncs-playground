/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/** @file
 *  @brief Central/Observer scanner sample
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <inttypes.h>
#include <errno.h>
#include <zephyr.h>
#include <sys/printk.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/gatt_dm.h>
#include <bluetooth/scan.h>
#include <bluetooth/services/bas_client.h>
#include <dk_buttons_and_leds.h>

#include <settings/settings.h>


#define MAX_DEVICES_SCAN_LIST 50
#define NAME_LEN 30
//static struct bt_scan_device_info device_info_list[MAX_DEVICES_SCAN_LIST];

static void scan_filter_match(struct bt_scan_device_info *device_info,
			      struct bt_scan_filter_match *filter_match,
			      bool connectable)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(device_info->recv_info->addr, addr, sizeof(addr));

	printk("Filters matched. Address: %s connectable: %s\n",
		addr, connectable ? "yes" : "no");
}

static bool data_cb(struct bt_data *data, void *user_data)
{
	char *name = user_data;

	switch (data->type) {
	case BT_DATA_NAME_SHORTENED:
	case BT_DATA_NAME_COMPLETE:
		memcpy(name, data->data, MIN(data->data_len, NAME_LEN - 1));
		return false;
	default:
		return true;
	}
}

static const char *phy2str(uint8_t phy)
{
	switch (phy) {
	case 0: return "No packets";
	case BT_GAP_LE_PHY_1M: return "LE 1M";
	case BT_GAP_LE_PHY_2M: return "LE 2M";
	case BT_GAP_LE_PHY_CODED: return "LE Coded";
	default: return "Unknown";
	}
}

static void scan_filter_no_match(struct bt_scan_device_info *device_info,
				 bool connectable)
{
	int err;
	char addr[BT_ADDR_LE_STR_LEN];
	char addr_copy[BT_ADDR_LE_STR_LEN];
    static bt_addr_le_t device_info_list[MAX_DEVICES_SCAN_LIST];
    static uint32_t devices_in_list;
	const bt_addr_le_t *addr_const;
    
	/*
    char le_addr[BT_ADDR_LE_STR_LEN];
	char name[NAME_LEN];

	(void)memset(name, 0, sizeof(name));

	bt_data_parse(device_info->adv_data, data_cb, name);

	bt_addr_le_to_str(device_info->recv_info->addr, le_addr, sizeof(le_addr));

		printk("[DEVICE]: %s, AD evt type %u, Tx Pwr: %i, RSSI %i %s "
	       "C:%u S:%u D:%u SR:%u E:%u Prim: %s, Secn: %s, "
	       "Interval: 0x%04x (%u ms), SID: %u\n",
	       le_addr, device_info->recv_info->adv_type, device_info->recv_info->tx_power, device_info->recv_info->rssi, name,
	       (device_info->recv_info->adv_props & BT_GAP_ADV_PROP_CONNECTABLE) != 0,
	       (device_info->recv_info->adv_props & BT_GAP_ADV_PROP_SCANNABLE) != 0,
	       (device_info->recv_info->adv_props & BT_GAP_ADV_PROP_DIRECTED) != 0,
	       (device_info->recv_info->adv_props & BT_GAP_ADV_PROP_SCAN_RESPONSE) != 0,
	       (device_info->recv_info->adv_props & BT_GAP_ADV_PROP_EXT_ADV) != 0,
	       phy2str(device_info->recv_info->primary_phy), phy2str(device_info->recv_info->secondary_phy),
	       device_info->recv_info->interval, device_info->recv_info->interval * 5 / 4, device_info->recv_info->sid);
	 */

	for (uint32_t i = 0; i < devices_in_list; i++)
	{
		// If address is alredy in the list, return.
		if(memcmp(device_info->recv_info->addr,
		          &device_info_list[i],
				  sizeof(bt_addr_le_t)
				  ) == 0)
		{
			return;
		}
	}
	printk("\n ===Adding new device to the list===\n");
	
	memcpy(&device_info_list[devices_in_list],
	       device_info->recv_info->addr,
		   sizeof(bt_addr_le_t)
		);

	devices_in_list= devices_in_list +1;
	if(devices_in_list > 49)
	{
		devices_in_list = 49;

	}

	for (uint32_t i = 0; i < devices_in_list; i++)
	{
		addr_const = &device_info_list[i];
		bt_addr_le_to_str(addr_const, addr_copy, sizeof(addr_copy));
		printk("List entry %d is %s\n",i,addr_copy);
	}
}

BT_SCAN_CB_INIT(scan_cb, scan_filter_match, scan_filter_no_match,
		NULL, NULL);


static void scan_init(void)
{
	int err;

	struct bt_scan_init_param scan_init = {
		.connect_if_match = 0,
		.scan_param = NULL,
		.conn_param = BT_LE_CONN_PARAM_DEFAULT
	};

	bt_scan_init(&scan_init);
	bt_scan_cb_register(&scan_cb);

	err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_UUID, BT_UUID_BAS);
	if (err) {
		printk("Scanning filters cannot be set (err %d)\n", err);

		return;
	}

	err = bt_scan_filter_enable(BT_SCAN_UUID_FILTER, false);
	if (err) {
		printk("Filters cannot be turned on (err %d)\n", err);
	}
}

void thread_printer(const struct k_thread *thread, void *user_data)
{
	ARG_UNUSED(user_data);
	printk("THREAD: %s\r\n", thread->name);
}

void main(void)
{
	int err;


	printk("Starting Bluetooth Scanner example\n");

	 k_thread_foreach(thread_printer, NULL);

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	scan_init();

	err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
	if (err) {
		printk("Scanning failed to start (err %d)\n", err);
		return;
	}

	printk("Scanning successfully started\n");
}
