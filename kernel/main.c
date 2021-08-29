
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

#define NULL ((void*)0)
#define DELAY_TIME 5000
/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	struct task* p_task;
	struct proc* p_proc= proc_table;
	char* p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16   selector_ldt = SELECTOR_LDT_FIRST;
        u8    privilege;
        u8    rpl;
	int   eflags;
	int   i, j;
	int   prio;
	for (i = 0; i < NR_TASKS+NR_PROCS; i++) {
	        if (i < NR_TASKS) {     /* 任务 */
                        p_task    = task_table + i;
                        privilege = PRIVILEGE_TASK;
                        rpl       = RPL_TASK;
                        eflags    = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
			prio      = 15;
                }
                else {                  /* 用户进程 */
                        p_task    = user_proc_table + (i - NR_TASKS);
                        privilege = PRIVILEGE_USER;
                        rpl       = RPL_USER;
                        eflags    = 0x202; /* IF=1, bit 2 is always 1 */
			prio      = 5;
                }

		strcpy(p_proc->name, p_task->name);	/* name of the process */
		p_proc->pid = i;			/* pid */

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(struct descriptor));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(struct descriptor));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = eflags;

		/* p_proc->nr_tty		= 0; */

		p_proc->p_flags = 0;
		p_proc->p_msg = 0;
		p_proc->p_recvfrom = NO_TASK;
		p_proc->p_sendto = NO_TASK;
		p_proc->has_int_msg = 0;
		p_proc->q_sending = 0;
		p_proc->next_sending = 0;

		for (j = 0; j < NR_FILES; j++)
			p_proc->filp[j] = 0;

		p_proc->ticks = p_proc->priority = prio;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

        /* proc_table[NR_TASKS + 0].nr_tty = 0; */
        /* proc_table[NR_TASKS + 1].nr_tty = 1; */
        /* proc_table[NR_TASKS + 2].nr_tty = 1; */

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	init_clock();
        init_keyboard();

	restart();

	while(1){}
}


/*****************************************************************************
 *                                get_ticks
 *****************************************************************************/
PUBLIC int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}


