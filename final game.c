#ifndef LIB_H
#define LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <windows.h>
#include <conio.h>


extern HANDLE h;


void gfxColor(int c);
void gfxPressAnyKey(void);
void gfxProgressBar(const char *label, int totalMs, int color);
void gfxScoreBar(int score, int total);
void gfxDoor(int open, int room, int color);
void gfxRoomIntro(int room, const char *name, int color);
void gfxKey(void);
void gfxTitle(void);
void gfxVictory(void);


#define TOTAL_LEVELS 9
#define SAVE_FILE    "savegame.dat"
#define SCORE_FILE   "highscores.txt"

int  saveProgress(const char *name, int levelsDone, const int correct[], const int incorrect[]);
int  loadProgress(char *name, int nameSize, int *levelsDone, int correct[], int incorrect[]);
void deleteSave(void);
void saveHighScore(const char *name, int score);
void showHighScores(void);

#endif


HANDLE h;

void gfxColor(int c)
{
    SetConsoleTextAttribute(h, c);
}

void gfxPressAnyKey(void)
{
    gfxColor(8);
    printf("\n  Press any key to continue...");
    gfxColor(7);
    fflush(stdout);
    getch();
    system("cls");
}


void gfxProgressBar(const char *label, int totalMs, int color)
{
    int i;
    gfxColor(color);
    printf("\n  %s\n\n  [", label);
    for(i = 0; i < 30; i++)
    {
        printf("#");
        fflush(stdout);
        Sleep(totalMs / 30);
    }
    printf("] 100%%\n");
    gfxColor(7);
}


void gfxScoreBar(int score, int total)
{
    int i;
    gfxColor(score >= 3 ? 10 : 12);
    printf("\n  Score  [");
    for(i = 0; i < total; i++)
        putchar(i < score ? '#' : '-');
    printf("]  %d/%d\n", score, total);
    gfxColor(7);
}

void gfxDoor(int open, int room, int color)
{
    gfxColor(color);
    printf("\n");
    printf("            .---------------.\n");
    printf("            |  .---------.  |\n");
    if(open)
    {
        printf("            |  |:::::::::|  |\n");
        printf("            |  |:ROOM %d::|  |\n", room);
        printf("            |  |::OPEN:::|  |\n");
        printf("            |  '---------'  |\n");
        printf("            |            o  |\n");
        printf("            |   [UNLOCKED]  |\n");
    }
    else
    {
        printf("            |  |         |  |\n");
        printf("            |  | ROOM %d  |  |\n", room);
        printf("            |  |         |  |\n");
        printf("            |  '---------'  |\n");
        printf("            |            o  |\n");
        printf("            |   [LOCKED]    |\n");
    }
    printf("            '---------------'\n");
    gfxColor(7);
}


void gfxRoomIntro(int room, const char *name, int color)
{
    system("cls");
    gfxDoor(0, room, 12);
    gfxColor(color);
    printf("\n            %s\n", name);
    gfxProgressBar("Picking the lock...", (room == 3) ? 3500 : 1500, 14);
    Sleep((room == 3) ? 1500 : 300);
    system("cls");
    gfxDoor(1, room, 10);
    gfxColor(10);
    printf("\n            Door unlocked! Entering ROOM %d...\n", room);
    gfxColor(7);
    Sleep((room == 3) ? 3500 : 1500);
    system("cls");
}

void gfxKey(void)
{
    gfxColor(14);
    printf("\n");
    printf("              .--.\n");
    printf("             /.-. '----------.\n");
    printf("             \\'-' .--\"--\"\"-\"-'\n");
    printf("              '--'\n");
    printf("        YOU FOUND THE KEY TO THE NEXT ROOM!\n");
    gfxColor(7);
}

void gfxTitle(void)
{
    gfxColor(13);
    printf("\n");
    printf("  ============================================================\n");
    printf("\n");
    printf("        #####   ####   ####   ###   ####   #####\n");
    printf("        #      #      #      #   #  #   #  #\n");
    printf("        ####    ###   #      #####  ####   ####\n");
    printf("        #          #  #      #   #  #      #\n");
    printf("        #####  ####    ####  #   #  #      #####\n");
    printf("        \n");
    printf("               ####    ###    ###   #   #\n");
    printf("               #   #  #   #  #   #  ## ##\n");
    printf("               ####   #   #  #   #  # # #\n");
    printf("               #  #   #   #  #   #  #   #\n");
    printf("               #   #   ###    ###   #   #\n");
    printf("\n");
    gfxColor(11);
    printf("               A  C A D E M I C   E S C A P E\n");
    gfxColor(13);
    printf("  ============================================================\n");
    gfxColor(7);
}

void gfxVictory(void)
{
    system("cls");
    gfxColor(14);
    printf("\n");
    printf("                      ___________\n");
    printf("                     '._==_==_=_.'\n");
    printf("                     .-\\:      /-.\n");
    printf("                    | (|:.     |) |\n");
    printf("                     '-|:.     |-'\n");
    printf("                       \\::.    /\n");
    printf("                        '::. .'\n");
    printf("                          ) (\n");
    printf("                        _.' '._\n");
    printf("                       '-------'\n");
    gfxColor(10);
    printf("\n         ==============================================\n");
    printf("               CONGRATULATIONS!  YOU ESCAPED!\n");
    printf("                 All 3 rooms. All 9 levels. Done.\n");
    printf("         ==============================================\n");
    gfxColor(7);
    gfxPressAnyKey();
}


int saveProgress(const char *name, int levelsDone, const int correct[], const int incorrect[])
{
    int i;
    FILE *f = fopen(SAVE_FILE, "w");
    if(f == NULL)
        return 0;

    fprintf(f, "ESCAPEROOM1\n%s\n%d\n", name, levelsDone);
    for(i = 0; i < TOTAL_LEVELS; i++)
        fprintf(f, "%d %d\n", correct[i], incorrect[i]);

    fclose(f);
    return 1;
}

/* returns 1 if a valid save was loaded, 0 otherwise */
int loadProgress(char *name, int nameSize, int *levelsDone, int correct[], int incorrect[])
{
    char line[128];
    int i, done;
    size_t len;
    FILE *f = fopen(SAVE_FILE, "r");
    if(f == NULL)
        return 0;

    if(fgets(line, sizeof(line), f) == NULL || strncmp(line, "ESCAPEROOM1", 11) != 0)
    { fclose(f); return 0; }

    if(fgets(name, nameSize, f) == NULL)
    { fclose(f); return 0; }
    len = strlen(name);
    while(len > 0 && (name[len-1] == '\n' || name[len-1] == '\r'))
        name[--len] = '\0';

    if(fscanf(f, "%d", &done) != 1 || done < 0 || done > TOTAL_LEVELS)
    { fclose(f); return 0; }

    for(i = 0; i < TOTAL_LEVELS; i++)
    {
        if(fscanf(f, "%d %d", &correct[i], &incorrect[i]) != 2 ||
           correct[i] < 0 || correct[i] > 5 || incorrect[i] < 0 || incorrect[i] > 5)
        { fclose(f); return 0; }
    }

    fclose(f);
    *levelsDone = done;
    return 1;
}

void deleteSave(void)
{
    remove(SAVE_FILE);
}

void saveHighScore(const char *name, int score)
{
    FILE *f = fopen(SCORE_FILE, "a");
    if(f == NULL)
        return;
    fprintf(f, "%d %s\n", score, name);
    fclose(f);
}

