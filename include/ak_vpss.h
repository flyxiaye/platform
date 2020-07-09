#ifndef __AK_VPSS_H__
#define __AK_VPSS_H__

/********************** effect *******************************/
enum vpss_effect_type {
	/* HUE to SHARP, value: [-50, 50], 0 means use the value in ISP config file */
	VPSS_EFFECT_HUE = 0x00,
	VPSS_EFFECT_BRIGHTNESS,
	VPSS_EFFECT_SATURATION,
	VPSS_EFFECT_CONTRAST,
	VPSS_EFFECT_SHARP,
	
	VPSS_STYLE_ID,		//[0, 2]
	VPSS_POWER_HZ		//50 or 60
};

enum vpss_gain_stat {
	VPSS_GAIN_NO_CHANGE_STAT = 0,
	VPSS_GAIN_LOW_STAT,
	VPSS_GAIN_HIGH_STAT,
	VPSS_GAIN_MID_STAT,
};


/********************** public *******************************/

/**
 * ak_vpss_get_version - get vpss version
 * return: version string
 */
const char *ak_vpss_get_version(void);

/**
 * brief: get fps ctrl stat
 * @dev[IN]: device id
 * @stat[OUT]:gain stat
 * @need_fps[OUT]:need fps
 * return: 0 success, otherwise error code
 * notes:for switch day high light and low light. use with ak_vpss_change_sensor_fps
 */
int ak_vpss_get_fps_ctrl_stat(int dev, enum vpss_gain_stat *stat, int *need_fps);

/**
 * brief: change sensor fps
 * @dev[IN]: device id
 * @need_fps[IN]:need fps out by ak_vpss_get_fps_ctrl_stat
 * return:  0 success, otherwise error code
 * notes:for switch day high light and low light. use with ak_vpss_get_fps_ctrl_stat
 */
int ak_vpss_change_sensor_fps(int dev, int need_fps);

#endif