/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	int fd;
	int i, n;

	char tty_name[]="/dev_tty0";
	
	char rdbuf[128];
	char command2[100], command3[100], command4[100], command5[100];

	int fd_stdin = open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	//	char filename[MAX_FILENAME_LEN+1] = "zsp01";
	const char bufw[80] = { 0 };
	
	clear();
	
	start();
	clear();
	

	while(1)
	{
		cmdlist();
		printf("\nAdministrator>: $ ");
		
		memset(command2, 0, sizeof(command2));
		memset(command3, 0, sizeof(command3));
		memset(command4, 0, sizeof(command4));
		memset(command5, 0, sizeof(command5));
		
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		mystrncpy(command2, rdbuf, 2);
		mystrncpy(command3, rdbuf, 3);
		mystrncpy(command4, rdbuf, 4);
		mystrncpy(command5, rdbuf, 5);
		
		if(!strcmp(command4,"help"))
		{
			clear();
		}
		else if (!strcmp(command5, "clear"))
		{
			clear();
		}
		else if(!strcmp(rdbuf,"file"))
		{
			clear();
			runFileManage();
		}
		else if(!strcmp(command2,"id"))
		{
			clear();
			checkProcess();
		}
		else if(!strcmp(rdbuf,"process"))
		{
			clear();
			runProcessManage();
		}
		else
		{
			clear();
			printf("[Hint] No such command. Please check the list.\n");
		}
	};
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	char tty_name[] = "/dev_tty1";

	int fd_stdin  = open(tty_name, O_RDWR);
	assert(fd_stdin  == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	char rdbuf[128];
	while (1) {
		printf("$ ");
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;

		if (strcmp(rdbuf, "hello") == 0)
			printf("hello world!\n");
		else
			if (rdbuf[0])
				printf("{%s}\n", rdbuf);
	}

	assert(0); /* never arrive here */
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestC()
{
	spin("TestC");
	/* assert(0); */
}

/*****************************************************************************
 *                                panic
 *****************************************************************************/
PUBLIC void panic(const char *fmt, ...)
{
	int i;
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
	__asm__ __volatile__("ud2");
}
void clear()
{
	clear_screen(0, console_table[current_console].cursor);
	console_table[current_console].crtc_start = 0;
	console_table[current_console].cursor = 0;
}

void start()
{

	for(int i = 0; i <= 25; i++)
	{
		printf("      _   _    ______   __           ___  ____  \n");
		printf("     | | / \\  |  _ \\ \\ / /          / _ \\/ ___| \n");
		printf("  _  | |/ _ \\ | |_) \\ V /   _____  | | | \\___ \\ \n");
		printf(" | |_| / ___ \\|  _ < | |   |_____| | |_| |___) |\n");
		printf("  \\___/_/   \\_\\_| \\_\\|_|            \\___/|____/ \n");
		printf("\n\n");
	

		printf("Please wait... Jary-lrj's os is starting...\n");
		printf("Current Percentage: %d%c",i*4,'%');
		milli_delay(1000);
		clear();
	}
	printf("      _   _    ______   __           ___  ____  \n");
	printf("     | | / \\  |  _ \\ \\ / /          / _ \\/ ___| \n");
	printf("  _  | |/ _ \\ | |_) \\ V /   _____  | | | \\___ \\ \n");
	printf(" | |_| / ___ \\|  _ < | |   |_____| | |_| |___) |\n");
	printf("  \\___/_/   \\_\\_| \\_\\|_|            \\___/|____/ \n");
	printf("\n\n");

	printf("\nCurrent Percentage: %d%c",100,'%');
	printf("\nThe OS started successfully!\n");
	milli_delay(5000);
}

void cmdlist()
{
	printf("      _   _    ______   __           ___  ____  \n");
	printf("     | | / \\  |  _ \\ \\ / /          / _ \\/ ___| \n");
	printf("  _  | |/ _ \\ | |_) \\ V /   _____  | | | \\___ \\ \n");
	printf(" | |_| / ___ \\|  _ < | |   |_____| | |_| |___) |\n");
	printf("  \\___/_/   \\_\\_| \\_\\|_|            \\___/|____/ \n");
	printf("\n\n");	
	printf("============================================================\n");
	printf("Welcome to Jary-lrj's OS! Dear Administrator!\n");
	printf("    [Command list and helps]\n");
	printf("$ help -- show the command list\n");
	printf("$ id -- check whether the id number is authorized.\n");
	printf("$ file -- manage your files.\n");
	printf("$ process -- manage the running processes in the system.\n");
	printf("============================================================\n");
}
void notfound()
{	printf("============================================================\n");
	printf("# command is not found in the system. #\n");
	printf("# please [help] to get commands in the system. #\n");
	printf("============================================================\n");
}
void mystrncpy(char* dest, char* src, int len)
{
	assert(dest != NULL && src != NULL);

	char* temp = dest;
	int i = 0;
	while (i++ < len && (*temp++ = *src++) != '\0');

	if (*(temp) != '\0') {
		*temp = '\0';
	}
}
/*======================
file system manage
=======================*/
#define MAX_FILE_PER_LAYER 10
#define MAX_FILE_NAME_LENGTH 20
#define MAX_CONTENT_ 50
#define MAX_FILE_NUM 100

 //文件ID计数器
int fileIDCount = 0;
int currentFileID = 0;

struct fileBlock {
	int fileID;
	char fileName[MAX_FILE_NAME_LENGTH];
	int fileType; //0 for txt, 1 for folder
	char content[MAX_CONTENT_];
	int fatherID;
	int children[MAX_FILE_PER_LAYER];
	int childrenNumber;
};
struct fileBlock blocks[MAX_FILE_NUM];
int IDLog[MAX_FILE_NUM];

void FSInit() {

	for (int i = 0; i < MAX_FILE_NUM; i++) {
		blocks[i].childrenNumber = 0;
		blocks[i].fileID = -2;
		IDLog[i] = '\0';
	}
	IDLog[0] = 1;
	blocks[0].fileID = 0;
	strcpy(blocks[0].fileName, "home");
	strcpy(blocks[0].content, "welcome to use file system!");
	blocks[0].fileType = 2;
	blocks[0].fatherID = 0;
	blocks[0].childrenNumber = 0;
	currentFileID = 0;
	fileIDCount = 1;
}

int ReadDisk() {
	char bufr[1000];
	int fd = 0;
	int n1 = 0;
	fd = open("ss", O_RDWR);
	assert(fd != -1);
	n1 = read(fd, bufr, 1000);
	assert(n1 == 1000);
	bufr[n1] = 0;
	int r = 1;
	fileIDCount = toInt(bufr + r);
	r = r + 4;
	for (int i = 0; i < fileIDCount; i++) {
		int ID = toInt(bufr + r);
		IDLog[ID] = 1;
		blocks[ID].fileID = ID;
		r = r + 4;
		for (int i = 0; i < MAX_FILE_NAME_LENGTH; i++) {
			if (bufr[r] == '^')
				break;
			else if (bufr[r] == (char)1)
				blocks[ID].fileName[i] = '^';
			else
				blocks[ID].fileName[i] = bufr[r];
			r++;
		}
		r++;
		blocks[ID].fileType = (int)bufr[r] - 48;
		r = r + 2;
		for (int j = 0; j < MAX_CONTENT_; j++) {
			if (bufr[r] == '^')
				break;
			else if (bufr[r] == (char)1)
				blocks[ID].content[j] = '^';
			else
				blocks[ID].content[j] = bufr[r];
			r++;
		}
		r++;
		blocks[ID].fatherID = toInt(bufr + r);
		r = r + 4;
		for (int j = 0; j < MAX_FILE_PER_LAYER; j++) {
			blocks[ID].children[j] = toInt(bufr + r);
			r = r + 3;
		}
		r++;
		blocks[ID].childrenNumber = toInt(bufr + r);
		r = r + 4;
	}
	return n1;
}

int toInt(char* temp) {
	int result = 0;
	for (int i = 0; i < 3; i++)
		result = result * 10 + (int)temp[i] - 48;
	return result;
}

void runFileManage(int fd_stdin) {
	char rdbuf[128];
	char cmd[8];
	char filename[120];
	char buf[1024];
	int m, n;
	char _name[MAX_FILE_NAME_LENGTH];
	FSInit();
	int len = ReadDisk();
	ShowMessage();

	while (1) {
		for (int i = 0; i <= 127; i++)
			rdbuf[i] = '\0';
		for (int i = 0; i < MAX_FILE_NAME_LENGTH; i++)
			_name[i] = '\0';
		printf("\n/Administrator/FileManage/%s:$ ", blocks[currentFileID].fileName);

		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		assert(fd_stdin == 0);


		char target[10];
		for (int i = 0; i <= 1 && i < r; i++) {
			target[i] = rdbuf[i];
		}
		if (rdbuf[0] == 't' && rdbuf[1] == 'o' && rdbuf[2] == 'u' && rdbuf[3] == 'c' && rdbuf[4] == 'h') {
			if (rdbuf[5] != ' ') {
				printf("[Hint] You should add the filename, like \"create XXX\".\n");
				printf("[Hint] Please input [help] to know more.\n");
				continue;
			}
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++) {
				_name[i] = rdbuf[i + 6];
			}
			CreateFIle(_name, 0);
		}
		else if (rdbuf[0] == 'm' && rdbuf[1] == 'k' && rdbuf[2] == 'd' && rdbuf[3] == 'i' && rdbuf[4] == 'r') {
			if (rdbuf[5] != ' ') {
				printf("[Hint] You should add the dirname, like \"mkdir XXX\".\n");
				printf("[Hint] Please input [help] to know more.\n");
				continue;
			}
			char N[MAX_FILE_NAME_LENGTH];
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++) {
				_name[i] = rdbuf[i + 6];
			}
			CreateFIle(_name, 1);
		}
		else if (strcmp(rdbuf, "ls") == 0) {
			showFileList();
		}
		else if (strcmp(target, "cd") == 0) {
			if (rdbuf[2] == ' ' && rdbuf[3] == '.' && rdbuf[4] == '.') {
				ReturnFile(currentFileID);
				continue;
			}
			else if (rdbuf[2] != ' ') {
				printf("[Hint] You should add the dirname, like \"cd XXX\".\n");
				printf("[Hint] Please input [help] to know more.\n");

				continue;
			}
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++) {
				_name[i] = rdbuf[i + 3];
			}
			printf("filename: %s\n", _name);
			int ID = SearchFile(_name);
			if (ID >= 0) {
				if (blocks[ID].fileType == 1) {
					currentFileID = ID;
					continue;
				}
				else if (blocks[ID].fileType == 0) {
					while (1) {
						printf("[Hint] Input the character representing the method you want to operate:"
							"\nu --- update"
							"\nd --- detail of the content"
							"\nq --- quit\n");
						int r = read(fd_stdin, rdbuf, 70);
						rdbuf[r] = 0;
						if (strcmp(rdbuf, "u") == 0) {
							printf("[Hint] Input the text you want to write:\n");
							int r = read(fd_stdin, blocks[ID].content, MAX_CONTENT_);
							blocks[ID].content[r] = 0;
						}
						else if (strcmp(rdbuf, "d") == 0) {
							printf("--------------------------------------------"
								"\n%s\n-------------------------------------\n", blocks[ID].content);
						}
						else if (strcmp(rdbuf, "q") == 0) {
							printf("[Hint] Would you like to save the change? y/n");
							int r = read(fd_stdin, rdbuf, 70);
							rdbuf[r] = 0;
							if (strcmp(rdbuf, "y") == 0) {
								printf("[Hint] Save changes!");
							}
							else {
								printf("[Hint] Quit without changing");
							}
							break;
						}
					}
				}
			}
			else
				printf("[Hint] No such file!");
		}
		else if (strcmp(target, "rm") == 0) {
			if (rdbuf[2] != ' ') {
				printf("[Hint] You should add the filename or dirname, like \"rm XXX\".\n");
				printf("[Hint] Please input [help] to know more.\n");
				continue;
			}
			for (int i = 0; i < MAX_FILE_NAME_LENGTH && i < r - 3; i++) {
				_name[i] = rdbuf[i + 3];
			}
			int ID = SearchFile(_name);
			if (ID >= 0) {
				printf("[Hint] Delete the file successfully!\n");
				DeleteFile(ID);
				for (int i = 0; i < blocks[currentFileID].childrenNumber; i++) {
					if (ID == blocks[currentFileID].children[i]) {
						for (int j = i + 1; j < blocks[currentFileID].childrenNumber; j++) {
							blocks[currentFileID].children[i] = blocks[currentFileID].children[j];
						}
						blocks[currentFileID].childrenNumber--;
						break;
					}
				}
			}
			else
				printf("[Hint] No such file!\n");
		}
		else if (strcmp(target, "sv") == 0) {
			WriteDisk(1000);
			printf("[Hint] Save to disk successfully!\n");
		}
		else if (strcmp(rdbuf, "help") == 0) {
			clear();
			printf("\n");
			ShowMessage();
		}
		else if (strcmp(rdbuf, "quit") == 0) {
			clear();
			break;
		}
		else if (!strcmp(rdbuf, "clear")) {
			clear();
		}
		else {
			printf("[Hint] Sorry, there no such command in the File Manager.\n");
			printf("[Hint] You can input [help] to know more.\n");
		}
	}

}

