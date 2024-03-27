#ifndef _NOM_H_
#define _NOM_H_

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
#else
    #include <ftw.h>
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/stat.h>
    #include <sys/wait.h>
#endif

#ifdef _WIN32
    #define PATH_SEP '\\'
#else
    #define PATH_SEP '/'
#endif

#ifndef NOM_ALLOC
    #define NOM_ALLOC(size) malloc(size)
#endif

#ifndef NOM_REALLOC
    #define NOM_REALLOC(ptr, size) realloc(ptr, size)
#endif

#ifndef NOM_FREE
    #define NOM_FREE(ptr) free(ptr)
#endif

#ifndef NOM_ASSET
    #include <assert.h>
    #define NOM_ASSET(exp) assert(exp)
#endif

#define DA_INIT_CAP 512

#define VA_ARGS_FOREACH(args, arg, type, param, body)    \
    do {                                                 \
        va_start(args, param);                           \
            type arg = va_arg(args, type);               \
            while (arg != NULL) {                        \
                body;                                    \
                arg = va_arg(args, type);                \
            }                                            \
        va_end(args);                                    \
    } while (0);

#define DA_APPEND(da, item)                                              \
    do {                                                                 \
        if ((da)->Count >= (da)->Size) {                                 \
            (da)->Size = (da)->Size == 0 ? DA_INIT_CAP : (da)->Size * 2; \
            (da)->Items = NOM_REALLOC((da)->Items, (da)->Size);          \
            NOM_ASSET((da)->Items != NULL);                              \
        }                                                                \
        (da)->Items[(da)->Count] = item;                                 \
        (da)->Count += 1;                                                \
    } while (0)

#define DA_FOREACH(da, type, element, body)     \
    do {                                        \
        for (int i = 0; i < (da)->Count; i++) { \
            type element = (da)->Items[i];      \
            body;                               \
        }                                       \
    } while (0);

#define SB_APPEND(sb, chr) DA_APPEND(sb, chr)
#define SB_APPEND_CSTR(sb, ...) __Nom_SB_AppendCstr(sb, __VA_ARGS__, NULL);
#define SB_APPEND_NULL(sb) DA_APPEND(sb, '\0')

#define CONCAT(...) __Nom_Concat(0, __VA_ARGS__, NULL)
#define CONCAT_SEP(sep, ...) __Nom_ConcatSep(sep, __VA_ARGS__, NULL)

#define PATH(...) __Nom_ConcatSep(PATH_SEP, __VA_ARGS__, NULL)

#define NOM_ERROR(...) __Nom_Log(LOG_LEVEL_ERROR, __VA_ARGS__)
#define NOM_WARN(...) __Nom_Log(LOG_LEVEL_WARN, __VA_ARGS__)
#define NOM_INFO(...) __Nom_Log(LOG_LEVEL_INFO, __VA_ARGS__)

#define Nom_CmdAppend(cmd, ...) __Nom_CmdAppend(cmd, __VA_ARGS__, NULL);

#ifdef _WIN32
    typedef HANDLE Pid;
    #define NOM_INVALID_PID INVALID_HANDLE_VALUE
#else
    typedef int Pid;
    #define NOM_INVALID_PID -1
#endif

#ifndef DT_REG
    #define DT_REG 8
    #define DT_DIR 4
#endif

typedef long i64;
typedef unsigned long u64;
typedef int i32;
typedef unsigned int u32;
typedef short i16;
typedef unsigned short u16;
typedef char i8;
typedef unsigned char u8;

typedef enum {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO
} Nom_LogLevel;

typedef struct {
    char** Items;
    u32 Count;
    u32 Size;
} Nom_Cmd;

typedef struct {
    char* Items;
    u32 Count;
    u32 Size;
} Nom_SB;

// ------------------------------------------
// ------------------ FILE ------------------
// ------------------------------------------

#define Nom_Mkdir(...) __Nom_Mkdir(0, __VA_ARGS__, NULL)
#define Nom_TouchFile(...) __Nom_TouchFile(0, __VA_ARGS__, NULL)

#define Nom_RemoveFile(...) __Nom_RemoveFile(0, __VA_ARGS__, NULL)
#define Nom_RemoveDir(...) __Nom_RemoveDir(0, __VA_ARGS__, NULL)

FILE* Nom_FOpen(const char* Path, const char* mode);

