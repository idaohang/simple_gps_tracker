import curses

screen = curses.initscr()
curses.noecho()
curses.cbreak()
curses.curs_set(0)
screen.keypad(1)