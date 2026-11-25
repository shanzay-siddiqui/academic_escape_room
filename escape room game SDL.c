#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_test_font.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define GUI_WIDTH 1100
#define GUI_HEIGHT 720
#define TRANSCRIPT_SIZE 12000

enum {
    GUI_IDLE,
    GUI_START,
    GUI_ANSWER,
    GUI_ANSWER_WITH_LIFELINE,
    GUI_YES_NO
};

static SDL_Window *guiWindow;
static SDL_Renderer *guiRenderer;
static char transcript[TRANSCRIPT_SIZE];
static int currentRoom;
static int guiRunning = 1;
static int skipLegacyIntro;
static SDL_Color textColor = {238, 242, 250, 255};
static const SDL_Color roomColors[3] = {
    {155, 85, 220, 255},
    {65, 145, 235, 255},
    {65, 190, 105, 255}
};
static const char *roomNames[3] = {
    "RIDDLE ROOM",
    "MATHEMATICS ROOM",
    "CODING ROOM"
};
static const SDL_Rect startButton = {400, 565, 300, 64};
static const SDL_Rect choiceButtons[5] = {
    {350, 575, 96, 58},
    {458, 575, 96, 58},
    {566, 575, 96, 58},
    {674, 575, 96, 58},
    {800, 575, 150, 58}
};
static const SDL_Rect yesNoButtons[2] = {
    {490, 575, 150, 58},
    {660, 575, 150, 58}
};

static int pointInRect(int x, int y, const SDL_Rect *rect)
{
    return x >= rect->x && x < rect->x + rect->w &&
           y >= rect->y && y < rect->y + rect->h;
}