void showHighScores(void)
{
    char names[100][32];
    int scores[100];
    char line[128];
    int n = 0, i, j, top;
    FILE *f = fopen(SCORE_FILE, "r");

    if(f != NULL)
    {
        while(n < 100 && fgets(line, sizeof(line), f) != NULL)
        {
            int s;
            char nm[32];
            if(sscanf(line, "%d %31[^\n\r]", &s, nm) == 2)
            {
                scores[n] = s;
                strcpy(names[n], nm);
                n++;
            }
        }
        fclose(f);
    }


    for(i = 0; i < n - 1; i++)
        for(j = i + 1; j < n; j++)
            if(scores[j] > scores[i])
            {
                int ts = scores[i]; char tn[32];
                scores[i] = scores[j]; scores[j] = ts;
                strcpy(tn, names[i]); strcpy(names[i], names[j]); strcpy(names[j], tn);
            }

    gfxColor(14);
    printf("\n==================== TOP 5 PLAYERS ====================\n");
    gfxColor(7);
    top = (n < 5) ? n : 5;
    if(top == 0)
        printf("  No scores yet.\n");
    for(i = 0; i < top; i++)
        printf("  %d.  %-20s %3d/45\n", i + 1, names[i], scores[i]);
    gfxColor(14);
    printf("=======================================================\n");
    gfxColor(7);
}



int levelsDone = 0;
char playerName[32] = "Player";
void autosave(void);



// room 1
int roomonelevelonecorrect = 0;
int roomoneleveloneincorrect = 0;

int roomoneleveltwocorrect = 0;
int roomoneleveltwoincorrect = 0;

int roomonelevelthreecorrect = 0;
int roomonelevelthreeincorrect = 0;


int room1AskMCQ(const char question[], const char optionA[], const char optionB[],
                const char optionC[], const char optionD[], char correctOption,
                const char hint[], int *hintsLeft)
{
    char userAnswer;
    char choice;

    printf("\n%s\n", question);
    printf("A. %s\n", optionA);
    printf("B. %s\n", optionB);
    printf("C. %s\n", optionC);
    printf("D. %s\n", optionD);

    if(hintsLeft != NULL && hint != NULL && hint[0] != '\0' && *hintsLeft > 0)
    {
        printf("\nWould you like to use a hint? (Y/N): ");
        scanf(" %c", &choice);

        choice = toupper((unsigned char)choice);

        if(choice == 'Y')
        {
            printf("\nHint: %s\n", hint);

            (*hintsLeft)--;

            printf("Hints remaining: %d\n", *hintsLeft);
        }
    }

    printf("\nEnter your answer (A/B/C/D): ");
    scanf(" %c", &userAnswer);

    userAnswer = toupper((unsigned char)userAnswer);

    if(userAnswer == correctOption)
    {
        SetConsoleTextAttribute(h, 10);
        printf("\nCorrect Answer!\n");
        Sleep(1500);
        system("cls");
        SetConsoleTextAttribute(h, 7);
        return 1;
    }
    else
    {
        SetConsoleTextAttribute(h, 12);
        printf("\nIncorrect Answer!\n");
        Sleep(1500);
        system("cls");
        SetConsoleTextAttribute(h, 7);
        return 0;
    }
}

int room1level1()
{
    while(1)
    {
        int hintsLeft = 1;

        roomonelevelonecorrect = 0;
        roomoneleveloneincorrect = 0;

        system("cls");
        SetConsoleTextAttribute(h, 13);
        printf("\n================ ROOM 1 | LEVEL 1 OF 3 | EASY ================\n");
        SetConsoleTextAttribute(h, 7);
        printf("You have 1 hint for the entire level.\n");

        if(room1AskMCQ("1. What gets wetter the more it dries?",
        "Sponge", "Towel", "Tissue", "Cloth", 'B',
        "It is something you use after taking a bath.", &hintsLeft))
            roomonelevelonecorrect++;
        else
            roomoneleveloneincorrect++;

        if(room1AskMCQ("2. Which musical instrument has 88 keys?",
        "Organ", "Harpsichord", "Piano", "Accordion", 'C',
        "It has black and white keys.", &hintsLeft))
            roomonelevelonecorrect++;
        else
            roomoneleveloneincorrect++;

        if(room1AskMCQ("3. What follows you in daylight but disappears in complete darkness?",
        "Reflection", "Shadow", "Silhouette", "Footprint", 'B',
        "It is made when something blocks light.", &hintsLeft))
            roomonelevelonecorrect++;
        else
            roomoneleveloneincorrect++;

        if(room1AskMCQ("4. What has many teeth and is used to untangle hair?",
        "Brush", "Comb", "Razor", "Clip", 'B',
        "It is a small grooming tool with a row of teeth.", &hintsLeft))
            roomonelevelonecorrect++;
        else
            roomoneleveloneincorrect++;

        if(room1AskMCQ("5. David's father has three sons: Snap, Crackle and _____?",
        "Pop", "David", "Jack", "Crackle", 'B',
        "The question names David as one of the sons.", &hintsLeft))
            roomonelevelonecorrect++;
        else
            roomoneleveloneincorrect++;

        SetConsoleTextAttribute(h, 14);
        printf("\n========== EASY LEVEL RESULTS ==========\n");

        printf("Correct Answers: %d/5\n", roomonelevelonecorrect);
        printf("Incorrect Answers: %d/5\n", roomoneleveloneincorrect);
        gfxScoreBar(roomonelevelonecorrect, 5);
        SetConsoleTextAttribute(h, 7);

        if(roomonelevelonecorrect >= 3)
        {
            SetConsoleTextAttribute(h, 10);
            printf("\nCongratulations! EASY LEVEL CLEARED!\n");
            printf("You have unlocked the NORMAL LEVEL!\n");
            Sleep(3500);
            system("cls");
            SetConsoleTextAttribute(h, 7);
            return 1;
        }
        else
        {
            SetConsoleTextAttribute(h, 12);
            printf("\nYou failed the EASY LEVEL!\n");
            printf("You need at least 3 correct answers to pass.\n");
            printf("Please try again!\n");
            Sleep(3500);
            system("cls");
            SetConsoleTextAttribute(h, 7);
        }
    }
}

int room1level2()
{
    while(1)
    {
        int hintsLeft = 1;

        roomoneleveltwocorrect = 0;
        roomoneleveltwoincorrect = 0;

        system("cls");
        SetConsoleTextAttribute(h, 13);
        printf("\n=============== ROOM 1 | LEVEL 2 OF 3 | NORMAL ===============\n");
        SetConsoleTextAttribute(h, 7);
        printf("You have 1 hint for the entire level.\n");
        printf("You may use it on any question you choose.\n");

        if(room1AskMCQ("1. The more you take, the more you leave behind. What am I?",
        "Footprints", "Footsteps", "Tracks", "Shadows", 'B',
        "Think about what remains on the ground after someone walks.",
        &hintsLeft))
            roomoneleveltwocorrect++;
        else
            roomoneleveltwoincorrect++;

        if(room1AskMCQ("2. What travels around the world while staying in one corner?",
        "Postcard", "Envelope", "Stamp", "Sticker", 'C',
        "Think about something attached to mail before it is sent.",
        &hintsLeft))
            roomoneleveltwocorrect++;
        else
            roomoneleveltwoincorrect++;

        if(room1AskMCQ("3. I have cities but no houses, forests but no trees, and water but no fish. What am I?",
        "Globe", "Map", "Atlas", "Blueprint", 'B',
        "Think about something that represents places without containing them.",
        &hintsLeft))
            roomoneleveltwocorrect++;
        else
            roomoneleveltwoincorrect++;

        if(room1AskMCQ("4. What can fill an entire room but takes up no physical space?",
        "Sound", "Air", "Light", "Heat", 'C',
        "Think about something that allows you to see objects around you.",
        &hintsLeft))
            roomoneleveltwocorrect++;
        else
            roomoneleveltwoincorrect++;

        if(room1AskMCQ("5. What has one eye but cannot see?",
        "Camera", "Needle", "Storm", "Potato", 'B',
        "Think about a small opening found in a sewing tool.",
        &hintsLeft))
            roomoneleveltwocorrect++;
        else
            roomoneleveltwoincorrect++;

        SetConsoleTextAttribute(h, 14);
        printf("\n========== NORMAL LEVEL RESULTS ==========\n");

        printf("Correct Answers: %d/5\n", roomoneleveltwocorrect);
        printf("Incorrect Answers: %d/5\n", roomoneleveltwoincorrect);
        gfxScoreBar(roomoneleveltwocorrect, 5);
        SetConsoleTextAttribute(h, 7);

        if(roomoneleveltwocorrect >= 3)
        {
            SetConsoleTextAttribute(h, 10);
            printf("\nCongratulations! NORMAL LEVEL CLEARED!\n");
            printf("You have unlocked the DIFFICULT LEVEL!\n");
            Sleep(3500);
            system("cls");
            SetConsoleTextAttribute(h, 7);
            return 1;
        }
        else
        {
            SetConsoleTextAttribute(h, 12);
            printf("\nYou failed the NORMAL LEVEL!\n");
            printf("You need at least 3 correct answers to pass.\n");
            printf("Please try again!\n");
            Sleep(3500);
            system("cls");
            SetConsoleTextAttribute(h, 7);
        }
    }
}

