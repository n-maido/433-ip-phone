/*
    Pjsua interface for application
    Created by Sanseerat Virk

    heavily Modiefied sample application code provided at the following links
   
       
    Using the resources menitoned in the h file this was used a as a barebone tempelate
    after understanding the template this sample pjsua code was heavily modified to match our needs 
    https://www.pjsip.org/pjlib/docs/html/page_pjlib_thread_test.htm
    https://github.com/chakrit/pjsip/blob/master/pjsip-apps/src/samples/simple_pjsua

    Usage / cautions:
    all threads attempting to access the pjsua fucntions must be registerd to pjsua otherwise application will crash


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

#ifndef PJSUA_INTERFACE
#define PJSUA_INTERFACE
#define CURRENT_URI_SIZE 1024
#include <pthread.h>

//Set up pjsua and thread. Returns -1 upon failure.
int pjsua_interface_init(pthread_cond_t * cond, pthread_mutex_t * lock);


//make call fucntion given a formatted sip uri 
// returns 0 on failure, 1 on success
int pjsua_interface_make_call(char *str);

//pass 1 to pick up call and pass 2 to decline call
int pjsua_interface_pickup_incoming_call(int ack);


//hang up active call fuction make it safe if no call in session return false
int pjsua_interface_hang_up_call(void);


//0=no call in session , 1 = incoming call ringing, 2= call in session, 3=outgoing call
int pjsua_interface_get_status_call(void);


//if call in session or inocoming uri of incoming call


//link with potentiometer


//add mic amplifier functions



//add output volume control fucntion 0 to 100
void pjsua_interface_set_volume(int volume);

int pjsua_interface_get_volume();

//pass a buffer this will be filled with the ip 
void pjsua_interface_get_ip(char *buffer);

//pass a buffer this will be filled with the uri 
void pjsua_interface_get_uri(char *buffer);

//Cleanup. Returns -1 if there were any issues closing.
int pjsua_interface_cleanup(void);

#endif