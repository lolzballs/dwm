/* See LICENSE file for copyright and license details. */

#include "gaplessgrid.c"

#define NUMCOLORS 17

/* appearance */
static const char *fonts[] = {
    "monospace:size=10",
    "FontAwesome:size=10"
};
static const char dmenufont[]       = "monospace:size=10";
static const char dwmpath[]         = "/home/bcheng/src/dwm/dwm";
static const unsigned int gappx     = 8;        /* gaps between windows */
static const unsigned int tagpadding = 50;      /* inner padding of tags */
static const unsigned int taglinepx = 2;        /* size of tagline */
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, 0: display systray on the last monitor*/
static const int showsystray        = 1;        /* 0 means no systray */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */

static const char colors[NUMCOLORS][MAXCOLORS][17] = {
    /* border    fg         bg */
    { "#282828", "#928374", "#282828" },        /* [0]  01 - Client normal */
    { "#ebdbb2", "#458588", "#282828" },        /* [1]  02 - Client selected */
    { "#83a598", "#fb4934", "#282828" },        /* [2]  03 - Client urgent */
    { "#83a598", "#83a598", "#282828" },        /* [3]  04 - Client occupied */
    { "#282828", "#fb4934", "#282828" },        /* [4]  05 - Red */
    { "#282828", "#fabd2f", "#282828" },        /* [5]  06 - Yellow */
    { "#282828", "#b8bb26", "#282828" },        /* [6]  07 - Green */
    { "#282828", "#928374", "#282828" },        /* [7]  08 - Dark grey */
    { "#282828", "#d5c4a1", "#282828" },        /* [8]  09 - Light grey */
    { "#928374", "#928374", "#282828" },        /* [9]  0A - Bar normal*/
    { "#3c3836", "#a89985", "#282828" },        /* [10] 0B - Bar selected*/
    { "#fb4934", "#fb4934", "#282828" },        /* [11] 0C - Bar urgent*/
    { "#928374", "#458588", "#282828" },        /* [12] 0D - Bar occupied*/
    { "#3c3836", "#3c3836", "#282828" },        /* [13] 0E - Tag normal*/
    { "#83a598", "#83a598", "#282828" },        /* [14] 0F - Tag selected*/
    { "#fb4934", "#fb4934", "#282828" },        /* [15] 10 - Tag urgent*/
    { "#3c3836", "#928374", "#282828" },        /* [16] 11 - Tag occupied*/
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 0 means don't respect size hints in tiled resizals */

static const Layout layouts[] = {
    /* symbol     gaps,      arrange function */
    { "[]=",      True,      tile },    /* first entry is default */
    { "><>",      True,      NULL },    /* no layout function means floating behavior */
    { "[M]",      True,      monocle },
    { "[G]",      True,      gaplessgrid },
    { "=[]",      True,      bstack }
};

/* tagging */
static const Tag tags[] = {
	/* name     layout         mfact     nmaster */
	{"",       &layouts[0],   -1,       -1},
	{"",       &layouts[2],   -1,       -1},
	{"",       &layouts[3],   -1,       -1},
	{"",       &layouts[3],   -1,       -1},
	{"5",       &layouts[0],   -1,       -1},
	{"6",       &layouts[0],   -1,       -1}
};

static const Rule rules[] = {
    /* class      instance    title       tags mask     isfloating   monitor */
    { "Firefox",  NULL,       NULL,       1 << 1,       0,           -1 },
    { "Thunar",   NULL,       NULL,       1 << 3,       0,           -1 }
};

/* key definitions */
#define ALTKEY Mod1Mask
#define WINKEY Mod4Mask
#define MODKEY WINKEY
#define TAGKEYS(KEY,TAG) \
    { MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
    { MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", colors[0][2], "-nf", colors[0][1], "-sb", colors[1][2], "-sf", colors[1][1], NULL };
static const char *termcmd[]  = { "urxvt", NULL };
static const char *filecmd[]  = { "thunar", NULL };

static Key keys[] = {
    /* modifier                     key        function        argument */
    { MODKEY,                       XK_d,      spawn,          {.v = dmenucmd } },
    { MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
    { MODKEY,                       XK_n,      spawn,          {.v = filecmd } },
    { MODKEY,                       XK_b,      togglebar,      {0} },
    { MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
    { MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
    { MODKEY,                       XK_u,      incnmaster,     {.i = +1 } },
    { MODKEY,                       XK_i,      incnmaster,     {.i = -1 } },
    { MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
    { MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
    { MODKEY,                       XK_p,      zoom,           {0} },
    { MODKEY,                       XK_Tab,    view,           {0} },
    { MODKEY|ShiftMask,             XK_q,      killclient,     {0} },
    { MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
    { MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
    { MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
    { MODKEY,                       XK_g,      setlayout,      {.v = &layouts[3]} },
    { MODKEY,                       XK_space,  setlayout,      {0} },
    { MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
    { MODKEY,                       XK_0,      view,           {.ui = ~0 } },
    { MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
    { MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
    { MODKEY,                       XK_period, focusmon,       {.i = +1 } },
    { MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
    { MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
    { ALTKEY|ShiftMask,             XK_Delete, self_restart,   {0} },
    TAGKEYS(                        XK_1,                      0)
    TAGKEYS(                        XK_2,                      1)
    TAGKEYS(                        XK_3,                      2)
    TAGKEYS(                        XK_4,                      3)
    TAGKEYS(                        XK_5,                      4)
    TAGKEYS(                        XK_6,                      5)
    { MODKEY|ShiftMask,             XK_Delete, quit,           {0} },
};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
    /* click                event mask      button          function        argument */
    { ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
    { ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
    { ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
    { ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
    { ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
    { ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
    { ClkTagBar,            0,              Button1,        view,           {0} },
    { ClkTagBar,            0,              Button3,        toggleview,     {0} },
    { ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
    { ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