int room1level3()
{
    while(1)
    {
        int hintsLeft = 2;

        roomonelevelthreecorrect = 0;
        roomonelevelthreeincorrect = 0;

        system("cls");
        SetConsoleTextAttribute(h, 13);
        printf("\n============== ROOM 1 | LEVEL 3 OF 3 | DIFFICULT ==============\n");
        SetConsoleTextAttribute(h, 7);
        printf("You have 2 hints for the entire level.\n");
        printf("You may use them on any two separate questions.\n");

        if(room1AskMCQ("1. I speak without a mouth and hear without ears. I have no body, but come alive with the wind. What am I?",
        "Whisper", "Thunder", "Echo", "Wind", 'C',
        "It is something you hear when a sound returns to you.",
        &hintsLeft))
            roomonelevelthreecorrect++;
        else
            roomonelevelthreeincorrect++;

        if(room1AskMCQ("2. What disappears as soon as you say its name?",
        "Secret", "Silence", "Darkness", "Quietness", 'B',
        "Think about the absence of sound.",
        &hintsLeft))
            roomonelevelthreecorrect++;
        else
            roomonelevelthreeincorrect++;

        if(room1AskMCQ("3. I am taken from a mine, ground into powder, and used inside a pencil to leave marks. What am I?",
        "Charcoal", "Coal", "Graphite", "Ink", 'C',
        "It is a naturally occurring form of carbon.",
        &hintsLeft))
            roomonelevelthreecorrect++;
        else
            roomonelevelthreeincorrect++;

        if(room1AskMCQ("4. I have keys, a space bar, and an enter key, but no locks or rooms. What am I?",
        "Remote Control", "Piano", "Calculator", "Keyboard", 'D',
        "Think about a device used to type words.",
        &hintsLeft))
            roomonelevelthreecorrect++;
        else
            roomonelevelthreeincorrect++;

        if(room1AskMCQ("5. The more there is of me, the less you are able to see. What am I?",
        "Fog", "Smoke", "Darkness", "Shadow", 'C',
        "It is the opposite of brightness.",
        &hintsLeft))
            roomonelevelthreecorrect++;
        else
            roomonelevelthreeincorrect++;

        SetConsoleTextAttribute(h, 14);
        printf("\n========== DIFFICULT LEVEL RESULTS ==========\n");

        printf("Correct Answers: %d/5\n", roomonelevelthreecorrect);
        printf("Incorrect Answers: %d/5\n", roomonelevelthreeincorrect);
        gfxScoreBar(roomonelevelthreecorrect, 5);
        SetConsoleTextAttribute(h, 7);

        if(roomonelevelthreecorrect >= 3)
        {
            SetConsoleTextAttribute(h, 10);
            printf("\nCongratulations! DIFFICULT LEVEL CLEARED!\n");
            Sleep(3500);
            system("cls");
            SetConsoleTextAttribute(h, 7);
            return 1;
        }
        else
        {
            SetConsoleTextAttribute(h, 12);
            printf("\nYou failed the DIFFICULT LEVEL!\n");
            printf("You need at least 3 correct answers to pass.\n");
            printf("Please try again!\n");
            Sleep(3500);
            system("cls");
            SetConsoleTextAttribute(h, 7);
        }
    }
}

int room1()
{
    int totalCorrect;
    int totalIncorrect;

    if(levelsDone == 0)
    {
        gfxRoomIntro(1, "ROOM 1  -  RIDDLES", 13);
        system("cls");
        SetConsoleTextAttribute(h, 13);
        printf("\n====================================\n");
        printf("          WELCOME TO ROOM 1\n");
        printf("             RIDDLE ROOM\n");
        printf("====================================\n");
        SetConsoleTextAttribute(h, 7);

        SetConsoleTextAttribute(h, 11);
        printf("\n========== INSTRUCTIONS ==========\n");
        SetConsoleTextAttribute(h, 7);
        printf("- There are 3 difficulty levels.\n");
        printf("- Each level contains 5 questions.\n");
        printf("- You need at least 3 correct answers out of 5 to pass.\n");
        printf("- Clear each level to unlock the next one.\n");
        printf("- EASY MODE: One hint is available for the entire level.\n");
        printf("- NORMAL MODE: One hint can be used on any one question.\n");
        printf("- DIFFICULT MODE: Two hints can be used on any two separate questions.\n");
        printf("- Incorrect answers will not reveal the correct option.\n");
        printf("- Failed levels must be attempted again.\n");

        gfxPressAnyKey();
    }

    if(levelsDone < 1) { room1level1(); levelsDone = 1; autosave(); }
    if(levelsDone < 2) { room1level2(); levelsDone = 2; autosave(); }
    if(levelsDone < 3) { room1level3(); levelsDone = 3; autosave(); }

    SetConsoleTextAttribute(h, 14);
    printf("\n====================================\n");
    printf("          ROOM 1 LEADERBOARD\n");
    printf("====================================\n");
    SetConsoleTextAttribute(h, 7);

    printf("\nEasy Level:\n");
    printf("Correct: %d | Incorrect: %d\n",roomonelevelonecorrect, roomoneleveloneincorrect);

    printf("\nNormal Level:\n");
    printf("Correct: %d | Incorrect: %d\n",roomoneleveltwocorrect, roomoneleveltwoincorrect);

    printf("\nDifficult Level:\n");
    printf("Correct: %d | Incorrect: %d\n",roomonelevelthreecorrect, roomonelevelthreeincorrect);

    totalCorrect = roomonelevelonecorrect + roomoneleveltwocorrect + roomonelevelthreecorrect;

    totalIncorrect = roomoneleveloneincorrect + roomoneleveltwoincorrect + roomonelevelthreeincorrect;

    printf("\nTotal Correct Answers: %d\n", totalCorrect);
    printf("Total Incorrect Answers: %d\n", totalIncorrect);
    printf("Final Score: %d/15\n", totalCorrect);

    SetConsoleTextAttribute(h, 10);
    printf("\n====================================\n");
    printf(" CONGRATULATIONS! YOU PASSED ROOM 1!\n");
    printf(" You have successfully completed all three levels :D.\n");
    printf(" Proceeding to ROOM 2...\n");
    printf("====================================\n");
    gfxKey();
    gfxPressAnyKey();
    SetConsoleTextAttribute(h, 7);

    return 1;
}


//             room 2

int r2_lvl1_correct=0;
int r2_lvl1_incorrect=0;

int r2_lvl2_correct=0;
int r2_lvl2_incorrect=0;

int r2_lvl3_correct=0;
int r2_lvl3_incorrect=0;

