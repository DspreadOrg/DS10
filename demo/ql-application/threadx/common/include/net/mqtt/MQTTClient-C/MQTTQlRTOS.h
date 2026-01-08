/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
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
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#if !defined(MQTTQlRTOS_H)
#define MQTTQlRTOS_H

#include "ql_rtos.h"

typedef struct Mutex
{
	ql_mutex_t mutex;
} Mutex;

void MutexInit(Mutex*);
void MutexDeinit(Mutex* mutex);
int MutexLock(Mutex*);
int MutexUnlock(Mutex*);

typedef struct Thread
{
	ql_task_t task;
	unsigned int stack_size;
	unsigned char priority;
} Thread;

int ThreadStart(Thread*, void (*fn)(void*), void* arg);
void ThreadStop(Thread*);


#endif