static void drawText(int x, int y, const char *text, SDL_Color color)
{
    SDL_SetRenderDrawColor(guiRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderSetScale(guiRenderer, 2.0f, 2.0f);
    SDLTest_DrawString(guiRenderer, x / 2, y / 2, text);
    SDL_RenderSetScale(guiRenderer, 1.0f, 1.0f);
}

static void drawButton(SDL_Rect rect, const char *label, SDL_Color accent,
                       int mouseX, int mouseY)
{
    SDL_Color fill = pointInRect(mouseX, mouseY, &rect)
        ? accent
        : (SDL_Color){35, 44, 64, 255};
    int textWidth = (int)strlen(label) * 16;
    int textX = rect.x + (rect.w - textWidth) / 2;
    SDL_Rect shadow = rect;

    shadow.y += 5;
    SDL_SetRenderDrawColor(guiRenderer, 4, 8, 18, 255);
    SDL_RenderFillRect(guiRenderer, &shadow);
    SDL_SetRenderDrawColor(guiRenderer, fill.r, fill.g, fill.b, fill.a);
    SDL_RenderFillRect(guiRenderer, &rect);
    SDL_SetRenderDrawColor(guiRenderer, accent.r, accent.g, accent.b, accent.a);
    SDL_RenderDrawRect(guiRenderer, &rect);
    if(textX < rect.x + 8){
        textX = rect.x + 8;
    }
    drawText(textX, rect.y + (rect.h - 18) / 2, label,
             (SDL_Color){248, 249, 255, 255});
}

static void appendTranscript(const char *text)
{
    size_t length = strlen(text);
    size_t used = strlen(transcript);

    while(used + length >= sizeof(transcript)){
        char *newline = strchr(transcript, '\n');
        if(newline == NULL){
            transcript[0] = '\0';
            used = 0;
            break;
        }
        memmove(transcript, newline + 1, strlen(newline + 1) + 1);
        used = strlen(transcript);
    }
    memcpy(transcript + used, text, length + 1);
}

static void addLine(char lines[][96], int *lineCount, char *line, int *length)
{
    if(*lineCount == 128){
        memmove(lines[0], lines[1], 127 * sizeof(lines[0]));
        (*lineCount)--;
    }
    line[*length] = '\0';
    strcpy(lines[*lineCount], line);
    (*lineCount)++;
    *length = 0;
}

static void drawTranscript(void)
{
    char lines[128][96];
    char line[96];
    const char *cursor = transcript;
    int lineCount = 0;
    int lineLength = 0;
    int firstLine;
    int i;

    while(*cursor != '\0'){
        if(*cursor == '\r'){
            cursor++;
            continue;
        }
        if(*cursor == '\n'){
            addLine(lines, &lineCount, line, &lineLength);
            cursor++;
            continue;
        }
        if(*cursor == '\t'){
            int spaces;
            for(spaces = 0; spaces < 4; spaces++){
                line[lineLength++] = ' ';
            }
            cursor++;
        } else {
            line[lineLength++] = *cursor++;
        }
        if(lineLength == 44){
            addLine(lines, &lineCount, line, &lineLength);
        }
    }
    if(lineLength > 0){
        addLine(lines, &lineCount, line, &lineLength);
    }

    firstLine = lineCount > 17 ? lineCount - 17 : 0;
    for(i = firstLine; i < lineCount; i++){
        drawText(350, 162 + (i - firstLine) * 23, lines[i], textColor);
    }
}

static void drawDoor(int x, int y, SDL_Color color)
{
    SDL_Rect frame = {x, y, 80, 100};
    SDL_Rect door = {x + 7, y + 7, 66, 93};
    SDL_Rect topPanel = {x + 15, y + 16, 50, 27};
    SDL_Rect bottomPanel = {x + 15, y + 52, 50, 30};
    int dx;
    int dy;

    SDL_SetRenderDrawColor(guiRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(guiRenderer, &frame);
    SDL_SetRenderDrawColor(guiRenderer, 18, 25, 43, 255);
    SDL_RenderFillRect(guiRenderer, &door);
    SDL_SetRenderDrawColor(guiRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(guiRenderer, &topPanel);
    SDL_RenderDrawRect(guiRenderer, &bottomPanel);
    for(dy = -5; dy <= 5; dy++){
        for(dx = -5; dx <= 5; dx++){
            if(dx * dx + dy * dy <= 25){
                SDL_RenderDrawPoint(guiRenderer, x + 59 + dx, y + 49 + dy);
            }
        }
    }
}

static void drawIntroScreen(int mouseX, int mouseY)
{
    const char *titles[3] = {
        "ROOM 1: RIDDLES",
        "ROOM 2: MATH",
        "ROOM 3: C CODE"
    };
    const char *descriptions[3] = {
        "Solve clever puzzles",
        "Crack the number clues",
        "Read and reason about C"
    };
    SDL_Rect frame = {30, 30, GUI_WIDTH - 60, GUI_HEIGHT - 60};
    int i;

    SDL_SetRenderDrawColor(guiRenderer, 9, 14, 28, 255);
    SDL_RenderClear(guiRenderer);
    SDL_SetRenderDrawColor(guiRenderer, 19, 27, 46, 255);
    SDL_RenderFillRect(guiRenderer, &frame);
    SDL_SetRenderDrawColor(guiRenderer, 155, 85, 220, 255);
    {
        SDL_Rect topStripe = {30, 30, GUI_WIDTH - 60, 7};
        SDL_RenderFillRect(guiRenderer, &topStripe);
    }

    drawText(390, 72, "ACADEMIC ESCAPE ROOM",
             (SDL_Color){245, 247, 255, 255});
    drawText(190, 112, "THREE ROOMS. NINE CHALLENGES. ONE WAY OUT.",
             (SDL_Color){155, 170, 195, 255});

    for(i = 0; i < 3; i++){
        SDL_Rect card = {70 + i * 330, 175, 300, 230};
        SDL_Rect cardStripe = {card.x, card.y, card.w, 6};
        SDL_Color color = roomColors[i];
        int titleX = card.x + (card.w - (int)strlen(titles[i]) * 16) / 2;
        int descriptionX = card.x +
            (card.w - (int)strlen(descriptions[i]) * 16) / 2;

        SDL_SetRenderDrawColor(guiRenderer, 29, 38, 58, 255);
        SDL_RenderFillRect(guiRenderer, &card);
        SDL_SetRenderDrawColor(guiRenderer, 55, 70, 96, 255);
        SDL_RenderDrawRect(guiRenderer, &card);
        SDL_SetRenderDrawColor(guiRenderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(guiRenderer, &cardStripe);
        drawDoor(card.x + 110, card.y + 22, color);
        drawText(titleX, card.y + 145, titles[i],
                 (SDL_Color){245, 247, 255, 255});
        drawText(descriptionX, card.y + 177, descriptions[i], color);
    }

    drawText(380, 435, "5 QUESTIONS PER LEVEL",
             (SDL_Color){245, 247, 255, 255});
    drawText(326, 465, "3 CORRECT TO UNLOCK THE NEXT LEVEL",
             (SDL_Color){155, 170, 195, 255});
    drawText(245, 502, "Use your hints and 50-50 lifeline wisely.",
             (SDL_Color){155, 170, 195, 255});
    drawButton(startButton, "START ADVENTURE", roomColors[0], mouseX, mouseY);
    drawText(318, 650, "Click the button or press ENTER to begin",
             (SDL_Color){155, 170, 195, 255});
    SDL_RenderPresent(guiRenderer);
}

static void drawInterface(int mode, int mouseX, int mouseY)
{
    SDL_Color accent = roomColors[currentRoom];
    SDL_Rect outer = {25, 25, GUI_WIDTH - 50, GUI_HEIGHT - 50};
    SDL_Rect sidePanel = {45, 122, 260, 520};
    SDL_Rect outputPanel = {325, 122, 730, 520};
    int i;

    if(mode == GUI_START){
        drawIntroScreen(mouseX, mouseY);
        return;
    }

    SDL_SetRenderDrawColor(guiRenderer, 10, 15, 29, 255);
    SDL_RenderClear(guiRenderer);
    SDL_SetRenderDrawColor(guiRenderer, 19, 26, 45, 255);
    SDL_RenderFillRect(guiRenderer, &outer);
    SDL_SetRenderDrawColor(guiRenderer, accent.r, accent.g, accent.b, accent.a);
    SDL_Rect stripe = {25, 25, GUI_WIDTH - 50, 7};
    SDL_RenderFillRect(guiRenderer, &stripe);

    drawText(55, 55, "ACADEMIC ESCAPE ROOM",
             (SDL_Color){245, 247, 255, 255});
    drawText(780, 55, roomNames[currentRoom], accent);

    SDL_SetRenderDrawColor(guiRenderer, 24, 32, 52, 255);
    SDL_RenderFillRect(guiRenderer, &sidePanel);
    SDL_SetRenderDrawColor(guiRenderer, 55, 70, 96, 255);
    SDL_RenderDrawRect(guiRenderer, &sidePanel);
    SDL_SetRenderDrawColor(guiRenderer, 24, 32, 52, 255);
    SDL_RenderFillRect(guiRenderer, &outputPanel);
    SDL_SetRenderDrawColor(guiRenderer, 55, 70, 96, 255);
    SDL_RenderDrawRect(guiRenderer, &outputPanel);

    for(i = 0; i < 3; i++){
        SDL_Rect roomCard = {62, 155 + i * 150, 226, 130};
        SDL_Color color = roomColors[i];
        SDL_SetRenderDrawColor(guiRenderer, 31, 40, 60, 255);
        SDL_RenderFillRect(guiRenderer, &roomCard);
        SDL_SetRenderDrawColor(guiRenderer, color.r, color.g, color.b, 255);
        SDL_RenderDrawRect(guiRenderer, &roomCard);
        drawDoor(roomCard.x + 12, roomCard.y + 10, color);
        {
            char roomLabel[16];
            sprintf(roomLabel, "ROOM %d", i + 1);
            drawText(roomCard.x + 128, roomCard.y + 42, roomLabel,
                     (SDL_Color){245, 247, 255, 255});
        }
        drawText(roomCard.x + 128, roomCard.y + 72,
                 i == 0 ? "RIDDLES" : (i == 1 ? "MATH" : "C CODE"), color);
    }

    drawText(350, 135, "GAME SCREEN", (SDL_Color){155, 170, 195, 255});
    drawTranscript();

    if(mode == GUI_START){
        drawButton(startButton, "START GAME", accent, mouseX, mouseY);
    } else if(mode == GUI_ANSWER || mode == GUI_ANSWER_WITH_LIFELINE){
        static const char *labels[5] = {"A", "B", "C", "D", "LIFELINE"};
        int count = mode == GUI_ANSWER_WITH_LIFELINE ? 5 : 4;
        for(i = 0; i < count; i++){
            drawButton(choiceButtons[i], labels[i], accent, mouseX, mouseY);
        }
    } else if(mode == GUI_YES_NO){
        drawButton(yesNoButtons[0], "YES", accent, mouseX, mouseY);
        drawButton(yesNoButtons[1], "NO", accent, mouseX, mouseY);
    } else {
        drawText(500, 595, "GAME IN PROGRESS", (SDL_Color){155, 170, 195, 255});
    }

    SDL_RenderPresent(guiRenderer);
}

static void updateRoom(const char *text)
{
    if(strstr(text, "ROOM 1") != NULL ||
       strstr(text, "Room 1") != NULL ||
       strstr(text, "RIDDLE ROOM") != NULL){
        currentRoom = 0;
    } else if(strstr(text, "ROOM 2") != NULL ||
              strstr(text, "Room 2") != NULL ||
              strstr(text, "MATHEMATICS") != NULL){
        currentRoom = 1;
    } else if(strstr(text, "ROOM 3") != NULL ||
              strstr(text, "Room 3") != NULL ||
              strstr(text, "coding with C") != NULL){
        currentRoom = 2;
    }
}

static int guiPrintf(const char *format, ...)
{
    char message[2048];
    int result;
    va_list arguments;

    va_start(arguments, format);
    result = vsnprintf(message, sizeof(message), format, arguments);
    va_end(arguments);
    if(result < 0){
        return result;
    }
    message[sizeof(message) - 1] = '\0';
    if(skipLegacyIntro){
        if(strstr(message, "GAME STARTING") != NULL){
            skipLegacyIntro = 0;
        }
        return result;
    }
    updateRoom(message);
    appendTranscript(message);
    drawInterface(GUI_IDLE, 0, 0);
    return result;
}

static int guiSystem(const char *command)
{
    if(command != NULL && strcmp(command, "cls") == 0){
        transcript[0] = '\0';
        if(skipLegacyIntro){
            return 0;
        }
        drawInterface(GUI_IDLE, 0, 0);
        return 0;
    }
    return -1;
}

static BOOL guiSetConsoleTextAttribute(HANDLE console, WORD attribute)
{
    (void)console;
    switch(attribute & 0x0F){
        case 10:
            textColor = (SDL_Color){100, 225, 155, 255};
            break;
        case 11:
            textColor = (SDL_Color){105, 205, 235, 255};
            break;
        case 12:
            textColor = (SDL_Color){255, 142, 145, 255};
            break;
        case 13:
            textColor = (SDL_Color){205, 145, 245, 255};
            break;
        case 14:
            textColor = (SDL_Color){255, 218, 125, 255};
            break;
        default:
            textColor = (SDL_Color){238, 242, 250, 255};
            break;
    }
    return TRUE;
}

static void exitGui(void)
{
    guiRunning = 0;
    exit(0);
}

static void serviceDelayEvents(void)
{
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        if(event.type == SDL_QUIT ||
           (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)){
            exitGui();
        }
    }
}

static void guiSleep(DWORD milliseconds)
{
    Uint32 start = SDL_GetTicks();
    int mouseX;
    int mouseY;

    if(skipLegacyIntro){
        return;
    }

    while(guiRunning && SDL_GetTicks() - start < milliseconds){
        serviceDelayEvents();
        SDL_GetMouseState(&mouseX, &mouseY);
        drawInterface(GUI_IDLE, mouseX, mouseY);
        SDL_Delay(16);
    }
}

static int chooseButton(int mode, int x, int y, char *value)
{
    int i;

    if(mode == GUI_START){
        if(pointInRect(x, y, &startButton)){
            *value = '\n';
            return 1;
        }
        return 0;
    }
    if(mode == GUI_YES_NO){
        if(pointInRect(x, y, &yesNoButtons[0])){
            *value = 'Y';
            return 1;
        }
        if(pointInRect(x, y, &yesNoButtons[1])){
            *value = 'N';
            return 1;
        }
        return 0;
    }

    for(i = 0; i < 4; i++){
        if(pointInRect(x, y, &choiceButtons[i])){
            *value = (char)('A' + i);
            return 1;
        }
    }
    if(mode == GUI_ANSWER_WITH_LIFELINE &&
       pointInRect(x, y, &choiceButtons[4])){
        *value = 'L';
        return 1;
    }
    return 0;
}

static int waitForClick(int mode, char *value)
{
    SDL_Event event;
    int mouseX = 0;
    int mouseY = 0;

    while(guiRunning){
        SDL_GetMouseState(&mouseX, &mouseY);
        drawInterface(mode, mouseX, mouseY);
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT ||
               (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)){
                exitGui();
            }
            if(event.type == SDL_KEYDOWN){
                SDL_Keycode key = event.key.keysym.sym;
                if(mode == GUI_START && (key == SDLK_RETURN || key == SDLK_SPACE)){
                    *value = '\n';
                    return 1;
                }
                if(mode == GUI_YES_NO && (key == SDLK_y || key == SDLK_n)){
                    *value = (char)key;
                    return 1;
                }
                if((mode == GUI_ANSWER || mode == GUI_ANSWER_WITH_LIFELINE) &&
                   key >= SDLK_a && key <= SDLK_d){
                    *value = (char)key;
                    return 1;
                }
                if(mode == GUI_ANSWER_WITH_LIFELINE && key == SDLK_l){
                    *value = 'L';
                    return 1;
                }
            }
            if(event.type == SDL_MOUSEBUTTONDOWN &&
               event.button.button == SDL_BUTTON_LEFT &&
               chooseButton(mode, event.button.x, event.button.y, value)){
                return 1;
            }
        }
        SDL_Delay(16);
    }
    return 0;
}

static int guiScanf(const char *format, ...)
{
    va_list arguments;
    char *destination;
    char value = 'A';
    int mode = GUI_ANSWER;
    const char *prompt;
    const char *lastPrompt = NULL;

    va_start(arguments, format);
    destination = va_arg(arguments, char *);
    va_end(arguments);

    if(strstr(transcript, "(Y/N)") != NULL ||
       strstr(transcript, "Y/N:") != NULL){
        mode = GUI_YES_NO;
    } else {
        prompt = transcript;
        while((prompt = strstr(prompt, "Enter your answer")) != NULL){
            lastPrompt = prompt;
            prompt++;
        }
        if((lastPrompt != NULL &&
            (strstr(lastPrompt, "L for") != NULL ||
             strstr(lastPrompt, "Press L") != NULL)) ||
           ((strstr(transcript, "Press L") != NULL ||
             strstr(transcript, "L for 50-50") != NULL ||
             strstr(transcript, "L for 50 - 50") != NULL) &&
            strstr(transcript, "options removed") == NULL &&
            strstr(transcript, "options have been removed") == NULL)){
            mode = GUI_ANSWER_WITH_LIFELINE;
        }
    }

    if(waitForClick(mode, &value)){
        *destination = value;
        return 1;
    }
    return 0;
}

static void waitForStart(void)
{
    char ignored;
    waitForClick(GUI_START, &ignored);
    transcript[0] = '\0';
}

#define printf guiPrintf
#define scanf guiScanf
#define system guiSystem
#define Sleep guiSleep
#define SetConsoleTextAttribute guiSetConsoleTextAttribute
#define main originalGameMain
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <windows.h>

HANDLE h;

//room 1

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
        roomonelevelonecorrect = 0;
        roomoneleveloneincorrect = 0;

        system("cls");
        SetConsoleTextAttribute(h, 11);
        printf("\n========== EASY LEVEL ==========\n");
        SetConsoleTextAttribute(h, 7);

        if(room1AskMCQ("1. What gets wetter the more it dries?",
        "Sponge", "Towel", "Tissue", "Cloth", 'B', "", NULL))
            roomonelevelonecorrect++;
        else
            roomoneleveloneincorrect++;

        if(room1AskMCQ("2. Which musical instrument has 88 keys?",
        "Organ", "Harpsichord", "Piano", "Accordion", 'C', "", NULL))
            roomonelevelonecorrect++;
        else
            roomoneleveloneincorrect++;

        if(room1AskMCQ("3. What follows you in daylight but disappears in complete darkness?",
        "Reflection", "Shadow", "Silhouette", "Footprint", 'B', "", NULL))
            roomonelevelonecorrect++;
        else
            roomoneleveloneincorrect++;

        if(room1AskMCQ("4. What has many teeth and is used to untangle hair?",
        "Brush", "Comb", "Razor", "Clip", 'B', "", NULL))
            roomonelevelonecorrect++;
        else
            roomoneleveloneincorrect++;

        if(room1AskMCQ("5. David's father has three sons: Snap, Crackle and _____?",
        "Pop", "David", "Jack", "Crackle", 'B', "", NULL))
            roomonelevelonecorrect++;
        else
            roomoneleveloneincorrect++;

        SetConsoleTextAttribute(h, 14);
        printf("\n========== EASY LEVEL RESULTS ==========\n");

        printf("Correct Answers: %d/5\n", roomonelevelonecorrect);
        printf("Incorrect Answers: %d/5\n", roomoneleveloneincorrect);
        SetConsoleTextAttribute(h, 7);

        if(roomonelevelonecorrect >= 3)
        {
            SetConsoleTextAttribute(h, 10);
            printf("\nCongratulations! EASY LEVEL CLEARED!\n");
            printf("You have unlocked the NORMAL LEVEL!\n");
            Sleep(2000);
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
            Sleep(2000);
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
        SetConsoleTextAttribute(h, 11);
        printf("\n========== NORMAL LEVEL ==========\n");
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
        SetConsoleTextAttribute(h, 7);

        if(roomoneleveltwocorrect >= 3)
        {
            SetConsoleTextAttribute(h, 10);
            printf("\nCongratulations! NORMAL LEVEL CLEARED!\n");
            printf("You have unlocked the DIFFICULT LEVEL!\n");
            Sleep(2000);
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
            Sleep(2000);
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
        SetConsoleTextAttribute(h, 11);
        printf("\n========== DIFFICULT LEVEL ==========\n");
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
        SetConsoleTextAttribute(h, 7);

        if(roomonelevelthreecorrect >= 3)
        {
            SetConsoleTextAttribute(h, 10);
            printf("\nCongratulations! DIFFICULT LEVEL CLEARED!\n");
            Sleep(2000);
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
            Sleep(2000);
            system("cls");
            SetConsoleTextAttribute(h, 7);
        }
    }
}

int room1()
{
    int totalCorrect;
    int totalIncorrect;

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
    printf("- EASY MODE: No hints are available.\n");
    printf("- NORMAL MODE: One hint can be used on any one question.\n");
    printf("- DIFFICULT MODE: Two hints can be used on any two separate questions.\n");
    printf("- Incorrect answers will not reveal the correct option.\n");
    printf("- Failed levels must be attempted again.\n");

    room1level1();
    room1level2();
    room1level3();

    SetConsoleTextAttribute(h, 14);
    printf("\n====================================\n");
    printf("          ROOM 1 LEADERBOARD\n");
    printf("====================================\n");
    SetConsoleTextAttribute(h, 7);

    printf("\nEasy Level:\n");
    printf("Correct: %d | Incorrect: %d\n",
    roomonelevelonecorrect, roomoneleveloneincorrect);

    printf("\nNormal Level:\n");
    printf("Correct: %d | Incorrect: %d\n",
    roomoneleveltwocorrect, roomoneleveltwoincorrect);

    printf("\nDifficult Level:\n");
    printf("Correct: %d | Incorrect: %d\n",
    roomonelevelthreecorrect, roomonelevelthreeincorrect);

    totalCorrect = roomonelevelonecorrect +
                   roomoneleveltwocorrect +
                   roomonelevelthreecorrect;

    totalIncorrect = roomoneleveloneincorrect +
                     roomoneleveltwoincorrect +
                     roomonelevelthreeincorrect;

    printf("\nTotal Correct Answers: %d\n", totalCorrect);
    printf("Total Incorrect Answers: %d\n", totalIncorrect);
    printf("Final Score: %d/15\n", totalCorrect);

    SetConsoleTextAttribute(h, 10);
    printf("\n====================================\n");
    printf(" CONGRATULATIONS! YOU PASSED ROOM 1!\n");
    printf(" You have successfully completed all three levels :D.\n");
    printf(" Proceeding to ROOM 2...\n");
    printf("====================================\n");
    Sleep(2500);
    system("cls");
    SetConsoleTextAttribute(h, 7);

    return 1;
}

//room 1 end
//room 2

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
    printf("\n\n\t\t========== LEVEL 1: EASY ==========\n");
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
    SetConsoleTextAttribute(h, 7);

    return score;
}