int room2AskMCQ(const char question[],const char optionA[],const char optionB[],const char optionC[],const char optionD[],char correctAnswer,int *lifelineAvailable)
{
    char answer;
    const char *options[4]={optionA,optionB,optionC,optionD};
    char labels[4]={'A','B','C','D'};
    int correctIndex=correctAnswer-'A';
    int wrongIndex=-1;
    int i;

    printf("\n%s\n",question);

    for(i=0;i<4;i++)
    {
        printf("\n%c. %s",labels[i],options[i]);
    }

    printf("\n\nEnter your answer (A/B/C/D or L for 50-50): ");
    scanf(" %c",&answer);
    answer=toupper((unsigned char)answer);

    while(answer=='L')
    {
        if(*lifelineAvailable==1)
        {
            *lifelineAvailable=0;

            for(i=0;i<4;i++)
            {
                if(i!=correctIndex)
                {
                    wrongIndex=i;
                    break;
                }
            }

            SetConsoleTextAttribute(h, 14);
            printf("\n50-50 LIFELINE ACTIVATED!");
            printf("\nTwo incorrect options have been removed.\n");
            SetConsoleTextAttribute(h, 7);

            for(i=0;i<4;i++)
            {
                if(i==correctIndex || i==wrongIndex)
                {
                    printf("\n%c. %s",labels[i],options[i]);
                }
            }
        }
        else
        {
            SetConsoleTextAttribute(h, 12);
            printf("\n50-50 lifeline is not available!");
            SetConsoleTextAttribute(h, 7);
        }

        printf("\n\nEnter your answer (A/B/C/D): ");
        scanf(" %c",&answer);
        answer=toupper((unsigned char)answer);
    }

    if(answer!='A' && answer!='B' && answer!='C' && answer!='D')
    {
        SetConsoleTextAttribute(h, 12);
        printf("\nInvalid answer! Counted as incorrect.\n");
        Sleep(1500);
        system("cls");
        SetConsoleTextAttribute(h, 7);
        return 0;
    }

    if(answer==correctAnswer)
    {
        SetConsoleTextAttribute(h, 10);
        printf("\nCorrect Answer!\n");
        Sleep(1500);
        system("cls");
        SetConsoleTextAttribute(h, 7);
        return 1;
    }
    else
    {
        SetConsoleTextAttribute(h, 12);
        printf("\nIncorrect Answer!\n");
        Sleep(1500);
        system("cls");
        SetConsoleTextAttribute(h, 7);
        return 0;
    }
}

int room2level1(int firstAttempt)
{
    int score=0;
    int lifelineAvailable=firstAttempt;

    SetConsoleTextAttribute(h, 11);
    printf("\n\n\t\t========== ROOM 2 | LEVEL 1 OF 3 | EASY ==========\n");
    SetConsoleTextAttribute(h, 7);

    score+=room2AskMCQ("Question 1. What is 25 percent of 360?","90","80","100","120",'A',&lifelineAvailable);

    score+=room2AskMCQ("Question 2. Solve: 3x + 7 = 28.","5","7","6","8",'C',&lifelineAvailable);

    score+=room2AskMCQ("Question 3. What is the square root of 2025?","35","45","40","50",'B',&lifelineAvailable);

    score+=room2AskMCQ("Question 4. Divide 120 in the ratio 2:3. What is the larger share?","48","72","60","80",'B',&lifelineAvailable);

    score+=room2AskMCQ("Question 5. Find the area of a rectangle with length 12 cm and width 8 cm.","96","80","88","100",'A',&lifelineAvailable);

    r2_lvl1_correct=score;
    r2_lvl1_incorrect=5-score;

    SetConsoleTextAttribute(h, 14);
    printf("\nLEVEL 1 SCORE: %d/5",score);
    gfxScoreBar(score, 5);
    SetConsoleTextAttribute(h, 7);

    return score;
}

int room2level2(int firstAttempt)
{
    int score=0;
    int lifelineAvailable=firstAttempt;

    SetConsoleTextAttribute(h, 11);
    printf("\n\n\t\t========== ROOM 2 | LEVEL 2 OF 3 | MEDIUM ==========\n");
    SetConsoleTextAttribute(h, 7);

    score+=room2AskMCQ("Question 1. Solve: x^2 - 7x + 12 = 0.","1 and 12","3 and 4","2 and 6","4 and 5",'B',&lifelineAvailable);

    score+=room2AskMCQ("Question 2. Simplify: 2 raised to power 3 multiplied by 2 raised to power 4.","32","64","128","256",'C',&lifelineAvailable);

    score+=room2AskMCQ("Question 3. Find the value of log base 2 of 64.","4","6","5","8",'B',&lifelineAvailable);

    score+=room2AskMCQ("Question 4. Two numbers have a sum of 15 and a difference of 3. What is the larger number?","9","6","8","12",'A',&lifelineAvailable);

    score+=room2AskMCQ("Question 5. What is the probability of getting an even number when rolling a fair dice?","1/6","1/3","1/2","2/3",'C',&lifelineAvailable);

    r2_lvl2_correct=score;
    r2_lvl2_incorrect=5-score;

    SetConsoleTextAttribute(h, 14);
    printf("\nLEVEL 2 SCORE: %d/5",score);
    gfxScoreBar(score, 5);
    SetConsoleTextAttribute(h, 7);

    return score;
}

int room2level3(int firstAttempt)
{
    int score=0;
    int lifelineAvailable=firstAttempt;

    SetConsoleTextAttribute(h, 11);
    printf("\n\n\t\t========== ROOM 2 | LEVEL 3 OF 3 | HARD ==========\n");
    SetConsoleTextAttribute(h, 7);

    score+=room2AskMCQ("Question 1. If f(x) = 2x + 5, what is f(3)?","9","10","11","12",'C',&lifelineAvailable);

    score+=room2AskMCQ("Question 2. Evaluate: lim x->2 (x + 3).","5","3","4","6",'A',&lifelineAvailable);

    score+=room2AskMCQ("Question 3. Differentiate: f(x) = x^3.","3x^2","2x","x^2","3x",'A',&lifelineAvailable);

    score+=room2AskMCQ("Question 4. Differentiate: f(x) = 5x^2 + 3x.","5x + 3","10x + 3","10x","5x^2",'B',&lifelineAvailable);

    score+=room2AskMCQ("Question 5. Integrate: integral of 2x dx.","2x^2 + C","x + C","x^2 + C","x^2/2 + C",'C',&lifelineAvailable);

    r2_lvl3_correct=score;
    r2_lvl3_incorrect=5-score;

    SetConsoleTextAttribute(h, 14);
    printf("\nLEVEL 3 SCORE: %d/5",score);
    gfxScoreBar(score, 5);
    SetConsoleTextAttribute(h, 7);

    return score;
}

