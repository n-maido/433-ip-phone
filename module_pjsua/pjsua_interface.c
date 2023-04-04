
#include <pthread.h>
#include "pjsua_interface.h"
#include <pjsua-lib/pjsua.h>
#include <pjlib.h>
#include "../udp_server/udp_server.h"

#include "../dependencies/utils/util.h"
#include "../dependencies/buzzer/buzzer.h"
#include "../dependencies/LED/led.h"
#include "../dependencies/ipaddr/ipaddr.h"
#include "../dependencies/interface/interface.h"

#define THIS_FILE	"pjsuaInterface"

#define SIP_DOMAIN	"192.168.7.2" //make this automatic look into sample app 
#define SIP_USER_COMPUTER "debian"
#define SIP_USER_NETWORK  "san"
#define CURRENT_URI_SIZE 1024
static pthread_t pjsuaThreadPID = -1;
static pthread_cond_t * shutdownRequest = NULL;
static pthread_mutex_t * shutdownLock = NULL;

static pthread_mutex_t call_mutex;
static pjsua_call_id current_call = PJSUA_INVALID_ID;


static pthread_mutex_t pickup_call_mutex;
static int pickup_call;


static pthread_mutex_t status_call_mutex;
static int status_call;


static pthread_mutex_t tx_volume_mutex;
static int tx_volume;


static pthread_mutex_t current_uri_mutex;
static char *current_uri;

static pjsua_acc_id linphone_account_id;
static pjsua_acc_id network_account_id;



static pj_status_t status;

//

static void sendShutdownRequest(void){
    pthread_mutex_lock(shutdownLock);
    pthread_cond_signal(shutdownRequest);
    pthread_mutex_unlock(shutdownLock);
}

//return 0 if no incoming call to pick up 
//return 1 if incoming call and is picked up
//pickup value reset in on call state when it is picked up;
int pjsua_interface_pickup_incoming_call(int ack){

    
    pthread_mutex_lock(&pickup_call_mutex);
    if(pickup_call==0){

        pickup_call=ack;
        pthread_mutex_unlock(&pickup_call_mutex);
        return 1;
    }
    
    pthread_mutex_unlock(&pickup_call_mutex);
    return 0;
}


