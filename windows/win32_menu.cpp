#include "win32_menu.h"


HMENU BuildMenu()
{
    HMENU mainMenu = CreateMenu();

    //HMENU testMenu = CreateMenu();

    //unsigned int test = 1234;
    UINT_PTR testp = 12345;
    char* testx = "Test Item\0";
    AppendMenu(mainMenu, MF_STRING | MF_ENABLED, testp,  testx);

    return mainMenu;   
}