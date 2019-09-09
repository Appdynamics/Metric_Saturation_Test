// ************************************************************************************************************
// ******                                                                                                ******
// ****** PROGRAM: saturationtest.c                                                                      ******
// ****** AUTHOR:  Robb Kane                                                                             ******
// ****** DATE:    September 3rd, 2019                                                                   ******
// ******                                                                                                ******
// ****** PROPERTY OF APPDYNAMICS, INC.  ALL RIGHTS RESERVED.                                            ******
// ******                                                                                                ******
// ****** This program is example code only.  There is no warranties implied or otherwise regarding its  ******
// ****** performance.  AppDynamics is not responsible for its use, nor any consequence of its use.      ******
// ******                                                                                                ******
// ************************************************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#include "appdynamics.h"
#include "appdynamics_advanced.h"

#define FALSE (1==0)
#define TRUE  (1==1)

#define SATURATION_NODE_NAME      "Saturation_Node______________________"
#define SATURATION_NODE_OFFSET    17

#define CUSTOM_METRIC_PATH        "Custom Metrics|Metric_________________________________________________________________"
#define CUSTOM_METRIC_OFFSET      25

#define DEFAULT_BT_NAME           "Default_BT__________________________________________________________________"
#define DEFAULT_BT_OFFSET         11

#define SATURATION_BT_NAME        "Saturation_BT_______________________________________________________________"
#define SATURATION_BT_OFFSET      15

#define SATURATION_CONTEXT        "saturation_context"
#define APP_NAME                  "Metric_Saturation_Test_App"
#define TIER_NAME                 "Metric_Saturation_Tier"
#define DEFAULT_NODE_NAME         "Default_Node"
#define LOCAL_HOST                "localhost"
#define DEFAULT_ACCT_NAME         "customer1"
#define DEFAULT_ACCESS_KEY        "Access Key Not Set"
#define CONTROLLER_PORT           8090
#define PROXY_PORT                9191
#define METRICS                   100
#define CYCLES                    10
#define INIT_TIMEOUT              60000
#define CYCLE_DELAY               60
#define REVIEW_DELAY              5

/* 
 * with reference to: https://stackoverflow.com/questions/1485805/whats-the-difference-between-the-printf-and-vprintf-function-families-and-when
*/
int info(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	char header[128];
	sprintf(header, "INFO|%d|", getpid());
	fputs(header, stdout);

	int result = vfprintf(stdout, fmt, ap);
	va_end(ap);
	return result;
}

