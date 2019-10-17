/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2009                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.hom                 */
/*  Description: A chess playing program.         */
/**************************************************/

/*
TODO:
-add SMP
-add Chess 960 support
-replace iterative SEE with a recursive one
-use compiler intrinsics for 64 bit for popCount and bitCount
*/

// ancestors:
// 20080422z, 20080424, 20080428a2, 20080501c, 20080504x, 20080506, 20080515
// 20080516z, 20080517, 20080519z, 20080523, 20080524, 20080525, 20080528x,
// 20080529x, 20080530, 20080620, 20080625, 20080716, 20080723, 20081013
// 20081120, 20081205x, 20081208, 20081215, 20090105, 20090107, 20090108b
// 20090113, 20090114, 20090115, 20090116x, 20090119b, 20090120, 20090121
// 20090121x, 20090123, 20090126x, 20090319b, 20090324x, 20090324z, 20090402x
// 20090407, 20090413, 20090414, 20090414x, 20090414z, 20090415, 20090416
// 20090420, 20090422, 20090423, 20090424z, 20090427, 20090428, 20090429
// 20090430, 20090720, 20090721,20090723, 20090728, 20090730, 20090731
// 20090803, 20090804, 20090805, 20090806, 20090807, 20090808, 20090809
// 20090809x, 20090811, 20090812, 20090815, 20090815z, 20090816, 20090817
// 20090819, 20090820, 20090821x, 20090825, 20090826, 20090829, 20090831
// 20090902, 20090904, 20090907, 20090908, 20090913, 20090913y, 20090915
// 20090917, 20090921x, 20091112, 20091113, 20091202, 20091209x, 20091211x
// 20091215, 20091224, 20091228x, 20091229, 20091230, 20091231, 20100102
// 20100209z, 20100214, 20100215, 20100217, 20100218, 20100219z, 20100224
// 20100723, 20100724, 20100725, 20100728

#define VERSION            "20100815"
//#define DEBUG

#if defined(__x86_64) || defined(_WIN64)
#define VERSION64BIT
#endif

#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <sys/timeb.h>
#else
#include <pthread.h>
#include <sys/time.h>
#endif

#include "macros.h"
#include "typedefs.h"
#include "protos.h"
#include "constants.h"
#include "data.h"
#include "init.h"
#include "material.h"
#include "utils.h"
#include "bitutils.h"
#include "attacks.h"
#include "movegen.h"
#include "position.h"
#include "eval.h"
#include "trans.h"
#include "movepicker.h"
#include "search.h"
#ifdef DEBUG
#include "debug.h"
#include "tests.h"
#endif
#include "uci.h"
#include "main.h"
