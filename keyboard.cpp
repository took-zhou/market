#include <termio.h>
#include <stdio.h> 
#include <stdlib.h>
int scanKeyboard()
{
    int in;
    struct termios new_settings;
    struct termios stored_settings;
    tcgetattr(0, &stored_settings);
    new_settings = stored_settings;
    new_settings.c_lflag &= (~ICANON);
    new_settings.c_cc[VTIME] = 0;
    tcgetattr(0, &stored_settings);
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &new_settings);

    in = getchar();

    tcsetattr(0, TCSANOW, &stored_settings);
    return in;
}
//这个方法就可以，返回值是该键的ASCII码值，不需要回车的，

void monitorKeyBoard()
{
    while (true)
    {
        if (scanKeyboard() == 'z' && scanKeyboard() == 'x' && scanKeyboard() == 'c')
        {
            exit(0);
        }

    }
}
