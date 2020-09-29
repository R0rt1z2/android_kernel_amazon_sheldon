/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#if defined(__KERNEL__)  /* || !defined (__CTP__) */
#include <linux/export.h>
#include <linux/module.h>
#endif /* #if !defined (__CTP__) */

/* #include <asm/system.h> */
#include "mtk_spower_data.h"
#include "mtk_static_power.h"

/*********************************
 * macro for log
 **********************************/
#define SPOWER_LOG_NONE                                0
#define SPOWER_LOG_WITH_XLOG                           1
#define SPOWER_LOG_WITH_PRINTK                         2

#define SPOWER_LOG_PRINT SPOWER_LOG_WITH_PRINTK

#if (SPOWER_LOG_PRINT == SPOWER_LOG_NONE)
#define SPOWER_INFO(fmt, args...)
#elif (SPOWER_LOG_PRINT == SPOWER_LOG_WITH_XLOG)
#define SPOWER_INFO(fmt, args...)		pr_debug("Power/spower", fmt, ##args)
#elif (SPOWER_LOG_PRINT == SPOWER_LOG_WITH_PRINTK)
#define SPOWER_INFO(fmt, args...)		pr_debug("[Power/spower] "fmt, ##args)
#endif

static sptbl_t sptab[MT_SPOWER_MAX];

/****************************************************************
 * this table is generated by scramble function.                *
 * (plz refer to DE team.)                                      *
 ****************************************************************/
int devinfo_table[] = {
		3539,	492,	1038,	106,	231,	17,		46,		2179,
		4,		481,	1014,	103,	225,	17,		45,		2129,
		3,		516,	1087,	111,	242,	19,		49,		2282,
		4,		504,	1063,	108,	236,	18,		47,		2230,
		4,		448,	946,	96,		210,	15,		41,		1986,
		2,		438,	924,	93,		205,	14,		40,		1941,
		2,		470,	991,	101,	220,	16,		43,		2080,
		3,		459,	968,	98,		215,	16,		42,		2033,
		3,		594,	1250,	129,	279,	23,		57,		2621,
		6,		580,	1221,	126,	273,	22,		56,		2561,
		6,		622,	1309,	136,	293,	24,		60,		2745,
		7,		608,	1279,	132,	286,	23,		59,		2683,
		6,		541,	1139,	117,	254,	20,		51,		2390,
		5,		528,	1113,	114,	248,	19,		50,		2335,
		4,		566,	1193,	123,	266,	21,		54,		2503,
		5,		553,	1166,	120,	260,	21,		53,		2446,
		5,		338,	715,	70,		157,	9,		29,		1505,
		3153,	330,	699,	69,		153,	9,		28,		1470,
		3081,	354,	750,	74,		165,	10,		31,		1576,
		3302,	346,	732,	72,		161,	10,		30,		1540,
		3227,	307,	652,	63,		142,	8,		26,		1371,
		2875,	300,	637,	62,		139,	7,		25,		1340,
		2809,	322,	683,	67,		149,	8,		27,		1436,
		3011,	315,	667,	65,		146,	8,		26,		1404,
		2942,	408,	862,	86,		191,	13,		37,		1811,
		1,		398,	842,	84,		186,	12,		36,		1769,
		1,		428,	903,	91,		200,	14,		39,		1896,
		2,		418,	882,	89,		195,	13,		38,		1853,
		2,		371,	785,	78,		173,	11,		33,		1651,
		3458,	363,	767,	76,		169,	10,		32,		1613,
		3379,	389,	823,	82,		182,	12,		35,		1729,
		1,		380,	804,	80,		177,	11,		34,		1689,

};



int interpolate(int x1, int x2, int x3, int y1, int y2)
{
	/* BUG_ON(x1==x2); */
	if (x1 == x2)
		return (y1+y2)/2;

	return (x3-x1) * (y2-y1) / (x2 - x1) + y1;
}