void ShowMessage()
{
	printf("      _   _    ______   __           ___  ____  \n");
	printf("     | | / \\  |  _ \\ \\ / /          / _ \\/ ___| \n");
	printf("  _  | |/ _ \\ | |_) \\ V /   _____  | | | \\___ \\ \n");
	printf(" | |_| / ___ \\|  _ < | |   |_____| | |_| |___) |\n");
	printf("  \\___/_/   \\_\\_| \\_\\|_|            \\___/|____/ \n");
	printf("\n\n");
	printf("============================================================\n");
	printf("Welcome to FileManage, Dear Administrator! \n");
	printf("    [Command List]\n");
	printf("# $ touch [filename] -- to create a new file in the disk. #\n");
	printf("# $ mkdir [dirname] -- to create a new directory in the disk. #\n");
	printf("# $ cd [dirname] -- to enter the target directory. #\n");
	printf("# $ cd .. -- to back the last directory. #\n");
	printf("# $ ls -- to show the file list in the level of files. #\n");
	printf("# $ rm [filename] -- to delete a file or a directory. #\n");
	printf("# $ help -- to show the command list of the filemanage. #\n");
	printf("# $ quit -- to quit the filemanage. #\n");
	printf("# $ clear -- to clear the console. #\n");
	printf("============================================================\n");
}
void initFileBlock(int fileID, char* fileName, int fileType) {
	blocks[fileID].fileID = fileID;
	strcpy(blocks[fileID].fileName, fileName);
	blocks[fileID].fileType = fileType;
	blocks[fileID].fatherID = currentFileID;
	blocks[fileID].childrenNumber = 0;
}
void toStr3(char* temp, int i) {
	if (i / 100 < 0)
		temp[0] = (char)48;
	else
		temp[0] = (char)(i / 100 + 48);
	if ((i % 100) / 10 < 0)
		temp[1] = '0';
	else
		temp[1] = (char)((i % 100) / 10 + 48);
	temp[2] = (char)(i % 10 + 48);
}
void WriteDisk(int len) {
	char temp[MAX_FILE_NUM * 150 + 103];
	int i = 0;
	temp[i] = '^';
	i++;
	toStr3(temp + i, fileIDCount);
	i = i + 3;
	temp[i] = '^';
	i++;
	for (int j = 0; j < MAX_FILE_NUM; j++) {
		if (IDLog[j] == 1) {
			toStr3(temp + i, blocks[j].fileID);
			i = i + 3;
			temp[i] = '^';
			i++;
			for (int h = 0; h < strlen(blocks[j].fileName); h++) {
				temp[i + h] = blocks[j].fileName[h];
				if (blocks[j].fileName[h] == '^')
					temp[i + h] = (char)1;
			}
			i = i + strlen(blocks[j].fileName);
			temp[i] = '^';
			i++;
			temp[i] = (char)(blocks[j].fileType + 48);
			i++;
			temp[i] = '^';
			i++;
			for (int h = 0; h < strlen(blocks[j].content); h++) {
				temp[i + h] = blocks[j].content[h];
				if (blocks[j].content[h] == '^')
					temp[i + h] = (char)1;
			}
			i = i + strlen(blocks[j].content);
			temp[i] = '^';
			i++;
			toStr3(temp + i, blocks[j].fatherID);
			i = i + 3;
			temp[i] = '^';
			i++;
			for (int m = 0; m < MAX_FILE_PER_LAYER; m++) {
				toStr3(temp + i, blocks[j].children[m]);
				i = i + 3;
			}
			temp[i] = '^';
			i++;
			toStr3(temp + i, blocks[j].childrenNumber);
			i = i + 3;
			temp[i] = '^';
			i++;
		}
	}
	int fd = 0;
	int n1 = 0;
	fd = open("ss", O_RDWR);
	assert(fd != -1);
	n1 = write(fd, temp, strlen(temp));
	assert(n1 == strlen(temp));
	close(fd);
}
int CreateFIle(char* fileName, int fileType) {
	if (blocks[currentFileID].childrenNumber == MAX_FILE_PER_LAYER) {
		printf("[Hint] Sorry you cannot add more files in this layer.\n");
		return 0;
	}
	else {
		for (int i = 0; i < blocks[currentFileID].childrenNumber; i++) {
			if (strcmp(blocks[blocks[currentFileID].children[i]].fileName, fileName) == 0) {
				if (fileType) {
					printf("[Hint] You have a folder of same name!\n");
				}
				else {
					printf("[Hint] You have a file of same name!\n");
				}
				return 0;
			}
		}
		fileIDCount++;
		int target = 0;
		for (int i = 0; i < MAX_FILE_NUM; i++) {
			if (IDLog[i] == 0) {
				target = i;
				break;
			}
		}
		initFileBlock(target, fileName, fileType);
		blocks[currentFileID].children[blocks[currentFileID].childrenNumber] = target;
		blocks[currentFileID].childrenNumber++;
		if (fileType) {
			printf("[Hint] Create directory %s successful!\n", fileName);
		}
		else {
			printf("[Hint] Create file %s successful!\n", fileName);
		}
		IDLog[target] = 1;
		return 1;
	}
}