int __Nom_Mkdir(int Ignore, ...);
int __Nom_TouchFile(int Ignore, ...);

int __Nom_RemoveFile(int Ignore, ...);

int Nom_Move(const char* Path, const char* NewPath);
#define Nom_Rename(Path, NewPath) Nom_Move(Path, NewPath)

int Nom_Readdir(const char* Path, char** Buffer);

int Nom_GetDirFiles(const char* Path, char** Buffer);
int Nom_GetDirDirs(const char* Path, char** Buffer);

int Nom_ReadFile(const char* Path, char* Buffer);
int Nom_WriteFile(const char* Path, const char* Buffer, _Bool Append);

_Bool Nom_Exist(const char* Path);

// ------------------------------------------
// ------------------- API ------------------
// ------------------------------------------

void __Nom_Log(Nom_LogLevel level, const char* msg, ...);

void __Nom_SB_AppendCstr(Nom_SB* sb, ...);

const char* __Nom_Concat(int Ignore, ...);
const char* __Nom_ConcatSep(const char Sep, ...);

void __Nom_CmdAppend(Nom_Cmd* cmd, ...);

void Nom_ShowCmd(Nom_Cmd cmd, Nom_SB* sb);

#define Nom_CmdRun(cmd) Nom_CmdRun_Sync(cmd)

Pid Nom_CmdRun_Async(Nom_Cmd cmd);
int Nom_CmdRun_Sync(Nom_Cmd cmd);

int Nom_Wait(Pid proc);

void Nom_FreeSB(Nom_SB* sb);
void Nom_FreeCmd(Nom_Cmd* cmd);

#endif // _NOM_H_

#ifdef _NOM_IMPLEMENTATION_

// ------------------------------------------
// ------------------ FILE ------------------
// ------------------------------------------

FILE* Nom_FOpen(const char* Path, const char* mode) {
    #ifdef _WIN32
        FILE* file;
        fopen_s(&file, Path, mode);
        return file;
    #else
        return fopen(Path, mode);
    #endif
}

int __Nom_Mkdir(int Ignore, ...) {
    va_list args;
    VA_ARGS_FOREACH(args, Path, const char*, Ignore, {
        #ifdef _WIN32
            if (mkdir(Path)) {
                NOM_ERROR("Unable to Make Dir: %s Error: %s", Path, strerror(errno));
                return -1;
            }
        #else
            if (mkdir(Path, 0755) < 0) {
                NOM_ERROR("Unable to Make Dir: %s Error: %s", Path, strerror(errno));
                return -1;
            }
        #endif
    })

    return 0;
}

int __Nom_TouchFiles(int Ignore, ...) {
    va_list args;
    VA_ARGS_FOREACH(args, Path, const char*, Ignore, {
        FILE* File = Nom_FOpen(Path, "w");

        if (File == NULL) {
            NOM_ERROR("Unable to Make File: %s Error: %s", Path, strerror(errno));
            return -1;
        }

        fclose(File);
    })

    return 0;
}

int __Nom_RemoveFile(int Ignore, ...) {
    va_list args;
    VA_ARGS_FOREACH(args, Path, const char*, Ignore, {
        if (remove(Path) < 0) {
            NOM_ERROR("Unable to Remove File: %s Error: %s", Path, strerror(errno));
            return -1;
        }
    })

    return 0;
}

#ifndef _WIN32
    int __Nom_HELPER_DIR_Remove(const char *fpath, const struct stat *sb, int typeflag) {
        int rv = remove(fpath);
        return rv;
    }
#endif

int __Nom_RemoveDir(int Ignore, ...) {
    va_list args;
    VA_ARGS_FOREACH(args, Path, const char*, Ignore, {
        #ifdef _WIN32
            HANDLE FileHandle = NULL;
            WIN32_FIND_DATA ffd;

            const char* DirPath = PATH(Path, "*");
            FileHandle = FindFirstFile(DirPath, &ffd);

            if (FileHandle == INVALID_HANDLE_VALUE) {
                NOM_ERROR("Unable to Remove Dir: %s Error: %lu", Path, GetLastError());
                return -1;
            }

            do {
                if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    Nom_RemoveDir(ffd.cFileName);
                } else {
                    remove(ffd.cFileName);
                }
            } while (FindNextFile(FileHandle, &ffd));

            FindClose(FileHandle);

            if (!RemoveDirectoryA(Path)) {
                NOM_ERROR("Unable to Remove Dir: %s Error: %lu", Path, GetLastError());
                return -1;
            }
        #else
            if (ftw(Path, __Nom_HELPER_DIR_Remove, FTW_D | FTW_DNR | FTW_F | FTW_NS) < 0) {
                NOM_ERROR("Unable to Remove Dir: %s Error: %s", Path, strerror(errno));
                return -1;
            }
        #endif
    })

    return 0;
}

