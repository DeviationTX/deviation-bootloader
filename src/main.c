#include "common.h"

int PWR_CheckPowerSwitch() {
    return 1;
}

void PWR_Shutdown()
{
}

int main()
{
    USB_Enable(1);
    //Disable USB Exit
    while(1) {
        if(PWR_CheckPowerSwitch())
            PWR_Shutdown();
    }
}
