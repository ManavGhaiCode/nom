#define _NOM_IMPLEMENTATION_
#include "../nom.h"

#define CFLAGS "-Wall", "-Wextra", "-Werror"

#ifdef _WIN32
    #define Compiler "mingwc"
#else
    #define Compiler "cc"
#endif

void Compile(Nom_Cmd* cmd) {
    Nom_CmdAppend(cmd, Compiler, CFLAGS, "-c", "./main.c");

    Nom_CmdRun(*cmd);
    cmd->Count = 0;

    Nom_CmdAppend(cmd, Compiler, CFLAGS, "-c", "./hello.c");

    Nom_CmdRun(*cmd);
    cmd->Count = 0;

    Nom_CmdAppend(cmd, Compiler, CFLAGS, "-c", "./hello.h");

    Nom_CmdRun(*cmd);
    cmd->Count = 0;
}

void Link(Nom_Cmd* cmd) {
    Nom_CmdAppend(cmd, Compiler, "./hello.o", "./main.o", "-o", "hello");

    Nom_CmdRun(*cmd);
    cmd->Count = 0;
}

int main( void ) {
    Nom_Cmd cmd = {0};

    Compile(&cmd);
    Link(&cmd);

    #ifndef _WIN32
        Nom_CmdAppend(&cmd, "./hello");
    #else
        Nom_CmdAppend(&cmd, "./hello.exe");
    #endif

    Nom_CmdRun(cmd);
    Nom_FreeCmd(&cmd);

    return 0;
}