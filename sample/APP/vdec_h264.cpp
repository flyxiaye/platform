#include "vdec_h264.h"
#include <string.h>
extern "C"{
#include "ak_common.h"
}

#define RECORD_READ_LEN      1024*100 /* read video file buffer length */

Vdech264::Vdech264(Vo* v)
{
	vo = v;
	int ret;
	enum ak_vdec_input_type intype = AK_CODEC_H264;
	enum ak_vdec_output_type outtype = AK_YUV420SP;
	/* get the type for output */
	if (vo->type == NULL)
	{
		ak_print_error_ex(MODULE_ID_VDEC, "please input the type\n");
		/* print the hint and notice */
	}

	/* get the type for input */
	if (!strcmp(vo->type, "h264"))
	{
		/* h264 */
		ak_print_normal(MODULE_ID_VDEC, "h264 success\n");
		intype = AK_CODEC_H264;
	}
	else if (!strcmp(vo->type, "h265"))
	{
		/* h265 */
		ak_print_normal(MODULE_ID_VDEC, "h265 success\n");
		intype = AK_CODEC_H265;
	}
	else
		ak_print_normal_ex(MODULE_ID_VDEC, "unsupport video decode input type [%s] \n", vo->type);

	/* output vdec data set */
	if (intype != AK_CODEC_MJPEG)
		outtype = AK_TILE_YUV;

	/* open vdec */
	struct ak_vdec_param param;
	param.vdec_type = intype;
	param.sc_height = resolutions[vo->res].height;         //vdec height res set
	param.sc_width = resolutions[vo->res].width;          //vdec width res set
	param.output_type = outtype;
	
	/* open the vdec */
	ret = ak_vdec_open(&param, &handle_id);
	if (ret != 0)
	{
		ak_print_error_ex(MODULE_ID_VDEC, "ak_vdec_open failed!\n");

		/* destroy the layer */
	}
	ak_thread_mutex_lock(&(vo->refresh_flag_lock));
	/* refresh flag to record */
	vo->refresh_record_flag |= (1 << handle_id);
	ak_thread_mutex_unlock(&(vo->refresh_flag_lock));
}

Vdech264::~Vdech264()
{
	ak_vdec_close(handle_id);
}

void Vdech264::run()
{
	ak_thread_set_name("vdec_get_frame");
	int id = handle_id;
	int ret = -1;
	int status = 0;

	/* a loop for getting the frame and display */
	do {
		/* get frame */
		struct ak_vdec_frame frame;
		ret = ak_vdec_get_frame(id, &frame);

		if (ret == 0)
		{
			/* invoke the callback function to process the frame*/
			vo->demo_play_func(&frame);
			/* relase the frame and push back to decoder */
			ak_vdec_release_frame(id, &frame);
		}
		else
		{
			/* get frame failed , sleep 10ms before next cycle*/
			ak_sleep_ms(10);
		}

		/* check the status finished */
		ak_vdec_get_decode_finish(id, &status);
		/* true means finished */
		if (status)
		{
			//decoder_num--;  //that means current decoder is finished
			ak_thread_mutex_lock(&(vo->refresh_flag_lock));
			//clear_bit
			vo->refresh_record_flag &= ~(1 << id);
			vo->refresh_flag &= ~(1 << id);
			ak_thread_mutex_unlock(&(vo->refresh_flag_lock));
			ak_print_normal(MODULE_ID_VDEC, "id is [%d] status [%d]\n", id, status);
			return;
		}
	} while (1);
}

static void* callback(void* arg)
{
	((Vdech264*)arg)->run();
}

void Vdech264::start()
{
	BaseThread::start(callback);
}

VdecSend::VdecSend(Vo * v, int handle_id)
{
	vo = v;
	this->handle_id = handle_id;
	dbf = DataBuffer(MODULE_ID_VDEC, vdec_mem);
}


VdecSend::~VdecSend()
{
}

void VdecSend::run()
{
	ak_thread_set_name("vdec_demo_test");

	/* read video file stream and send to decode, */
	int read_len = 0;
	int total_len = 0;
	unsigned char* data = (unsigned char*)ak_mem_alloc(MODULE_ID_VDEC, RECORD_READ_LEN);
	memset(data, 0x00, RECORD_READ_LEN);
	/* loop for sending data to decode */
	do
	{
		/* read the record file stream */
		dbf.rb_read(data, RECORD_READ_LEN, &read_len);
		// vi->dbf.rb_read(data, RECORD_READ_LEN, &read_len);
		/* get the data and send to decoder */
		if (read_len > 0)
		{
			total_len += read_len;
			/* play loop */
			vo->decode_stream(handle_id, data, read_len);
			ak_sleep_ms(10);
		} else {
			ak_sleep_ms(10);
		}
	} while (1);

	/* if finish, notice the decoder for data sending finish */
	ak_vdec_end_stream(handle_id);
	if (data != NULL)
		ak_mem_free(data);
	ak_print_normal_ex(MODULE_ID_VDEC, "send stream th exit\n");
}

static void* callback2(void* arg)
{
	((VdecSend*)arg)->run();
}

void VdecSend::start()
{
	BaseThread::start(callback2);
}


void test_vdech264()
{
	ak_print_normal(MODULE_ID_VDEC, "test vdec\n");
	Vo vo;
	Vdech264 v1(&vo);
	VdecSend v2(&vo, v1.get_handle_id());
	v1.start();
	v2.start();
}