int Nom_Move(const char* Path, const char* NewPath) {
    if (rename(Path, NewPath) < 0) {
        NOM_ERROR("Unable to Move File: %s to %s Error: %s", Path, NewPath, strerror(errno));
        return -1;
    }

    return 0;
}

int Nom_Readdir(const char* Path, char** Buffer) {
    #ifdef _WIN32
            HANDLE FileHandle = NULL;
            WIN32_FIND_DATA ffd;

            const char* DirPath = PATH(Path, "*");
            FileHandle = FindFirstFile(DirPath, &ffd);

            if (FileHandle == INVALID_HANDLE_VALUE) {
                NOM_ERROR("Unable to Access Dir: %s Error: %lu", Path, GetLastError());
                return -1;
            }

            i32 i = 0;

            do {
                if (Buffer[i] != NULL) {
                    free(Buffer[i]);
                }

                Buffer[i] = NOM_ALLOC(256);
                memcpy(Buffer[i], ffd.cFileName, strlen(ffd.cFileName));

                i += 1;
            } while (FindNextFile(FileHandle, &ffd));

            FindClose(FileHandle);
    #else
        DIR* dir;
        struct dirent* ent;

        dir = opendir(Path);

        if (dir == NULL) {
            NOM_ERROR("Unable to Open Dir: %s Error: %s", Path, strerror(errno));
            return -1;
        }

        i32 i = 0;
        ent = readdir(dir);
        while (ent != NULL) {
            if (Buffer != NULL) {
                free(Buffer[i]);
            }

            Buffer[i] = NOM_ALLOC(256);
            memcpy(Buffer[i], ent->d_name, strlen(ent->d_name));

            ent = readdir(dir);
            i += 1;
        }

        closedir(dir);
    #endif

    return 0;
}

int Nom_GetDirFiles(const char* Path, char** Buffer) {
    #ifdef _WIN32
        HANDLE FileHandle = NULL;
        WIN32_FIND_DATA ffd;

        const char* DirPath = PATH(Path, "*");
        FileHandle = FindFirstFile(DirPath, &ffd);

        if (FileHandle == INVALID_HANDLE_VALUE) {
            NOM_ERROR("Unable to Access Dir: %s Error: %s", Path, strerror(errno));
            return -1;
        }

        i32 i = 0;

        do {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                if (Buffer[i] != NULL) {
                    free(Buffer);
                }

                Buffer[i] = NOM_ALLOC(256);
                memcpy(Buffer[i], ffd.cFileName, strlen(ffd.cFileName));

                i += 1;
            }
        } while (FindNextFile(FileHandle, &ffd));

        FindClose(FileHandle);
    #else
        DIR* dir;
        struct dirent* ent;

        dir = opendir(Path);

        if (dir == NULL) {
            NOM_ERROR("Unable to Open Dir: %s Error: %s", Path, strerror(errno));
            return -1;
        }

        i32 i = 0;
        ent = readdir(dir);
        while (ent != NULL) {
            if (ent->d_type == DT_REG) {
                if (Buffer[i] != NULL) {
                    free(Buffer);
                }

                Buffer[i] = NOM_ALLOC(256);
                memcpy(Buffer[i], ent->d_name, strlen(ent->d_name));

                i += 1;
            }

            ent = readdir(dir);
        }

        closedir(dir);
    #endif

    return 0;
}

