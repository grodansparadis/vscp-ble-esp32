/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/* Includes */
/* STD APIs */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* ESP APIs */
#include "esp_log.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

/* FreeRTOS APIs */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/* NimBLE stack APIs */
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#include "esp_system.h"

// BLE includes
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"
#include "esp_random.h"
#include "ble-example.h"

#include <bh1750.h>

TaskHandle_t numGenHandler = NULL;

static const char *TAG = "VSCP-BLE";

static int
ble_gap_event(struct ble_gap_event *event, void *arg);
static uint8_t own_addr_type;

// Private variables
static uint8_t own_addr_type;
static uint8_t addr_val[6] = { 0 };
static uint8_t esp_uri[]   = {
  BLE_GAP_URI_PREFIX_HTTPS, '/', '/', 'e', 's', 'p', 'r', 'e', 's', 's', 'i', 'f', '.', 'c', 'o', 'm'
};

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// start_advertising
//

static void
start_advertising(void)
{
  /* Local variables */
  int rc = 0;
  const char *name;
  struct ble_hs_adv_fields adv_fields  = { 0 };
  struct ble_hs_adv_fields rsp_fields  = { 0 };
  struct ble_gap_adv_params adv_params = { 0 };

  // Set advertising flags
  adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

  // Set device name
  name                        = ble_svc_gap_device_name();
  adv_fields.name             = (uint8_t *) name;
  adv_fields.name_len         = strlen(name);
  adv_fields.name_is_complete = 1;

  // Set device tx power
  adv_fields.tx_pwr_lvl            = BLE_HS_ADV_TX_PWR_LVL_AUTO;
  adv_fields.tx_pwr_lvl_is_present = 1;

  // Set device appearance
  adv_fields.appearance            = BLE_GAP_APPEARANCE_GENERIC_TAG;
  adv_fields.appearance_is_present = 1;

  // Set device LE role
  adv_fields.le_role            = BLE_GAP_LE_ROLE_PERIPHERAL;
  adv_fields.le_role_is_present = 1;

  // Set advertisement fields
  rc = ble_gap_adv_set_fields(&adv_fields);
  if (rc != 0) {
    ESP_LOGE(TAG, "failed to set advertising data, error code: %d", rc);
    return;
  }

  // Set device address
  rsp_fields.device_addr            = addr_val;
  rsp_fields.device_addr_type       = own_addr_type;
  rsp_fields.device_addr_is_present = 1;

  // Set URI
  rsp_fields.uri     = esp_uri;
  rsp_fields.uri_len = sizeof(esp_uri);

  // Set scan response fields
  rc = ble_gap_adv_rsp_set_fields(&rsp_fields);
  if (rc != 0) {
    ESP_LOGE(TAG, "failed to set scan response data, error code: %d", rc);
    return;
  }

  // Set non-connectable and general discoverable mode to be a beacon
  adv_params.conn_mode = BLE_GAP_CONN_MODE_NON;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

  // Start advertising
  rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, NULL, NULL);
  if (rc != 0) {
    ESP_LOGE(TAG, "failed to start advertising, error code: %d", rc);
    return;
  }
  ESP_LOGI(TAG, "advertising started!");
}

///////////////////////////////////////////////////////////////////////////////
// gap_init
//

static int
gap_init(void)
{
  /* Local variables */
  int rc = 0;

  /* Initialize GAP service */
  ble_svc_gap_init();

  /* Set GAP device name */
  rc = ble_svc_gap_device_name_set("VSCP");
  if (rc != 0) {
    ESP_LOGE(TAG, "failed to set device name to %s, error code: %d", "VSCP", rc);
    return rc;
  }

  /* Set GAP device appearance */
  rc = ble_svc_gap_device_appearance_set(BLE_GAP_APPEARANCE_GENERIC_TAG);
  if (rc != 0) {
    ESP_LOGE(TAG, "failed to set device appearance, error code: %d", rc);
    return rc;
  }
  return rc;
}

///////////////////////////////////////////////////////////////////////////////
// update_advertising_data
//