void showFileList() {
	printf("The files in %s.\n", blocks[currentFileID].fileName);		//通过currentFileID获取当前路径s

	printf("-----------------------------------------\n");
	printf("  filename |    type   | id  \n");
	for (int i = 0; i < blocks[currentFileID].childrenNumber; i++) {	//遍历每个孩子
		printf("%10s", blocks[blocks[currentFileID].children[i]].fileName);
		if (blocks[blocks[currentFileID].children[i]].fileType == 0) {
			printf(" | .txt file |");
		}
		else {
			printf(" |   folder  |");
		}
		printf("%3d\n", blocks[blocks[currentFileID].children[i]].fileID);
	}
	printf("-----------------------------------------\n");
}

int SearchFile(char* name) {
	for (int i = 0; i < blocks[currentFileID].childrenNumber; i++) {
		if (strcmp(name, blocks[blocks[currentFileID].children[i]].fileName) == 0) {
			return blocks[currentFileID].children[i];
		}
	}
	return -2;
}

void ReturnFile(int ID) {
	currentFileID = blocks[ID].fatherID;
}

void DeleteFile(int ID) {
	if (blocks[ID].childrenNumber > 0) {
		for (int i = 0; i < blocks[ID].childrenNumber; i++) {
			DeleteFile(blocks[blocks[ID].children[i]].fileID);
		}
	}
	IDLog[ID] = 0;
	blocks[ID].fileID = -2;
	blocks[ID].childrenNumber = 0;
	for (int i = 0; i < MAX_CONTENT_; i++)
		blocks[ID].content[i] = '\0';
	for (int i = 0; i < MAX_FILE_NAME_LENGTH; i++)
		blocks[ID].fileName[i] = '\0';
	blocks[ID].fileType = -1;
	for (int i = 0; i < MAX_FILE_PER_LAYER; i++)
		blocks[ID].children[i] = -1;
	blocks[ID].fatherID = -2;
	fileIDCount--;
}
/*****************************************************************************
 *                                processManager
 *****************************************************************************/
 //进程管理主函数
