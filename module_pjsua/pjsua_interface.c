/* 
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

/**
 * simple_pjsua.c
 *
 * This is a very simple but fully featured SIP user agent, with the 
 * following capabilities:
 *  - SIP registration
 *  - Making and receiving call
 *  - Audio/media to sound device.
 *
 * Usage:
 *  - To make outgoing call, start simple_pjsua with the URL of remote
 *    destination to contact.
 *    E.g.:
 *       simpleua sip:user@remote
 *
 *  - Incoming calls will automatically be answered with 200.
 *
 * This program will quit once it has completed a single call.
 */

#include <pthread.h>
#include "pjsua_interface.h"
#include <pjsua-lib/pjsua.h>

#define THIS_FILE	"pjsuaInterface"

#define SIP_DOMAIN	"192.168.7.2" //make this automatic look into sample app 
#define SIP_USER	"debian"


static pthread_t pjsuaThreadPID = -1;
static pthread_cond_t * shutdownRequest = NULL;
static pthread_mutex_t * shutdownLock = NULL;

static pthread_mutex_t call_mutex;
static pjsua_call_id current_call = PJSUA_INVALID_ID;

static pjsua_acc_id acc_id;
static pjsua_acc_id acc_id2;



static pj_status_t status;

//

static void sendShutdownRequest(void){
    pthread_mutex_lock(shutdownLock);
    pthread_cond_signal(shutdownRequest);
    pthread_mutex_unlock(shutdownLock);
}


/* Callback called by the library upon receiving incoming call */
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,
                             pjsip_rx_data *rdata)
{
    pjsua_call_info ci;

    PJ_UNUSED_ARG(acc_id);
    PJ_UNUSED_ARG(rdata);

    pthread_mutex_lock(&call_mutex);
    if(current_call != PJSUA_INVALID_ID){
       pthread_mutex_unlock(&call_mutex);

       pjsua_call_answer(call_id, PJSIP_SC_BUSY_HERE, NULL, NULL);
         PJ_LOG(3,(THIS_FILE, "Rejecting incoming call is busy"));
        return;
    }

    current_call=call_id;
    pthread_mutex_unlock(&call_mutex);


    pjsua_call_get_info(call_id, &ci);

    PJ_LOG(3,(THIS_FILE, "Incoming call from %.*s!!",
                         (int)ci.remote_info.slen,
                         ci.remote_info.ptr));

    /* Automatically answer incoming calls with 200/OK */
    pjsua_call_answer(call_id, 200, NULL, NULL);
}

/* Callback called by the library when call's state has changed */
static void on_call_state(pjsua_call_id call_id, pjsip_event *e)
{
    pjsua_call_info ci;
    PJ_UNUSED_ARG(e);
    pjsua_call_get_info(call_id, &ci);
    PJ_LOG(3,(THIS_FILE, "Call %d state=%.*s", call_id,
                         (int)ci.state_text.slen,
                         ci.state_text.ptr));
     
    //allows for new incoming call to be picked up after current in session call is diconected
    

    if(call_id==current_call){

        if(ci.state == PJSIP_INV_STATE_DISCONNECTED){
            pthread_mutex_lock(&call_mutex);
            current_call=PJSUA_INVALID_ID;
            pthread_mutex_unlock(&call_mutex);
            PJ_LOG(3,(THIS_FILE, "free to make and accept calls"));
        }

        
    }
        

}

/* Callback called by the library when call's media state has changed 
    Note that 0 is used as the slot ID for the default sound device. 
    This ID is predefined in PJSUA as the ID of the default audio port used for playback and recording.
    pjsua_set_snddev(9,3) make 9 and 3 default devices 
*/

static void on_call_media_state(pjsua_call_id call_id)
{
    pjsua_call_info ci;

    pjsua_call_get_info(call_id, &ci);

    if (ci.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
        // When media is active, connect call to sound device.
        pjsua_conf_connect(ci.conf_slot, 0);
        pjsua_conf_connect(0, ci.conf_slot);
    }
}

/* Display error and exit application */
static void error_exit(const char *title, pj_status_t status)
{
    pjsua_perror(THIS_FILE, title, status);
    pjsua_destroy();
    exit(1);
}

int pjsua_interface_make_call(char *str){

    pthread_mutex_lock(&call_mutex);
    if(current_call!=PJSUA_INVALID_ID)	{
        pthread_mutex_unlock(&call_mutex);
        return 0;
    }
    //pj_str_t uri = pj_str("sip:san@192.168.26.128");
    pj_str_t uri = pj_str(str);
    status = pjsua_call_make_call(acc_id, &uri, 0, NULL, NULL, &current_call);
    
    if (status != PJ_SUCCESS){

        PJ_LOG(3,(THIS_FILE, "make call uncsuccesful sip uri may be invalid call id: %d",current_call));
        current_call=PJSUA_INVALID_ID;
        pthread_mutex_unlock(&call_mutex);
        return 0;

    }else{

        PJ_LOG(3,(THIS_FILE, "make call succesful call id: %d",current_call));
        pthread_mutex_unlock(&call_mutex);
    }

    return 1;


}