void room2()
{
    int score1=0,score2=0,score3=0;
    int firstAttempt1=1;
    int firstAttempt2=1;
    int firstAttempt3=1;

    if(levelsDone == 3)
    {
        gfxRoomIntro(2, "ROOM 2  -  MATHEMATICS", 11);

        SetConsoleTextAttribute(h, 13);
        printf("\n\n\t\t====================================");
        printf("\n\t\t   WELCOME TO MATHEMATICS ESCAPE ROOM");
        printf("\n\t\t====================================");
        SetConsoleTextAttribute(h, 7);

        printf("\n\n\t\t========== GAME INSTRUCTIONS ==========");
        printf("\n\t\t1. The Mathematics Escape Room has 3 levels.");
        printf("\n\t\t2. Each level contains 5 multiple-choice questions.");
        printf("\n\t\t3. Select your answer by entering A, B, C or D.");
        printf("\n\t\t4. Each correct answer earns 1 point.");
        printf("\n\t\t5. You need at least 3/5 to clear a level.");
        printf("\n\t\t6. Enter L to activate the 50-50 lifeline.");
        printf("\n\t\t7. Each level has one lifeline in its first attempt only.");
        printf("\n\t\t8. The lifeline removes two incorrect options, leaving two options to choose from.");
        printf("\n\t\t9. Lifelines are not available during retries.");
        printf("\n\t\t10. Using a lifeline does not affect your score.");
        printf("\n\t\t11. Clear all 3 levels to escape the room!");
        printf("\n\t\t========================================");
        printf("\n\n\t\tGOOD LUCK, PLAYER! YOUR ADVENTURE BEGINS NOW!\n");
        gfxPressAnyKey();
    }

    if(levelsDone < 4)
    {
        do
        {
            score1=room2level1(firstAttempt1);
            firstAttempt1=0;

            if(score1<3)
            {
                SetConsoleTextAttribute(h, 12);
                printf("\n\nYou need to score at least 3/5 to unlock the next level.");
                printf("\nPlease try LEVEL 1 again.\n");
                SetConsoleTextAttribute(h, 7);
                Sleep(2500);
                system("cls");
            }
        }while(score1<3);

        SetConsoleTextAttribute(h, 10);
        printf("\n\nLEVEL 1 UNLOCKED! Moving to LEVEL 2...\n");
        SetConsoleTextAttribute(h, 7);
        Sleep(2500);
        system("cls");
        levelsDone = 4;
        autosave();
    }

    if(levelsDone < 5)
    {
        do
        {
            score2=room2level2(firstAttempt2);
            firstAttempt2=0;

            if(score2<3)
            {
                SetConsoleTextAttribute(h, 12);
                printf("\n\nYou need to score at least 3/5 to unlock the next level.");
                printf("\nPlease try LEVEL 2 again.\n");
                SetConsoleTextAttribute(h, 7);
                Sleep(2500);
                system("cls");
            }
        }while(score2<3);

        SetConsoleTextAttribute(h, 10);
        printf("\n\nLEVEL 2 UNLOCKED! Moving to LEVEL 3...\n");
        SetConsoleTextAttribute(h, 7);
        Sleep(2500);
        system("cls");
        levelsDone = 5;
        autosave();
    }

    if(levelsDone < 6)
    {
        do
        {
            score3=room2level3(firstAttempt3);
            firstAttempt3=0;

            if(score3<3)
            {
                SetConsoleTextAttribute(h, 12);
                printf("\n\nYou need to score at least 3/5 to escape.");
                printf("\nPlease try LEVEL 3 again.\n");
                SetConsoleTextAttribute(h, 7);
                Sleep(2500);
                system("cls");
            }
        }while(score3<3);
        levelsDone = 6;
        autosave();
    }

    SetConsoleTextAttribute(h, 10);
    printf("\n\n\t\t====================================");
    printf("\n\t\t CONGRATULATIONS! YOU ESCAPED!");
    printf("\n\t\t====================================");

    SetConsoleTextAttribute(h, 14);
    printf("\n\n\t\t========== LEADERBOARD ==========");
    SetConsoleTextAttribute(h, 7);

    SetConsoleTextAttribute(h, 11);
    printf("\n\n\t\tLEVEL 1");
    SetConsoleTextAttribute(h, 7);
    printf("\n\t\tCorrect Answers: %d",r2_lvl1_correct);
    printf("\n\t\tIncorrect Answers: %d",r2_lvl1_incorrect);

    SetConsoleTextAttribute(h, 11);
    printf("\n\n\t\tLEVEL 2");
    SetConsoleTextAttribute(h, 7);
    printf("\n\t\tCorrect Answers: %d",r2_lvl2_correct);
    printf("\n\t\tIncorrect Answers: %d",r2_lvl2_incorrect);

    SetConsoleTextAttribute(h, 11);
    printf("\n\n\t\tLEVEL 3");
    SetConsoleTextAttribute(h, 7);
    printf("\n\t\tCorrect Answers: %d",r2_lvl3_correct);
    printf("\n\t\tIncorrect Answers: %d",r2_lvl3_incorrect);

    SetConsoleTextAttribute(h, 10);
    printf("\n\n\t\tTOTAL SCORE: %d/15",r2_lvl1_correct+r2_lvl2_correct+r2_lvl3_correct);
    SetConsoleTextAttribute(h, 7);
    printf("\n\t\t====================================\n");

    printf("\n\n\t\t====================================");
    printf("\n\t\t       ROOM 2 COMPLETED!");
    printf("\n\t\t====================================");
    printf("\n\n\t\tYou have conquered the Mathematics Escape Room!");
    printf("\n\t\tYour next adventure awaits...");
    printf("\n\t\tGet ready to enter ROOM 3!");
    printf("\n\n\t\t       THE ADVENTURE CONTINUES!");
    printf("\n\t\t====================================\n");
    SetConsoleTextAttribute(h, 7);
    gfxKey();
    gfxPressAnyKey();
}


// room 3

int threeonecount = 0;
int threeonedcount = 0;
int threetwocount = 0;
int threetwodcount = 0;
int threethreecount = 0;
int threethreedcount = 0;

