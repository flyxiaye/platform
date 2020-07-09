#ifndef _AK_DRV_H_
#define _AK_DRV_H_

#define TS_POINT_MAX_NUM	10

enum key_stat{
    PRESS = 1,
    RELEASE,
};

struct key_event{
    int code;
    enum key_stat stat;
};

struct ak_ts_info{
    int x;
    int y;
};

struct ak_ts_event{
    int map;
    struct ak_ts_info info[TS_POINT_MAX_NUM];
};

/**
 * ak_drv_get_version - get drv module's version string
 * return: pointer to drv module's version string
 * note:
 */
const char* ak_drv_get_version(void);


/**
 * ak_drv_key_open: open key driver
 * return: 0 - success; otherwise error code;
 */
int ak_drv_key_open(void);

/**
 * ak_drv_key_get_event: get key event .
 * @key[OUT]:  key event
 * @ms[IN]:  time out , <0  block; = 0  unblock; > 0 wait time.
 * return:0 - success; otherwise error code;
 */
int ak_drv_key_get_event(struct key_event *key,long ms);

/**
 * ak_drv_key_close: close key handle .
 * return: 0 - success; otherwise error code;
 */
int ak_drv_key_close(void);


/**
 * ak_drv_ts_open: open touch screen driver
 * return: 0 - success; otherwise error code;
 */
int ak_drv_ts_open(void);

/**
 * ak_drv_ts_get_event: get touch screen event .
 * @ts[OUT]:  ts event
 * @ms[IN]:  time out , <0  block; = 0  unblock; > 0 wait time.
 * return: 0 - success; otherwise error code;
 */
int ak_drv_ts_get_event(struct ak_ts_event *ts, long ms);

/**
 * ak_drv_ts_close: close touch screen .
 * return: 0 - success; otherwise error code;
 */
int ak_drv_ts_close(void);


/* 
 * ak_drv_pwm_open - open pwm device
 * @device_no[IN]: pwm device minor-number,[0-4]
 * return: 0 - success; otherwise error code;
 */
int ak_drv_pwm_open(int device_no);

/* 
 * ak_drv_pwm_set - set pwd working param
 * @device_no[IN]: pwm device minor-number,[0-4]
 * @duty_ns[IN]: pwm duty time in ns.
 * @period_ns[IN]: pwm period time in ns.
 * return: 0 on success, otherwise error code.
 */
int ak_drv_pwm_set(int device_no, int duty_ns, int period_ns);

/* 
 * ak_drv_pwm_close - close pwm
 * @device_no[IN]: pwm device minor-number,[0-4]
 * return: 0 on success, otherwise error code.
 */
int ak_drv_pwm_close(int device_no);

/**
 * ak_drv_wdt_open - open watch dog and watch dog start work.
 * @feed_timeout: [in] second, [1, 357]
 * return: 0 - success; otherwise error code;
 * note:
 */
int ak_drv_wdt_open(unsigned int feed_timeout);

/**
 * ak_drv_wdt_feed - feed watch dog.
 * return:0 - success; otherwise error code;
 */
int ak_drv_wdt_feed(void);

/**
 * ak_drv_wdt_close - close watch dog.
 * return:0 - success; otherwise error code;
 */
int ak_drv_wdt_close(void);

/**
 * ak_drv_ir_init - init ircut, get ircut control mode, set gpio
 * return: 0 - success; otherwise error code;
 */
int ak_drv_ir_init(void);

/**
 * ak_drv_ir_set_ircut - set ircut to switch
 * @status_level[IN]:  status level to control day or night, [0,1]
 * return: 0 success, otherwise error code
 */
int ak_drv_ir_set_ircut(int status_level);

#endif

/* end of file */