static int pjsua_thread(void){

   
    

    /* Create pjsua first! */
    status = pjsua_create();
    if (status != PJ_SUCCESS) error_exit("Error in pjsua_create()", status);

    /* Init pjsua */
    {
        pjsua_config cfg;
        pjsua_logging_config log_cfg;

        pjsua_config_default(&cfg);
        cfg.cb.on_incoming_call = &on_incoming_call;
        cfg.cb.on_call_media_state = &on_call_media_state;
        cfg.cb.on_call_state = &on_call_state;

        pjsua_logging_config_default(&log_cfg);
        log_cfg.console_level = 4;

        status = pjsua_init(&cfg, &log_cfg, NULL);
        if (status != PJ_SUCCESS) error_exit("Error in pjsua_init()", status);
    }

    /* Add UDP transport. */
    {
        pjsua_transport_config cfg;
        pjsua_transport_config_default(&cfg);
        cfg.port = 5060;
        status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &cfg, NULL);
        if (status != PJ_SUCCESS) error_exit("Error creating transport", status);
    }

    /* Initialization is done, now start pjsua */
    status = pjsua_start();
    if (status != PJ_SUCCESS) error_exit("Error starting pjsua", status);
    
    //set hardware devices for input and output 
    status = pjsua_set_snd_dev(9, 3);

    //volume ajustement tested works still dont understand the 0 
    //pjsua_conf_adjust_tx_level(0, 5.0);

    if (status != PJ_SUCCESS) error_exit("Error adding sound device", status);
    /* Get the current input (microphone) volume */
  
    //register without sip server account
    pjsua_acc_config acc_cfg;
    pjsua_acc_config_default(&acc_cfg);
    acc_cfg.id = pj_str("sip:" SIP_USER "@" SIP_DOMAIN);
    status = pjsua_acc_add(&acc_cfg, PJ_TRUE, &acc_id);
    if (status != PJ_SUCCESS)  error_exit("Error first account", status);

    pjsua_acc_config acc_cfg2;
    pjsua_acc_config_default(&acc_cfg2);
    acc_cfg.id = pj_str("sip:debian@192.168.7.5");
    status = pjsua_acc_add(&acc_cfg, PJ_TRUE, &acc_id2);
    if (status != PJ_SUCCESS)  error_exit("Error second account", status);

    /* Register to SIP server by creating SIP account. */
    {
        // pjsua_acc_config cfg;
        // pjsua_acc_config_default(&cfg);
        // cfg.id = pj_str("sip:" SIP_USER "@" SIP_DOMAIN);
        // cfg.reg_uri = pj_str("sip:" SIP_DOMAIN);
        // status = pjsua_acc_add(&cfg, PJ_TRUE, &acc_id);
        // if (status != PJ_SUCCESS) error_exit("Error adding account", status);

        /*  if using server to route calls
            pjsua_acc_config cfg;

            pjsua_acc_config_default(&cfg);
            cfg.id = pj_str("sip:" SIP_USER "@" SIP_DOMAIN);
            cfg.reg_uri = pj_str("sip:" SIP_DOMAIN);
            cfg.cred_count = 1;
            cfg.cred_info[0].realm = pj_str(SIP_DOMAIN);
            cfg.cred_info[0].scheme = pj_str("digest");
            cfg.cred_info[0].username = pj_str(SIP_USER);
            cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
            cfg.cred_info[0].data = pj_str(SIP_PASSWD);

            status = pjsua_acc_add(&cfg, PJ_TRUE, &acc_id);
            if (status != PJ_SUCCESS) error_exit("Error adding account", status);


        */
    }

    /* If URL is specified, make call to the URL. */

    /* Wait until user press "q" to quit. */
    for (;;) {
        char option[10];

        puts("Press 'h' to hangup all calls, 'q' to quit, c to call");
        if (fgets(option, sizeof(option), stdin) == NULL) {
            puts("EOF while reading stdin, will quit now..");
            break;
        }

        if (option[0] == 'q')
            break;

        if (option[0] == 'h')
            pjsua_call_hangup_all();
            
        if (option[0]== 'c') {
            //pjsua_conf_adjust_tx_level(0, 0.01);
            // pj_str_t uri = pj_str("sip:san@192.168.26.128");

            // status = pjsua_call_make_call(acc_id, &uri, 0, NULL, NULL, NULL);
            // if (status != PJ_SUCCESS) error_exit("Error making call", status);
            if(pjsua_interface_make_call("sip:san@192.168.26.128")){

                 PJ_LOG(3,(THIS_FILE, "make call succesful, call is active"));
            }else{
                 PJ_LOG(3,(THIS_FILE, "make call unsuccesful, call in progress or invalid uri"));
            }
        }
            //make call
    }

    /* Destroy pjsua */

    pjsua_destroy();
    sendShutdownRequest();
    return 0;
}



int pjsua_interface_init(pthread_cond_t * cond, pthread_mutex_t * lock){

    shutdownRequest = cond;
    shutdownLock = lock;
    pthread_mutex_init(&call_mutex, NULL);
    if(pthread_create(&pjsuaThreadPID, NULL, (void*)&pjsua_thread, NULL) != 0){
        perror("Error in creating ");
        sendShutdownRequest();
        return -1;
    }

    return 1;
}

int pjsua_interface_cleanup(void){
    
    pthread_mutex_destroy(&call_mutex);
    if(pthread_join(pjsuaThreadPID, NULL) != 0){
        perror("Error in joining the phsua thread.");
    }
   
    return 1;
}