static void
update_advertising_data(void)
{
  static uint32_t counter              = 0;
  char name_data[12]                   = { 0 };
  uint8_t binary_data[22]              = { 0 };
  struct ble_hs_adv_fields adv_fields  = { 0 };
  struct ble_hs_adv_fields rsp_fields  = { 0 };
  struct ble_gap_adv_params adv_params = { 0 };

  // float temperature = read_temperature();
  // printf("Temperature: %.2f°C\n", temperature);

  /* Advertise two flags:
   *     o Discoverability in forthcoming advertisement (general)
   *     o BLE-only (BR/EDR unsupported).
   */
  // adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
  // ESP_LOGI(TAG, "Advertising data set");

  counter++;
  // if (runner % 2 == 0) {

  // snprintf(name_data, sizeof(name_data), "Count: %d", counter++);
  sprintf(name_data, "VSCP");
  adv_fields.name             = (uint8_t *) name_data;
  adv_fields.name_len         = 4; // strlen(name_data); 1 - 10
  adv_fields.name_is_complete = 1;
  //}
  // else {

  // Example binary data
  binary_data[0] = 0xFF; // Company identifier (lower byte)
  binary_data[1] = 0xFF; // Company identifier (upper byte)
  binary_data[2] = counter & 0xff;
  binary_data[3] = (counter >> 8) & 0xff;
  binary_data[4] = (counter >> 16) & 0xff;
  binary_data[5] = (counter >> 24) & 0xff;

  adv_fields.mfg_data     = binary_data;
  adv_fields.mfg_data_len = 6;
  // sizeof(binary_data);
  // }

  int rc = ble_gap_adv_set_fields(&adv_fields);
  if (rc != 0) {
    ESP_LOGE(TAG, "Error setting advertisement data; rc=%d", rc);
  }

  sprintf(name_data, "VSCP");
  rsp_fields.name             = (uint8_t *) name_data;
  rsp_fields.name_len         = 4;
  rsp_fields.name_is_complete = 1;

  // Set device address
  rsp_fields.device_addr            = addr_val;
  rsp_fields.device_addr_type       = own_addr_type;
  rsp_fields.device_addr_is_present = 1;

  // Set URI
  // rsp_fields.uri     = esp_uri;
  // rsp_fields.uri_len = sizeof(esp_uri);

  /* Set scan response fields */
  rc = ble_gap_adv_rsp_set_fields(&rsp_fields);
  if (rc != 0) {
    ESP_LOGE(TAG, "failed to set scan response data, error code: %d", rc);
    return;
  }
}

///////////////////////////////////////////////////////////////////////////////
// ble_store_config_init
//

static void
ble_store_config_init(void)
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
// print_conn_desc
//
// Logs information about a connection to the console.
//

static void
print_conn_desc(struct ble_gap_conn_desc *desc)
{
  ESP_LOGI(TAG,
           "handle=%d our_ota_addr_type=%d our_ota_addr=" MACSTR "",
           desc->conn_handle,
           desc->our_ota_addr.type,
           MAC2STR(desc->our_ota_addr.val));

  ESP_LOGI(TAG, " our_id_addr_type=%d our_id_addr=" MACSTR "", desc->our_id_addr.type, MAC2STR(desc->our_id_addr.val));

  ESP_LOGI(TAG,
           " peer_ota_addr_type=%d peer_ota_addr=" MACSTR "",
           desc->peer_ota_addr.type,
           MAC2STR(desc->peer_ota_addr.val));

  ESP_LOGI(TAG,
           " peer_id_addr_type=%d peer_id_addr=" MACSTR "",
           desc->peer_id_addr.type,
           MAC2STR(desc->peer_id_addr.val));

  ESP_LOGI(TAG,
           " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
           "encrypted=%d authenticated=%d bonded=%d\n",
           desc->conn_itvl,
           desc->conn_latency,
           desc->supervision_timeout,
           desc->sec_state.encrypted,
           desc->sec_state.authenticated,
           desc->sec_state.bonded);
}

