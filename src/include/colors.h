#ifndef COLORS_H_INCLUDED
#define COLORS_H_INCLUDED

#define IRC_WHITE   "\00300"
#define IRC_BLACK   "\00301"
#define IRC_DBLUE   "\00302"
#define IRC_GREEN   "\00303"
#define IRC_RED     "\00304"
#define IRC_BROWN   "\00305"
#define IRC_PURPLE  "\00306"
#define IRC_ORANGE  "\00307"
#define IRC_YELLOW  "\00308"
#define IRC_LGREEN  "\00309"
#define IRC_CYAN    "\00310"
#define IRC_LCYAN   "\00311"
#define IRC_LBLUE   "\00312"
#define IRC_PINK    "\00313"
#define IRC_GREY    "\00314"
#define IRC_LGREY   "\00315"
#define IRC_BOLD    "\002"
#define IRC_UNDRL   "\037"
#define IRC_NORMAL  "\017"

#define ANSI_RED    "\033[0;31m"
#define ANSI_GREEN  "\033[0;32m"
#define ANSI_YELLOW "\033[0;33m"
#define ANSI_BOLD "\033[1m"
#define ANSI_NORMAL "\033[0m"

#define ERR "["ANSI_RED ANSI_BOLD"!"ANSI_NORMAL"] "
#define WARN "["ANSI_YELLOW ANSI_BOLD"?"ANSI_NORMAL"] "
#define INFO "["ANSI_GREEN ANSI_BOLD"*"ANSI_NORMAL"] "

#endif
