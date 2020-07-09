#ifndef _AK_AUDIO_TOOL_H_
#define _AK_AUDIO_TOOL_H_

/**
 * ak_audio_tool_server_start - audio tool server start
 * @port[IN]: server port id
 * return: 0 success, -1 failed
 * notes:
 */
int ak_audio_tool_server_start(unsigned int port);

/**
 * ak_audio_tool_server_stop - audio tool server stop
 * return: 0 success, -1 failed
 * notes:
 */
void ak_audio_tool_server_stop(void);


#endif