int room2level2(int firstAttempt)
{
    int score=0;
    int lifelineAvailable=firstAttempt;

    SetConsoleTextAttribute(h, 11);
    printf("\n\n\t\t========== LEVEL 2: MEDIUM ==========\n");
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
    SetConsoleTextAttribute(h, 7);

    return score;
}

int room2level3(int firstAttempt)
{
    int score=0;
    int lifelineAvailable=firstAttempt;

    SetConsoleTextAttribute(h, 11);
    printf("\n\n\t\t========== LEVEL 3: HARD ==========\n");
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
    SetConsoleTextAttribute(h, 7);

    return score;
}

void room2()
{
    int score1,score2,score3;
    int firstAttempt1=1;
    int firstAttempt2=1;
    int firstAttempt3=1;

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
    Sleep(3000);
    system("cls");

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
    printf("\n\n\t\tTOTAL SCORE: %d/15",score1+score2+score3);
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
}
//room 3 begains
#include<stdio.h>
#include<windows.h>
#include<stdlib.h>





int threeonecount = 0;
int threeonedcount = 0;
int threetwocount = 0;
int threetwodcount = 0;
int threethreecount = 0;
int threethreedcount = 0;
HANDLE h;

int intro(void){
    h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h, 13);
    printf("\n==================================================\n");
    printf("\n=====  WELCOME TO THE ACADEMIC ESCAPE ROOM  ===== ");
    SetConsoleTextAttribute(h, 7);
    printf("\n=====================================================\n");
    Sleep(3000);
    system("cls");
    printf("\n        A ROOM BUILT TO TEST YOUR KNOWLEDGE ");
    Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 12);
    printf("\n               CAN YOU ESCAPE IT?\n");
    SetConsoleTextAttribute(h, 13);
    printf("\n********************************************************\n");
    Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 10);
    printf("\n==================   GAME INFO   ===================");
    SetConsoleTextAttribute(h, 7);
    printf("\nThere are three rooms:\nRoom 1: Riddles\nRoom 2: Mathematics\nRoom 3: C programming");
    printf("\nEach room has 3 levels. Score at least 3 correct answers per level to proceed.");
    Sleep(6000);
    system("cls");
    Sleep(3000);
    printf("\nGAME STARTING...");
    return 0;
}
int room3level1(){
    Sleep(3000);
    system("cls");
    int l_count=0;


    printf("==================ROOM 3 : coding with C language =================== \n LEVEL 1: EASY MODE \n");
    Sleep(3500);

    system("cls");
    printf("Press L to activate life line :   \n it will remove 2 incorrect options ");
    Sleep(2000);
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
            printf("Incorrect Answer!\nCorrect answer: C. math.h\n");
            threeonedcount++;
        }
    }
    else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\nCorrect answer: C. math.h\n");
        threeonedcount++;
    }
    Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 2. What is the correct way to declare a constant in C? \n");
    printf("\n A. const int x = 5; \n B. #define PI 3.14 \n C. constant int x = 5; \n D. int const x = 5; \n or Press L for 50 - 50");
    printf("\n Enter your answer: ");
    char ans2;
    scanf(" %c",&ans2);
    if(ans2 == 'l' || ans2 == 'L'){
        if(l_count < 2){
            l_count++;
            printf("\nTwo incorrect options removed. Remaining options:\n A. const int x = 5; \n B. #define PI 3.14");
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
        printf("Incorrect Answer!\nCorrect answer: B. #define PI 3.14\n");
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
        printf("Incorrect Answer!\nCorrect answer: A. 1\n");
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
        printf("Incorrect Answer!\nCorrect answer: B. variable_1\n");
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
        printf("Incorrect Answer!\nCorrect answer: A. 7\n");
        threeonedcount++;
    }
    Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);

    return 0;
}
int room3level2(){
    printf("\n\n\nROOM 3 : coding with C language  \n LEVEL 2: MEDIUM MODE \n");
    system("cls");
    printf("\n\nQuestion 1. What is the output of the following code?\n int a = 5; \n int b = 10; \n printf(\"%%d\", a > b ? a : b); \n");
    printf("\n A. 5 \n B. 10 \n C. 15 \n D. Error \n or Press L for 50 - 50 \n");
    printf("\n Enter your answer: ");
    char ans1;
    scanf(" %c",&ans1);
    if(ans1 == 'B' || ans1 == 'b'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threetwocount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\nCorrect answer: B. 10\n");
        threetwodcount++;
    }Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 2. Which of the following is used to allocate memory dynamically in C? \n");
    printf("\n A. malloc() \n B. calloc() \n C. realloc() \n D. All of the above \n or Press L for 50 - 50 \n");
    printf("\n Enter your answer: ");
    char ans2;
    scanf(" %c",&ans2);
    if(ans2 == 'D' || ans2 == 'd'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threetwocount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\nCorrect answer: D. All of the above\n");
        threetwodcount++;
    }Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 3. Find the output:\n for(int i=0;i<5;i++){\nif(i==3)continue;\nprintf(\"%%d\",i);}");
    printf("\n A. 01234 \n B. 0124 \n C. 1234 \n D. 123 \n or Press L for 50 - 50 \n");
    printf("\n Enter your answer: ");
    char ans3;
    scanf(" %c",&ans3);
    if(ans3 == 'B' || ans3 == 'b'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threetwocount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\nCorrect answer: B. 0124\n");
        threetwodcount++;
    }Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 4. Which of the following is used to read a character from the user in C? \n");
    printf("\n A. scanf() \n B. getchar() \n C. getchar() \n D. All of the above \n or Press L for 50 - 50 \n");
    printf("\n Enter your answer: ");
    char ans4;
    scanf(" %c",&ans4);
    if(ans4 == 'D' || ans4 == 'd'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threetwocount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\nCorrect answer: D. All of the above\n");
        threetwodcount++;
    }Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 5. What is the output of the following code?\n int x = 5; \n int y = 10; \n printf(\"%%d\", x ^ y); \n");
    printf("\n A. 0 \n B. 5 \n C. 10 \n D. 15 \n or Press L for 50 - 50 \n");
    printf("\n Enter your answer: ");
    char ans5;
    scanf(" %c",&ans5);
    if(ans5 == 'D' || ans5 == 'd'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threetwocount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\nCorrect answer: D. 15\n");
        threetwodcount++;
    }Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);

    return 0;
}
int room3level3(){
    printf("\n\n\nROOM 3 : coding with C language  \n LEVEL 3: HARD MODE \n");
    Sleep(3000);
    system("cls");
    printf("\n\nQuestion 1. What is the output of the following code?\n int x = 5; \n int y = 10; \n printf(\"%%d\", x & y); \n");
    printf("\n A. 0 \n B. 5 \n C. 10 \n D. 15 \n or Press L for 50 - 50 \n");
    printf("\n Enter your answer: ");
    char ans1;
    scanf(" %c",&ans1);
    if(ans1 == 'A' || ans1 == 'a'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threethreecount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\nCorrect answer: A. 0\n");
        threethreedcount++;
    }
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 2. Which of the following is used to terminate a program in C? \n");
    printf("\n A. exit() \n B. return() \n C. break() \n D. continue() \n or Press L for 50 - 50 \n");
    printf("\n Enter your answer: ");
    char ans2;
    scanf(" %c",&ans2);
    if(ans2 == 'A' || ans2 == 'a'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threethreecount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\nCorrect answer: A. exit()\n");
        threethreedcount++;
    }
    Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 3. Find the output of the following code:\n int arr[5] = {1, 2, 3, 4, 5}; \n printf(\"%%d\", *(arr + 2)); \n");
    printf("\n A. 1 \n B. 2 \n C. 3 \n D. 4 \n or Press L for 50 - 50 \n");
    printf("\n Enter your answer: ");
    char ans3;
    scanf(" %c",&ans3);
    if(ans3 == 'C' || ans3 == 'c'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        
        threethreecount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\nCorrect answer: C. 3\n");
        threethreedcount++;
    }Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 4. Which of the following is used to read a string from the user in C? \n");
    printf("\n A. scanf() \n B. gets() \n C. fgets() \n D. All of the above \n or Press L for 50 - 50 \n");
    printf("\n Enter your answer: ");
    char ans4;
    scanf(" %c",&ans4);
    if(ans4 == 'D' || ans4 == 'd'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threethreecount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\nCorrect answer: D. All of the above\n");
        threethreedcount++;
    }Sleep(3000);

    system("cls");
    SetConsoleTextAttribute(h, 7);
    printf("\n\nQuestion 5. What is the output of the following code?\n int x = 5; \n int y = 10; \n printf(\"%%d\", x | y); \n");
    printf("\n A. 0 \n B. 5 \n C. 10 \n D. 15 \n or Press L for 50 - 50 \n");
    printf("\n Enter your answer: ");
    char ans5;
    scanf(" %c",&ans5);
    if(ans5 == 'D' || ans5 == 'd'){
        SetConsoleTextAttribute(h, 10);
        printf("Correct Answer!\n");
        threethreecount++;
    } else {
        SetConsoleTextAttribute(h, 12);
        printf("Incorrect Answer!\nCorrect answer: D. 15\n");
        threethreedcount++;
    }Sleep(3000);
    system("cls");
    SetConsoleTextAttribute(h, 7);
    
    return 0;

}
int room3(){
    do {
        threeonecount = 0;
        threeonedcount = 0;
        room3level1();
        Sleep(3000);
        system("cls");
        SetConsoleTextAttribute(h, 10);
        printf("\n                   loading reults.....");
        Sleep(2000);
        system("cls");
        SetConsoleTextAttribute(h, 7);
        printf("\n               =======Level 1 results=======\n");
        printf("\n       Total correct answers in LEVEL 1: %d \n", threeonecount);
        printf("\n       Total incorrect answers in LEVEL 1: %d \n", threeonedcount);
        if(threeonecount < 3){
            printf("\nYou need at least 3 correct answers. Retrying Level 1.\n");
            Sleep(3000);
        }
    } while(threeonecount < 3);

    do {
        threetwocount = 0;
        threetwodcount = 0;
        room3level2();
        Sleep(7000);
        system("cls");
        SetConsoleTextAttribute(h, 10);
        printf("\n                   loading reults.....");
        Sleep(2000);
        system("cls");
        SetConsoleTextAttribute(h, 7);
        printf("\n           ===========Level 2 Results===========\n");
        printf("\n       Total correct answers in LEVEL 2: %d \n", threetwocount);
        printf("\n       Total incorrect answers in LEVEL 2: %d \n", threetwodcount);
        if(threetwocount < 3){
            printf("\nYou need at least 3 correct answers. Retrying Level 2.\n");
            Sleep(3000);
        }
    } while(threetwocount < 3);

    do {
        threethreecount = 0;
        threethreedcount = 0;
        room3level3();
        Sleep(7000);
        system("cls");
        SetConsoleTextAttribute(h, 10);
        printf("\n                   loading reults.....");
        Sleep(3000);
        system("cls");
        SetConsoleTextAttribute(h, 7);
        printf("\n           ==========Level 3 Results===========\n");
        printf("\n        Total correct answers in LEVEL 3: %d \n", threethreecount);
        printf("\n        Total incorrect answers in LEVEL 3: %d \n", threethreedcount);
        if(threethreecount < 3){
            printf("\nYou need at least 3 correct answers. Retrying Level 3.\n");
            Sleep(3000);
        }
    } while(threethreecount < 3);

    return 0;
}

