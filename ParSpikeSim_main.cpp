#include <stdio.h>
#include <stdlib.h>

#include <assert.h>


#include <OS/string.h>
#include "string.h"

#include "ParSpikeSim_mpi.h"


extern "C" {
	int ivocmain(int, char**, char**);

	int nrn_global_argc;
	char** nrn_global_argv;
	int always_false;
}



#if defined(CYGWIN)
// see iv/src/OS/directory.cpp
extern "C" { extern void hoc_dos2unixpath(char*, char*); }
#include <sys/stat.h>
static boolean isdir(const char* p) {
	struct stat st;
	boolean b =  stat((char*)p, &st) == 0 && S_ISDIR(st.st_mode);
	//printf("isdir %s returns %d\n", p, b);
	return b;
}
#endif


// in case we are running without IV then get some important args this way
static boolean nrn_optarg_on(const char* opt, int* argc, char** argv);
static char* nrn_optarg(const char* opt, int* argc, char** argv);
static int nrn_optargint(const char* opt, int* argc, char** argv, int dflt);

static boolean nrn_optarg_on(const char* opt, int* pargc, char** argv) {
	char* a;
	int i;
	for (i=0; i < *pargc; ++i) {
		if (strcmp(opt, argv[i]) == 0) {
			*pargc -= 1;
			for (; i < *pargc; ++i) {
				argv[i] = argv[i+1];
			}
//			printf("nrn_optarg_on %s  return true\n", opt);
			return true;
		}
	}
	return false;
}

static char* nrn_optarg(const char* opt, int* pargc, char** argv) {
	char* a;
	int i;
	for (i=0; i < *pargc - 1; ++i) {
		if (strcmp(opt, argv[i]) == 0) {
			a = argv[i+1];
			*pargc -= 2;
			for (; i < *pargc; ++i) {
				argv[i] = argv[i+2];
			}
//			printf("nrn_optarg %s  return %s\n", opt, a);
			return a;
		}
	}
	return 0;
}

static int nrn_optargint(const char* opt, int* pargc, char** argv, int dflt) {
	char* a;
	int i;
	i = dflt;
	a = nrn_optarg(opt, pargc, argv);
	if (a) {
		sscanf(a, "%d", &i);
	}
//	printf("nrn_optargint %s return %d\n", opt, i);
	return i;
}

#if USENRNJAVA
void nrn_InitializeJavaVM();	
#endif

#if 0 //for debugging
void prargs(const char* s, int argc, char** argv) {
	int i;
	printf("%s argc=%d\n", s, argc);
	for (i=0; i < argc; ++i) {
		printf(" %d |%s|\n", i, argv[i]);
	}
}
#endif

// see nrnmain.cpp for the real main()

