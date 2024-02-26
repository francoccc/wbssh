#pragma once

#include <stdio.h>

#define ERR(e) \
	 printf("%s:%s failed: %d [%s@%ld]\n",__FUNCTION__, e, WSAGetLastError(), __FILE__, __LINE__)