int interpolate_2d(sptbl_t *tab, int v1, int v2, int t1, int t2, int voltage, int degree)
{
	int c1, c2, p1, p2, p;

	if (v1 == v2 && t1 == t2) {
		p = mA(tab, v1, t1);
		return p;
	} else if (v1 == v2) {
		c1 = mA(tab, v1, t1);
		c2 = mA(tab, v1, t2);
		p = interpolate(deg(tab, t1), deg(tab, t2), degree, c1, c2);
		return p;
	} else if (t1 == t2) {
		c1 = mA(tab, v1, t1);
		c2 = mA(tab, v2, t1);
		p = interpolate(mV(tab, v1), mV(tab, v2), voltage, c1, c2);
		return p;
	}

	c1 = mA(tab, v1, t1);
	c2 = mA(tab, v1, t2);
	p1 = interpolate(deg(tab, t1), deg(tab, t2), degree, c1, c2);

	c1 = mA(tab, v2, t1);
	c2 = mA(tab, v2, t2);
	p2 = interpolate(deg(tab, t1), deg(tab, t2), degree, c1, c2);

	p = interpolate(mV(tab, v1), mV(tab, v2), voltage, p1, p2);
	return p;
}

void interpolate_table(sptbl_t *spt, int c1, int c2, int c3, sptbl_t *tab1, sptbl_t *tab2)
{
	int v, t;

	/* avoid divid error, if we have bad raw data table */
	if (unlikely(c1 == c2)) {
		*spt = *tab1;
		SPOWER_INFO("sptab equal to tab1:%d/%d\n",  c1, c3);
		return;
	}

	SPOWER_INFO("make sptab %d, %d, %d\n", c1, c2, c3);
	for (t = 0; t < tsize(spt); t++) {
		for (v = 0; v < vsize(spt); v++) {
			int *p = &mA(spt, v, t);

			p[0] = interpolate(c1, c2, c3,
					   mA(tab1, v, t),
					   mA(tab2, v, t));

			/* pr_debug("%d ", p[0]); */
		}
		/* pr_debug("\n"); */
	}
	SPOWER_INFO("make sptab done!\n");
}


int sptab_lookup(sptbl_t *tab, int voltage, int degree)
{
	int x1, x2, y1, y2, i;
	int mamper;

	/** lookup voltage **/
	for (i = 0; i < vsize(tab); i++) {
		if (voltage <= mV(tab, i))
			break;
	}

	if (unlikely(voltage == mV(tab, i))) {
		x1 = x2 = i;
	} else if (unlikely(i == vsize(tab))) {
		x1 = vsize(tab)-2;
		x2 = vsize(tab)-1;
	} else if (i == 0) {
		x1 = 0;
		x2 = 1;
	} else {
		x1 = i-1;
		x2 = i;
	}


	/** lookup degree **/
	for (i = 0; i < tsize(tab); i++) {
		if (degree <= deg(tab, i))
			break;
	}

	if (unlikely(degree == deg(tab, i))) {
		y1 = y2 = i;
	} else if (unlikely(i == tsize(tab))) {
		y1 = tsize(tab)-2;
		y2 = tsize(tab)-1;
	} else if (i == 0) {
		y1 = 0;
		y2 = 1;
	} else {
		y1 = i-1;
		y2 = i;
	}

	mamper = interpolate_2d(tab, x1, x2, y1, y2, voltage, degree);

	return mamper;
}


