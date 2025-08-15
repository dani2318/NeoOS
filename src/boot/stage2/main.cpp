#include <stdint.h>
#include <core/Defs.hpp>
#include <core/arch/i686/VGATextDevice.hpp>
#include <core/dev/TextDevice.hpp>

arch::i686::VGATextDevice g_VGADevice;

EXPORT void ASMCALL Start(uint16_t bootDrive,void* partition){

    g_VGADevice.Clear();

    TextDevice Screen(&g_VGADevice);

    Screen.Format("Loaded Stage 2!!\n");

end:
    for(;;);

}