//wait for 10 seconds to wait for user input to pick up call
static void call_incoming(){

    int count=0;

    while(count<100){

        pthread_mutex_lock(&pickup_call_mutex);
        if(pickup_call){
        pthread_mutex_unlock(&pickup_call_mutex);

        break;

        }
        pthread_mutex_unlock(&pickup_call_mutex);

        count++;
        sleepMs(100);
    }

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
    if (ci.state == PJSIP_INV_STATE_INCOMING)
    {

        pthread_mutex_lock(&status_call_mutex);
        status_call = 1;
        pthread_mutex_unlock(&status_call_mutex);

        PJ_LOG(3, (THIS_FILE, "Call is ringing, Incoming"));
    }

    buzzer_ring_on();
    call_incoming();
    buzzer_ring_off();

    /*  answer incoming calls with 200/OK only if pickup value has been changed to 1*/
    pthread_mutex_lock(&pickup_call_mutex);
    if(pickup_call==1){
        pthread_mutex_unlock(&pickup_call_mutex);
        pjsua_call_answer(call_id, 200, NULL, NULL);
        return;
    }
    pthread_mutex_unlock(&pickup_call_mutex);
    pjsua_call_answer(call_id, PJSIP_SC_BUSY_HERE, NULL, NULL);
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
     
    //////site this code
       /* Extract call information */
    pj_str_t remote_address = ci.remote_info;
    pj_str_t remote_contact = ci.remote_contact;
    pj_str_t remote_uri = ci.remote_info;

    /* Use the call information */
    printf("Remote address: %.*s\n", (int)remote_address.slen, remote_address);
    printf("Remote contact: %.*s\n", (int)remote_contact.slen, remote_contact);
    printf("Remote URI: %.*s\n", (int)remote_uri.slen, remote_uri);
    memset(current_uri, 0, sizeof(CURRENT_URI_SIZE));
    sprintf(current_uri,"%s",remote_uri); 
    /////////////


    //allows for new incoming call to be picked up after current in session call is diconected
    if(call_id==current_call){


        if(ci.state == PJSIP_INV_STATE_DISCONNECTED){
            pthread_mutex_lock(&call_mutex);
            current_call=PJSUA_INVALID_ID;
            pthread_mutex_unlock(&call_mutex);

            pthread_mutex_lock(&pickup_call_mutex);
            pickup_call=0;
            pthread_mutex_unlock(&pickup_call_mutex);

            pthread_mutex_lock(&status_call_mutex);
            status_call=0;
            pthread_mutex_unlock(&status_call_mutex);

            PJ_LOG(3,(THIS_FILE, "free to make and accept calls, no call in session"));
        }

    
        //icoming call is ancknowleged and pickup varibale is reset
        //this pick value is only modified once since only one call can be in session;
        if(ci.state == PJSIP_INV_STATE_CONFIRMED){

            pthread_mutex_lock(&status_call_mutex);
            status_call=2;
            pthread_mutex_unlock(&status_call_mutex);

            PJ_LOG(3,(THIS_FILE, "Call in session"));
        }


       
                
                

        
    }

    if(ci.state == PJSIP_INV_STATE_CALLING){

        pthread_mutex_lock(&status_call_mutex);
        status_call=3;
        pthread_mutex_unlock(&status_call_mutex);

        PJ_LOG(3,(THIS_FILE, "Outgoing call, Outgoing"));
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

int pjsua_interface_make_callO1(char *str){

    pthread_mutex_lock(&call_mutex);
    if(current_call!=PJSUA_INVALID_ID)    {
        pthread_mutex_unlock(&call_mutex);
        return 0;
    }
    //pj_str_t uri = pj_str("sip:san@192.168.26.128");
    pj_str_t uri = pj_str(str);
    status = pjsua_call_make_call(network_account_id, &uri, 0, NULL, NULL, &current_call);
    
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

int pjsua_interface_make_call(char *str){

    pthread_mutex_lock(&call_mutex);
    if(current_call!=PJSUA_INVALID_ID)	{
        pthread_mutex_unlock(&call_mutex);
        return 0;
    }
    //pj_str_t uri = pj_str("sip:san@192.168.26.128");
    pj_str_t uri = pj_str(str);
    status = pjsua_call_make_call(linphone_account_id, &uri, 0, NULL, NULL, &current_call);
    
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

int pjsua_interface_hang_up_call(){


    pjsua_call_hangup_all();
   
    return 1;
}

void pjsua_interface_set_volume(int set_volume)
{

    float vol = 0;

    if (set_volume < 0)
    {

        set_volume = 0;
    }

    if (set_volume > 100)
    {

        set_volume = 100;
    }

    vol = set_volume / 100.0;

    pthread_mutex_lock(&tx_volume_mutex);
    tx_volume = set_volume;
    PJ_LOG(3,(THIS_FILE, "volume changed to: %d -> %f",tx_volume,vol));
    pjsua_conf_adjust_tx_level(0, vol);
    pthread_mutex_unlock(&tx_volume_mutex);
}

int pjsua_interface_get_volume(){

    int curr_vol;

    pthread_mutex_lock(&tx_volume_mutex);
    curr_vol=tx_volume;
    pthread_mutex_unlock(&tx_volume_mutex);

    return curr_vol;
}

int pjsua_interface_get_status_call(){

    int getStatus=0;
    pthread_mutex_lock(&status_call_mutex);
    getStatus=status_call;
    pthread_mutex_unlock(&status_call_mutex);
    switch (getStatus)
    {
    case 0:
        //PJ_LOG(3,(THIS_FILE, "none,free to call ,no status to report"));
        break;
    case 1:

       // PJ_LOG(3,(THIS_FILE, "incoming call, ringing"));
        break;
    case 2:
       // PJ_LOG(3,(THIS_FILE, "call in session"));
        break;
    case 3:
      //  PJ_LOG(3,(THIS_FILE, "outgoing call"));
        break;

    default:
        break;
    }

    return getStatus;
   
}




//https://www.pjsip.org/pjlib/docs/html/page_pjlib_thread_test.htm
static void* thread_proc(){

    pj_thread_desc desc;
    pj_thread_t *this_thread;
    unsigned id;
    pj_status_t rc;

    PJ_UNUSED_ARG(id);
    PJ_LOG(3,(THIS_FILE, "RUNNING THREAD PROC "));

    pj_bzero(desc,sizeof(desc));


    //rc=pj_thread_register("thread",desc,&this_thread);

    if(rc!=PJ_SUCCESS){

        PJ_LOG(3,(THIS_FILE, "Registering thread proc failed "));
        return NULL;
    }
    
    /* Test that pj_thread_this() works */
    this_thread = pj_thread_this();
    if (this_thread == NULL) {
    PJ_LOG(3,(THIS_FILE, "...error: pj_thread_this() returns NULL!"));
    return NULL;
    }
    /* Test that pj_thread_get_name() works */
    if (pj_thread_get_name(this_thread) == NULL) {
    PJ_LOG(3,(THIS_FILE, "...error: pj_thread_get_name() returns NULL!"));
    return NULL;
    }
    /* Main loop */

    return NULL;
}





static int pjsua_thread(void){

   
    

    /* Create pjsua first! */
    status = pjsua_create();
    if (status != PJ_SUCCESS) error_exit("Error in pjsua_create()", status);

     
    /* Initialize PJLIB-UTIL */
    status = pjlib_util_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    

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
    pjsua_conf_adjust_tx_level(0, 1.0);
    tx_volume=100;

    if (status != PJ_SUCCESS) error_exit("Error adding sound device", status);
    /* Get the current input (microphone) volume */
  
    //register without sip server account
    pjsua_acc_config linphone_config;
    pjsua_acc_config_default(&linphone_config);
    linphone_config.id = pj_str("sip:" SIP_USER_COMPUTER "@" SIP_DOMAIN);
    status = pjsua_acc_add(&linphone_config, PJ_TRUE, &linphone_account_id);
    if (status != PJ_SUCCESS)  error_exit("Error first account", status);

    /*

    pass a buffer to the ip addr fucntion of lenght n
    
    */ 
   char buffer[16];//max ip lenght ipv4
   strcpy(buffer,"e");
   IP_get_eth0_ip(buffer);
   if(buffer[0]=='e'){
    PJ_LOG(3,(THIS_FILE,"ETH 0 NOT ASSINGED IP ADDRESS"));
   }else{
    PJ_LOG(3,(THIS_FILE,"ETH 0 ASSINGED IP ADDRESS: %s \n",buffer));


    pjsua_acc_config network_account_config;
    pjsua_acc_config_default(&network_account_config);
    char uri_buffer[1024];
    sprintf(uri_buffer,"sip:%s@%s",SIP_USER_NETWORK,buffer);
    //network_account_config.id = pj_str("sip:debian1@192.168.1.129");
    network_account_config.id = pj_str(uri_buffer);
    status = pjsua_acc_add(&network_account_config, PJ_TRUE, &network_account_id);
    if (status != PJ_SUCCESS)  error_exit("Error second account", status);


   }

    
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


    //create worker thread thread_proc
    pj_caching_pool cp;
    pj_pool_t *pool;
    pj_thread_t *thread , *network, *interface;
    pj_caching_pool_init(&cp, NULL, 1024);
    pj_status_t rc;
    pool = pj_pool_create(&cp.factory, NULL, 4000, 4000, NULL);
    if (!pool) return -1000;
    
    rc = pj_thread_create(pool, "thread", (pj_thread_proc *)&thread_proc,
                          NULL,
                          PJ_THREAD_DEFAULT_STACK_SIZE,
                          0,
                          &thread);
    if (rc != PJ_SUCCESS)
    {
        
        error_exit("Error creating worker thread", rc);
    };

     pj_thread_join(thread);

/*  threads 


    rc = pj_thread_create(pool, "network", (pj_thread_proc *)&udp_receive_thread,
                          NULL,
                          PJ_THREAD_DEFAULT_STACK_SIZE,
                          0,
                          &network);

    if (rc != PJ_SUCCESS)
    {
        
        error_exit("Error creating network thread", rc);
    };
    
    
   
    rc = pj_thread_create(pool, "interface", (pj_thread_proc *)&IFace_runner,
                          NULL,
                          PJ_THREAD_DEFAULT_STACK_SIZE,
                          0,
                          &interface);

    if (rc != PJ_SUCCESS)
    {
        
        error_exit("Error creating interface thread", rc);
    };
  
   
*/
    /* If URL is specified, make call to the URL. */

    /* Wait until user press "q" to quit. */
    for (;;) {
        char option[10];

        puts("Press 'h' to hangup all calls, 'q' to quit, c to call s to status p to pick d to decline");
        if (fgets(option, sizeof(option), stdin) == NULL) {
            puts("EOF while reading stdin, will quit now..");
            break;
        }

        if (option[0] == 'q')
            break;

        if (option[0] == 'h')
            
            pjsua_call_hangup_all();

        if (option[0] == 's')
            pjsua_interface_get_status_call();

        if (option[0] == 'p')
            pjsua_interface_pickup_incoming_call(1);
        
        if (option[0] == 'd')
            pjsua_interface_pickup_incoming_call(2);
            
        if (option[0]== 'c') {
            //pjsua_conf_adjust_tx_level(0, 0.01);
            // pj_str_t uri = pj_str("sip:san@192.168.26.128");

            // status = pjsua_call_make_call(linphone_account_id, &uri, 0, NULL, NULL, NULL);
           // if (status != PJ_SUCCESS) error_exit("Error making call", status);
            if(pjsua_interface_make_call("sip:san@192.168.26.128")){

                 PJ_LOG(3,(THIS_FILE, "make call succesful, call is active"));
            }else{
                 PJ_LOG(3,(THIS_FILE, "make call unsuccesful, call in progress or invalid uri"));
            }
        
        }
     
        if (option[0]== 'x') {
            
            // pj_str_t uri = pj_str("sip:ryan@192.168.1.207");

            // status = pjsua_call_make_call(acc_id2, &uri, 0, NULL, NULL, NULL);
            // if (status != PJ_SUCCESS) error_exit("Error making call", status);

            if(pjsua_interface_make_callO1("sip:ryan@192.168.1.207")){

                 PJ_LOG(3,(THIS_FILE, "make call succesful, call is active"));
            }else{
                 PJ_LOG(3,(THIS_FILE, "make call unsuccesful, call in progress or invalid uri"));
            }
     
        }


        if (option[0]== 'v') {
            
            if(pjsua_interface_get_volume()==100){

                pjsua_interface_set_volume(50);

            }else{

                pjsua_interface_set_volume(100);
            }

           
        }

        //print uri of remote call
        if (option[0]== 'u') {

            
            
            PJ_LOG(3,(THIS_FILE, "REMOTE URI: %s",current_uri));
            
           
        }
            //make call
    }

    /* Destroy pjsua */

   
    sendShutdownRequest();
    //pj_thread_join(network); 
    //IFace_endThread();
    //pj_thread_join(interface); 
    //IFace_cleanup();
    pjsua_destroy();
    return 0;
}




int pjsua_interface_init(pthread_cond_t * cond, pthread_mutex_t * lock){

    shutdownRequest = cond;
    shutdownLock = lock;
    pickup_call=0;
    status_call=0;
    current_uri=malloc(CURRENT_URI_SIZE*sizeof(char));

    //IFace_initialize();
    pthread_mutex_init(&call_mutex, NULL);
    pthread_mutex_init(&pickup_call_mutex, NULL);
    pthread_mutex_init(&status_call_mutex,NULL);
    pthread_mutex_init(&tx_volume_mutex,NULL);
    pthread_mutex_init(&current_uri_mutex,NULL);
    

    if(pthread_create(&pjsuaThreadPID, NULL, (void*)&pjsua_thread, NULL) != 0){
        perror("Error in creating ");
        sendShutdownRequest();
        return -1;
    }
    LED_startUp();
    return 1;
}

int pjsua_interface_cleanup(void){
    
    free(current_uri);
    pthread_mutex_destroy(&call_mutex);    
    pthread_mutex_destroy(&pickup_call_mutex);
    pthread_mutex_destroy(&status_call_mutex);
    pthread_mutex_destroy(&tx_volume_mutex);
    pthread_mutex_destroy(&current_uri_mutex);
    LED_cleanUp();
    if(pthread_join(pjsuaThreadPID, NULL) != 0){
        perror("Error in joining the phsua thread.");
    }
   
    return 1;
}