int mt_spower_make_table(sptbl_t *spt, spower_raw_t *spower_raw, int wat, int voltage, int degree)
{
	int i;
	int c1, c2, c =  -1;
	sptbl_t tab[MAX_TABLE_SIZE] = {};
	sptbl_t *tab1, *tab2, *tspt;

	/** FIXME, test only; please read efuse to assign. **/
	/* wat = 80; */
	/* voltage = 1150; */
	/* degree = 30; */

	WARN_ON(spower_raw->table_size < 3);

	/** structurize the raw data **/
	spower_tab_construct(&tab, spower_raw);

	/** lookup tables which the chip type locates to **/
	for (i = 0; i < spower_raw->table_size; i++) {
		c = sptab_lookup(&tab[i], voltage, degree);
		/** table order: ff, tt, ss **/
		if (wat >= c)
			break;
	}

	/** FIXME,
	 * There are only 2 tables are used to interpolate to form SPTAB.
	 * Thus, sptab takes use of the container which raw data is not used anymore.
	 **/
	if (wat == c) {
		/** just match **/
		tab1 = tab2 = &tab[i];
		/** pointer duplicate  **/
		tspt = tab1;

		SPOWER_INFO("sptab equal to tab:%d/%d\n",  wat, c);

	} else if (i == spower_raw->table_size) {
		/** above all **/
#if defined(EXTER_POLATION)
		tab1 = &tab[spower_raw->table_size-2];
		tab2 = &tab[spower_raw->table_size-1];

		/** occupy the free container**/
		tspt = &tab[spower_raw->table_size-3];
#else /* #if defined (EXTER_POLATION) */
		tspt = tab1 = tab2 = &tab[spower_raw->table_size-1];
#endif /* #if defined (EXTER_POLATION) */

		SPOWER_INFO("sptab max tab:%d/%d\n",  wat, c);
	} else if (i == 0) {
#if defined(EXTER_POLATION)
		/** below all **/
		tab1 = &tab[0];
		tab2 = &tab[1];

		/** occupy the free container**/
		tspt = &tab[2];
#else /* #if defined (EXTER_POLATION) */
		tspt = tab1 = tab2 = &tab[0];
#endif /* #if defined (EXTER_POLATION) */

		SPOWER_INFO("sptab min tab:%d/%d\n",  wat, c);
	} else {
		/** anyone **/
		tab1 = &tab[i-1];
		tab2 = &tab[i];

		/** occupy the free container**/
		tspt = &tab[(i+1)%spower_raw->table_size];

		SPOWER_INFO("sptab interpolate tab:%d/%d\n",  wat, c);
	}


	/** sptab needs to interpolate 2 tables. **/
	if (tab1 != tab2) {
		c1 = sptab_lookup(tab1, voltage, degree);
		c2 = sptab_lookup(tab2, voltage, degree);

		interpolate_table(tspt, c1, c2, wat, tab1, tab2);
	}

	/** update to global data **/
	*spt = *tspt;

	return 0;
}




/* #define MT_SPOWER_UT 1 */

#if defined(MT_SPOWER_UT)
void mt_spower_ut(void)
{
	int v, t, p, i;

	for (i = 0; i < MT_SPOWER_MAX; i++) {
		sptbl_t *spt = &sptab[i];

		v = 750;
		t = 22;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 750;
		t = 25;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 750;
		t = 28;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 750;
		t = 82;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 750;
		t = 120;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 820;
		t = 22;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 820;
		t = 25;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 820;
		t = 28;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 820;
		t = 82;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);


		v = 820;
		t = 120;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 1200;
		t = 22;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 1200;
		t = 25;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 1200;
		t = 28;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 1200;
		t = 82;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);


		v = 1200;
		t = 120;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);


		v = 950;
		t = 80;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 1000;
		t = 85;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);


		v = 1150;
		t = 105;
		p  = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

	}
}


#endif /* #if defined (MT_SPOWER_UT) */

int mt_spower_init(void)
{
#define DEVINFO_IDX0 (68)
#define DEVINFO_IDX1 (5)
#define DEVINFO_MP0_BIT (8)
#define DEVINFO_GPU_BIT (24)

	int devinfo;
	int mp0;
	int gpu;

	/* avoid side effect from multiple invocation */
	if (tab_validate(&sptab[MT_SPOWER_CA7]))
		return 0;

	devinfo = (int)get_devinfo_with_index(DEVINFO_IDX0);
	pr_debug("Leakage efuse = 0x%x\n", devinfo);

	mp0 = (devinfo >> DEVINFO_MP0_BIT) & 0x0ff;
	gpu = (devinfo >> DEVINFO_GPU_BIT) & 0x0ff;

	if (mp0)
		mp0 = (int)devinfo_table[mp0];
	else
		mp0 = 150;
	if (gpu)
		gpu = (int)devinfo_table[gpu];
	else
		gpu = 115;

	mt_spower_make_table(&sptab[MT_SPOWER_CA7], &ca7_spower_raw, mp0, 1150, 30);
	mt_spower_make_table(&sptab[MT_SPOWER_GPU], &gpu_spower_raw, gpu, 1150, 30);

	return 0;
}

/* late_initcall(mt_spower_init); */


/** return 0, means sptab is not yet ready. **/
int mt_spower_get_leakage(int dev, int vol, int deg)
{
	WARN_ON(!(dev < MT_SPOWER_MAX));

	if (!tab_validate(&sptab[dev]))
		return 0;

	return sptab_lookup(&sptab[dev], vol, deg);
}
EXPORT_SYMBOL(mt_spower_get_leakage);

