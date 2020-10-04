/* macros to simplify parsing positional arguments */

#ifndef ARG_H__
#define ARG_H__

#define ARGV(...) \
	(argv[0][i_ + 1] == '\0' && argv[1] == NULL ? __VA_ARGS__, NULL : \
	(brk_ = 1, argv[0][i_ + 1]) != '\0' ? &argv[0][i_ + 1] : \
	(argc--, argv++, argv[0]))

#define ARGF ARGV(NULL)
#define EARGF ARGV(fprintf(stderr, "%s: option requires an argument -- '%c'\n", \
			argv0, argv[0][i_]), exit(1))

#define ARGBEGIN \
	char *argv0; \
	for (argv0 = *argv, argc--, argv++; argc > 0; argc--, argv++) { \
		if (argv[0][0] == '-') { \
			if (argv[0][1] == '\0' || \
			    (argv[0][1] == '-' && argv[0][2] == '\0')) { \
				argc++; \
				argv++; \
				break; \
			} \
			else { \
				int brk_ = 0; \
				for (int i_ = 1; argv[0][i_] && !brk_; i_++) { \
					switch(argv[0][i_])

#define ARGELSE }}} else

#define ARGEND }

#endif