int ivocmain (int argc, char** argv, char** env) {
	int i;
//	prargs("at beginning", argc, argv);
	force_load();
	nrn_global_argc = argc;
	nrn_global_argv = new char*[argc];
	for (i = 0; i < argc; ++i) {
		nrn_global_argv[i] = argv[i];
	}
	if (nrn_optarg_on("-help", &argc, argv)
	    || nrn_optarg_on("-h", &argc, argv)) {
		printf("nrniv [options] [fileargs]\n\
  options:\n\
    -dll filename    dynamically load the linked mod files.\n\
    -h               print this help message\n\
    -help            print this help message\n\
    -isatty          unbuffered stdout, print prompt when waiting for stdin\n\
    -mpi             launched by mpirun or mpiexec, in parallel environment\n\
    -mswin_scale float   scales gui on screen\n\
    -NSTACK integer  size of stack (default 1000)\n\
    -NFRAME integer  depth of function call nesting (default 200)\n\
    -nobanner        do not print startup banner\n\
    -nogui           do not send any gui info to screen\n\
    -notatty         buffered stdout and no prompt\n\
    -python          Python is the interpreter\n\
    -realtime        For hard real-time simulation for dynamic clamp\n\
    --version        print version info\n\
    and all InterViews and X11 options\n\
  fileargs:          any number of following\n\
    -                input from stdin til ^D (end of file)\n\
    -c \"statement\"    execute next statement\n\
    filename         execute contents of filename\n\
");
		exit(0);
	}
	if (nrn_optarg_on("--version", &argc, argv)) {
		printf("%s\n", nrn_version(1));
		exit(0);
	}
	if (nrn_optarg_on("-nobanner", &argc, argv)) {
		nrn_nobanner_ = 1;
	}

	nrnmpi_numprocs = nrn_optargint("-bbs_nhost", &argc, argv, nrnmpi_numprocs);
	hoc_usegui = 1;
	if (nrn_optarg_on("-nogui", &argc, argv)) {
		hoc_usegui = 0;
		hoc_print_first_instance = 0;
	}
	if (nrnmpi_numprocs > 1) {
		hoc_usegui = 0;
		hoc_print_first_instance = 0;
	}
#if NRNMPI
	if (nrnmpi_use) {
		hoc_usegui = 0;
		hoc_print_first_instance = 0;
	}
#else

// check if user is trying to use -mpi or -p4 when it was not
// enabled at build time.  If so, issue a warning.

	int b;
	b = 0;
	for (i=0; i < argc; ++i) {
	  if (strncmp("-p4", (argv)[i], 3) == 0) {
	    b = 1;
	    break;
	  }
	  if (strcmp("-mpi", (argv)[i]) == 0) {
	    b = 1;
	    break;
	  }
	}
	if (b) {
	  printf("Warning: detected user attempt to enable MPI, but MPI support was disabled at build time.\n");
	}

#endif 		
#if NRN_REALTIME
	if (nrn_optarg_on("-realtime", &argc, argv)) {
		nrn_realtime_ = 1;
		nrn_setscheduler();
	}
	if (nrn_optarg_on("-schedfifo", &argc, argv)) {
		if (nrn_realtime_ != 1) {
			nrn_setscheduler();
		}
	}

#endif
#if !HAVE_IV
	hoc_usegui = 0;
	hoc_print_first_instance = 0;
#endif
	int our_argc = argc;
	char** our_argv = argv;
	int exit_status = 0;
	Session* session = nil;
#if !defined(WIN32)&&!defined(MAC) && !defined(CYGWIN)
// Gary Holt's first pass at this was:
//
// Set the NEURONHOME environment variable.  This should override any setting
// in the environment, so someone doesn't accidently use data files from an
// old version of neuron.
//
// But I have decided to use the environment variable if it exists
	neuron_home = getenv("NEURONHOME");
	if (!neuron_home) {
#if defined(HAVE_PUTENV)
		// the only reason the following is static is to prevent valgrind
		// from complaining it is a memory leak.
		static char* buffer = new char[strlen(NEURON_DATA_DIR) + 12];
		sprintf(buffer, "NEURONHOME=%s", NEURON_DATA_DIR);
		putenv(buffer);
		neuron_home = NEURON_DATA_DIR;
#elif defined(HAVE_SETENV)
		setenv("NEURONHOME", NEURON_DATA_DIR, 1);
		neuron_home = NEURON_DATA_DIR;
#else
#error "I don't know how to set environment variables."
// Maybe in this case the user will have to set it by hand.
#endif
	}

#else // Not unix:
	neuron_home = getenv("NEURONHOME");
	if (!neuron_home) {
		setneuronhome((argc > 0)?argv[0]:0);
	}
	if (!neuron_home) {
#if defined(WIN32) && HAVE_IV
		MessageBox(0, "No NEURONHOME environment variable.", "NEURON Incomplete Installation", MB_OK);
#else
		neuron_home = ".";
		fprintf(stderr, "Warning: no NEURONHOME environment variable-- setting\
 to %s\n", neuron_home);
#endif
	}
#endif // !unix.
    
#if HAVE_IV
#if OCSMALL
	our_argc = 2;
	our_argv = new char*[2];
	our_argv[0] = "Neuron";
	our_argv[1] = ":lib:hoc:macload.hoc";
	session = new Session("NEURON", our_argc, our_argv, options, properties);
#else
#if MAC
	our_argc = 1;
	our_argv = new char*[1];
	our_argv[0] = "Neuron";
	session = new Session("NEURON", our_argc, our_argv, options, properties);
	SIOUXSettings.asktosaveonclose = false;
#else
#if defined(WIN32) || carbon
IFGUI
	session = new Session("NEURON", our_argc, our_argv, options, properties);
ENDGUI
#else
IFGUI
	if (getenv("DISPLAY")) {
		session = new Session("NEURON", our_argc, our_argv, options, properties);
	}else{
		fprintf(stderr, "Warning: no DISPLAY environment variable.\
\n--No graphics will be displayed.\n");
		hoc_usegui = 0;
	}
ENDGUI
#endif
#endif
	char nrn_props[256];
	if (session) {
		sprintf(nrn_props, "%s/%s", neuron_home, "lib/nrn.defaults");
#ifdef WIN32
		FILE* f;
		if ((f = fopen(nrn_props, "r")) != (FILE*)0) {
			fclose(f);
			session->style()->load_file(String(nrn_props), -5);
		}else{
			sprintf(nrn_props, "%s\\%s", neuron_home, "lib\\nrn.def");
			if ((f = fopen(nrn_props, "r")) != (FILE*)0) {
				fclose(f);
				session->style()->load_file(String(nrn_props), -5);
			}else{
				char buf[256];
				sprintf(buf, "Can't load NEURON resources from %s[aults]",
					nrn_props);
				printf("%s\n", buf);
			}
		}
#else
		 session->style()->load_file(String(nrn_props), -5);
#endif
#if ! MAC
		char* h = getenv("HOME");
		if (h) {
		    	sprintf(nrn_props, "%s/%s", h, ".nrn.defaults");
		    	session->style()->load_file(String(nrn_props), -5);
		}
#endif
	}

#endif /*OCSMALL*/

	if (session) {
		session->style()->find_attribute("NSTACK", hoc_nstack);
		session->style()->find_attribute("NFRAME", hoc_nframe);
	}else
#endif //HAVE_IV
	{
		hoc_nstack = nrn_optargint("-NSTACK", &our_argc, our_argv, 0);
		hoc_nframe = nrn_optargint("-NFRAME", &our_argc, our_argv, 0);
	}

#if defined(WIN32) && HAVE_IV
IFGUI
	double scale = 1.;
	int pw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	if (pw < 1100) {
		scale = 1200./double(pw);
	}
	session->style()->find_attribute("mswin_scale", scale); 
	iv_display_scale(float(scale));
ENDGUI
#endif

	// just eliminate from arg list
	nrn_optarg_on("-mpi", &our_argc, our_argv);

#if (defined(NRNMECH_DLL_STYLE) || defined(WIN32))
	String str;
#if HAVE_IV
	if (session) {
		if (session->style()->find_attribute("nrnmechdll", str)) {
			nrn_mech_dll = str.string();
		}
	}else
#endif
	{ // if running without IV.
		nrn_mech_dll = nrn_optarg("-dll", &our_argc, our_argv);
		// may be duplicated since nrnbbs adds all args to special
		// which is often a script that adds a -dll arg
		nrn_optarg("-dll", &our_argc, our_argv);
	}
#if NRNMPI
	if (nrnmpi_use && !nrn_mech_dll) {
		// for heterogeneous clusters, mpirun allows different programs
		// but not different arguments. So the -dll is insufficient.
		// Therefore we check to see if it makes sense to load
		// a dll from the usual location.
		// Actually this is done by default in src/nrnoc/init.c
	}
#endif

#endif //NRNMECH_DLL_STYLE


#if HAVE_IV
	if (session) {
		long i;
		if (session->style()->find_attribute("isatty", i)) {
			nrn_istty_ = i;
		}		
	}else
#endif
	{
		nrn_istty_ = nrn_optargint("-isatty", &our_argc, our_argv, 0);
	}

#if HAVE_IV
	if (session && session->style()->value_is_on("units_on_flag")) {
		units_on_flag_ = 1;
	};
	Oc oc(session, our_argv[0], env);
#if defined(WIN32) && !defined(CYGWIN)
	if (session->style()->find_attribute("showwinio", str)
      && !session->style()->value_is_on("showwinio")
	) {
		ShowWindow(hCurrWnd, SW_HIDE);
		hoc_obj_run("pwman_place(100,100)\n", 0);
	}
#endif
#else
	hoc_main1_init(our_argv[0], env);
#endif //HAVE_IV

#if USENRNJAVA
	nrn_InitializeJavaVM();	
#endif
#if OCSMALL
	if (argc == 1) {
		ocsmall_argv[0] = our_argv[0];
		oc.run(2, ocsmall_argv);
	}else
#endif
#if defined(USE_PYTHON)
#if HAVE_IV
	if (session && session->style()->value_is_on("python")) {
		use_python_interpreter = 1;
	}
#endif
	if (nrn_optarg_on("-python", &our_argc, our_argv)) {
		use_python_interpreter = 1;
	}

	if (nrn_is_python_extension) { return 0; }
#if defined(CYGWIN) && defined(HAVE_SETENV)
	if (!isdir("/usr/lib/python2.5")) {
		char* path;
		path = new char[strlen(neuron_home) + 20];
		hoc_dos2unixpath(neuron_home, path);
		char* buf = new char[strlen(path) + 20];
		sprintf(buf, "%s/lib/%s", path, "python2.5");
		if (isdir(buf)) {
			setenv("PYTHONHOME", path, 0);
		}
		delete [] buf;
		delete [] path;
		//printf("PYTHONHOME %s\n", getenv("PYTHONHOME"));
	}
#endif
	//printf("p_nrnpython_start = %lx\n", p_nrnpython_start);
	if (p_nrnpython_start) { (*p_nrnpython_start)(1); }
	if (use_python_interpreter && !p_nrnpython_start) {
		fprintf(stderr, "Python not available\n");
		exit(1);
	}
#endif
#if NRN_REALTIME
	nrn_maintask_init();
#endif
#if HAVE_IV
	oc.run(our_argc, our_argv);
#else
	hoc_main1(our_argc, our_argv, env);
#endif
#if HAVE_IV
	if (session && session->style()->value_is_on("neosim")) {
		if (p_neosim_main) {
			(*p_neosim_main)(argc, argv, env);
		}else{
			printf("neosim not available.\nModify nrn/src/nrniv/Imakefile and remove nrniv/$CPU/netcvode.o\n");
		}
	}
#endif
PR_PROFILE
#if defined(USE_PYTHON)
	if (use_python_interpreter) {
		// process the .py files and an interactive interpreter
		if (p_nrnpython_start) {(*p_nrnpython_start)(2);}
	}
	if (p_nrnpython_start) { (*p_nrnpython_start)(0); }
#endif
	hoc_final_exit();
	ivoc_final_exit();
	return exit_status;
}

void ivoc_final_exit() {
#if NRNMPI
	nrnmpi_terminate();
#endif
#if NRN_REALTIME
	nrn_maintask_delete();
#endif
}

extern "C" {

extern void hoc_ret(), hoc_pushx(double);
extern double *getarg(int i);
extern int ifarg(int);

void hoc_single_event_run() {
#if HAVE_IV
IFGUI
	void single_event_run();
	
	single_event_run();
ENDGUI
#endif
	hoc_ret();
	hoc_pushx(1.);
}

#if !HAVE_IV
int run_til_stdin() {return 1;}
void hoc_notify_value(){}
#endif
}
