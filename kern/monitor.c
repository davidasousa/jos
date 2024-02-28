// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line

// Colors For Lab 1 Extra Credit
#define RED_ANSI "\x1b[31m"
#define GREEN_ANSI "\x1b[32m"
#define YELLOW_ANSI "\x1b[33m"
#define BLUE_ANSI "\x1b[34m"
#define MAGENTA_ANSI "\x1b[35m"
#define CYAN_ANSI "\x1b[36m"
#define WHITE_ANSI "\x1b[37m"
#define ANSI_RESET "\x1b[0m"


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

// LAB 1: add your command to here...
static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "checkprint", "Check The cprintf Function, 5000 in octal is 11610", mon_checkprint },
	{ "backtrace", "Testing The EBP/ESP Backtrace", mon_backtrace },
	{ "show", "Printing Rainbow Quater Ascii Art", mon_showcolors },
};

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(commands); i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

// Added To Test Octal Printing
int
mon_checkprint(int argc, char **argv, struct Trapframe *tf)
{
    // Added By David Sousa - Testing Octal
    unsigned int i = 0x00646c72;
    cprintf("H%x Wo%s\n", 57616, &i);
    cprintf("%o\n", 5000);
    return 0;
}
// 

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);

	return 0;
}

int
mon_showcolors(int argc, char **argv, struct Trapframe *tf)
{
    cprintf("%s        _.-'~~`~~'-._%s\n", BLUE_ANSI, ANSI_RESET);
    cprintf("%s     .'`  B   E   R  `%s\n", MAGENTA_ANSI, ANSI_RESET);
    cprintf("%s    / I               T \\%s\n", YELLOW_ANSI, ANSI_RESET);
    cprintf("%s  /`       .-'~\"-.       `\\%s\n", GREEN_ANSI, ANSI_RESET);
    cprintf("%s ; L      / `-    \\      Y ;%s\n", RED_ANSI, ANSI_RESET);
    cprintf("%s;        />  `.  -.|        ;%s\n", BLUE_ANSI, ANSI_RESET);
    cprintf("%s|       /_     '-.__)       |%s\n", MAGENTA_ANSI, ANSI_RESET);
    cprintf("%s|        |-  _.' \\ |        |%s\n", YELLOW_ANSI, ANSI_RESET);
    cprintf("%s;        `~~;     \\\\        ;%s\n", GREEN_ANSI, ANSI_RESET);
    cprintf("%s ;  INGODWE /      \\\\)P    ;%s\n", RED_ANSI, ANSI_RESET);
    cprintf("%s  \\  TRUST '.___.-'`\"     /%s\n", BLUE_ANSI, ANSI_RESET);
    cprintf("%s   `\\                   /`%s\n", MAGENTA_ANSI, ANSI_RESET);
    cprintf("%s     '._   1 9 9 7   _.'%s\n", YELLOW_ANSI, ANSI_RESET);
    cprintf("%s        `'-..,,,..-'`%s\n", GREEN_ANSI, ANSI_RESET);
    return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// LAB 1: Your code here.
    // HINT 1: use read_ebp().
    // HINT 2: print the current ebp on the first line (not current_ebp[0])
    // Added By David Sousa

    // Defining Registers And Their Pointers
    
    uint32_t ebp_const = read_ebp(); // Current EBP As A Int
    uint32_t* ebp = &ebp_const; // EBP 

    struct Eipdebuginfo info;
    uint32_t* eip = (uint32_t*)(*ebp + 4);

    uint32_t* p1 = (uint32_t*)(*ebp + 8);
    uint32_t* p2 = (uint32_t*)(*ebp + 12);
    uint32_t* p3 = (uint32_t*)(*ebp + 16);
    uint32_t* p4 = (uint32_t*)(*ebp + 20);
    uint32_t* p5 = (uint32_t*)(*ebp + 24);

    // ------- //

    cprintf("Stack backtrace:\n"); 
    
    // Ebp Terminates At 0 ~ No Stack
    // Ebp - Actual Basepointer Value -> Cast To Pointer To Find Address Of Previous EBP
    
    while(*ebp) { 
        // Printing Output
        cprintf("  ebp %08x", *ebp);
        cprintf("  eip %08x", *eip);
        cprintf("  args");
        cprintf(" %08x", *p1);
        cprintf(" %08x", *p2);
        cprintf(" %08x", *p3);
        cprintf(" %08x", *p4);
        cprintf(" %08x", *p5);
        cprintf("\n");

        // Exercise 12
        debuginfo_eip(*eip, &info);
		
		//Name buffer for eip_fn_name so we can terminate
		char buffer[100] = {'\0'};
		memcpy(buffer,info.eip_fn_name, info.eip_fn_namelen);

        cprintf("       ");
        cprintf("%s:", info.eip_file);
		cprintf("%d: ", info.eip_line);
		cprintf("%s", buffer);
		cprintf("+%d", ((*eip) - info.eip_fn_addr));
		cprintf("\n");

        ebp = (uint32_t*)(*ebp);
        eip = (uint32_t*)(*ebp + 4);

        p1 = (uint32_t*)(*ebp + 8);
        p2 = (uint32_t*)(*ebp + 12);
        p3 = (uint32_t*)(*ebp + 16);
        p4 = (uint32_t*)(*ebp + 20);
        p5 = (uint32_t*)(*ebp + 24);
    }
	return 0;
}



/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