int main(void){
    
    intro();
    room1();
    room2();
    room3();


    SetConsoleTextAttribute(h, 14);
    printf("\n\n=============== LEADERBOARD ================\n\n");
    SetConsoleTextAttribute(h, 7);
    SetConsoleTextAttribute(h, 10);
    printf("\nRoom 3 score: %d/15\n", threeonecount + threetwocount + threethreecount);
    SetConsoleTextAttribute(h, 7);
    return 0;
}


#undef main
#undef SetConsoleTextAttribute
#undef Sleep
#undef system
#undef scanf
#undef printf

static void drawFinalLeaderboard(int mouseX, int mouseY)
{
    const char *levelNames[3] = {"EASY", "NORMAL", "DIFFICULT"};
    const char *roomLabels[3] = {"ROOM 1 - RIDDLES", "ROOM 2 - MATH", "ROOM 3 - C"};
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
    SDL_Rect panel = {55, 105, 990, 500};
    SDL_Rect closeButton = {835, 620, 190, 54};
    int room;
    int level;
    int row = 0;
    int grandCorrect = 0;
    int grandIncorrect = 0;
    char text[128];

    SDL_SetRenderDrawColor(guiRenderer, 10, 15, 29, 255);
    SDL_RenderClear(guiRenderer);
    SDL_SetRenderDrawColor(guiRenderer, 20, 28, 48, 255);
    SDL_RenderFillRect(guiRenderer, &panel);
    SDL_SetRenderDrawColor(guiRenderer, 100, 225, 155, 255);
    SDL_RenderDrawRect(guiRenderer, &panel);

    drawText(345, 48, "FINAL LEADERBOARD",
             (SDL_Color){245, 247, 255, 255});
    drawText(85, 128, "ROOM", (SDL_Color){155, 170, 195, 255});
    drawText(355, 128, "LEVEL", (SDL_Color){155, 170, 195, 255});
    drawText(535, 128, "CORRECT", (SDL_Color){155, 170, 195, 255});
    drawText(700, 128, "INCORRECT", (SDL_Color){155, 170, 195, 255});
    drawText(900, 128, "SCORE", (SDL_Color){155, 170, 195, 255});

    for(room = 0; room < 3; room++){
        int roomCorrect = 0;
        int roomIncorrect = 0;
        for(level = 0; level < 3; level++){
            SDL_Rect rowRect = {75, 157 + row * 31, 950, 29};
            SDL_Color rowColor = row % 2 == 0
                ? (SDL_Color){29, 39, 59, 255}
                : (SDL_Color){24, 33, 51, 255};
            SDL_SetRenderDrawColor(guiRenderer, rowColor.r, rowColor.g, rowColor.b, rowColor.a);
            SDL_RenderFillRect(guiRenderer, &rowRect);

            drawText(90, rowRect.y + 6, roomLabels[room], roomColors[room]);
            drawText(355, rowRect.y + 6, levelNames[level],
                     (SDL_Color){235, 239, 248, 255});
            sprintf(text, "%d", correct[room][level]);
            drawText(565, rowRect.y + 6, text,
                     (SDL_Color){100, 225, 155, 255});
            sprintf(text, "%d", incorrect[room][level]);
            drawText(755, rowRect.y + 6, text,
                     (SDL_Color){255, 165, 165, 255});
            sprintf(text, "%d / 5", correct[room][level]);
            drawText(900, rowRect.y + 6, text,
                     (SDL_Color){235, 239, 248, 255});

            roomCorrect += correct[room][level];
            roomIncorrect += incorrect[room][level];
            row++;
        }

        sprintf(text, "ROOM %d TOTAL: %d / 15 CORRECT, %d INCORRECT",
                room + 1, roomCorrect, roomIncorrect);
        drawText(90, 455 + room * 25, text, roomColors[room]);
        grandCorrect += roomCorrect;
        grandIncorrect += roomIncorrect;
    }

    sprintf(text, "GRAND TOTAL: %d / 45 CORRECT, %d INCORRECT",
            grandCorrect, grandIncorrect);
    drawText(90, 535, text, (SDL_Color){255, 218, 125, 255});
    drawButton(closeButton, "CLOSE", roomColors[2], mouseX, mouseY);
    SDL_RenderPresent(guiRenderer);
}

