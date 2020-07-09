#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "ak_common.h"
#include "ak_log.h"
#include "ak_drv.h"

#define LEN_HINT         512
#define LEN_OPTION_SHORT 512

static char *pc_prog_name = NULL;  
static struct ak_ts_event ts;
static int wait_time = 20;//second

char ac_option_hint[  ][ LEN_HINT ] = {                                         //操作提示数组
    "help info" ,
    "[NUM] run time in seconds, default 20s" ,
    "" ,
};

struct option option_long[ ] = {
    { "help"      , no_argument       , NULL , 'h' } ,    //"help info"
    { "time"      , required_argument , NULL , 'a' } ,    //"[NUM] run time in seconds, default 20s"
    {0, 0, 0, 0}
 };

void usage(const char * name)
{
    ak_print_normal(MODULE_ID_VO," %s -a [time]\n", name);
    ak_print_normal(MODULE_ID_VO,"eg: %s -a 10\n", name);
}


/*
 * help_hint: use the -h --help option.Print option of help information
 * return: 0
 */
static int help_hint(char *pc_prog_name)
{
    int i;

    printf("%s\n" , pc_prog_name);
    for(i = 0; i < sizeof(option_long) / sizeof(struct option); i ++) {
        if( option_long[ i ].val != 0 ) {
            printf("\t--%-16s -%c %s\n" , option_long[ i ].name , option_long[ i ].val , ac_option_hint[ i ]);
        }
    }

    usage(pc_prog_name);
    
    printf("\n\n");
    return 0;
}

/*
 * get_option_short: fill the stort option string.
 * return: option short string addr.
 */
char *get_option_short( struct option *p_option, int i_num_option, char *pc_option_short, int i_len_option )
{
    int i;
    int i_offset = 0;
    char c_option;

    for( i = 0 ; i < i_num_option ; i ++ ) {
        if( ( c_option = p_option[ i ].val ) == 0 ) {
            continue;
        }
        switch( p_option[ i ].has_arg ){
        case no_argument:
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c" , c_option );
            break;
        case required_argument:
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c:" , c_option );
            break;
        case optional_argument:
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c::" , c_option );
            break;
        }
    }
    return pc_option_short;
}

int parse_option( int argc, char **argv )
{
    int i_option;
    char ac_option_short[ LEN_OPTION_SHORT ];
    int i_array_num = sizeof( option_long ) / sizeof( struct option ) ;
    char c_flag = 1;
    pc_prog_name = argv[ 0 ];

    get_option_short( option_long, i_array_num , ac_option_short , LEN_OPTION_SHORT );
    while((i_option = getopt_long(argc , argv , ac_option_short , option_long , NULL)) > 0) {
        switch(i_option) {
        case 'h' :  //help
            help_hint(argv[0]);
            c_flag = 0;
            goto parse_option_end;
        case 'a' :  //time
            wait_time = atoi(optarg);
            break;
        default :
            help_hint(argv[0]);
            c_flag = AK_FALSE;
            goto parse_option_end;
        }
    }
parse_option_end:
    return c_flag;
}



int main (int argc, char **argv)
{
    int ret = 0;
    struct ak_timeval start_tv;
    struct ak_timeval end_tv;

    /*
    *sdk init first
    */
    sdk_run_config config;
    config.mem_trace_flag = SDK_RUN_DEBUG;
    ak_sdk_init( &config );

    ak_print_normal(MODULE_ID_DRV, "***********************************************\n");
    ak_print_normal(MODULE_ID_DRV, "***** drv version: %s *****\n", ak_drv_get_version());
    ak_print_normal(MODULE_ID_DRV, "***********************************************\n");

    if (0 == parse_option(argc, argv)) 
    {
        return 0;
    }

    if (wait_time <= 0) 
    {
        ak_print_error(MODULE_ID_DRV, "wait time should be over 0 second.\n");
        return -1;
    }
    
    ak_print_normal(MODULE_ID_DRV, "ts test start.\n");
    
    /* 
    *open ts driver
    */    
    ret = ak_drv_ts_open();
    if(ret)
    {
        ak_print_error_ex(MODULE_ID_DRV, "ak_drv_ts_open fail.\n");
        return -1;
    }
        
    ak_get_ostime(&start_tv);
    
    do 
    {
        ak_print_info(MODULE_ID_DRV, "ak_drv_ts_get_event timeout 1000 ms.\n"); 
        ret = ak_drv_ts_get_event(&ts, 1000);
        
        /* 
        *get ts, output 
        */
        if(ret == 0) 
        {
            for (int i=0; i<TS_POINT_MAX_NUM; i++) 
            {
                /* 
                *check map, print ts info.
                */
                if (ts.map & (1 << i))
                    ak_print_normal(MODULE_ID_DRV, "id %d, get ts:[%d , %d].\n", i, ts.info[i].x, ts.info[i].y); 
            }
        }

        ak_get_ostime(&end_tv);
        /* 
        *run time is over and exit 
        */
        if (ak_diff_ms_time(&end_tv, &start_tv) > (wait_time * 1000))
            break;
    } while(1);
    
    /* 
    *close ts handle  
    */    
    ak_drv_ts_close();
    ak_print_normal(MODULE_ID_DRV, "ts test end.\n");    
    
    return 0;
}