int intro(){
    h = GetStdHandle(STD_OUTPUT_HANDLE);
    system("cls");
    gfxTitle();
    SetConsoleTextAttribute(h, 14);
    printf("\n       THREE ROOMS. NINE LEVELS. CAN YOU ESCAPE?\n");
    SetConsoleTextAttribute(h, 7);

    SetConsoleTextAttribute(h, 13);
    printf("\n  ROOM 1  -  Riddles\n");
    SetConsoleTextAttribute(h, 11);
    printf("  ROOM 2  -  Mathematics\n");
    SetConsoleTextAttribute(h, 10);
    printf("  ROOM 3  -  C Programming\n");

    SetConsoleTextAttribute(h, 14);
    printf("\nHOW TO ESCAPE\n");
    SetConsoleTextAttribute(h, 7);
    printf("  - Each room has 3 levels with 5 questions per level.\n");
    printf("  - Get at least 3 answers correct to clear a level.\n");
    printf("  - A failed level must be replayed before you can continue.\n");
    printf("  - Use hints and 50-50 lifelines when they are available.\n");

    SetConsoleTextAttribute(h, 13);
    printf("\n============================================================\n");
    printf("Press ENTER when you are ready to begin...");
    SetConsoleTextAttribute(h, 7);
    fflush(stdout);
    getchar();
    system("cls");
    return 0;
}
//               room 3
int room3level1(){
    int l_count=0;

    system("cls");
    SetConsoleTextAttribute(h, 10);
    printf("\n============================================================\n");
    printf("          ROOM 3 - C PROGRAMMING | LEVEL 1 OF 3\n");
    printf("                      EASY MODE\n");
    printf("============================================================\n");
    SetConsoleTextAttribute(h, 7);
    Sleep(5000);

    system("cls");
    SetConsoleTextAttribute(h, 14);
    printf("\nLIFELINE: Press L to remove two incorrect options.\n");
    SetConsoleTextAttribute(h, 7);
    Sleep(5000);
    system("cls");

    printf("\n\n\nQuestion 1. Which library is used for mathematical operations in c? \n");
    printf("\n A. stdio.h \n B. stdlib.h \n C. math.h \n D. string.h \n or Press L for 50 - 50");
    printf("\n Enter your answer: ");
    char ans1;
    scanf(" %c",&ans1);
    if(ans1 == 'C' || ans1 == 'c'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threeonecount++;
    }
    else if((ans1 == 'l' || ans1 == 'L') && l_count < 2){
        l_count++;    
        printf("\nTwo incorrect options removed. Remaining options:\n A. stdio.h \n C. math.h");
        scanf(" %c",&ans1);
        if(ans1 == 'C' || ans1 == 'c'){
            SetConsoleTextAttribute(h, 10);
            printf("Correct Answer!\n");
            threeonecount++;
        } else {
            SetConsoleTextAttribute(h, 12);
            printf("Incorrect Answer!\n");
            threeonedcount++;
        }
    }
    else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\n");
        threeonedcount++;
    }
    Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 2. Which directive is used to define a constant (macro) in C? \n");
    printf("\n A. #include \n B. #define \n C. #ifdef \n D. #pragma \n or Press L for 50 - 50");
    printf("\n Enter your answer: ");
    char ans2;
    scanf(" %c",&ans2);
    if(ans2 == 'l' || ans2 == 'L'){
        if(l_count < 2){
            l_count++;
            printf("\nTwo incorrect options removed. Remaining options:\n A. #include \n B. #define");
        } else {
            printf("\nThe 50-50 lifeline has already been used twice.");
        }
        printf("\n Enter your answer (A/B/C/D): ");
        scanf(" %c",&ans2);
    }
    if(ans2 == 'B' || ans2 == 'b'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threeonecount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\n");
        threeonedcount++;
    }
    Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 3. Find the output of:\n printf(\"%%d\", 10 %% 3);\n");
    printf("\n A. 1 \n B. 2 \n C. 3 \n D. 0 \n or Press L for 50 - 50");
    printf("\n Enter your answer: ");
    char ans3;
    scanf(" %c",&ans3);
    if(ans3 == 'l' || ans3 == 'L'){
        if(l_count < 2){
            l_count++;
            printf("\nTwo incorrect options removed. Remaining options:\n A. 1 \n B. 2");
        } else {
            printf("\nThe 50-50 lifeline has already been used twice.");
        }
        printf("\n Enter your answer (A/B/C/D): ");
        scanf(" %c",&ans3);
    }
    if(ans3 == 'A' || ans3 == 'a'){
        SetConsoleTextAttribute(h, 10);
        printf("\nCorrect Answer!\n");
        threeonecount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\n");
        threeonedcount++;
    }
    Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 4. Which of the following is a valid variable name in C? \n");
    printf("\n A. 1variable \n B. variable_1 \n C. variable-1 \n D. variable 1 \n or Press L for 50 - 50");
    printf("\n Enter your answer: ");
    char ans4;
    scanf(" %c",&ans4);
    if(ans4 == 'l' || ans4 == 'L'){
        if(l_count < 2){
            l_count++;
            printf("\nTwo incorrect options removed. Remaining options:\n A. 1variable \n B. variable_1");
        } else {
            printf("\nThe 50-50 lifeline has already been used twice.");
        }
        printf("\n Enter your answer (A/B/C/D): ");
        scanf(" %c",&ans4);
    }
    if(ans4 == 'B' || ans4 == 'b'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threeonecount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\n");
        threeonedcount++;
    }
    Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 5. What is the output of the following code?\n int x = 5; \n printf(\"%%d\", x++ + 2); \n");
    printf("\n A. 7 \n B. 6 \n C. 5 \n D. 8 \n or Press L for 50 - 50");
    printf("\n Enter your answer: ");
    char ans5;
    scanf(" %c",&ans5);
    if(ans5 == 'l' || ans5 == 'L'){
        if(l_count < 2){
            l_count++;
            printf("\nTwo incorrect options removed. Remaining options:\n A. 7 \n B. 6");
        } else {
            printf("\nThe 50-50 lifeline has already been used twice.");
        }
        printf("\n Enter your answer (A/B/C/D): ");
        scanf(" %c",&ans5);
    }
    if(ans5 == 'A' || ans5 == 'a'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threeonecount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\n");
        threeonedcount++;
    }
    Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);

    return 0;
}
int room3level2(){
    int l_count=0;
    system("cls");
    SetConsoleTextAttribute(h, 10);
    printf("\n============================================================\n");
    printf("          ROOM 3 - C PROGRAMMING | LEVEL 2 OF 3\n");
    printf("                      MEDIUM MODE\n");
    printf("============================================================\n");
    SetConsoleTextAttribute(h, 7);
    Sleep(5000);
    system("cls");
    SetConsoleTextAttribute(h, 14);
    printf("\nLIFELINE: Press L to remove two incorrect options.\n");
    SetConsoleTextAttribute(h, 7);
    Sleep(5000);
    system("cls");
    printf("\n\nQuestion 1. What is the output of the following code?\n int a = 5; \n int b = 10; \n printf(\"%%d\", a > b ? a : b); \n");
    printf("\n A. 5 \n B. 10 \n C. 15 \n D. Error \n or Press L for 50 - 50");
    printf("\n Enter your answer: ");
    char ans1;
    scanf(" %c",&ans1);
    if(ans1 == 'l' || ans1 == 'L'){
        if(l_count < 2){
            l_count++;
            printf("\nTwo incorrect options removed. Remaining options:\n A. 5 \n B. 10");
        } else {
            printf("\nThe 50-50 lifeline has already been used twice.");
        }
        printf("\n Enter your answer (A/B/C/D): ");
        scanf(" %c",&ans1);
    }
    if(ans1 == 'B' || ans1 == 'b'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threetwocount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\n");
        threetwodcount++;
    }Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 2. Which of the following is used to allocate memory dynamically in C? \n");
    printf("\n A. malloc() \n B. calloc() \n C. realloc() \n D. All of the above \n or Press L for 50 - 50");
    printf("\n Enter your answer: ");
    char ans2;
    scanf(" %c",&ans2);
    if(ans2 == 'l' || ans2 == 'L'){
        if(l_count < 2){
            l_count++;
            printf("\nTwo incorrect options removed. Remaining options:\n C. realloc() \n D. All of the above");
        } else {
            printf("\nThe 50-50 lifeline has already been used twice.");
        }
        printf("\n Enter your answer (A/B/C/D): ");
        scanf(" %c",&ans2);
    }
    if(ans2 == 'D' || ans2 == 'd'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threetwocount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\n");
        threetwodcount++;
    }Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 3. Find the output:\n for(int i=0;i<5;i++){\nif(i==3)continue;\nprintf(\"%%d\",i);}");
    printf("\n A. 01234 \n B. 0124 \n C. 1234 \n D. 123 \n or Press L for 50 - 50");
    printf("\n Enter your answer: ");
    char ans3;
    scanf(" %c",&ans3);
    if(ans3 == 'l' || ans3 == 'L'){
        if(l_count < 2){
            l_count++;
            printf("\nTwo incorrect options removed. Remaining options:\n A. 01234 \n B. 0124");
        } else {
            printf("\nThe 50-50 lifeline has already been used twice.");
        }
        printf("\n Enter your answer (A/B/C/D): ");
        scanf(" %c",&ans3);
    }
    if(ans3 == 'B' || ans3 == 'b'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threetwocount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\n");
        threetwodcount++;
    }Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 4. Which of the following is used to read a character from the user in C? \n");
    printf("\n A. scanf() \n B. getchar() \n C. fgetc(stdin) \n D. All of the above \n or Press L for 50 - 50");
    printf("\n Enter your answer: ");
    char ans4;
    scanf(" %c",&ans4);
    if(ans4 == 'l' || ans4 == 'L'){
        if(l_count < 2){
            l_count++;
            printf("\nTwo incorrect options removed. Remaining options:\n C. fgetc(stdin) \n D. All of the above");
        } else {
            printf("\nThe 50-50 lifeline has already been used twice.");
        }
        printf("\n Enter your answer (A/B/C/D): ");
        scanf(" %c",&ans4);
    }
    if(ans4 == 'D' || ans4 == 'd'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threetwocount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\n");
        threetwodcount++;
    }Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 5. What is the output of the following code?\n int x = 5; \n int y = 10; \n printf(\"%%d\", x ^ y); \n");
    printf("\n A. 0 \n B. 5 \n C. 10 \n D. 15 \n or Press L for 50 - 50");
    printf("\n Enter your answer: ");
    char ans5;
    scanf(" %c",&ans5);
    if(ans5 == 'l' || ans5 == 'L'){
        if(l_count < 2){
            l_count++;
            printf("\nTwo incorrect options removed. Remaining options:\n C. 10 \n D. 15");
        } else {
            printf("\nThe 50-50 lifeline has already been used twice.");
        }
        printf("\n Enter your answer (A/B/C/D): ");
        scanf(" %c",&ans5);
    }
    if(ans5 == 'D' || ans5 == 'd'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threetwocount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\n");
        threetwodcount++;
    }Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);

    return 0;
}
int room3level3(){
    int l_count=0;
    system("cls");
    SetConsoleTextAttribute(h, 10);
    printf("\n============================================================\n");
    printf("          ROOM 3 - C PROGRAMMING | LEVEL 3 OF 3\n");
    printf("                       HARD MODE\n");
    printf("============================================================\n");
    SetConsoleTextAttribute(h, 7);
    Sleep(5000);
    system("cls");
    SetConsoleTextAttribute(h, 14);
    printf("\nLIFELINE: Press L to remove two incorrect options.\n");
    SetConsoleTextAttribute(h, 7);
    Sleep(5000);
    system("cls");
    printf("\n\nQuestion 1. What is the output of the following code?\n int x = 5; \n int y = 10; \n printf(\"%%d\", x & y); \n");
    printf("\n A. 0 \n B. 5 \n C. 10 \n D. 15 \n or Press L for 50 - 50");
    printf("\n Enter your answer: ");
    char ans1;
    scanf(" %c",&ans1);
    if(ans1 == 'l' || ans1 == 'L'){
        if(l_count < 2){
            l_count++;
            printf("\nTwo incorrect options removed. Remaining options:\n A. 0 \n B. 5");
        } else {
            printf("\nThe 50-50 lifeline has already been used twice.");
        }
        printf("\n Enter your answer (A/B/C/D): ");
        scanf(" %c",&ans1);
    }
    if(ans1 == 'A' || ans1 == 'a'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threethreecount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\n");
        threethreedcount++;
    }
    Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 2. Which of the following is used to terminate a program in C? \n");
    printf("\n A. exit() \n B. return() \n C. break() \n D. continue() \n or Press L for 50 - 50");
    printf("\n Enter your answer: ");
    char ans2;
    scanf(" %c",&ans2);
    if(ans2 == 'l' || ans2 == 'L'){
        if(l_count < 2){
            l_count++;
            printf("\nTwo incorrect options removed. Remaining options:\n A. exit() \n B. return()");
        } else {
            printf("\nThe 50-50 lifeline has already been used twice.");
        }
        printf("\n Enter your answer (A/B/C/D): ");
        scanf(" %c",&ans2);
    }
    if(ans2 == 'A' || ans2 == 'a'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threethreecount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\n");
        threethreedcount++;
    }
    Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 3. Find the output of the following code:\n int arr[5] = {1, 2, 3, 4, 5}; \n printf(\"%%d\", *(arr + 2)); \n");
    printf("\n A. 1 \n B. 2 \n C. 3 \n D. 4 \n or Press L for 50 - 50");
    printf("\n Enter your answer: ");
    char ans3;
    scanf(" %c",&ans3);
    if(ans3 == 'l' || ans3 == 'L'){
        if(l_count < 2){
            l_count++;
            printf("\nTwo incorrect options removed. Remaining options:\n B. 2 \n C. 3");
        } else {
            printf("\nThe 50-50 lifeline has already been used twice.");
        }
        printf("\n Enter your answer (A/B/C/D): ");
        scanf(" %c",&ans3);
    }
    if(ans3 == 'C' || ans3 == 'c'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        
        threethreecount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\n");
        threethreedcount++;
    }Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 4. Which function is the safest way to read a whole line (with spaces) from the user? \n");
    printf("\n A. scanf() \n B. gets() \n C. fgets() \n D. All of the above \n or Press L for 50 - 50");
    printf("\n Enter your answer: ");
    char ans4;
    scanf(" %c",&ans4);
    if(ans4 == 'l' || ans4 == 'L'){
        if(l_count < 2){
            l_count++;
            printf("\nTwo incorrect options removed. Remaining options:\n B. gets() \n C. fgets()");
        } else {
            printf("\nThe 50-50 lifeline has already been used twice.");
        }
        printf("\n Enter your answer (A/B/C/D): ");
        scanf(" %c",&ans4);
    }
    if(ans4 == 'C' || ans4 == 'c'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threethreecount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!");
        threethreedcount++;
    }Sleep(3000);

    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 5. What is the output of the following code?\n int x = 5; \n int y = 10; \n printf(\"%%d\", x | y); \n");
    printf("\n A. 0 \n B. 5 \n C. 10 \n D. 15 \n or Press L for 50 - 50");
    printf("\n Enter your answer: ");
    char ans5;
    scanf(" %c",&ans5);
    if(ans5 == 'l' || ans5 == 'L'){
        if(l_count < 2){
            l_count++;
            printf("\nTwo incorrect options removed. Remaining options:\n C. 10 \n D. 15");
        } else {
            printf("\nThe 50-50 lifeline has already been used twice.");
        }
        printf("\n Enter your answer (A/B/C/D): ");
        scanf(" %c",&ans5);
    }
    if(ans5 == 'D' || ans5 == 'd'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threethreecount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\n");
        threethreedcount++;
    }Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    
    return 0;

}
int room3(){
    if(levelsDone == 6)
        gfxRoomIntro(3, "ROOM 3  -  C PROGRAMMING", 10);
    if(levelsDone < 7)
    {
        do {
            threeonecount = 0;
            threeonedcount = 0;
            room3level1();
            Sleep(3000);
            system("cls");
            SetConsoleTextAttribute(h, 14);
            printf("\n                   PREPARING RESULTS...");
            Sleep(3500);
            system("cls");
            SetConsoleTextAttribute(h, 7);
            printf("\n               =======Level 1 results=======\n");
            printf("\n       Total correct answers in LEVEL 1: %d \n", threeonecount);
            printf("\n       Total incorrect answers in LEVEL 1: %d \n", threeonedcount);
            gfxScoreBar(threeonecount, 5);
            Sleep(6000);
            if(threeonecount < 3){
                printf("\nYou need at least 3 correct answers. Retrying Level 1.\n");
                Sleep(3000);
            }
        } while(threeonecount < 3);
        levelsDone = 7;
        autosave();
    }

    if(levelsDone < 8)
    {
        do {
            threetwocount = 0;
            threetwodcount = 0;
            room3level2();
            Sleep(1000);
            system("cls");
            SetConsoleTextAttribute(h, 14);
            printf("\n                   PREPARING RESULTS...");
            Sleep(3500);
            system("cls");
            SetConsoleTextAttribute(h, 7);
            printf("\n           ===========Level 2 Results===========\n");
            printf("\n       Total correct answers in LEVEL 2: %d \n", threetwocount);
            printf("\n       Total incorrect answers in LEVEL 2: %d \n", threetwodcount);
            gfxScoreBar(threetwocount, 5);
            Sleep(6000);
            if(threetwocount < 3){
                printf("\nYou need at least 3 correct answers. Retrying Level 2.\n");
                Sleep(3000);
            }
        } while(threetwocount < 3);
        levelsDone = 8;
        autosave();
    }

    if(levelsDone < 9)
    {
        do {
            threethreecount = 0;
            threethreedcount = 0;
            room3level3();
            Sleep(1000);
            system("cls");
            SetConsoleTextAttribute(h, 14);
            printf("\n                   PREPARING RESULTS...");
            Sleep(3000);
            system("cls");
            SetConsoleTextAttribute(h, 7);
            printf("\n           ==========Level 3 Results===========\n");
            printf("\n        Total correct answers in LEVEL 3: %d \n", threethreecount);
            printf("\n        Total incorrect answers in LEVEL 3: %d \n", threethreedcount);
            gfxScoreBar(threethreecount, 5);
            Sleep(6000);
            if(threethreecount < 3){
                printf("\nYou need at least 3 correct answers. Retrying Level 3.\n");
                Sleep(3000);
            }
        } while(threethreecount < 3);
        levelsDone = 9;
        autosave();
    }

    {
        int room3TotalCorrect = threeonecount + threetwocount + threethreecount;
        int room3TotalIncorrect = threeonedcount + threetwodcount + threethreedcount;

        system("cls");
        SetConsoleTextAttribute(h, 11);
        printf("\n============================================================\n");
        SetConsoleTextAttribute(h, 14);
        printf("                ROOM 3 LEADERBOARD\n");
        SetConsoleTextAttribute(h, 11);
        printf("============================================================\n");

        SetConsoleTextAttribute(h, 10);
        printf("\n  EASY LEVEL       ");
        SetConsoleTextAttribute(h, 7);
        printf("Correct: %d | Incorrect: %d\n", threeonecount, threeonedcount);
        gfxScoreBar(threeonecount, 5);

        SetConsoleTextAttribute(h, 11);
        printf("\n  MEDIUM LEVEL     ");
        SetConsoleTextAttribute(h, 7);
        printf("Correct: %d | Incorrect: %d\n", threetwocount, threetwodcount);
        gfxScoreBar(threetwocount, 5);

        SetConsoleTextAttribute(h, 13);
        printf("\n  HARD LEVEL       ");
        SetConsoleTextAttribute(h, 7);
        printf("Correct: %d | Incorrect: %d\n", threethreecount, threethreedcount);
        gfxScoreBar(threethreecount, 5);

        SetConsoleTextAttribute(h, 14);
        printf("\n------------------------------------------------------------\n");
        printf("  ROOM 3 TOTAL     ");
        SetConsoleTextAttribute(h, 10);
        printf("Correct: %d | ", room3TotalCorrect);
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect: %d\n", room3TotalIncorrect);
        SetConsoleTextAttribute(h, 14);
        printf("  FINAL SCORE: %d/15\n", room3TotalCorrect);
        printf("============================================================\n");
        SetConsoleTextAttribute(h, 7);
        gfxPressAnyKey();
    }

    return 0;
}