static void waitForLeaderboard(void)
{
    SDL_Rect closeButton = {835, 620, 190, 54};
    SDL_Event event;
    int running = 1;
    int mouseX;
    int mouseY;

    SDL_SetWindowTitle(guiWindow, "Academic Escape Room - Final Leaderboard");
    while(running){
        SDL_GetMouseState(&mouseX, &mouseY);
        drawFinalLeaderboard(mouseX, mouseY);
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT ||
               (event.type == SDL_KEYDOWN &&
                (event.key.keysym.sym == SDLK_ESCAPE ||
                 event.key.keysym.sym == SDLK_RETURN))){
                running = 0;
            } else if(event.type == SDL_MOUSEBUTTONDOWN &&
                      event.button.button == SDL_BUTTON_LEFT &&
                      pointInRect(event.button.x, event.button.y, &closeButton)){
                running = 0;
            }
        }
        SDL_Delay(16);
    }
}

int main(void)
{
    int result;

    SDL_SetMainReady();
    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    guiWindow = SDL_CreateWindow(
        "Academic Escape Room - Click to play",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        GUI_WIDTH, GUI_HEIGHT, SDL_WINDOW_SHOWN
    );
    if(guiWindow == NULL){
        fprintf(stderr, "Could not create SDL window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    guiRenderer = SDL_CreateRenderer(guiWindow, -1, SDL_RENDERER_ACCELERATED);
    if(guiRenderer == NULL){
        guiRenderer = SDL_CreateRenderer(guiWindow, -1, SDL_RENDERER_SOFTWARE);
    }
    if(guiRenderer == NULL){
        fprintf(stderr, "Could not create SDL renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(guiWindow);
        SDL_Quit();
        return 1;
    }

    waitForStart();
    skipLegacyIntro = 1;
    result = originalGameMain();
    waitForLeaderboard();

    SDLTest_CleanupTextDrawing();
    SDL_DestroyRenderer(guiRenderer);
    SDL_DestroyWindow(guiWindow);
    SDL_Quit();
    return result;
}