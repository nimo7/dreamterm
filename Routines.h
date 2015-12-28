/*
**  Rutiinien structuret/definaatiot joita muiden osien ei tarvitse tiet‰‰.
**
*/

#include <exec/types.h>

#define TAGVER		2
#define TAGID			0x54414721	/* TAG!				*/
#define INFOVER		16					/* Info node	*/

/*
#define	CSID			0x44544353	/* For 1.0 - 1.3 conversion */
#define	DLID			0x4454444C
#define	PBID			0x44545042
#define	XPID			0x44545850
*/

/* Levyll‰ olevan infolistan global header */
struct Header
{
  ulong id;
  word  ver;
  word  flags;
  long  size;
  long  count;
};

/* info node

  ULONG node name size
  ...   node name
  ULONG	extension data size (currently 0)
  tagdata

*/

/* Levyll‰ olevan taglistan header */
struct TagEntry
{
  ulong id;					/* ID       */
	WORD	version;		/* version  */
	WORD	flags;
	LONG	size;
	/* Tagit:	0000	TagId
						0004	jos VALTAG niin data
									jos STRTAG niin merkkijonon pituus ilman nollaa.
									jos DATTAG niin koko
						.. mahdollinen str/dat tagin data
						(TAG_END) lopettaa)
	*/
};

struct SubStr
{
  ulong tag;
  ulong tagsize;
};
