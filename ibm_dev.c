/*******************************************************************************
 * Copyright (c) 2018 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ranjan Dasgupta - Initial drop of deviceSample.c
 * 
 *******************************************************************************/

/*
 * This sample shows how-to develop a device code using Watson IoT Platform
 * iot-c device client library, connect and interact with Watson IoT Platform Service.
 * 
 * This sample includes the function/code snippets to perform the following actions:
 * - Initiliaze client library
 * - Configure device from configuration parameters specified in a configuration file
 * - Set client logging
 * - Enable error handling routines
 * - Send device events to WIoTP service
 * - Receive and process commands from WIoTP service
 *
 * SYNTAX:
 * deviceSample --config <config_file_path>
 *
 * Pre-requisite:
 * 1. This sample requires a device configuration file. 
 *    Refer to "Device Confioguration File" section of iot-c docs for details.
 * 2. Register type and id (specified in the configuration file) with IBM Watson IoT Platform service.
 *
 */

#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

/* Include header file of IBM Watson IoT platform C Client for devices */ 
#include <iotp_device.h>
#include <libubox/blobmsg_json.h>
#include <libubus.h>

char *configFilePath = NULL;
volatile int interrupt = 0;
char *progname = "deviceSample";
int useEnv = 0;
int testCycle = 0;

int rc = 0;

enum {
	TOTAL_MEMORY,
	FREE_MEMORY,
	SHARED_MEMORY,
	BUFFERED_MEMORY,
	__MEMORY_MAX,
};

enum {
	MEMORY_DATA,
	__INFO_MAX,
};

/* Usage text */
void usage(void) {
    fprintf(stderr, "Usage: %s --config config_file_path\n", progname);
    exit(1);
}

/* Signal handler - to support CTRL-C to quit */
void sigHandler(int signo) {
    signal(SIGINT, NULL);
    fprintf(stdout, "Received signal: %d\n", signo);
    interrupt = 1;
}

/* Get and process command line options */
void getopts(int argc, char** argv)
{
    int count = 1;

    while (count < argc)
    {
        if (strcmp(argv[count], "--config") == 0)
        {
            if (++count < argc)
                configFilePath = argv[count];
            else
                usage();
        }
        if (strcmp(argv[count], "--useEnv") == 0) {
            useEnv = 1;
        }
        if (strcmp(argv[count], "--testCycle") == 0) {
            if (++count < argc)
                testCycle = atoi(argv[count]);
            else
                usage();
        }
        count++;
    }
}

 /* 
 * Device command callback function
 * Device developers can customize this function based on their use case
 * to handle device commands sent by WIoTP.
 * Set this callback function using API setCommandHandler().
 */

void  deviceCommandCallback (char* type, char* id, char* commandName, char *format, void* payload, size_t payloadSize)
{
    fprintf(stdout, "Received device command:\n");
    fprintf(stdout, "Type=%s ID=%s CommandName=%s Format=%s Len=%d\n", 
        type?type:"", id?id:"", commandName?commandName:"", format?format:"", (int)payloadSize);
    fprintf(stdout, "Payload: %s\n", payload?(char *)payload:"");

    /* Device developers - add your custom code to process device commands */
}

void logCallback (int level, char * message)
{
    if ( level > 0 )
        fprintf(stdout, "%s\n", message? message:"NULL");
    fflush(stdout);
}

void MQTTTraceCallback (int level, char * message)
{
    if ( level > 0 )
        fprintf(stdout, "%s\n", message? message:"NULL");
    fflush(stdout);
}



static const struct blobmsg_policy memory_policy[__MEMORY_MAX] = {
	[TOTAL_MEMORY] = { .name = "total", .type = BLOBMSG_TYPE_INT64 },
	[FREE_MEMORY] = { .name = "free", .type = BLOBMSG_TYPE_INT64 },
	[SHARED_MEMORY] = { .name = "shared", .type = BLOBMSG_TYPE_INT64 },
	[BUFFERED_MEMORY] = { .name = "buffered", .type = BLOBMSG_TYPE_INT64 },
};

static const struct blobmsg_policy info_policy[__INFO_MAX] = {
	[MEMORY_DATA] = { .name = "memory", .type = BLOBMSG_TYPE_TABLE },
};

struct memory_data{
    char Total_memory[100];
    char Free_memory[50];
    char Shared_memory[50];
    char Buffered_memory[50];
};

static void board_cb(struct ubus_request *req, int type, struct blob_attr *msg) {
	//struct blob_buf *buf = (struct blob_buf *)req->priv;
    struct memory_data *temp_data = (struct memory_data *)req->priv;
	struct blob_attr *tb[__INFO_MAX];
	struct blob_attr *memory[__MEMORY_MAX];

	blobmsg_parse(info_policy, __INFO_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[MEMORY_DATA]) {
		fprintf(stderr, "No memory data received\n");
		rc=-1;
		return;
	}

	blobmsg_parse(memory_policy, __MEMORY_MAX, memory,
	        blobmsg_data(tb[MEMORY_DATA]), blobmsg_data_len(tb[MEMORY_DATA]));

    /* sprintf(buf,"{\"d\" : {\"Total memory\": %lld, \"Free memory\" : %lld, \"Shared memory\" : %lld, \"Buffered memory\" : %lld}}",\
    blobmsg_get_u64(memory[TOTAL_MEMORY]),blobmsg_get_u64(memory[FREE_MEMORY]),blobmsg_get_u64(memory[SHARED_MEMORY]),\
    blobmsg_get_u64(memory[BUFFERED_MEMORY]));
    */
    sprintf(temp_data->Total_memory,"%lld", blobmsg_get_u64(memory[TOTAL_MEMORY]));
    printf("Printing Total memory %s\n", temp_data->Total_memory);

    sprintf(temp_data->Free_memory,"%lld",blobmsg_get_u64(memory[FREE_MEMORY]));
    printf("Printing Free memory %s\n", temp_data->Free_memory);

    sprintf(temp_data->Shared_memory,"%lld",blobmsg_get_u64(memory[SHARED_MEMORY]));
    printf("Printing shared memory %s\n", temp_data->Shared_memory);

    sprintf(temp_data->Buffered_memory,"%lld",blobmsg_get_u64(memory[BUFFERED_MEMORY]));
    printf("Printing buffered memory %s\n", temp_data->Buffered_memory);

    //temp_data->Free_memory = blobmsg_get_u64(memory[FREE_MEMORY]);
    //temp_data->Shared_memory = blobmsg_get_u64(memory[SHARED_MEMORY]);
    //temp_data->Buffered_memory = blobmsg_get_u64(memory[SHARED_MEMORY]);
}

    


