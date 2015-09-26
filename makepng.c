#include "makepng.h"

void
print_help() {
	printf("Usage: makepng -mio[vdx]\n"
"cmdline args\n"
"   	-m mode (string : encode, decode)\n"
"   	-i input file (string)\n"
"   	-o output file (string)\n"
"   	-v validation (flag)\n");
	return;
}


int
main(int argc, char *argv[]) 
{
	/*
	 * arg parser
	 */

	// defaults 
	opterr = 0;
	int _x_ = 0;
	char *mode = NULL;
	char *inputfile = NULL;
	char *outputfile = NULL;
	int flagValidation = 0;

	while ((_x_ = getopt (argc, argv, "m:i:o:vh")) != -1)
	switch (_x_)
	{
	case 'm':
	  mode = optarg;
	  break;
	case 'i':
	  inputfile = optarg;
	  break;
	case 'o':
	  outputfile = optarg;
	  break;
	case 'v':
	  flagValidation = 1;
	  break;
	case 'h':
		 print_help();
		 return 1;
		 break;
	case '?':
	  if (optopt == 'm' || optopt == 'i' || optopt == 'o') {
		printf("Option -%c requires an argument.\n", optopt);
	  } else {
		printf("Unknown option `-%c'.\n", optopt);
	  }
		 print_help();
	  return 1;
	default:
	  abort ();
	}

	if ( mode == NULL || inputfile == NULL || outputfile == NULL) {
		printf("Invalid arguments\n");
		   exit(EXIT_FAILURE);
		}

	Dprintf("**** User args **** \n"
			  "Mode (-m)       = %s\n"
			  "Inputfile (-i)  = %s\n"
			  "Outputfile (-o) = %s\n"
			  "Validation (-v) = %s\n",
			   mode,
			   inputfile,
			   outputfile,
			   flagValidation?"On":"Off");

	if(!strcmp(mode, "encode")) {
		Dprintf("**** Encoding data file into a PNG image ****\n");
		encode(inputfile, outputfile, flagValidation);
	} else if(!strcmp(mode, "decode")) {
		Dprintf("**** Decoding a PNG image into a data file ****\n");
		decode(inputfile, outputfile);
	} else {
		printf("Invalid mode, mode should be either \"encode\" or \"decode\", exiting.\n");
		return 1;
	}
}
