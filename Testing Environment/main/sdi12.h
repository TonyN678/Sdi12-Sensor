#ifndef SDI12_HANDLER_H
#define SDI12_HANDLER_H

#include <Arduino.h>

void sdi12Init(char address, int dirPin);
void sdi12Handle();

void sdiSend(String response);
void parseCommand(String cmd);

#endif
// contains all functions and variables related to SDI-12 communication, including initialization, command parsing, and response sending.