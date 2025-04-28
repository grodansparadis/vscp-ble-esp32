
/*!
  @file vscp-ble.h

  @note This file is part of the VSCP (https://www.vscp.org)

  @license  The MIT License (MIT)

  @copyright  Copyright (C) 2000-2025 Ake Hedman, the VSCP project
  <info@vscp.org>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vscp.h>
#include "vscp-ble.h"

/*!
  Frame counter
  -------------
  This is used to make sure that the same event is not sent
  multiple times. The frame counter is incremented for each
  event sent. The frame counter is a 3-bit value, so it
  will roll over after 8 events. It is located in the low
  three bits of the head byte.
 */
static uint8_t s_rolling_index = 0;

///////////////////////////////////////////////////////////////////////////////
// vscp_ble_ev_to_frame
//

int
vscp_ble_ev_to_frame(vscp_ble_ctx_t *ctx, uint8_t *pbuf, uint8_t bufsize, vscpEvent *pev)
{
  // Check pointers
  if ((NULL == ctx) || (NULL == pbuf) || (NULL == pev)) {
    return -1; // Invalid pointer
  }

  // Check if the buffer is large enough to hold the event
  if (bufsize < (VSCP_BLE_FRAME_MIN_SIZE + VSCP_BLE_FRAME_MAX_DATA_SIZE)) {
    return -1; // Buffer too small
  }

  // Manufacturer code (little endian)
  pbuf[VSCP_BLE_FRAME_POS_MANUFACTURER]     = ctx->m_manufacturer & 0xff;
  pbuf[VSCP_BLE_FRAME_POS_MANUFACTURER + 1] = (ctx->m_manufacturer >> 8) & 0xff;

  // Flags (frame type = 0, no encryption, no authentication)
  pbuf[VSCP_BLE_FRAME_POS_FLAGS] = 0;

  // Node ID (big endian)
  pbuf[VSCP_BLE_FRAME_POS_NODEID]     = ((pev->GUID[14] >> 8) & 0xff);
  pbuf[VSCP_BLE_FRAME_POS_NODEID + 1] = (pev->GUID[15] & 0xff);

  // Head (bit 4 (hard coded) is always set to one)
  pbuf[VSCP_BLE_FRAME_POS_HEAD] = ((pev->head & 0x0f) | 0x10) + (s_rolling_index & 0x07);

  // Increment the frame counter
  ctx->m_rolling_index++;

  // VSCP Class (big endian)
  pbuf[VSCP_BLE_FRAME_POS_CLASS]     = (pev->vscp_class >> 8) & 0xff;
  pbuf[VSCP_BLE_FRAME_POS_CLASS + 1] = (pev->vscp_class & 0xff);

  // VSCP Type (big endian)
  pbuf[VSCP_BLE_FRAME_POS_TYPE]     = (pev->vscp_type >> 8) & 0xff;
  pbuf[VSCP_BLE_FRAME_POS_TYPE + 1] = (pev->vscp_type & 0xff);

  // Size of data
  pbuf[VSCP_BLE_FRAME_POS_SIZE_DATA] = (pev->sizeData & 0xff);

  // Data (up to VSCP_BLE_FRAME_MAX_DATA_SIZE bytes)
  memcpy(pbuf + VSCP_BLE_FRAME_POS_DATA,
         pev->pdata,
         (pev->sizeData <= VSCP_BLE_FRAME_MAX_DATA_SIZE) ? pev->sizeData : VSCP_BLE_FRAME_MAX_DATA_SIZE);

  // Return the size of the buffer content
  return sizeof(vscpEvent);
}

///////////////////////////////////////////////////////////////////////////////
// vscp_ble_ex_to_frame
//

int
vscp_ble_ex_to_frame(vscp_ble_ctx_t *ctx, uint8_t *pbuf, uint8_t bufsize, vscpEventEx *pex, uint16_t mancode)
{
  // Check pointers
  if ((NULL == ctx) || (NULL == pbuf) || (NULL == pex)) {
    return -1; // Invalid pointer
  }

  // Check if the buffer is large enough to hold the event
  if (bufsize < (VSCP_BLE_FRAME_MIN_SIZE + pex->sizeData)) {
    return -1; // Buffer too small
  }

  // Return the size of the event
  return sizeof(vscpEvent);
}

///////////////////////////////////////////////////////////////////////////////
// vscp_ble_frame_to_ev
//

int
vscp_ble_frame_to_ev(vscp_ble_ctx_t *ctx, vscpEvent *pev, uint8_t *pbuf, uint8_t bufsize)
{
  // Check pointers
  if ((NULL == ctx) || (NULL == pbuf) || (NULL == pev)) {
    return -1; // Invalid pointer
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_ble_frame_to_ex
//

int
vscp_ble_frame_to_ex(vscp_ble_ctx_t *ctx, vscpEventEx *pex, uint8_t *pbuf, uint8_t bufsize)
{
  // Check pointers
  if ((NULL == ctx) || (NULL == pbuf) || (NULL == pex)) {
    return -1; // Invalid pointer
  }

  return VSCP_ERROR_SUCCESS;
}
