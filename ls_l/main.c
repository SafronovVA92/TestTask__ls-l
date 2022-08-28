#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/param.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cvector.h"


// Constants
const size_t PERM_STR_MAX_LEN = sizeof("drwxrwxrwx");
const size_t DATE_TIME_STR_MAX_LEN = sizeof("mnt DD HH:MM");

// Typedefs
typedef struct s_FileInfo
{
	// Information to print
	char* fileMode;
	char* userName;
	char* groupName;
	char* dateTime;
	char* fileName;
	nlink_t linksCount;
	off_t fileSize;
} t_FileInfo;

typedef enum
{
	LINKS,
	USERNAME,
	GROUPNAME,
	FILE_SIZE,
	
	COUNT
} t_ColumnIdx;

// Variables
t_vector vFileInfoArray;
int columnsDigits[COUNT];

// Private functions prototypes
static void PrintFolder(const char *pathToFolder);
static void PrintFile(const char *pathToFile, const char* fileName);
static void ShowFileList(void);
static void PrintPermissions(mode_t mode, char* fileModeTxt);
static int  CompareFunction(const void* a, const void* b);
static int  CountDigits(size_t value);
static void Cleanup(void);

// Public functions
int main(int argc, char **argv)
{
	if (argc > 1)
	{
		for (int argIdx = 1; argIdx < argc; argIdx++)
		{
			PrintFolder(argv[argIdx]);
			
			if ((argIdx + 1) != argc)
				putchar('\n');
		}
	}
	else
		PrintFolder(".");
	
	return 0;
}