int Nom_GetDirDirs(const char* Path, char** Buffer) {
    #ifdef _WIN32
        HANDLE FileHandle = NULL;
        WIN32_FIND_DATA ffd;

        const char* DirPath = PATH(Path, "*");
        FileHandle = FindFirstFile(DirPath, &ffd);

        if (FileHandle == INVALID_HANDLE_VALUE) {
            NOM_ERROR("Unable to Access Dir: %s Error: %s", Path, strerror(errno));
            return -1;
        }

        i32 i = 0;

        do {
            if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                if (Buffer[i] != NULL) {
                    free(Buffer);
                }

                Buffer[i] = NOM_ALLOC(256);
                memcpy(Buffer[i], ffd.cFileName, strlen(ffd.cFileName));

                i += 1;
            }
        } while (FindNextFile(FileHandle, &ffd));

        FindClose(FileHandle);
    #else
        DIR* dir;
        struct dirent* ent;

        dir = opendir(Path);

        if (dir == NULL) {
            NOM_ERROR("Unable to Open Dir: %s Error: %s", Path, strerror(errno));
            return -1;
        }

        i32 i = 0;
        ent = readdir(dir);
        while (ent != NULL) {
            if (ent->d_type == DT_DIR) {
                if (Buffer[i] != NULL) {
                    free(Buffer);
                }

                Buffer[i] = NOM_ALLOC(256);
                memcpy(Buffer[i], ent->d_name, strlen(ent->d_name));
                i += 1;
            }

            ent = readdir(dir);
        }

        closedir(dir);
    #endif

    return 0;
}

int Nom_ReadFile(const char* Path, char* Buffer) {
    FILE* file = Nom_FOpen(Path, "r");

    if (file == NULL) {
        NOM_ERROR("Unable to Read File: %s Error: %s", Path, strerror(errno));
        return -1;
    }

    fseek(file, 0, SEEK_END);
    i32 Lenght = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (Buffer != NULL) {
        free(Buffer);
    }

    Buffer = NOM_ALLOC(Lenght);
    fread(Buffer, sizeof (char), Lenght, file);

    fclose(file);

    return 0;
}

int Nom_WriteFile(const char* Path, const char* Buffer, _Bool Append) {
    FILE* file = {0};
    
    if (Append) {
        file = Nom_FOpen(Path, "a");
    } else {
        file = Nom_FOpen(Path, "a");
    }

    if (file == NULL) {
        NOM_ERROR("Unable to Read File: %s Error: %s", Path, strerror(errno));
        return -1;
    }

    fseek(file, 0, SEEK_END);
    i32 Lenght = ftell(file);
    fseek(file, 0, SEEK_SET);

    fwrite(Buffer, sizeof (char), Lenght, file);
    fclose(file);

    return 0;
}

_Bool Nom_Exist(const char* Path) {
    if (access(Path, F_OK) == 0) {
        return true;
    } else {
        return false;
    }
}

// ------------------------------------------
// ------------- Implementation -------------
// ------------------------------------------

void __Nom_Log(Nom_LogLevel level, const char* msg, ...) {
    char Prefix[9] = {0};
    char Out[1000] = {0};

    switch (level) {
        case LOG_LEVEL_ERROR: memcpy(Prefix, "[ERROR] ", 9); break;
        case LOG_LEVEL_WARN: memcpy(Prefix, "[WARN ] ", 9); break;
        case LOG_LEVEL_INFO: memcpy(Prefix, "[INFO ] ", 9); break;
    }

    va_list args;
    va_start(args, msg);
        vsnprintf(Out, 1000, msg, args);
    va_end(args);

    char FinalOut[1010] = {0};
    sprintf(FinalOut, "%s%s\n", Prefix, Out);
    printf(FinalOut);
}

void __Nom_SB_AppendCstr(Nom_SB* sb, ...) {
    va_list args;
    VA_ARGS_FOREACH(args, cstr, const char*, sb, {
        while (*cstr != '\0') {
            SB_APPEND(sb, *cstr);
            cstr += 1;
        }
    })
}

const char* __Nom_Concat(int Ignore, ...) {
    Nom_SB sb = {0};

    va_list args;
    VA_ARGS_FOREACH(args, Str, const char*, Ignore, {
        SB_APPEND_CSTR(&sb, Str);
    })

    SB_APPEND_NULL(&sb);

    return sb.Items;
}

const char* __Nom_ConcatSep(const char Sep, ...) {
    Nom_SB sb = {0};

    va_list args;
    i32 i = 0;

    VA_ARGS_FOREACH(args, Str, const char*, Sep, {
        if (i != 0) SB_APPEND(&sb, Sep);
        SB_APPEND_CSTR(&sb, Str);

        i += 1;
    })

    SB_APPEND_NULL(&sb);

    return sb.Items;
}

