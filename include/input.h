/**
 * Author: Bobby Dilley
 * Created: 2019
 * SPDX-FileCopyrightText: 2019 Bobby Dilley <bobby@dilley.uk>
 * SPDX-License-Identifier: GPL-3.0-or-later
 **/

#ifndef INPUT_H_
#define INPUT_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "jvs.h"

int initInput(JVSCapabilities *sentCapabilities, char *name, int analogueFuzz);
int closeInput();
int updateSwitches(char *switches);
int updateAnalogues(char *analogues);
int sendUpdate();

#endif // INPUT_H_
