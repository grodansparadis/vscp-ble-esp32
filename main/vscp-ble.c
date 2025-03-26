
/*!
  @file vscp-ble.h

  @note This file is part of the VSCP (https://www.vscp.org)

  @license  The MIT License (MIT)

  @copyright  Copyright (C) 2000-2025 Ake Hedman, the VSCP project <info@vscp.org>

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

#include "vscp.h"
#include "vscp-ble.h"

///////////////////////////////////////////////////////////////////////////////
// vscp_ble_ev_to_buf
//

int
vscp_ble_ev_to_buf(uint8_t *pbuf, uint16_t bufsize, vscpEvent *pev, uint16_t mancode)
{
  // Check pointers
  if (NULL == pbuf || NULL == pev) {
    return -1; // Invalid pointer
  }

  // Check if the buffer is large enough to hold the event
  if (bufsize < (VSCP_BLE_FRAME_MIN_SIZE + pev)) {
    return -1; // Buffer too small
  }

  // Manufacturer code (little endian)
  pbuf[VSCP_BLE_FRAME_POS_MANUFACTURER]     = mancode & 0xff;
  pbuf[VSCP_BLE_FRAME_POS_MANUFACTURER + 1] = (mancode >> 8) & 0xff;

  // Flags (frame type = 0)
  pbuf[VSCP_BLE_FRAME_POS_FLAGS] = 0;

  // Node ID (big endian)
  pbuf[VSCP_BLE_FRAME_POS_NODEID]     = ((pev->GUID[14] >> 8) & 0xff);
  pbuf[VSCP_BLE_FRAME_POS_NODEID + 1] = (pev->GUID[15] & 0xff);

  // Head
  pbuf[VSCP_BLE_FRAME_POS_HEAD] = (pev->head & 0x0f) | 0x10; // Head (bit 4 is always set to one)

  // Timestamp (big endian)
  pbuf[VSCP_BLE_FRAME_POS_TIMESTAMP]     = (pev->timestamp >> 24) & 0xff;
  pbuf[VSCP_BLE_FRAME_POS_TIMESTAMP + 1] = (pev->timestamp >> 16) & 0xff;
  pbuf[VSCP_BLE_FRAME_POS_TIMESTAMP + 2] = (pev->timestamp >> 8) & 0xff;
  pbuf[VSCP_BLE_FRAME_POS_TIMESTAMP + 3] = (pev->timestamp & 0xff);

  // Class (big endian)
  pbuf[VSCP_BLE_FRAME_POS_CLASS]     = (pev->vscp_class >> 8) & 0xff;
  pbuf[VSCP_BLE_FRAME_POS_CLASS + 1] = (pev->vscp_class & 0xff);

  // Type (big endian)
  pbuf[VSCP_BLE_FRAME_POS_TYPE]     = (pev->vscp_type >> 8) & 0xff;
  pbuf[VSCP_BLE_FRAME_POS_TYPE + 1] = (pev->vscp_type & 0xff);

  // Data (up to VSCP_BLE_FRAME_MAX_DATA_SIZE bytes)
  memcpy(pbuf + VSCP_BLE_FRAME_POS_DATA,
    pev->pdata,
         (pev->sizeData <= VSCP_BLE_FRAME_MAX_DATA_SIZE) ? pev->sizeData : VSCP_BLE_FRAME_MAX_DATA_SIZE);

  // Return the size of the event
  return sizeof(vscpEvent);
}

///////////////////////////////////////////////////////////////////////////////
// vscp_ble_ex_to_buf
//

int
vscp_ble_ex_to_buf(uint8_t *pbuf, uint16_t bufsize, vscpEventEx *pex, uint16_t mancode)
{
  // Check pointers
  if (NULL == pbuf || NULL == pex) {
    return -1; // Invalid pointer
  }

  // Check if the buffer is large enough to hold the event
  if (bufsize < (VSCP_BLE_FRAME_MIN_SIZE + pex)) {
    return -1; // Buffer too small
  }

  ///////////////////////////////////////////////////////////////////////////////
  // vscp_ble_buf_to_ev
  //

  int vscp_ble_buf_to_ev(vscpEvent * pev, uint8_t *pbuf, uint16_t bufsize) {}

  ///////////////////////////////////////////////////////////////////////////////
  // vscp_ble_buf_to_ex
  //

  int vscp_ble_buf_to_ex(vscpEventEx * pex, uint8_t *pbuf, uint16_t bufsize) {}