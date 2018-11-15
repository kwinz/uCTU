#pragma once

#include <stdint.h>
#include <util.h>

error_t halWT41FcUartInit(void (*sndCallback)(), void (*rcvCallback)(uint8_t));
error_t halWT41FcUartSend(uint8_t byte);