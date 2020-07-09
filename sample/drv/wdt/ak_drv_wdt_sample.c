#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "ak_common.h"
#include "ak_log.h"
#include "ak_drv.h"


#define LEN_HINT             512
#define LEN_OPTION_SHORT     512
#define DEFAULT_TIMEOUT        8

static char *pc_prog_name = NULL;  
static int feed_time = 0;//second
static int feed_circle = 0;//second
static int timeout = 0;//second



char ac_option_hint[  ][ LEN_HINT ] = {                                         //操作提示数组
    "help info" ,
    "[NUM] [1-357]timeout to set" ,
    "[NUM] feed time" ,
    "[NUM] feed circle" ,
    "" ,
};

struct option option_long[ ] = {
    { "help"      , no_argument       , NULL , 'h' } ,    //"help info"
    { "timeout"   , required_argument , NULL , 'a' } ,    //"[NUM] timeout to set"
    { "feed_time" , required_argument , NULL , 'b' } ,    //"[NUM] feed time"
    { "feed_circle" , required_argument , NULL , 'c' } ,    //"[NUM] feed circle"
    {0, 0, 0, 0}
 };

void usage(const char * name)
{
    ak_print_normal(MODULE_ID_VO," %s -a [timeout] -b [feed_time] -c [feed_circle]\n", name);
    ak_print_normal(MODULE_ID_VO,"eg: %s -a 10 -b 9 -c 3\n", name);
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
        case 'a' :  //timeout
            timeout = atoi(optarg);
            break;
        case 'b' :  //feed time
            feed_time = atoi(optarg);
            break;
        case 'c' :  //feed circle
            feed_circle = atoi(optarg);
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
    
    ak_print_normal(MODULE_ID_DRV, "wdt test start.\n");

    /*
    *check timeout
    */
    if ((timeout < 1) || (timeout > 357)) {
        ak_print_normal(MODULE_ID_DRV, "timeout error,use default value.\n");
        timeout = DEFAULT_TIMEOUT;
    }

    /*
    *check feedtime
    */
    if (feed_time <= 0) {
        ak_print_normal(MODULE_ID_DRV, "feed_time error,use default value.\n");
        
        if (timeout > 1)
            feed_time = timeout - 1;
        else
            feed_time = 1;
    }

    /*
    *check feed_circle
    */
    if (feed_circle <= 0) {
        ak_print_normal(MODULE_ID_DRV, "feed_circle error,use default value.\n");
        feed_circle = 5;
    }

    /*
    *watchdog dev open
    */
    if (ak_drv_wdt_open(timeout)){
        return 0;
    }

    /*
    *feed watchdog
    */
    for(int i = 0; i < feed_circle; i++){
        ak_print_normal(MODULE_ID_DRV, "feed dog\n");
        ak_drv_wdt_feed();
        sleep(feed_time);
    }
    /* 
    *close watch dog 
    */
    ak_drv_wdt_close();
    ak_print_normal(MODULE_ID_DRV, "wdt test end.\n");    
    
    return 0;
}
