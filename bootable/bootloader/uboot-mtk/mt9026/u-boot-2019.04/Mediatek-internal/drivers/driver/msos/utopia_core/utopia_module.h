#ifndef _UTOPIA_MODULE_H_
#define _UTOPIA_MODULE_H_

#include "MsTypes.h"

MS_U32 _UtopiaModule_Ioctl(void** pInstant, MS_U32 u32Cmd, MS_U32* pu32Args);
MS_U32 UTOPIARegisterToUtopia(FUtopiaOpen ModuleType);

#endif
