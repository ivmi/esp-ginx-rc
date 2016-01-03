/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Israel Lot <me@israellot.com> and Jeroen Domburg <jeroen@spritesmods.com> 
 * wrote this file. As long as you retain this notice you can do whatever you 
 * want with this stuff. If we meet some day, and you think this stuff is 
 * worth it, you can buy us a beer in return. 
 * ----------------------------------------------------------------------------
 */


#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"
#include "cgi.h"
#include "cgi_wifi.h"
#include "cgi_relay.h"
#include "user_config.h"

#include "http.h"
#include "http_process.h"
#include "websocket.h"
#include "http_websocket_server.h"
#include "driver/uart.h"
#include "hdlc/hdlc.h"

#include "ws_uart_app.h"

#define HTTP_PORT 80
static struct ws_app_context* clients[MAX_CONNECTIONS]={NULL};


struct ws_app_context {
	uint8_t stream_data;
	int packet_requested_size;
	http_connection *conn;
	uint8_t waiting_sent;
	char * packet;
	int packet_size;
};

void uart_put_char(char data)
{
    uart_write(0, &data, 1);
}

static void ICACHE_FLASH_ATTR ws_app_send_packet(struct ws_app_context *context){

	//NODE_DBG("Webscoket app send packet size %d, requested: %d",context->packet_size,context->packet_requested_size);
	
    http_ws_push_bin(context->conn,context->packet,context->packet_size);
	context->waiting_sent=1;

}


static int  ICACHE_FLASH_ATTR ws_app_msg_sent(http_connection *c){
	NODE_DBG("Webscoket app msg sent %p",c);

	struct ws_app_context *context = (struct ws_app_context*)c->reverse;

	if(context!=NULL){

		NODE_DBG("\tcontext found %p",context);

		context->waiting_sent=0;

//		if(context->stream_data==1){
//			//no requet to stop made, send next packet
//			ws_app_send_packet(context);
//		}

	}

}

static int  ICACHE_FLASH_ATTR ws_app_client_disconnected(http_connection *c){

	NODE_DBG("Webscoket app client disconnected %p",c);
	//clean up
    
	struct ws_app_context *context = (struct ws_app_context*)c->reverse;

	if(context!=NULL){

        int i;
        for (i=0; i<MAX_CONNECTIONS; i++)
        {
            if (clients[i] == context)
                clients[i] = NULL;
        }
    
		if(context->packet!=NULL)
			os_free(context->packet);
		context->packet=NULL;
	}
	os_free(context);	

}

void on_uart_data(uint8_t *data,int len)
{
    int i;
    
    for (i=0; i<len; i++)
    {
        hdlc_on_rx_byte(data[i]);
    }
}

void on_uart_frame(const u8_t *buffer,u16_t bytes_received)
{
    int i;
    for (i=0; i<MAX_CONNECTIONS; i++)
    {
        struct ws_app_context *context = clients[i];
        if (context != NULL)
        {
            context->packet = (char*) buffer;
            context->packet_size = bytes_received;
            ws_app_send_packet(context);
        }
    }    
}

static int  ICACHE_FLASH_ATTR ws_app_client_connected(http_connection *c){

	NODE_DBG("Webscoket app client connected %p",c);
		
	//create config
	struct ws_app_context *context = (struct  ws_app_context*)os_zalloc(sizeof(struct ws_app_context));
	context->conn=c;
	c->reverse = context; //so we may find it on callbacks

    int i;
    for (i=0; i<MAX_CONNECTIONS; i++)
    {
        if (clients[i] == NULL)
        {
            clients[i] = context;
            break;
        }
    }

    // set uart interrupt handler
    uart_register_data_callback(on_uart_data);
    hdlc_init(uart_put_char, on_uart_frame);
	NODE_DBG("\tcontext %p",context);
}


static int  ICACHE_FLASH_ATTR ws_app_msg_received(http_connection *c){

	NODE_DBG("Webscoket app msg received %p",c);

	struct ws_app_context *context = (struct ws_app_context*)c->reverse;

	ws_frame *msg = (ws_frame *)c->cgi.argument;
	if(msg==NULL)
		return HTTP_WS_CGI_MORE; //just ignore and move on

	if(msg->SIZE <=0)
		return HTTP_WS_CGI_MORE; //just ignore and move on

	char * s = msg->DATA;
	char *last = s+msg->SIZE;

    //os_printf("%.*s", msg->SIZE, msg->DATA);
    #ifdef DEVELOP_VERSION
        uart_write(UART0, msg->DATA,msg->SIZE);
    #endif
    
    hdlc_tx_frame( msg->DATA, msg->SIZE);
	
    return HTTP_WS_CGI_MORE;

}

void ICACHE_FLASH_ATTR init_ws_server(){

	NODE_DBG("Webscoket app init");

	//ws
	http_ws_server_init(ws_app_client_connected,ws_app_client_disconnected,ws_app_msg_received,ws_app_msg_sent);
	http_ws_server_start();

}