void runProcessManage(int fd_stdin)
{
	clear();
	char readbuffer[128];
	showProcessWelcome();
	while (1)
	{
		printf("\nAdministrator/ProcessManage>: $ ");

		int end = read(fd_stdin, readbuffer, 70);
		readbuffer[end] = 0;
		int i = 0, j = 0;
		//获得命令指令
		char cmd[20] = { 0 };
		while (readbuffer[i] != ' ' && readbuffer[i] != 0)
		{
			cmd[i] = readbuffer[i];
			i++;
		}
		i++;
		//获取命令目标
		char target[20] = { 0 };
		while (readbuffer[i] != ' ' && readbuffer[i] != 0)
		{
			target[j] = readbuffer[i];
			i++;
			j++;
		}
		//结束进程;
		if (strcmp(cmd, "kill") == 0)
		{
			killProcess(target);
			continue;
		}
		//重启进程
		else if (strcmp(cmd, "restart") == 0)
		{
			restartProcess(target);
			continue;
		}
		//弹出提示
		else if (strcmp(readbuffer, "help") == 0)
		{
			clear();
			showProcessWelcome();
		}
		//打印全部进程
		else if (strcmp(readbuffer, "ps") == 0)
		{
			showProcess();
		}
		//退出进程管理
		else if (strcmp(readbuffer, "quit") == 0)
		{
			clear();

			break;
		}
		else if (!strcmp(readbuffer, "clear")) {
			clear();
		}
		//错误命令提示
		else
		{
			printf("Sorry, there no such command in the Process Manager.\n");
			printf("You can input [help] to know more.\n");
			printf("\n");
		}
	}
}


