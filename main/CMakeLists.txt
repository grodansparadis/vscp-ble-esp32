set(srcs "main.c"    
         "gatt_svr.c"   
         "vscp-ble.c")

idf_component_register(SRCS "crypto.c" "${srcs}"
                       INCLUDE_DIRS "." "../third-party/vscp-firmware/common")