static void gatherStats(int c[], int in[])
{
    c[0] = roomonelevelonecorrect;  in[0] = roomoneleveloneincorrect;
    c[1] = roomoneleveltwocorrect;  in[1] = roomoneleveltwoincorrect;
    c[2] = roomonelevelthreecorrect; in[2] = roomonelevelthreeincorrect;
    c[3] = r2_lvl1_correct;  in[3] = r2_lvl1_incorrect;
    c[4] = r2_lvl2_correct;  in[4] = r2_lvl2_incorrect;
    c[5] = r2_lvl3_correct;  in[5] = r2_lvl3_incorrect;
    c[6] = threeonecount;    in[6] = threeonedcount;
    c[7] = threetwocount;    in[7] = threetwodcount;
    c[8] = threethreecount;  in[8] = threethreedcount;
}

static void applyStats(const int c[], const int in[])
{
    roomonelevelonecorrect = c[0];   roomoneleveloneincorrect = in[0];
    roomoneleveltwocorrect = c[1];   roomoneleveltwoincorrect = in[1];
    roomonelevelthreecorrect = c[2]; roomonelevelthreeincorrect = in[2];
    r2_lvl1_correct = c[3];  r2_lvl1_incorrect = in[3];
    r2_lvl2_correct = c[4];  r2_lvl2_incorrect = in[4];
    r2_lvl3_correct = c[5];  r2_lvl3_incorrect = in[5];
    threeonecount = c[6];    threeonedcount = in[6];
    threetwocount = c[7];    threetwodcount = in[7];
    threethreecount = c[8];  threethreedcount = in[8];
}

