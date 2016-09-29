#include "makepng.h"

void
print_help() {
	printf("Usage: makepng -e|d,i,o,[m,v,p]\n"
"cmdline args\n"
"   	-e encode (flag)\n"
"   	-d decode (flag), -e and -d are mutually exclusive\n"
"   	-m meta-data (string), optional\n"
"   	-i input-file (string), mandatory\n"
"   	-o output-file (string), mandatory \n"
"   	-p print-metadata (flag), optional\n"
"   	-v validation (flag), optional\n");
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
	char *meta = NULL;
	char *inputfile = NULL;
	char *outputfile = NULL;
	int flagValidation = 0;
	int flagPrintMetadata = 0;

	while ((_x_ = getopt (argc, argv, "edpm:i:o:vh")) != -1)
	switch (_x_)
	{
	case 'e':
	  mode = "encode";
	  break;
	case 'd':
	  mode = "decode";
	  break;
	case 'm':
	  meta = optarg;
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
	case 'p':
	  flagPrintMetadata = 1;
	  break;
	case 'h':
		 print_help();
		 return 1;
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
			  "Mode (-e or -d) = %s\n"
			  "Metadata (-m)   = %s\n"
			  "Inputfile (-i)  = %s\n"
			  "Outputfile (-o) = %s\n"
			  "Validation (-v) = %s\n",
			   mode,
			   meta? meta: "No metadata provided",
			   inputfile,
			   outputfile,
			   flagValidation?"On":"Off");

	if(!strcmp(mode, "encode")) {
		Dprintf("**** Encoding data file into a PNG image ****\n");
		encode(inputfile, outputfile, flagValidation, meta);
	} else if(!strcmp(mode, "decode")) {
		Dprintf("**** Decoding a PNG image into a data file ****\n");
		decode(inputfile, outputfile, flagPrintMetadata);
	} else {
		printf("Invalid mode, mode should be either \"encode\" or \"decode\", exiting.\n");
		return 1;
	}
}
