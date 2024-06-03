// Simona Ceskova xcesko00
// 24.04.2024
// PRL 2
// Implementace closed
// soubor pocita se vstupnim souborem, kterym je zakoncen jednim prazdnym radkem

// 00000000
// 00111000
// 01110000
// 00000000
//

// Pro vstupni soubor, ktery nema jeden prazdny radek (jako vyse), se vyhodnoti program jako segmentation for
// prosim o pripradnou upravu formatu vstupniho souboru


#include <bits/stdc++.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <time.h>
#include <stdio.h>
using namespace std;
#include <assert.h>
#include <queue>
#include <mpi.h>

//constanty pro prehlednejsi implementaci
const int ALIVE = 1, DEAD = 0;

//pomocna funkce na vypis posledniho kroku hry
void print_game(vector<vector<int>> game)
{
    int proc = 0;
    for (vector<int> &row : game)
    {
        cout << proc << ": ";
        proc++;
        for (int cell : row)
            cout << cell;
        cout << "\n";
    }
}

//funkce pro nacteni souboru
//world_size je pro stopnuti nacitani po nacteni n-teho radku podle poctu procesoru
// 1 procesor - 1 radek
vector<vector<int>> load_file(string filePath, int world_size)
{
    //counter pro pocitani radku
    int counter = 0;

    vector<vector<int>> array;
    ifstream input(filePath);

    if (!input.is_open())
    {
        std::cerr << "Failed to open the numbers file" << std::endl; // neotevřel se soubor
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    char ch;
    int numCols = 0;
    vector<int> row;

    // Read file and populate vector
    while (input.get(ch))
    {

        if (ch == '0' || ch == '1')
        {
            row.push_back(ch - '0');
            numCols++;
        }
        else if ((ch == '\n') || (ch == EOF))
        {
            if(counter >= world_size)
            {
                cout << counter << "\n";
                break;
            }
            counter++;
            //konec jednoho radku pushnu jako vector do vectoru
            array.push_back(row);
            row.clear();
            numCols = 0;
        }
    }

    input.close();
    return array;
}

//updatovani hry podle ostatnich radku
vector<vector<int>> update_game(vector<vector<int>> game, vector<vector<int>> counts, int col_start, int col_end, int row_size)
{
    vector<vector<int>> updated;
    vector<int> row;
    int m = 0, n = 0;

    for (int j = col_start; j < col_end; j++)
    {
        for (int i = 0; i < row_size; i++)
        {
            // každá živá buňka s více než třemi živými sousedy umírá
            // každá živá buňka s méně než dvěma živými sousedy umírá
            if ((game[j][i] == ALIVE) && ((counts[m][n] > 3) || (counts[m][n] < 2)))
            {
                row.push_back(DEAD);
            }
            // každá mrtvá buňka s právě třemi živými sousedy ožívá
            else if ((game[j][i] == DEAD) && (counts[m][n] == 3))
            {
                row.push_back(ALIVE);
            }
            // každá živá buňka se dvěma nebo třemi živými sousedy zůstává žít
            else if ((game[j][i] == ALIVE) && ((counts[m][n] == 3) || (counts[m][n] == 2)))
                row.push_back(ALIVE);
            else
                row.push_back(DEAD);
            n++;
        }
        m++;
        updated.push_back(row);
        row.clear();
    }
    return updated;
}

//pomocna funkce pro spocitani sousedicich
//vysledkem je 2D vector s pocty sousedu pro dany prvek
vector<vector<int>> count_surroundings(vector<vector<int>> game, int col_start, int col_end, int col_size, int row_size)
{
    vector<vector<int>> counts;
    vector<int> row;
    int point = 0;

    for (int j = col_start; j < col_end; j++)
    {
        for (int i = 0; i < row_size; i++)
        {
            if (j != 0)
            {
                point += game[j - 1][i];
                if (i != row_size - 1)
                    point += game[j - 1][i + 1];
                if (i != 0)
                    point += game[j - 1][i - 1];
            }
            
            if (j != col_size - 1)
            {
                if (i != row_size - 1)
                    point += game[j + 1][i + 1];
                if (i != 0)
                    point += game[j + 1][i - 1];
                point += game[j + 1][i];
            }

            if (i != row_size - 1)
                point += game[j][i + 1];
            if (i != 0)
                point += game[j][i - 1];
            
            row.push_back(point);
            point = 0;
        }
        counts.push_back(row);
        row.clear();
    }
    return counts;
}

int main(int argc, char **argv)
{
    //vektor pro ulozeni hry
    vector<vector<int>> game;
    //pomocny vektor tvorici jeden radek hry
    vector<int> row;
    vector<vector<int>> counts;
    int work, rest;
    int world_rank, world_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    //vraceni nacteneho posle ze vstupniho souboru
    game = load_file(argv[argc - 2], world_size);

    //pocet kroku co se ma vykonat
    int steps = stoi(argv[argc - 1]);

    //sloupce x radek
    int col_size = game.size();
    int row_size = game[0].size();

    MPI_Barrier(MPI_COMM_WORLD);

    //--------------------------------------------------- hlavni cast algoritmu-------------------------------------------------------
    //pomocne promenne pro pouziti MPI_Allgatherv
    int displacements[world_size], N[world_size];
    for (int r = 0; r < world_size; r++)
    {
        displacements[r] = r * row_size;
        N[r] = row_size;
    }

    int to_send[row_size];
    int buffer[world_size * row_size]; // prijimam[vsechny prvky]
    for (int step = 0; step < steps; step++)
    {

        //kazdym procesor si spocita pocet svych sousedu sam
        counts = count_surroundings(game, world_rank, world_rank + 1, col_size, row_size);
        //kazdy procesor si updatuje svuj vlastni radek
        game = update_game(game, counts, world_rank, world_rank + 1, row_size);

        //formatovani vstupu do MPI funkce
        for (int k = 0; k < row_size; k++)
        {
            to_send[k] = game[0][k];
        }

        //pomoci teto funkce kazdy procesor posle informace v danem poradi sveho ranku a podle toho je i prijme
        MPI_Allgatherv(to_send, row_size, MPI_INT, buffer, N, displacements, MPI_INT, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        //vycisteni puvodnih dat hry
        game.clear();

        //pomocny cyklus pro formatovani vystupu 
        for (int m = 0; m < world_size; m++)
        {
            for (int k = row_size * m; k < row_size * (m + 1); k++)
                row.push_back(buffer[k]);
            game.push_back(row);
            row.clear();
        }
    }
    //--------------------------------------------------- hlavni cast algoritmu-------------------------------------------------------
    //vytisknuti nakonec hry po x-tem tahu
    if (world_rank == 0)
        print_game(game);

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    return 0;
}
