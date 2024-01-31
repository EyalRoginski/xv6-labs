#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"
#include "kernel/fs.h"

// From ls.c, but I changed the name to something more sensible and removed
// the padding.
char *filename_from_path(char *path)
{
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    return p;
}

int find(char *where, char *name)
{
    int file = open(where, 0);
    if (file < 0)
    {
        printf("find: cannot open %s\n", where);
        return 1;
    }

    struct stat where_stat;
    if (fstat(file, &where_stat) < 0)
    {
        printf("find: cannot stat %s\n", where);
        return 1;
    }

    switch (where_stat.type)
    {
    case T_DEVICE:
    case T_FILE:
        if (strcmp(filename_from_path(where), name) == 0)
        {
            printf("%s\n", where);
        }
        break;
    case T_DIR:
        struct dirent subfile_dirent;

        while (read(file, &subfile_dirent, sizeof(struct dirent)) == sizeof(struct dirent))
        {
            if (subfile_dirent.inum == 0)
            {
                continue;
            }

            if (strcmp(subfile_dirent.name, ".") == 0 || strcmp(subfile_dirent.name, "..") == 0)
            {
                continue;
            }

            char path_to_file[512];

            strcpy(path_to_file, where);
            int length = strlen(path_to_file);
            path_to_file[length] = '/';
            path_to_file[length + 1] = '\0';
            memmove(path_to_file + strlen(path_to_file), subfile_dirent.name, DIRSIZ);
            path_to_file[strlen(path_to_file) + DIRSIZ] = '\0';
            find(path_to_file, name);
        }
    }

    close(file);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("find: not enough arguments\n");
        exit(1);
    }

    find(argv[1], argv[2]);
    exit(0);
}