void __Nom_CmdAppend(Nom_Cmd* cmd, ...) {
    va_list args;
    VA_ARGS_FOREACH(args, arg, const char*, cmd, {
        DA_APPEND(cmd, arg);
    })
}

void Nom_ShowCmd(Nom_Cmd cmd, Nom_SB* sb) {
    for (int i = 0; i < cmd.Count; i++) {
        SB_APPEND_CSTR(sb, cmd.Items[i]);
        SB_APPEND(sb, ' ');
    }

    SB_APPEND_NULL(sb);
}

Pid Nom_CmdRun_Async(Nom_Cmd cmd) {
    Nom_SB sb = {0};

    Nom_ShowCmd(cmd, &sb);
    NOM_INFO("Running Cmd: %s", sb.Items);

    #ifdef _WIN32
        STARTUPINFO StartUpInfo;
        ZeroMemory(&StartUpInfo, sizeof(StartUpInfo));
        StartUpInfo.cb = sizeof(STARTUPINFO);
        
        StartUpInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        StartUpInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        StartUpInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        StartUpInfo.dwFlags |= STARTF_USESTDHANDLES;

        PROCESS_INFORMATION ProcessInfo;
        ZeroMemory(&ProcessInfo, sizeof(PROCESS_INFORMATION));

        BOOL ProcCreate = CreateProcessA(
            NULL,
            sb.Items,
            NULL, NULL, TRUE, 0, NULL, NULL,
            &StartUpInfo,
            &ProcessInfo
        );

        if (!ProcCreate) {
            NOM_ERROR("Could not create child process: %lu", GetLastError());
            return NOM_INVALID_PID;
        }

        CloseHandle(ProcessInfo.hThread);

        return ProcessInfo.hProcess;
    #else
        Pid ChildPid = fork();

        if (ChildPid < 0) {
            NOM_ERROR("Failed to fork child process: %s", strerror(errno));
            return NOM_INVALID_PID;
        }

        if (ChildPid == 0) {
            if (execvp(cmd.Items[0], (char* const*)cmd.Items) < 0) {
                NOM_ERROR("Failed to run child process: %i, Error: %s", ChildPid, strerror(errno));
                exit(126);
            }
        }

        return ChildPid;
    #endif
}

int Nom_CmdRun_Sync(Nom_Cmd cmd) {
    Pid proc = Nom_CmdRun_Async(cmd);
    if (Nom_Wait(proc) < 0) {
        return -1;
    }

    return 0;
}

int Nom_Wait(Pid proc) {
    if (proc == NOM_INVALID_PID) return -1;

    #ifdef _WIN32
        DWORD result = WaitForSingleObject( proc, INFINITE);

        if (result == WAIT_FAILED) {
            NOM_ERROR("could not wait on child process: %lu", GetLastError());
            return -1;
        }

        DWORD exit_status;
        if (!GetExitCodeProcess(proc, &exit_status)) {
            NOM_ERROR("could not get process exit code: %lu", GetLastError());
            return -1;
        }

        if (exit_status != 0) {
            NOM_ERROR("command exited with exit code %lu", exit_status);
            return -1;
        }

        CloseHandle(proc);
    #else
        i32 wstatus = 0;
        i32 ExitStatus = 0;

        for (;;) {
            if (waitpid(proc, &wstatus, 0) < 0) {
                NOM_ERROR("could not wait on command (pid %i): %s", proc, strerror(errno));
                return -1;
            }

            if (WIFEXITED(wstatus)) {
                ExitStatus = WEXITSTATUS(wstatus);

                if (ExitStatus != 0) {
                    NOM_ERROR("command exited with exit code %i", ExitStatus);
                    return -1;
                }

                break;
            }

            if (WIFSIGNALED(wstatus)) {
                NOM_ERROR("command process was terminated by %s", strsignal(WTERMSIG(wstatus)));
                return -1;
            }
        }
    #endif

    return 0;
}

void Nom_FreeSB(Nom_SB* sb) {
    NOM_FREE(sb->Items);
    sb->Count = 0;
    sb->Size = 0;
}

void Nom_FreeCmd(Nom_Cmd* cmd) {
    NOM_FREE(cmd->Items);
    cmd->Count = 0;
    cmd->Size = 0;
}

#endif // _NOM_IMPLEMENTATION_