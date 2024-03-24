# Nom

Nom stands for NoMake.
Get it as you are using a build tool without make. The name is officially Nom... totaly different!

Nom was inspired by nob (by Tsoding).

So the main is idea is that you only need a Compiler for mking a project in C or C++ No need to install make, permake, cmake, ninja and all these make systems for all of these tool are very convaluted (make is fine but, the other tools are) and restrictive you can all a shell script on tool off these but, it gets very hard to use sheel script for things and it also get hard to under stand what is going on.

So you can run the commands:

```shell
    cc -o nom nom.c
    ./nom
```

and that's it you are done. the file nom will go and build every thing for you. You don't need to do anything else, at all.

So you can make *.c file like:

```c
#define _NOM_IMPLEMENTATION_
#include "nom.h"

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
```

and then you just don't need to think about the build system.