///////////////////////////////////////////////////////////////////////////////
// std_advertise
//
// Enables advertising with the following parameters:
//     o General discoverable mode.
//     o Undirected connectable mode.
//

static void
std_advertise(void)
{
  struct ble_gap_adv_params adv_params;
  int rc;

  // Set advertisement data
  update_advertising_data();

  // Begin advertising.
  memset(&adv_params, 0, sizeof adv_params);

  adv_params.conn_mode = BLE_GAP_CONN_MODE_NON; // Non connectable
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // General discoverable
  // Set sensible defaults if the following is not set
  adv_params.itvl_min = BLE_GAP_ADV_ITVL_MS(20);
  adv_params.itvl_max = BLE_GAP_ADV_ITVL_MS(20);
  rc = ble_gap_adv_start(own_addr_type, NULL, /*200 */ BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
  if (rc != 0) {
    ESP_LOGE(TAG, "error enabling advertisement; rc=%d\n", rc);
    return;
  }
}

///////////////////////////////////////////////////////////////////////////////
// ble_gap_event
//

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * bleprph uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param ctxt                  Various information pertaining to the event.
 * @param arg                   Application-specified argument; unused by
 *                                  bleprph.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */

static int
ble_gap_event(struct ble_gap_event *event, void *arg)
{
  struct ble_gap_conn_desc desc;
  int rc;

  switch (event->type) {
    case BLE_GAP_EVENT_LINK_ESTAB:
      // A new connection was established or a connection attempt failed.
      ESP_LOGI(TAG,
               "connection %s; status=%d ",
               event->connect.status == 0 ? "established" : "failed",
               event->connect.status);
      if (event->connect.status == 0) {
        rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
        assert(rc == 0);
        print_conn_desc(&desc);
      }
      ESP_LOGI(TAG, "\n");

      if (event->connect.status != 0) {
        // Connection failed; resume advertising.
        std_advertise();
      }

      return 0;

    case BLE_GAP_EVENT_DISCONNECT:
      ESP_LOGI(TAG, "disconnect; reason=%d ", event->disconnect.reason);
      print_conn_desc(&event->disconnect.conn);
      ESP_LOGI(TAG, "\n");

      // Connection terminated; resume advertising.
      std_advertise();

      return 0;

    case BLE_GAP_EVENT_CONN_UPDATE:
      // The central has updated the connection parameters.
      ESP_LOGI(TAG, "connection updated; status=%d ", event->conn_update.status);
      rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
      assert(rc == 0);
      print_conn_desc(&desc);
      ESP_LOGI(TAG, "\n");
      return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
      ESP_LOGI(TAG, "advertise complete; reason=%d", event->adv_complete.reason);
      std_advertise();

      return 0;

    case BLE_GAP_EVENT_ENC_CHANGE:
      // Encryption has been enabled or disabled for this connection.
      ESP_LOGI(TAG, "encryption change event; status=%d ", event->enc_change.status);
      rc = ble_gap_conn_find(event->enc_change.conn_handle, &desc);
      assert(rc == 0);
      print_conn_desc(&desc);
      ESP_LOGI(TAG, "\n");
      return 0;

    case BLE_GAP_EVENT_NOTIFY_TX:
      ESP_LOGI(TAG,
               "notify_tx event; conn_handle=%d attr_handle=%d "
               "status=%d is_indication=%d",
               event->notify_tx.conn_handle,
               event->notify_tx.attr_handle,
               event->notify_tx.status,
               event->notify_tx.indication);
      return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
      ESP_LOGI(TAG,
               "subscribe event; conn_handle=%d attr_handle=%d "
               "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
               event->subscribe.conn_handle,
               event->subscribe.attr_handle,
               event->subscribe.reason,
               event->subscribe.prev_notify,
               event->subscribe.cur_notify,
               event->subscribe.prev_indicate,
               event->subscribe.cur_indicate);
      return 0;

    case BLE_GAP_EVENT_MTU:
      ESP_LOGI(TAG,
               "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
               event->mtu.conn_handle,
               event->mtu.channel_id,
               event->mtu.value);
      return 0;

    case BLE_GAP_EVENT_REPEAT_PAIRING:
      /*
       * We already have a bond with the peer, but it is attempting to
       * establish a new secure link.  This app sacrifices security for
       * convenience: just throw away the old bond and accept the new link.
       */

      // Delete the old bond.
      rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
      assert(rc == 0);
      ble_store_util_delete_peer(&desc.peer_id_addr);

      /*
       * Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
       * continue with the pairing operation.
       */
      return BLE_GAP_REPEAT_PAIRING_RETRY;

    case BLE_GAP_EVENT_PASSKEY_ACTION:
      ESP_LOGI(TAG, "PASSKEY_ACTION_EVENT started");
      struct ble_sm_io pkey = { 0 };
      int key               = 0;

      if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
        pkey.action  = event->passkey.params.action;
        pkey.passkey = 123456; // This is the passkey to be entered on peer
        ESP_LOGI(TAG, "Enter passkey %" PRIu32 "on the peer side", pkey.passkey);
        rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
        ESP_LOGI(TAG, "ble_sm_inject_io result: %d", rc);
      }
      else if (event->passkey.params.action == BLE_SM_IOACT_NUMCMP) {
        ESP_LOGI(TAG, "Passkey on device's display: %" PRIu32, event->passkey.params.numcmp);
        ESP_LOGI(TAG, "Accept or reject the passkey through console in this format -> key Y or key N");
        pkey.action = event->passkey.params.action;
        if (scli_receive_key(&key)) {
          pkey.numcmp_accept = key;
        }
        else {
          pkey.numcmp_accept = 0;
          ESP_LOGE(TAG, "Timeout! Rejecting the key");
        }
        rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
        ESP_LOGI(TAG, "ble_sm_inject_io result: %d", rc);
      }
      else if (event->passkey.params.action == BLE_SM_IOACT_OOB) {
        static uint8_t tem_oob[16] = { 0 };
        pkey.action                = event->passkey.params.action;
        for (int i = 0; i < 16; i++) {
          pkey.oob[i] = tem_oob[i];
        }
        rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
        ESP_LOGI(TAG, "ble_sm_inject_io result: %d", rc);
      }
      else if (event->passkey.params.action == BLE_SM_IOACT_INPUT) {
        ESP_LOGI(TAG, "Enter the passkey through console in this format-> key 123456");
        pkey.action = event->passkey.params.action;
        if (scli_receive_key(&key)) {
          pkey.passkey = key;
        }
        else {
          pkey.passkey = 0;
          ESP_LOGE(TAG, "Timeout! Passing 0 as the key");
        }
        rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
        ESP_LOGI(TAG, "ble_sm_inject_io result: %d", rc);
      }
      return 0;

    case BLE_GAP_EVENT_AUTHORIZE:
      ESP_LOGI(TAG,
               "authorize event: conn_handle=%d attr_handle=%d is_read=%d",
               event->authorize.conn_handle,
               event->authorize.attr_handle,
               event->authorize.is_read);

      // The default behaviour for the event is to reject authorize request
      event->authorize.out_response = BLE_GAP_AUTHORIZE_REJECT;
      return 0;

#if MYNEWT_VAL(BLE_POWER_CONTROL)
    case BLE_GAP_EVENT_TRANSMIT_POWER:
      ESP_LOGI(TAG,
               "Transmit power event : status=%d conn_handle=%d reason=%d "
               "phy=%d power_level=%x power_level_flag=%d delta=%d",
               event->transmit_power.status,
               event->transmit_power.conn_handle,
               event->transmit_power.reason,
               event->transmit_power.phy,
               event->transmit_power.transmit_power_level,
               event->transmit_power.transmit_power_level_flag,
               event->transmit_power.delta);
      return 0;

    case BLE_GAP_EVENT_PATHLOSS_THRESHOLD:
      ESP_LOGI(TAG,
               "Pathloss threshold event : conn_handle=%d current path loss=%d "
               "zone_entered =%d",
               event->pathloss_threshold.conn_handle,
               event->pathloss_threshold.current_path_loss,
               event->pathloss_threshold.zone_entered);
      return 0;
#endif
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// handle_on_reset
//

static void
handle_on_reset(int reason)
{
  ESP_LOGE(TAG, "Resetting state; reason=%d\n", reason);
}

///////////////////////////////////////////////////////////////////////////////
// handle_on_sync
//

static void
handle_on_sync(void)
{
  int rc;

  ESP_LOGI(TAG, "NimBLE Host Sync");

  // Make sure we have proper identity address set (public preferred)
  rc = ble_hs_util_ensure_addr(0);
  assert(rc == 0);

  // Figure out address to use while advertising (no privacy for now)
  rc = ble_hs_id_infer_auto(0, &own_addr_type);
  if (rc != 0) {
    ESP_LOGE(TAG, "error determining address type; rc=%d\n", rc);
    return;
  }

  // Printing ADDR
  uint8_t addr_val[6] = { 0 };
  rc                  = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

  ESP_LOGI(TAG, "Device Address: " MACSTR "", MAC2STR(addr_val));

  // Begin advertising.
  std_advertise();
}

///////////////////////////////////////////////////////////////////////////////
// main_host_task
//

void
main_host_task(void *param)
{
  ESP_LOGI(TAG, "BLE Host Task Started");

  /*
    This function will return only when nimble_port_stop()
    is executed
  */
  nimble_port_run();

  nimble_port_freertos_deinit();
}

///////////////////////////////////////////////////////////////////////////////
// eventGenerator
//

void
eventGenerator(void *params)
{
  // Prevent advertising updates for a while to allow
  // the system to be initialized
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  while (true) {
    uint32_t rNum = esp_random();
    update_advertising_data();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

///////////////////////////////////////////////////////////////////////////////
// ble_host_config_init
//

static void
ble_host_config_init(void)
{
  /* Set host callbacks */
  ble_hs_cfg.reset_cb          = handle_on_reset;
  ble_hs_cfg.sync_cb           = handle_on_sync;
  ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
  ble_hs_cfg.store_status_cb   = ble_store_util_status_rr;

  ble_hs_cfg.sm_io_cap = CONFIG_EXAMPLE_IO_TYPE;

  ble_hs_cfg.sm_sc = 1;

  /* Stores the IRK */
  ble_hs_cfg.sm_our_key_dist |= BLE_SM_PAIR_KEY_DIST_ID;
  ble_hs_cfg.sm_their_key_dist |= BLE_SM_PAIR_KEY_DIST_ID;

  /* Initialize the BLE store configuration */
  ble_store_config_init();
}

///////////////////////////////////////////////////////////////////////////////
// ble_host_task
//

static void
ble_host_task(void *param)
{
  /* Task entry log */
  ESP_LOGI(TAG, "nimble host task has been started!");

  /* This function won't return until nimble_port_stop() is executed */
  nimble_port_run();

  /* Clean up at exit */
  vTaskDelete(NULL);
}

///////////////////////////////////////////////////////////////////////////////
// app_main
//

void
app_main(void)
{
  int rc;

  // Initialize NVS — it is used to store PHY calibration data
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ret = nimble_port_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init nimble %d ", ret);
    return;
  }

  // GAP service initialization
  // rc = gap_init();
  // if (rc != 0) {
  //   ESP_LOGE(TAG, "failed to initialize GAP service, error code: %d", rc);
  //   return;
  // }

  // NimBLE host configuration initialization
  ble_host_config_init();

  ble_hs_cfg.sm_io_cap = CONFIG_EXAMPLE_IO_TYPE;
  ble_hs_cfg.sm_sc     = 0;

  rc = gatt_svr_init();
  assert(rc == 0);

  // Set the default device name.
  rc = ble_svc_gap_device_name_set("nimble-ble-vscp");
  assert(rc == 0);

  // XXX Need to have template for store
  ble_store_config_init();

  nimble_port_freertos_init(main_host_task);

  xTaskCreate(&eventGenerator, "main Task", 4 * 1024, NULL, 2, &numGenHandler);
}