/* Main program */
int main(int argc, char *argv[])
{
    int cycle = 0;
    //int rc =0;

    IoTPConfig *config = NULL;
    IoTPDevice *device = NULL;
    /* 
     * DEV_NOTES:
     * Specifiy variable for WIoT client object 
     */
    
    IOTPRC rc = IOTPRC_SUCCESS;

    /* check for args */
    if ( argc < 2 )
        usage();

    /* Set signal handlers */
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    /* get argument options */
    getopts(argc, argv);

    /* Set IoTP Client log handler */
    rc = IoTPConfig_setLogHandler(IoTPLog_FileDescriptor, stdout);
    if ( rc != 0 ) {
        fprintf(stderr, "WARN: Failed to set IoTP Client log handler: rc=%d\n", rc);
        goto close_error;
    }



    /* Create IoTPConfig object using configuration options defined in the configuration file. */
    rc = IoTPConfig_create(&config, configFilePath);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to initialize configuration: rc=%d\n", rc);
        goto close_error;
    }

    /* read additional config from environment */
    if ( useEnv == 1 ) {
        IoTPConfig_readEnvironment(config);
    } 

    IoTPConfig_setProperty(config, "identity.orgId", "gut697");
    IoTPConfig_setProperty(config, "identity.typeId", "roko_rutx");
    IoTPConfig_setProperty(config, "identity.deviceId", "rutx10qwerty");
    IoTPConfig_setProperty(config, "token.deviceId", "@WFY&!O9!9ghST+dOz");

    /* Create IoTPDevice object */
    rc = IoTPDevice_create(&device, config);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to configure IoTP device: rc=%d\n", rc);
        goto close_error;
    }

    /* Set MQTT Trace handler
    rc = IoTPDevice_setMQTTLogHandler(device, &MQTTTraceCallback);
    if ( rc != 0 ) {
        fprintf(stderr, "WARN: Failed to set MQTT Trace handler: rc=%d\n", rc);
    } */

    /* Invoke connection API IoTPDevice_connect() to connect to WIoTP. */
    rc = IoTPDevice_connect(device);
    if ( rc != 0 ) {
        fprintf(stderr, "ERROR: Failed to connect to Watson IoT Platform: rc=%d\n", rc);
        fprintf(stderr, "ERROR: Returned error reason: %s\n", IOTPRC_toString(rc));
        goto close_error;
    }

    /*
     * Set device command callback using API IoTPDevice_setCommandsHandler().
     * Refer to deviceCommandCallback() function DEV_NOTES for details on
     * how to process device commands received from WIoTP.
     */
    IoTPDevice_setCommandsHandler(device, deviceCommandCallback);

    /*
     * Invoke device command subscription API IoTPDevice_subscribeToCommands().
     * The arguments for this API are commandName, format, QoS
     * If you want to subscribe to all commands of any format, set commandName and format to "+"
     */
    /* Use IoTPDevice_sendEvent() API to send device events to Watson IoT Platform. */
    
	struct ubus_context *ctx;
	uint32_t id;

    ctx = ubus_connect(NULL);
	if (!ctx) {
	fprintf(stderr, "Failed to connect to ubus\n");
	return -1;
	}
	

    /* Sample event - this sample device will send this event once in every 10 seconds. */

    struct memory_data *Data1;

    char *data[200];
    //memset(data,' ',200);

    while(!interrupt)
    {
        if (ubus_lookup_id(ctx, "system", &id) ||
	    ubus_invoke(ctx, id, "info", NULL, board_cb, Data1, 3000)) {
		fprintf(stderr, "cannot request memory info from procd\n");
		rc=-1;
	    }
        printf("\nPrinting data in main %s\n", Data1->Total_memory);
        fprintf(stdout, "Send status event\n");


        //rc = IoTPDevice_sendEvent(device,"status", data, "json", QoS0, NULL);
        fprintf(stdout, "RC from publishEvent(): %d\n", rc);

        if ( testCycle > 0 ) {
            cycle += 1;
            if ( cycle >= testCycle ) {
                break;
            }
        }
        sleep(10);
    }

    fprintf(stdout, "Publish event cycle is complete.\n");

    /* Disconnect device */
    rc = IoTPDevice_disconnect(device);
    if ( rc != IOTPRC_SUCCESS ) {
        fprintf(stderr, "ERROR: Failed to disconnect from  Watson IoT Platform: rc=%d\n", rc);
        goto close_error;
    }

close_error:
        exit(1);

close_noerror:
        exit(0);

    /* Destroy client */
    IoTPDevice_destroy(device);
	ubus_free(ctx);
    /* Clear configuration */
    IoTPConfig_clear(config);
    
    return 0;
}