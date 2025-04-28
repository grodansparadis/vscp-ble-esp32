
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

#include <vscp.h>

// Legacy advertising
#define VSCP_BLE_FRAME_MIN_SIZE      19     // Without name field
#define VSCP_BLE_FRAME_MAX_SIZE      31     // With name field of 10 character is size
#define VSCP_BLE_FRAME_MAX_DATA_SIZE 8 + 16 // advertising data + response data

#define VSCP_BLE_FRAME_POS_MANUFACTURER 0  // 2 bytes (note !!!! little endian)
#define VSCP_BLE_FRAME_POS_FLAGS        2  // 1 byte
#define VSCP_BLE_FRAME_POS_NODEID       3  // 2 bytes
#define VSCP_BLE_FRAME_POS_HEAD         5  // 1 byte
#define VSCP_BLE_FRAME_POS_CLASS        6  // 2 bytes
#define VSCP_BLE_FRAME_POS_TYPE         8  // 2 bytes
#define VSCP_BLE_FRAME_POS_SIZE_DATA    10 // 1 byte
#define VSCP_BLE_FRAME_POS_DATA         11 // Always 8 bytes (padded with zeros if needed)

/*!
  VSCP BLE context
*/
typedef struct vscp_ble_ctx {
  uint16_t m_manufacturer;     // Manufacturer code
  uint8_t m_rolling_index : 3; // Rolling index updated for each sent frame
  uint8_t m_bScanResponse : 1; // Scan response flag
  uint8_t m_bEncryption : 1;   // Set if frames should be encrypted
} vscp_ble_ctx_t;

/*!
  @brief Convert a VSCP event to a buffer format.
  @param pbuf Pointer to the buffer where the event will be stored.
  @param bufsize Size of the buffer.
  @param pev Pointer to the VSCP event structure.
  @return The number of bytes written to the buffer, or -1 on error.

  @note This function converts a VSCP event to a buffer format suitable for
  transmission over Bluetooth Low Energy (BLE). The event is formatted
  according to the VSCP BLE frame format. The function also handles the
  rolling index for the head byte to ensure that the same event is not sent
  multiple times.The function checks for valid pointers and buffer size before
  proceeding with the conversion. If the buffer is too small or
  the pointers are invalid, the function returns -1 to indicate an
  error. The function returns the size of the event in bytes
  after successful conversion.

  The manufacturer code is specified as a parameter and is
  included in the buffer in little-endian format. The head byte
  is set with bit 4 always set to one (hardcoded) and the rolling
  index is added to the low three bits of the head byte.

  The size of the data must be 0-8 bytes for a valid frame but can be
  max 24 bytes in which case a scan response packet is sent to the server. If data
  is less than or equal to eight bytes padding is done with zeros up to
  8 bytes. If the data is larger than 8 bytes the scan response packet is padded
  with zeros up to 16 bytes.


*/
int
vscp_ble_ev_to_frame(vscp_ble_ctx_t *ctx, uint8_t *pbuf, uint8_t bufsize, vscpEvent *pev);

/*!
 * @brief Convert a VSCP event ex to a buffer.
 * @param pbuf Pointer to the buffer where the event exchange will be stored.
 * @param bufsize Size of the buffer.
 * @param pex Pointer to the VSCP event ex structure.
 * @return The number of bytes written to the buffer, or -1 on error.
 *
 * @note This function is not implemented yet.
 */
int
vscp_ble_ex_to_frame(vscp_ble_ctx_t *ctx, uint8_t *pbuf, uint8_t bufsize, vscpEventEx *pex, uint16_t mancode);

/*!
 * @brief Convert a buffer to a VSCP event.
 * @param pev Pointer to the VSCP event structure where the event will be stored.
 * @param pbuf Pointer to the buffer containing the event data.
 * @param bufsize Size of the buffer.
 * @return The number of bytes read from the buffer, or -1 on error.
 */
int
vscp_ble_frame_to_ev(vscp_ble_ctx_t *ctx, vscpEvent *pev, uint8_t *pbuf, uint8_t bufsize);

/*!
 * @brief Convert a buffer to a VSCP event exchange.
 *
 * @param pex Pointer to the VSCP event exchange structure where the event will be stored.
 * @param pbuf Pointer to the buffer containing the event data.
 * @param bufsize Size of the buffer.
 * @return The number of bytes read from the buffer, or -1 on error.
 */
int
vscp_ble_frame_to_ex(vscp_ble_ctx_t *ctx, vscpEventEx *pex, uint8_t *pbuf, uint8_t bufsize);

// ----------------------------------------------------------------------------
//                              CALLBACKS
// ----------------------------------------------------------------------------

/*!
  @brief Callback function to fetch encryption key
  @param pkey Pointer to the buffer where the encryption key will be stored.
  @note This function is called when the encryption key is needed for
  encryption/decryption of the VSCP event data. The function should fill
  the buffer with the appropriate encryption key. The size of the key
  is expected to be 16 bytes (128 bits) for AES encryption.
*/

void
vscp_ble_cb_fetch_encryption_key(uint8_t *pkey);