//打印欢迎界面
void showProcessWelcome()
{
	printf("      _   _    ______   __           ___  ____  \n");
	printf("     | | / \\  |  _ \\ \\ / /          / _ \\/ ___| \n");
	printf("  _  | |/ _ \\ | |_) \\ V /   _____  | | | \\___ \\ \n");
	printf(" | |_| / ___ \\|  _ < | |   |_____| | |_| |___) |\n");
	printf("  \\___/_/   \\_\\_| \\_\\|_|            \\___/|____/ \n");
	printf("\n\n");
	printf("============================================================\n");
	printf("Welcome to use Process Manager, Dear Administrator!\n");
	printf("    [Command List]\n");
	printf("# $ ps -- show all the processes. #\n");
	printf("# $ kill [pid] -- kill the target process. #\n");
	printf("# $ restart [pid] -- restart the target process. #\n");
	printf("# $ quit -- show all the processes. #\n");
	printf("# $ clear -- clean the current console #\n");
	printf("# $ help -- show all the commands about processes. #\n");
	printf("============================================================\n");
}

//打印所有进程
void showProcess()
{
	int i;
	printf("===============================================================================\n");
	printf("    ProcessID    *    ProcessName    *    ProcessPriority    *    Running?           \n");
	//进程号，进程名，优先级，是否在运行
	printf("-------------------------------------------------------------------------------\n");
	for (i = 0; i < NR_TASKS + NR_PROCS; i++)//逐个遍历
	{
		printf("        %d", proc_table[i].pid);
		printf("                 %5s", proc_table[i].name);
		printf("                   %2d", proc_table[i].priority);
		if (proc_table[i].priority == 0)
		{
			printf("                   no\n");
		}
		else
		{
			printf("                   yes\n");
		}
		//printf("        %d                 %s                   %d                   yes\n", proc_table[i].pid, proc_table[i].name, proc_table[i].priority);
	}
	printf("===============================================================================\n\n");
}