// Private functions
static void PrintFolder(const char *pathToFolder)
{
	DIR *dir = opendir(pathToFolder);
	struct dirent *entry;
	
	if (!dir)
	{
		fprintf(stderr, "'%s': No such file or directory\n", pathToFolder);
		return;
	}
	
	printf("%s:\n", pathToFolder);
	
	CVECTOR_INIT(vFileInfoArray, t_FileInfo*);
	memset(columnsDigits, 0, sizeof(columnsDigits));
	
	while ((entry = readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (entry->d_name[0] == '.')
			continue;
		
		char *absolutePathToFile = (char*)malloc(strlen(pathToFolder) + 1 + strlen(entry->d_name) + 1);
		sprintf(absolutePathToFile, "%s/%s", pathToFolder, entry->d_name);
		
		PrintFile(absolutePathToFile, entry->d_name);
		
		free(absolutePathToFile);
	}
	
	ShowFileList();
	
	Cleanup();
	closedir(dir);
}

static void PrintFile(const char *pathToFile, const char* fileName)
{
	t_FileInfo* sFileInfo;
	struct stat statbuf;
	struct passwd *pass;
	struct group *grp;
	
	if (stat(pathToFile, &statbuf) == -1)
		return;
	if ((pass = getpwuid(statbuf.st_uid)) == NULL)
		return;
	if ((grp = getgrgid(statbuf.st_gid)) == NULL)
		return;
	
	sFileInfo = (t_FileInfo*)malloc(sizeof(t_FileInfo));
	
	// Get file permissions as a string
	char* fileMode = (char*)malloc(PERM_STR_MAX_LEN);
	PrintPermissions(statbuf.st_mode, fileMode);
	sFileInfo->fileMode = fileMode;
	
	// Get date and time string
	time_t currentTime;
	time(&currentTime);
	int currentYear = localtime(&currentTime)->tm_year;
	
	char* dateAndTime = (char*)malloc(DATE_TIME_STR_MAX_LEN);
	struct tm *currentDateTime = localtime(&statbuf.st_mtime);
	
	if (currentDateTime->tm_year != currentYear)
		strftime(dateAndTime, DATE_TIME_STR_MAX_LEN, "%b %d %Y", currentDateTime);
	else
		strftime(dateAndTime, DATE_TIME_STR_MAX_LEN, "%b %d %H:%M", currentDateTime);
	sFileInfo->dateTime = dateAndTime;

	// Get number of hard links to the file
	sFileInfo->linksCount = statbuf.st_nlink;
	sFileInfo->fileSize = statbuf.st_size;
	
	int linksColLen = CountDigits(sFileInfo->linksCount);
	if (linksColLen > columnsDigits[LINKS])
		columnsDigits[LINKS] = linksColLen;
	
	int sizeColLen = CountDigits(sFileInfo->fileSize);
	if (sizeColLen > columnsDigits[FILE_SIZE])
		columnsDigits[FILE_SIZE] = sizeColLen;
	
	// Get username
	int userNameLen = strlen(pass->pw_name);
	sFileInfo->userName = (char*)malloc(userNameLen + 1);
	strcpy(sFileInfo->userName, pass->pw_name);
	if (userNameLen > columnsDigits[USERNAME])
		columnsDigits[USERNAME] = userNameLen;
	
	// Get group
	int groupNameLen = strlen(grp->gr_name);
	sFileInfo->groupName = (char*)malloc(groupNameLen + 1);
	strcpy(sFileInfo->groupName, grp->gr_name);
	if (groupNameLen > columnsDigits[GROUPNAME])
		columnsDigits[GROUPNAME] = groupNameLen;
	
	// Get filename
	sFileInfo->fileName = (char*)malloc(strlen(fileName) + 1);
	strcpy(sFileInfo->fileName, fileName);
	
	CVECTOR_PUSHBACK(vFileInfoArray, t_FileInfo*, sFileInfo);
}

static void ShowFileList(void)
{
	qsort(vFileInfoArray.data, CVECTOR_SIZE(vFileInfoArray), sizeof(t_FileInfo*), CompareFunction);
	
	for (int idx = 0; idx < CVECTOR_SIZE(vFileInfoArray); idx++)
	{
		t_FileInfo* item = CVECTOR_GET(vFileInfoArray, t_FileInfo*, idx);
		
		printf("%s %*lu %*s %*s %*lu %s %s\n",
			item->fileMode, columnsDigits[LINKS], item->linksCount, -columnsDigits[USERNAME], item->userName,
			-columnsDigits[GROUPNAME], item->groupName, columnsDigits[FILE_SIZE], item->fileSize, item->dateTime, item->fileName);
	}
}

static void PrintPermissions(mode_t mode, char* fileModeTxt)
{
	if (S_ISDIR(mode))
		fileModeTxt[0] = 'd';
	else if (S_ISCHR(mode))
		fileModeTxt[0] = 'c';
	else if (S_ISBLK(mode))
		fileModeTxt[0] = 'b';
	else if (S_ISLNK(mode))
		fileModeTxt[0] = 'l';
	else
		fileModeTxt[0] = '-';
	
	fileModeTxt[1] = (mode & S_IRUSR) ? 'r' : '-';
	fileModeTxt[2] = (mode & S_IWUSR) ? 'w' : '-';
	fileModeTxt[3] = (mode & S_IXUSR) ? 'x' : '-';
	fileModeTxt[4] = (mode & S_IRGRP) ? 'r' : '-';
	fileModeTxt[5] = (mode & S_IWGRP) ? 'w' : '-';
	fileModeTxt[6] = (mode & S_IXGRP) ? 'x' : '-';
	fileModeTxt[7] = (mode & S_IROTH) ? 'r' : '-';
	fileModeTxt[8] = (mode & S_IWOTH) ? 'w' : '-';
	fileModeTxt[9] = (mode & S_IXOTH) ? 'x' : '-';
	fileModeTxt[10] = 0;
}

static int CompareFunction(const void* a, const void* b)
{
	t_FileInfo* A = *(t_FileInfo**)a;
	t_FileInfo* B = *(t_FileInfo**)b;
	
	return strcmp(A->fileName, B->fileName);
}

static int CountDigits(size_t value)
{
	if (value == 0)
		return 1;
	
	int count = 0;
	while (value != 0)
	{
		value = value / 10;
		++count;
	}
	return count;
}

static void Cleanup(void)
{
	for (size_t idx = 0; idx < CVECTOR_SIZE(vFileInfoArray); idx++)
	{
		t_FileInfo* item = CVECTOR_GET(vFileInfoArray, t_FileInfo*, idx);
		
		free(item->fileMode);
		free(item->userName);
		free(item->groupName);
		free(item->dateTime);
		free(item->fileName);
		
		free(item);
	}
	
	CVECTOR_FREE(vFileInfoArray);
}


