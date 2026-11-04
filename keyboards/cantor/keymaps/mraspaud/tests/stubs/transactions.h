#pragma once
#include <stdint.h>
#include <stdbool.h>
#define SHORE_SYNC 1
void transaction_register_rpc(int,void(*)(uint8_t,const void*,uint8_t,void*));
bool transaction_rpc_send(int,uint8_t,const void*);
