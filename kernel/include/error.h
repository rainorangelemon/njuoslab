#ifndef ERROR_H
#define ERROR_H

#ifndef error_e
#define error_e
enum
{
	E_UNSPECIFIED = 1,
	E_BAD_ENV = 2,
	E_INVAL = 3,
	E_NO_MEM = 4,
	E_NO_FREE_ENV = 5,
	E_FAULT = 6,
	MAXERROR
};
#endif
#endif
