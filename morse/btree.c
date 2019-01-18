#include "morse.h"

static BTreeNode h = {'h', NULL, NULL};
static BTreeNode v = {'v', NULL, NULL};
static BTreeNode f = {'f', NULL, NULL};

static BTreeNode l = {'l', NULL, NULL};
static BTreeNode p = {'p', NULL, NULL};
static BTreeNode j = {'j', NULL, NULL};

static BTreeNode b = {'b', NULL, NULL};
static BTreeNode x = {'x', NULL, NULL};
static BTreeNode c = {'c', NULL, NULL};
static BTreeNode y = {'y', NULL, NULL};

static BTreeNode z = {'z', NULL, NULL};
static BTreeNode q = {'q', NULL, NULL};

//---

static BTreeNode s = {'s', &h, &v};
static BTreeNode u = {'u', &f, NULL};

static BTreeNode r = {'r', &l, NULL};
static BTreeNode w = {'w', &p, &j};

static BTreeNode d = {'d', &b, &x};
static BTreeNode k = {'k', &c, &y};

static BTreeNode g = {'g', &z, &q};
static BTreeNode o = {'o', NULL, NULL};

//---

static BTreeNode i = {'i', &s, &u};
static BTreeNode a = {'a', &r, &w};
static BTreeNode n = {'n', &d, &k};
static BTreeNode m = {'m', &g, &o};

//---

static BTreeNode e = {'e', &i, &a};
static BTreeNode t = {'t', &n, &m};


static struct BTreeNode morseTreeRoot = {0, &e, &t};


static char findInNode(BTreeNode* node, enum MorseState list[MORSE_MAXLEN], uint8_t offs)
{
	if(node == NULL || offs > MORSE_MAXLEN)
	{
		return 0;
	}
	if(list[offs] == none || offs == MORSE_MAXLEN)
	{
		return node->val;
	}
	else
	{
		if(list[offs] == shortt)
		{
			return findInNode(node->shortt, list, ++offs);
		}
		else if(list[offs] == longg)
		{
			return findInNode(node->longg, list, ++offs);
		}
		else
		{
			return 0;
		}
	}
}

char findChar(enum MorseState list[MORSE_MAXLEN])
{
	return findInNode(&morseTreeRoot, list, 0);
}