int main(int argc, char** argv)
{
	int            init_timeout = INIT_TIMEOUT;
	int            cycle_delay = CYCLE_DELAY;
	int            review_delay = REVIEW_DELAY;
	int            rc = 0;

	appd_bt_handle        bt_handle = NULL;
	appd_bt_handle        saturation_bt_handle = NULL;
	struct appd_config*   cfg = NULL;

	long  metrics = METRICS;
	long  cycles = CYCLES;
	long  metric = 0;
	long  cycle = 0;

	unsigned long  controller_port = CONTROLLER_PORT;
	unsigned long  proxy_port = PROXY_PORT;

	unsigned       use_SSL = FALSE;
	unsigned       use_cert_file = FALSE;
	unsigned       use_proxy = FALSE;

	char*          cert_file = NULL;
	char*          cert_dir = NULL;

	char*          controller_host = LOCAL_HOST; 
	char*          acct_name = DEFAULT_ACCT_NAME; 

	char*          app_name = APP_NAME; 
	char*          tier_name = TIER_NAME; 
	char*          default_node_name = DEFAULT_NODE_NAME; 
	char           saturation_node_name[] = SATURATION_NODE_NAME;

	char*          proxy_host = NULL;
	char*          proxy_user = NULL;
	char*          proxy_pswd = NULL;
	char*          proxy_pswd_file = NULL;

	char*          access_key = DEFAULT_ACCESS_KEY; 
	const char*    saturation_context = SATURATION_CONTEXT;

	if (argc > 1)
	{
		controller_host = argv[1];
		info("Setting Controller Host to \"%s\" (argv[1])\n", argv[1]);
	}

	if (argc > 2)
	{
		unsigned int temp = atoi(argv[2]);
		controller_port = temp & 0xFFFF;
		info("Setting Controller Port to %ld (argv[2] was \"%s\") \n", controller_port, argv[2]);
	}

	if (argc > 3)
	{
		access_key = argv[3];
		info("Setting Controller Access Key to \"%s\" (argv[3])\n", argv[3]);
	}

	if (argc > 4)
	{
		acct_name = argv[4];
		info("Setting Controller Account to \"%s\" (argv[4])\n", argv[4]);
	}

	if (argc > 5)
	{
		cycles = atol(argv[5]);
		if (cycles == 0 || cycles > 10000)
		{
			info("Error: Cycles (%ld) exceeds range.  Setting to 1", cycles);
			cycles = 1;
		}
		info("Setting Cycles to %ld (argv[5] was \"%s\") \n", cycles, argv[5]);
	}

	if (argc > 6)
	{
		metrics = atol(argv[6]);
		if (metrics == 0 || metrics > 10000)
		{
			info("Error: metrics (%ld) exceeds range.  Setting to 1", metrics);
			metrics = 1;
		}
		info("Setting Metrics to %ld (argv[6] was \"%s\") \n", metrics, argv[6]);
	}

	sprintf(&saturation_node_name[SATURATION_NODE_OFFSET], "%d%c", getpid(),0);

	info("Controller Wait Timeout is %d seconds\n", (init_timeout/1000)); 
	info("Controller Host is \"%s\"\n",             controller_host);
	info("Controller Port is %lu\n",                controller_port);
	info("Controller useSSL is %s\n",               use_SSL ? "true" : "false");
	info("Controller Application is \"%s\"\n",      app_name);
	info("Controller Tier is \"%s\"\n",             tier_name);
	info("Controller Default Node is \"%s\"\n",     default_node_name);
	info("Controller Saturation Node is \"%s\"\n",  saturation_node_name);
	info("Controller Node is \"%s\"\n",             default_node_name);
	info("Controller Account Name is \"%s\"\n",     acct_name);
	info("Controller License is \"%s\"\n",          access_key);

	info("\n\nCycling %ld times, creating %ld Custom Metics, with 1 BT per Node created per cycle (per minute)\n", cycles, metrics); 
	info("\nContinuing in %d seconds...\n", review_delay);
	sleep(review_delay);
	
	cfg = appd_config_init();

	if (cfg == NULL)
	{
	   info("appd_config_init() failed!\n");
	   exit(1);
	}

	appd_config_set_controller_host(cfg, controller_host);
	appd_config_set_controller_port(cfg, controller_port);
	appd_config_set_controller_account(cfg, acct_name);
	appd_config_set_controller_access_key(cfg, access_key);
	appd_config_set_controller_use_ssl(cfg, use_SSL);
	appd_config_set_app_name(cfg, app_name);
	appd_config_set_tier_name(cfg, tier_name);
	appd_config_set_node_name(cfg, default_node_name);
	appd_config_set_flush_metrics_on_shutdown(cfg, TRUE);
	appd_config_set_init_timeout_ms(cfg, init_timeout);

	if (use_cert_file)
	{
	   appd_config_set_controller_certificate_file(cfg, cert_file);
	   appd_config_set_controller_certificate_dir( cfg, cert_dir);
	}

	if (use_proxy)
	{
	   appd_config_set_controller_http_proxy_host(         cfg, proxy_host);
	   appd_config_set_controller_http_proxy_port(         cfg, proxy_port & 0xFFFF);
	   appd_config_set_controller_http_proxy_username(     cfg, proxy_user);
	   appd_config_set_controller_http_proxy_password(     cfg, proxy_pswd);
	   appd_config_set_controller_http_proxy_password_file(cfg, proxy_pswd_file);
	}

	struct appd_context_config* saturation_cfg = appd_context_config_init(saturation_context);
	appd_context_config_set_controller_host(      saturation_cfg, controller_host);
	appd_context_config_set_controller_port(      saturation_cfg, controller_port & 0xFFFF);
	appd_context_config_set_controller_account(   saturation_cfg, acct_name);
	appd_context_config_set_controller_access_key(saturation_cfg, access_key);
	appd_context_config_set_controller_use_ssl(   saturation_cfg, use_SSL);
	appd_context_config_set_app_name(             saturation_cfg, app_name);
	appd_context_config_set_tier_name(            saturation_cfg, tier_name);
	appd_context_config_set_node_name(            saturation_cfg, saturation_node_name);

	info("About to call appd_sdk_add_app_context()...\n");

	rc = appd_sdk_add_app_context(saturation_cfg);

	if (rc != 0)
	{
	   info("appd_sdk_add_app_context() failed!\n");
	   exit(1);
	}

	info("About to call appd_sdk_init(), waiting up to %d seconds for Controller response...\n", (init_timeout/1000));

	rc = appd_sdk_init(cfg);

	if (rc != 0)
	{
	   info("appd_sdk_init() failed!\n");
	   exit(1);
	}

	info("\nInitialization complete, now registering metrics...\n\n");

	for (metric = 1; metric <= metrics; metric++)
	{
		char metric_path[] = CUSTOM_METRIC_PATH; 
		sprintf(&metric_path[CUSTOM_METRIC_OFFSET ], "%ld%c", metric, 0);

		info("appd_custom_metric_add(\"%s\", \"%s\", %d, %d, %d)\n", saturation_context, metric_path, 
			APPD_TIMEROLLUP_TYPE_AVERAGE, APPD_CLUSTERROLLUP_TYPE_INDIVIDUAL,  APPD_HOLEHANDLING_TYPE_RATE_COUNTER);

		appd_custom_metric_add (saturation_context, metric_path, 
			APPD_TIMEROLLUP_TYPE_AVERAGE, APPD_CLUSTERROLLUP_TYPE_INDIVIDUAL,  APPD_HOLEHANDLING_TYPE_RATE_COUNTER);
	}

	info("\nMetric registration complete, now creating custom metrics and BTs...\n\n");

	for (cycle = 1; cycle <= cycles; cycle++)
	{
		char default_bt_name[]    = DEFAULT_BT_NAME; 
		char saturation_bt_name[] = SATURATION_BT_NAME; 
	
		sprintf(&default_bt_name[DEFAULT_BT_OFFSET], "%ld%c", (cycle % 4), 0);
		sprintf(&saturation_bt_name[SATURATION_BT_OFFSET], "%ld%c", (cycle % 32), 0);
	
		info("Calling appd_bt_begin(\"%s\", NULL)...", default_bt_name);
		bt_handle = appd_bt_begin(default_bt_name, NULL);
		info("handle = %p\n", bt_handle);
		
		info("Calling appd_bt_begin_with_app_context(\"%s\", \"%s\", NULL)...", saturation_context, saturation_bt_name);
		saturation_bt_handle = appd_bt_begin_with_app_context(saturation_context, saturation_bt_name, NULL);
		info("handle = %p\n\n", saturation_bt_handle);
	
		for (metric = 1; metric <= metrics; metric++)
		{
			char metric_path[] = CUSTOM_METRIC_PATH;
	
			sprintf(&metric_path[CUSTOM_METRIC_OFFSET], "%ld%c", metric, 0);
	
			appd_custom_metric_report(saturation_context, metric_path, metric);
	
			info("appd_custom_metric_report(\"%s\", \"%s\", %ld)\n", saturation_context, metric_path, metric);
		}
	
		appd_bt_end(saturation_bt_handle);
		appd_bt_end(bt_handle);
	
		info("Cycle %ld Completed, sleeping %d seconds before continuing...\n", cycle, cycle_delay);
		sleep(cycle_delay);
	}

	info("\nCycles completed.\n");
	info("Created %ld custom metrics, ran %ld cycles.\n", metrics, cycles);
	info("Created %ld unique Default BTS, sent 1 BT begin/end pair per cycle.\n", (cycles < 4) ? cycles : 4);
	info("Created %ld unique Saturation BTS, sent 1 BT begin/end pair per cycle.\n", (cycles < 32) ? cycles : 32);
	info("Waiting for SDK (libagent) to finish flushing to Controller.\n");
	info("This can take between one and two minutes to complete...\n");
	sleep(1);

	appd_sdk_term();

	info("\n\nAll SDK API calls have completed, process terminated nominally.\n\n");

	exit(0);
}

// end of saturationtest.cpp file.
