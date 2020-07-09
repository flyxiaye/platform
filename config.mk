# Screen support config, 0 : RGB,  1 : MIPI
CONFIG_SCREEN_SUPPORT      = 1

# File System config, 0 : nor flash,  1 : nand flash
CONFIG_FLASH_TYPE          = 0

# WIFI support config 0 : no wifi,  1 : RTL8188,  2 : RTL8189,  3 : ATBM603X 
CONFIG_WIFI_DEVICE         = 0

# Other config
CONFIG_UTILS_SUPPORT       = n

export CONFIG_SCREEN_SUPPORT CONFIG_FLASH_TYPE CONFIG_WIFI_DEVICE CONFIG_UTILS_SUPPORT