void autosave(void)
{
    int c[TOTAL_LEVELS], in[TOTAL_LEVELS];
    gatherStats(c, in);
    gfxColor(8);
    if(saveProgress(playerName, levelsDone, c, in))
        printf("\n  [Progress saved: %d/%d levels cleared]\n", levelsDone, TOTAL_LEVELS);
    else
        printf("\n  [Could not write save file]\n");
    gfxColor(7);
    Sleep(1000);
}


static void startMenu(void)
{
    int c[TOTAL_LEVELS], in[TOTAL_LEVELS];
    int done;
    char name[32];
    char ch;
    size_t len;

    system("cls");

    if(loadProgress(name, sizeof(name), &done, c, in))
    {
        gfxColor(14);
        printf("\n  SAVED GAME FOUND\n");
        gfxColor(7);
        printf("  Player   : %s\n", name);
        printf("  Progress : %d/%d levels cleared\n", done, TOTAL_LEVELS);
        printf("\n  [C] Continue    [N] New game\n");

        do { ch = (char)toupper((unsigned char)getch()); }
        while(ch != 'C' && ch != 'N');

        if(ch == 'C')
        {
            strcpy(playerName, name);
            levelsDone = done;
            applyStats(c, in);
            gfxColor(10);
            printf("\n  Welcome back, %s! Resuming...\n", playerName);
            gfxColor(7);
            Sleep(1500);
            system("cls");
            return;
        }
        deleteSave();
        system("cls");
    }

    gfxColor(11);
    printf("\n  Enter your name: ");
    gfxColor(7);
    if(fgets(name, sizeof(name), stdin) == NULL)
        name[0] = '\0';
    len = strlen(name);
    while(len > 0 && (name[len-1] == '\n' || name[len-1] == '\r'))
        name[--len] = '\0';
    if(len == 0)
        strcpy(name, "Player");
    strcpy(playerName, name);
    levelsDone = 0;
    system("cls");
}

static void printFinalLeaderboard(void)
{
    const char *roomNames[3] = {
        "ROOM 1 - RIDDLES",
        "ROOM 2 - MATHEMATICS",
        "ROOM 3 - C PROGRAMMING"
    };
    const char *levelNames[3] = {"Easy", "Normal", "Difficult"};
    int correct[3][3] = {
        {roomonelevelonecorrect, roomoneleveltwocorrect, roomonelevelthreecorrect},
        {r2_lvl1_correct, r2_lvl2_correct, r2_lvl3_correct},
        {threeonecount, threetwocount, threethreecount}
    };
    int incorrect[3][3] = {
        {roomoneleveloneincorrect, roomoneleveltwoincorrect, roomonelevelthreeincorrect},
        {r2_lvl1_incorrect, r2_lvl2_incorrect, r2_lvl3_incorrect},
        {threeonedcount, threetwodcount, threethreedcount}
    };
    const WORD roomColors[3] = {13, 11, 10};
    int grandCorrect = 0;
    int grandIncorrect = 0;
    int room;
    int level;

    system("cls");
    SetConsoleTextAttribute(h, 14);
    printf("\n==================== FINAL LEADERBOARD ====================\n");
    SetConsoleTextAttribute(h, 11);
    printf("\n%-26s %8s %10s %10s\n","ROOM / LEVEL", "CORRECT", "INCORRECT", "SCORE");
    printf("------------------------------------------------------------\n");
    SetConsoleTextAttribute(h, 7);

    for(room = 0; room < 3; room++){
        int roomCorrect = 0;
        int roomIncorrect = 0;

        SetConsoleTextAttribute(h, roomColors[room]);
        printf("\n%s\n", roomNames[room]);
        SetConsoleTextAttribute(h, 7);

        for(level = 0; level < 3; level++){
            printf("  %-24s %8d %10d %7d/5\n",
                   levelNames[level],
                   correct[room][level],
                   incorrect[room][level],
                   correct[room][level]);
            roomCorrect += correct[room][level];
            roomIncorrect += incorrect[room][level];
        }

        SetConsoleTextAttribute(h, roomColors[room]);
        printf("  %-24s %8d %10d %7d/15\n","ROOM TOTAL",roomCorrect,roomIncorrect,roomCorrect);
        SetConsoleTextAttribute(h, 7);

        grandCorrect += roomCorrect;
        grandIncorrect += roomIncorrect;
    }

    SetConsoleTextAttribute(h, 14);
    printf("\n============================================================\n");
    printf("%-26s %8d %10d %7d/45\n","GRAND TOTAL",grandCorrect,grandIncorrect,grandCorrect);
    printf("============================================================\n");
    SetConsoleTextAttribute(h, 7);
}

int main(void){
    int c[TOTAL_LEVELS], in[TOTAL_LEVELS];
    int i, grand = 0;

    intro();
    startMenu();

    gfxColor(13);
    printf("\n A R E  Y O U  R E A D Y ,  %s !", playerName);
    gfxColor(7);
    Sleep(3000);

    if(levelsDone < 3) room1();
    if(levelsDone < 6) room2();
    if(levelsDone < 9) room3();

    gfxVictory();

    gatherStats(c, in);
    for(i = 0; i < TOTAL_LEVELS; i++)
        grand += c[i];
    saveHighScore(playerName, grand);
    deleteSave();            

    printFinalLeaderboard();
    showHighScores();
    gfxPressAnyKey();
    return 0;
}