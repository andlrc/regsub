#include <stdlib.h>
#include <string.h>
#include "regsub.h"

#define N_MATCH 10

static int ctoi(char c)
{
  return (c - '0') % 48;
}

int regsub(const regex_t *preg, char *input_p, char *rep_p, int flags_p)
{
  int rc;
  regmatch_t pmatch[N_MATCH],
	     capture;
  size_t nmatch = N_MATCH,
	 replen,
	 repsize,
	 newsize,
	 n;

  char *inputp = input_p,
       *destp,
       *srcp,
       *rep,
       *repp;

  rc = regexec(preg, inputp, nmatch, pmatch, 0);
  if (rc != 0)
    return rc;

  while(rc == 0 && *inputp != '\0')
  {
    repsize = strlen(rep_p) + 1;
    repsize = repsize > 256 ? 256 : repsize;
    rep = malloc(repsize);
    strcpy(rep, rep_p);
    repp = rep;
    replen = strlen(rep);

    while (*repp != '\0')
    {
      switch (*repp)
      {
	case '\\':
	  repp++;
	  switch (*repp)
	  {
	    case '0': /* Capturing groups */
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
	      capture = pmatch[ctoi(*repp)];

	      if (capture.rm_so == -1) /* Error in reference */
		return REG_EXIT_CAPTURE;

	      /* Make room for captured text */
	      destp = repp - 1 + (capture.rm_eo - capture.rm_so);
	      srcp  = repp + 1;
	      n     = replen - (repp - rep);

	      /* Allocate more memory to rep as needed */
	      newsize = repsize + n;
	      if (repsize < newsize)
	      {
		do
		{
		  repsize = repsize * 1.5;
		} while(repsize < newsize);

		if (!(rep = realloc(rep, repsize)))
		{
		  free(rep);
		  return REG_EXIT_REALLOC;
		}
	      }

	      memmove(destp, srcp, n);

	      /* Move captured text into replacement */
	      destp = repp - 1;
	      srcp  = inputp + capture.rm_so;
	      n     = capture.rm_eo - capture.rm_so;
	      memmove(destp, srcp, n);

	      replen += n - 2;
	      repp   += n - 1;

	      break;
	    case '\\': /* Literal backslash */
	      memmove(repp - 1, repp, replen - (repp - rep) + 1);
	      repp--;   /* Remove the backslash from the length, and move */
	      replen--; /* back the pointer */
	      break;

	    default: /* Error in escape sequence */
	      free(rep);
	      return REG_EXIT_ESCAPE;
	      break;
	  }
	  break;
      }
      repp++;
    }

    capture = pmatch[0];

    /* Make room for replacement in input */
    destp = inputp + capture.rm_so + replen;
    srcp  = inputp + capture.rm_eo;
    n     = strlen(inputp) - capture.rm_eo + 1;
    memmove(destp, srcp, n);

    /* Move replacement into input */
    destp = inputp + capture.rm_so;
    srcp  = rep;
    n     = replen;
    memmove(destp, srcp, n);

    /* Bump pointers */
    inputp += replen + (capture.rm_so == 0 ? 1 : capture.rm_so);

    free(rep);

    if (!(flags_p & REG_GLOBAL))
      break;

    rc = regexec(preg, inputp, nmatch, pmatch, REG_NOTBOL);
  }

  return EXIT_SUCCESS;
}
