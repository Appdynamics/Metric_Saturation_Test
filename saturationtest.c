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

#include "appdynamics.h"
#include "appdynamics_advanced.h"

#define FALSE (1==0)

int main(int argc, char** argv)
{
    int            init_timeout = 60000;
    int            pause = 1;
    int            rc = 0;

    appd_bt_handle        bt_handle = NULL;
    appd_bt_handle        saturation_bt_handle = NULL;
    struct appd_config*   cfg = NULL;

    unsigned long  metrics = 1000;
    unsigned long  cycles = 10;
    unsigned long  metric = 0;
    unsigned long  cycle = 0;

    unsigned long  controller_port = 8090;
    unsigned long  proxy_port = 9191;

    unsigned       use_SSL = FALSE;
    unsigned       use_cert_file = FALSE;
    unsigned       use_proxy = FALSE;

    char*          cert_file = NULL;
    char*          cert_dir = NULL;

    char*          controller_host = "localhost";
    char*          acct_name = "customer1";

    char*          app_name = "Metric_Saturation_Test_App";
    char*          tier_name = "Metric_Saturation_Tier";
    char*          default_node_name = "Default_Node";
    char*          saturation_node_name = "Saturation_Node";

    char*          proxy_host = NULL;
    char*          proxy_user = NULL;
    char*          proxy_pswd = NULL;
    char*          proxy_pswd_file = NULL;

    char*          access_key = "Access Key Not Set";
    const char*    saturation_context = "saturation_context";

    if (argc > 1)
    {
		printf("Setting Controller Host to \"%s\" (argv[1])\n", argv[1]);
		controller_host = argv[1];
    }

    if (argc > 2)
    {
		printf("Setting Controller Port to \"%s\" (argv[2])\n", argv[2]);
		unsigned int temp = atoi(argv[2]);
		controller_port = temp & 0xFFFF;
    }

    if (argc > 3)
    {
		printf("Setting Controller Access Key to \"%s\" (argv[3])\n", argv[3]);
		access_key = argv[3];
    }

    if (argc > 4)
    {
		printf("Setting Controller Account to \"%s\" (argv[4])\n", argv[4]);
		acct_name = argv[4];
    }

    if (argc > 5)
    {
		printf("Setting Cycles to \"%s\" (argv[5])\n", argv[5]);
		cycles = atol(argv[5]);
		if (cycles == 0 || cycles > 10000)
		{
			printf("Error: Cycles (%ld) exceeds range.  Setting to 1", cycles);
			cycles = 1;
		}
    }

    if (argc > 6)
    {
		printf("Setting Metrics to \"%s\" (argv[6])\n", argv[6]);
		metrics = atol(argv[5]);
		if (metrics == 0 || metrics > 10000)
		{
			printf("Error: metrics (%ld) exceeds range.  Setting to 1", metrics);
			metrics = 1;
		}
    }

    printf("Controller Wait Timeout is %d seconds\n", (init_timeout/1000)); 
    printf("Controller Host is \"%s\"\n",             controller_host);
    printf("Controller Port is %lu\n",                controller_port);
    printf("Controller useSSL is %s\n",               use_SSL ? "true" : "false");
    printf("Controller Application is \"%s\"\n",      app_name);
    printf("Controller Tier is \"%s\"\n",             tier_name);
    printf("Controller Default Node is \"%s\"\n",     default_node_name);
    printf("Controller Saturation Node is \"%s\"\n",  saturation_node_name);
    printf("Controller Node is \"%s\"\n",             default_node_name);
    printf("Controller Account Name is \"%s\"\n",     acct_name);
    printf("Controller License is \"%s\"\n",          access_key);

    printf("\n\nCycling %ld times, creating %ld Custom Metics, with 1 BT per Node created per cycle (per minute)\n", cycles, metrics); 
	printf("\nContinuing in 5 seconds...\n");
	sleep(5);
	
   cfg = appd_config_init();

   if (cfg == NULL)
   {
       printf("appd_config_init() failed!\n");
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
   appd_config_set_flush_metrics_on_shutdown(cfg, 1);
   appd_config_set_init_timeout_ms(cfg, 10000);

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

   printf("About to call appd_sdk_add_app_context()...\n");

   rc = appd_sdk_add_app_context(saturation_cfg);

   if (rc != 0)
   {
       printf("appd_sdk_add_app_context() failed!\n");
       exit(1);
   }

   printf("About to call appd_sdk_init(), waiting up to %d seconds for Controller response...\n", (init_timeout/1000));

   rc = appd_sdk_init(cfg);

   if (rc != 0)
   {
       printf("appd_sdk_init() failed!\n");
       exit(1);
   }

   printf("\nInitialization complete, now registering metrics...\n\n");

    for (metric = 1; metric <= metrics; metric++)
	{
		char metric_path[] = "Custom Metrics|Metric_________________________________________________________________"; 
		sprintf(&metric_path[25], "%ld%c", metric, 0);

		printf("appd_custom_metric_add(\"%s\", \"%s\", %d, %d, %d)\n", saturation_context, metric_path, 
			APPD_TIMEROLLUP_TYPE_AVERAGE, APPD_CLUSTERROLLUP_TYPE_INDIVIDUAL,  APPD_HOLEHANDLING_TYPE_RATE_COUNTER);

		appd_custom_metric_add (saturation_context, metric_path, 
			APPD_TIMEROLLUP_TYPE_AVERAGE, APPD_CLUSTERROLLUP_TYPE_INDIVIDUAL,  APPD_HOLEHANDLING_TYPE_RATE_COUNTER);
	}

   printf("\nMetric registration complete, now creating custom metrics and BTs...\n\n");

   for (cycle = 1; cycle <= cycles; cycle++)
   {

	char default_bt_name[]    = "Default_BT__________________________________________________________________"; 
	char saturation_bt_name[] = "Saturation_BT_______________________________________________________________"; 

	sprintf(&default_bt_name[25], "%ld%c", (cycle % 4), 0);
	sprintf(&saturation_bt_name[25], "%ld%c", (cycle % 32), 0);

	printf("Calling appd_bt_begin(\"%s\", NULL)...", default_bt_name);
    bt_handle = appd_bt_begin(default_bt_name, NULL);
	printf("handle = %p\n", bt_handle);
	
	printf("Calling appd_bt_begin_with_app_context(\"%s\", \"%s\", NULL)...", saturation_context, saturation_bt_name);
    saturation_bt_handle = appd_bt_begin_with_app_context(saturation_context, saturation_bt_name, NULL);
	printf("handle = %p\n\n", saturation_bt_handle);

    for (metric = 1; metric <= metrics; metric++)
	{
		char metric_path[] = "Custom Metrics|Metric_________________________________________________________________"; 

		sprintf(&metric_path[25], "%ld%c", metric, 0);

		appd_custom_metric_report(saturation_context, metric_path, metric);

		printf("appd_custom_metric_report(\"%s\", \"%s\", %ld)\n", saturation_context, metric_path, metric);
	}

	printf("Cycle %ld Completed\n", cycle);

    appd_bt_end(saturation_bt_handle);
    appd_bt_end(bt_handle);
	sleep(60);

   }

    printf("\nCycles completed.\n");
    printf("Created %ld custom metrics, ran %ld cycles.\n", metrics, cycles);
	printf("Created %ld unique Default BTS, sent 1 BT begin/end pair per cycle.\n", (cycles < 4) ? cycles : 4);
	printf("Created %ld unique Saturation BTS, sent 1 BT begin/end pair per cycle.\n", (cycles < 32) ? cycles : 32);
	printf("Waiting for SDK (libagent) to finish flushing to Controller.\n");
	printf("This can take between one and two minutes to complete...\n");
	sleep(1);

    appd_sdk_term();

    printf("\n\nAll SDK API calls have completed, process terminated nominally.\n\n");

	exit(0);
}

// end of saturationtest.cpp file.
