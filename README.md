# Escape Room Puzzle Engine

A console-based escape room game written in C for Windows. Clear three themed rooms, each with three levels of multiple-choice questions, to escape.

Made for the **Programming Fundamentals** course at FAST-NUCES, Karachi (Section BSCY-B1).

<!-- Add screenshots here, for example: ![Title screen](screenshots/title.png) -->

## How the game works

- There are **3 rooms**, and each room has **3 levels** (Easy, Medium, Hard) with **5 questions** per level, so 45 questions in total.
- You need **at least 3 correct answers out of 5** to clear a level. A failed level must be replayed.
- Rooms open in order: clear all three levels of a room to unlock the next one.
- After Room 3, a victory screen and a final leaderboard show your scores.

| Room | Theme | Help available |
|------|-------|----------------|
| 1 | Riddles | Hints: 1 in Easy, 1 in Normal, 2 in Difficult |
| 2 | Mathematics | One 50-50 lifeline per level, first attempt only |
| 3 | C Programming | 50-50 lifeline, up to 2 uses per level |

## Features

- Multi-room, multi-level progression with a pass mark
- Hint system (Room 1) and 50-50 lifeline (Rooms 2 and 3)
- Colour-coded feedback: green for correct, red for incorrect
- ASCII art title, locked and open doors, key and trophy
- Score bar after every level, for example `[###--]  3/5`
- Room 3 results screen and a final leaderboard with room totals and a grand total out of 45

## Requirements

- Windows (the game uses `windows.h` and `conio.h`)
- GCC, for example through [MinGW-w64](https://www.mingw-w64.org/) or MSYS2
- Command Prompt or Windows Terminal, with the window tall enough to show about 30 lines

## Build and run

```
gcc final_game.c -o escape_room.exe
escape_room.exe
```

If you rename the source file, use its name in the `gcc` command.

## Controls

| Key | Action |
|-----|--------|
| `A` `B` `C` `D` | Choose an answer (not case-sensitive) |
| `L` | Use the 50-50 lifeline, where available |
| `Y` / `N` | Use a hint or skip it (Room 1) |
| Any key / `Enter` | Continue past a screen |

## Project structure

```
.
|-- final_game.c   # the whole game: Room 1, Room 2, Room 3, graphics and leaderboard
`-- README.md
```

Inside `final_game.c`:

- `main()` runs the game in order: `intro()`, `room1()`, `room2()`, `room3()`, the victory screen and the final leaderboard.
- `room1AskMCQ()` and `room2AskMCQ()` ask a question and check the answer. Room 3 asks its questions inside `room3level1()`, `room3level2()` and `room3level3()`.
- Score counters are kept in global variables, one correct and one incorrect counter for each level.
- `printFinalLeaderboard()` builds the final table from 2D arrays of those counters.
- Functions starting with `gfx` draw the title, doors, key, score bars and trophy.

## Limitations and ideas for the future

- Progress and scores are not saved when the program closes.
- Questions and answers are fixed in the code, so every playthrough is the same.
- Windows only, and text-only (no graphical window).
- Planned improvements: save and resume using files, a leaderboard file with names and times, questions loaded from a file, randomised puzzles, and key items that unlock later rooms.

## Authors

- Maheen Shafiq (26K-2050)
- Faaleha Haq (26K-2052)
- Shanzay Siddiqui (26K-2059)

## License

[Choose a license, for example MIT, and add a `LICENSE` file.]