int getMag(int n)
{
	int mag = 1;
	for (int i = 0; i < n; i++)
	{
		mag = mag * 10;
	}
	return mag;
}

//计算进程pid
int getPid(char str[])
{
	int length = 0;
	for (; length < MAX_FILENAME_LEN; length++)
	{
		if (str[length] == '\0')
		{
			break;
		}
	}
	int pid = 0;
	for (int i = 0; i < length; i++)
	{
		if (str[i] - '0' > -1 && str[i] - '9' < 1)
		{
			pid = pid + (str[i] + 1 - '1') * getMag(length - 1 - i);
		}
		else
		{
			pid = -1;
			break;
		}
	}
	return pid;
}

//结束进程
void killProcess(char str[])
{
	int pid = getPid(str);

	//健壮性处理以及结束进程
	if (pid >= NR_TASKS + NR_PROCS || pid < 0)
	{
		printf("[Hint] The pid exceeded the range\n");
	}
	else if (pid < NR_TASKS)
	{
		printf("[Hint] System tasks cannot be killed.\n");
	}
	else if (proc_table[pid].priority == 0 || proc_table[pid].p_flags == -1)
	{
		printf("[Hint] Process not found.\n");
	}
	else if (pid == 4 || pid == 6)
	{
		printf("[Hint] This process cannot be killed.\n");
	}
	else
	{
		proc_table[pid].priority = 0;
		proc_table[pid].p_flags = -1;
		printf("[Hint] Aim process is killed.\n");
	}

	showProcess();
}

