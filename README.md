# Interactive Backup Management System

A command-line application written in C that automatically creates and manages backups of directories.

The program allows the user to create backups of a selected source directory to one or more target locations and keeps them synchronized with the source directory. After the initial copy is completed, the program monitors the source directory and mirrors any changes in the backup locations.

This project was developed as part of an Operating Systems course and demonstrates practical usage of POSIX system programming.

## Features

- Create a backup of a directory to one or multiple target locations  
- Automatically create target directories if they do not exist  
- Recursive copying of directory structures and files  
- Support for symbolic links  
- Monitoring of the source directory and mirroring changes to backup targets  
- Ability to stop selected backups  
- Listing currently active backups  
- Restoring the source directory from a backup  
- Parallel processing using separate subprocesses  
- Interactive command interface

## Available Commands

### Add backup

```
add <source_path> <target_path> [target_path...]
```

Creates a backup of the source directory in one or more target directories.

Example:

```
add /home/user/project /backup/project1 /backup/project2
```

### End backup

```
end <source_path> <target_path>
```

Stops synchronization for the selected backup target.

### List backups

```
list
```

Displays all active backups and their corresponding target directories.

### Restore backup

```
restore <source_path> <target_path>
```

Restores the source directory from the selected backup.

### Exit program

```
exit
```

Terminates the program and stops all active processes.

## Technologies

- C
- POSIX API
- fork / process management
- inotify (filesystem monitoring)
- open / read / write system calls
- lstat
- realpath
- signal handling (SIGINT, SIGTERM)
