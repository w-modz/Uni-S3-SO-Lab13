#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <atlstr.h>

#define BOARD_SIZE 3

struct SharedData
{
    char board[9];
    bool is_x_turn;
    bool game_ended;
};

// Funkcja rysujaca stan gry
void refreshBoard(SharedData* sharedData) 
{
    system("cls");

    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++)
    {
        if (sharedData->board[i] == ' ')
        {
            printf(" %i ", i + 1);
        }
        else
        {
            printf(" %c ", sharedData->board[i]);
        }
        if ((i + 1) % 3 != 0)
        {
            printf("|");
        }
        else if (i < 8)
        {
            printf("\n---+---+---\n");
        }
    }

    printf("\n");
}

bool isValidMove(SharedData* sharedData, int move) {
    if (move < BOARD_SIZE * BOARD_SIZE && move >= 0 && sharedData->board[move] == ' ')
        return true;
    printf("Invalid move.\n");
    return false;
}

int getMove()
{
    int move;
    printf("Make a move: \n");
    scanf("%d", &move);
    return move - 1;
}

bool checkWin(SharedData* sharedData, char player_char) {
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        // Sprawdzenie wygranej w wierszu
        if (sharedData->board[0+(i*3)] == player_char && sharedData->board[1 + (i * 3)] == player_char && sharedData->board[2 + (i * 3)] == player_char)
        {
            return true;
        }
        // Sprawdzenie wygranej w kolumnie
        if (sharedData->board[0 + i] == player_char && sharedData->board[3 + i] == player_char && sharedData->board[6 + i] == player_char)
        {
            return true;
        }
    }
    // Sprawdzenie wygranych po skosie
    if (sharedData->board[0] == player_char && sharedData->board[4] == player_char && sharedData->board[8] == player_char)
    {
        return true;
    }
    if (sharedData->board[2] == player_char && sharedData->board[4] == player_char && sharedData->board[6] == player_char)
    {
        return true;
    }
    return false;
}

int main(int argc, char* argv[])
{
    // Weryfikacja ilosci argumentow
    if (argc != 2)
    {
        perror("Invalid argument count\n");
        return EXIT_FAILURE;
    }
    bool is_x = false;
    CString sharedMemoryName = CString(argv[1]);
    HANDLE mapfileHandle;
    // Proba otworzenia pamieci wspoldzielonej
    mapfileHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS ,NULL, (LPCWSTR)sharedMemoryName);
    if (mapfileHandle == NULL)
    {
        // W przypadku niepowodzenia, inicjalizacja pamieci
        printf("Connecting to shared memory failed, initializing shared memory.\n");
        is_x = true;
        mapfileHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedData), (LPCWSTR)sharedMemoryName);
        if (mapfileHandle == NULL) {
            fprintf(stderr, "Unable to map file.\n");
            printf("%d", GetLastError());
            return 1;
        }
    }
    // Podlaczenie do pamieci wspoldzielonej
    SharedData* sharedData = (SharedData*)MapViewOfFile(mapfileHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));
    if (sharedData == NULL) {
        fprintf(stderr, "Unable to create map view of file.\n");
        CloseHandle(mapfileHandle);
        return 1;
    }
    // Inicjalizacja gry
    if (is_x)
    {
        for (int i = 0; i < BOARD_SIZE*BOARD_SIZE; i++)
        {
            sharedData->board[i] = ' ';
        }
        printf("Waiting for other player...\n");
        while (sharedData->is_x_turn == false);
    }
    else
    {
        sharedData->game_ended = false;
        sharedData->is_x_turn = true;
    }
    // Petla do komunikacji miedzy programami
    while (true)
    {
        if (sharedData->is_x_turn && is_x)
        {
            if (sharedData->game_ended == true)
            {
                refreshBoard(sharedData);
                printf("Winner: o\n");
                sharedData->game_ended = true;
                sharedData->is_x_turn = false;
                break;
            }
            refreshBoard(sharedData);
            int move = getMove();
            while (!isValidMove(sharedData, move))
            {
                move = getMove();
            }
            sharedData->board[move] = 'x';
            if (checkWin(sharedData, 'x'))
            {
                refreshBoard(sharedData);
                printf("Winner: x\n");
                sharedData->game_ended = true;
                sharedData->is_x_turn = false;
                break;
            }
            sharedData->is_x_turn = false;
            while (sharedData->is_x_turn == false);
        }
        if (!sharedData->is_x_turn && !is_x)
        {
            if (sharedData->game_ended == true)
            {
                refreshBoard(sharedData);
                printf("Winner: x\n");
                sharedData->game_ended = true;
                sharedData->is_x_turn = true;
                break;
            }
            refreshBoard(sharedData);
            int move = getMove();
            while (!isValidMove(sharedData, move))
            {
                move = getMove();
            }
            sharedData->board[move] = 'o';
            if (checkWin(sharedData, 'o'))
            {
                refreshBoard(sharedData);
                printf("Winner: o\n");
                sharedData->game_ended = true;
                sharedData->is_x_turn = true;
                break;
            }
            sharedData->is_x_turn = true;
            while (sharedData->is_x_turn == true);
        }
    }
    // Odlaczenie i zakonczenie pamieci wspoldzielonej
    UnmapViewOfFile(mapfileHandle);
    CloseHandle(mapfileHandle);
    return EXIT_SUCCESS;
}

