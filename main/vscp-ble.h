
/*!
  @file vscp-ble.h
  @brief VSCP Bluetooth Low Energy (BLE) protocol definitions.
  
  This file contains the definitions and functions for handling VSCP
  events and event exchanges over Bluetooth Low Energy (BLE).
  
  @note This file is part of the VSCP project.
  @note For more information, visit https://www.vscp.org
  
  @date 2025-03-26
  @version 1.0.0
  
  The MIT License (MIT)

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

  Frame format
  ------------

  | Manufacturer | 2 bytes | Bluetooth manufacturer id, use 0xFFFF for test. Note that little endian is used here. |
  | Flags | 1 byte | 0x01 |
  | node id | 2 bytes | Node id. This is the last two bytes of the GUID. |
  | head | 1 byte | VSCP Head. Bit 4 is always set to one (hardcoded). |
  | timestamp | 4 bytes | Microsecond timestamp |
  | vscp-class | 2 bytes | VSCP class |
  | vscp-type | 2 bytes | VSCP type |
  | VSCP data | 0-17 bytes | VSCP data |
*/

// Legacy advertising
#define VSCP_BLE_FRAME_MIN_SIZE 14
#define VSCP_BLE_FRAME_MAX_SIZE 31
#define VSCP_BLE_FRAME_MAX_DATA_SIZE VSCP_BLE_FRAME_MAX_SIZE-VSCP_BLE_FRAME_MIN_SIZE

// BLE 5.x advanced advertising
#define VSCP_BLE_ADV_FRAME_MAX_SIZE 255
#define VSCP_BLE_ADV_FRAME_MAX_DATA_SIZE VSCP_BLE_ADV_FRAME_MAX_SIZE-VSCP_BLE_ADV_FRAME_MIN_SIZE

#define VSCP_BLE_FRAME_POS_MANUFACTURER 0
#define VSCP_BLE_FRAME_POS_FLAGS 2
#define VSCP_BLE_FRAME_POS_NODEID 3
#define VSCP_BLE_FRAME_POS_HEAD 5
#define VSCP_BLE_FRAME_POS_TIMESTAMP 6
#define VSCP_BLE_FRAME_POS_CLASS 10
#define VSCP_BLE_FRAME_POS_TYPE 12
#define VSCP_BLE_FRAME_POS_DATA 14

/*!
@brief Convert a VSCP event to a buffer format.
@param pbuf Pointer to the buffer where the event will be stored.
@param bufsize Size of the buffer.
@param pev Pointer to the VSCP event structure.
@return The number of bytes written to the buffer, or -1 on error.

*/
int
vscp_ble_ev_to_buf(uint8_t *pbuf, uint16_t bufsize, vscpEvent *pev);

/*!
 * @brief Convert a VSCP event ex to a buffer.
 *
 * @param pbuf Pointer to the buffer where the event exchange will be stored.
 * @param bufsize Size of the buffer.
 * @param pex Pointer to the VSCP event ex structure.
 *
 * @return The number of bytes written to the buffer, or -1 on error.
 */
int
vscp_ble_ex_to_buf(uint8_t *pbuf, uint16_t bufsize, vscpEventEx *pex);

/*!
 * @brief Convert a buffer to a VSCP event.
 *
 * @param pev Pointer to the VSCP event structure where the event will be stored.
 * @param pbuf Pointer to the buffer containing the event data.
 * @param bufsize Size of the buffer.
 *
 * @return The number of bytes read from the buffer, or -1 on error.
 */
int
vscp_ble_buf_to_ev(vscpEvent *pev, uint8_t *pbuf, uint16_t bufsize);

/*!
 * @brief Convert a buffer to a VSCP event exchange.
 *
 * @param pex Pointer to the VSCP event exchange structure where the event will be stored.
 * @param pbuf Pointer to the buffer containing the event data.
 * @param bufsize Size of the buffer.
 *
 * @return The number of bytes read from the buffer, or -1 on error.
 */
int
vscp_ble_buf_to_ex(vscpEventEx *pex, uint8_t *pbuf, uint16_t bufsize);