//重启进程
void restartProcess(char str[])
{
	int pid = getPid(str);

	if (pid >= NR_TASKS + NR_PROCS || pid < 0)
	{
		printf("[Hint] The pid exceeded the range\n");
	}
	else if (proc_table[pid].p_flags != -1)
	{
		printf("[Hint] This process is already running.\n");
	}
	else
	{
		proc_table[pid].priority = 1;
		proc_table[pid].p_flags = 1;
		printf("[Hint] Aim process is running.\n");
	}

	showProcess();
}
void checkProcess(int fd_stdin)
{
	clear();
	char readbuffer[128];
	showCheckWelcome();
	while(1)
	{
		printf("\nAdministrator/IDCheck>: $ ");
		
		int end = read(fd_stdin, readbuffer, 70);
		readbuffer[end] = 0;
		int i = 0, j = 0;
		//获得ID
		char cmd[20] = { 0 };
		while (readbuffer[i] != ' ' && readbuffer[i] != 0)
		{
			cmd[i] = readbuffer[i];
			i++;
		}
		i++;
		//获取命令目标
		char target[20] = { 0 };
		while (readbuffer[i] != ' ' && readbuffer[i] != 0)
		{
			target[j] = readbuffer[i];
			i++;
			j++;
		}
		if(!strcmp(readbuffer,"quit"))
		{
			clear();
			break;
		}
		else if(!strcmp(cmd,"check"))
		{
			clear();
			checkID(target);
		}	
		else if(!strcmp(cmd,"help"))
		{
			clear();
			showCheckWelcome();
		}
		else if (strcmp(readbuffer, "ps") == 0)
		{
			showProcess();
		}
		else if (!strcmp(readbuffer, "clear")) {
			clear();
		}
		//错误命令提示
		else
		{
			printf("[Hint] Sorry, there no such command in the Process Manager.\n");
			printf("[Hint] You can input [help] to know more.\n");
			printf("\n");
		}
	}
	
}
void showCheckWelcome()
{
	printf("      _   _    ______   __           ___  ____  \n");
	printf("     | | / \\  |  _ \\ \\ / /          / _ \\/ ___| \n");
	printf("  _  | |/ _ \\ | |_) \\ V /   _____  | | | \\___ \\ \n");
	printf(" | |_| / ___ \\|  _ < | |   |_____| | |_| |___) |\n");
	printf("  \\___/_/   \\_\\_| \\_\\|_|            \\___/|____/ \n");
	printf("\n\n");
	printf("============================================================\n");
	printf("Welcome to use ID Number Check, Dear Administrator!\n");
	printf("    [Command List]\n");
	printf("# $ check [id] -- check if the id number is valid. #\n");
	printf("# $ quit -- show all the processes. #\n");
	printf("# $ clear -- clean the current console #\n");
	printf("# $ help -- show all the commands about processes. #\n");
	printf("============================================================\n");
}
int id_cal(char a,int c)
{
	int b=a-'0';
	return b*c;
}
void checkID(char ID[])
{
	int wi[17]={7,9,10,5,8,4,2,1,6,3,7,9,10,5,8,4,2};
	char y[11]={'1','0','X','9','8','7','6','5','4','3','2'};
	int sum[17];
	int res = 0;
	memset(sum,0,sizeof(sum));
	
	for(int i=0; i<17; i++)
	{
		sum[i] = id_cal(ID[i], wi[i]);
		res += sum[i];
	}
	int mod = res % 11;
	if(y[mod] == ID[17])
		printf("[Hint] the id is valid. \n");
	else
		printf("[hint] the id is not valid. \n");
}
			
