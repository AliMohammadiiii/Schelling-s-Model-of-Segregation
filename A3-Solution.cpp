#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

using namespace std;

constexpr int INFINITE_SIMULATION = 0;
constexpr int DEFAULT_HAPPINESS_THRESHOLD = 30;

constexpr char RED_REPRESENTATION = 'R';
constexpr char BLUE_REPRESENTATION = 'B';
constexpr char EMPTY_REPRESENTATION = 'E';

enum CellType
{
    RED,
    BLUE,
    EMPTY
};

typedef vector<vector<CellType>> World;

typedef pair<int, int> Coordinate;

struct CommandLineArgs
{
    int happiness_threshold = DEFAULT_HAPPINESS_THRESHOLD;
    int simulation_count = INFINITE_SIMULATION;
    string map_file_path;
};

CommandLineArgs get_command_line_args(int arg_count, char const* args[]);
CellType get_cell_type_from_char(char cell);
char get_char_from_cell_type(CellType cell);
World get_world_from_file(string file_path);
void print_world(World world);
bool is_inside(int x, int y, World world);
double calculate_happiness(Coordinate coordinate, World world);
bool is_happy(Coordinate coordinate, World world, int happiness_threshold);
int get_unhappy_count(World world, int happiness_threshold);
vector<Coordinate> get_jumpable_coordinates(World world, int happiness_threshold);
World run_one_generation(World old_world, int happiness_threshold);
World run_finite_simulation(World old_world, int simulation_count, int happiness_threshold);
World run_infinite_simulation(World old_world, int simulation_count, int happiness_threshold);
World run_simulation(World old_world, int simulation_count, int happiness_threshold);
void create_ppm_file(World world);
void print_result(World world, int happiness_threshold);

int main(int argc, char const *argv[])
{
    CommandLineArgs command_line_args = get_command_line_args(argc, argv);
    World world = get_world_from_file(command_line_args.map_file_path);
    World final_world = run_simulation(world, command_line_args.simulation_count, command_line_args.happiness_threshold);
    print_result(final_world, command_line_args.happiness_threshold);
    create_ppm_file(final_world);
    return 0;
}

CommandLineArgs get_command_line_args(int arg_count, char const* args[])
{
    const string FILE_NAME_PREFIX = "-f";
    const string HAPPINESS_THRESHOLD_PREFIX = "-p";
    const string SIMULATION_COUNT_PREFIX = "-s";

    CommandLineArgs command_line_args;
    for (int i = 0; i < arg_count; i++)
    {
        if (args[i] == HAPPINESS_THRESHOLD_PREFIX)
            command_line_args.happiness_threshold = stoi(args[i+1]);
        else if (args[i] == SIMULATION_COUNT_PREFIX)
            command_line_args.simulation_count = stoi(args[i+1]);
        else if (args[i] == FILE_NAME_PREFIX)
            command_line_args.map_file_path = args[i+1];
    }
    return command_line_args;
}

CellType get_cell_type_from_char(char cell)
{
    if (cell == RED_REPRESENTATION)
        return CellType::RED;
    else if (cell == BLUE_REPRESENTATION)
        return CellType::BLUE;
    else
        return CellType::EMPTY;
}

char get_char_from_cell_type(CellType cell)
{
    if (cell == CellType::RED)
        return RED_REPRESENTATION;
    else if (cell == CellType::BLUE)
        return BLUE_REPRESENTATION;
    else
        return EMPTY_REPRESENTATION;
}

World get_world_from_file(string file_path)
{
    ifstream input_stream(file_path);

    World world;
    string line;

    while (getline(input_stream, line))
    {
        vector<CellType> row;
        for (int i = 0; i < line.size(); i++)
        {
            row.push_back(get_cell_type_from_char(line[i]));
        }
        world.push_back(row);
    }
    input_stream.close();
    return world;
}

void print_world(World world)
{
    for (int i = 0; i < world.size(); i++)
    {
        for (int j = 0; j < world[i].size(); j++)
        {
            cout << get_char_from_cell_type(world[i][j]);
        }
        cout << endl;
    }
}

bool is_inside(int x, int y, World world)
{
    return (x >= 0 && x < world.size() && y >=0 && y < world[0].size());
}

double calculate_happiness(Coordinate coordinate, World world)
{
    int new_x, new_y;
    int neighbours_count = 0;
    int similar_neighbours_count = 0;
    for (int dx = -1 ; dx <= 1 ; dx++) {
        for (int dy = -1 ; dy <= 1 ; dy++)
        {
            if (abs(dx) + abs(dy) == 1)
            {
                new_x = coordinate.first + dx;
                new_y = coordinate.second + dy;
                if (is_inside(new_x, new_y, world))
                {
                    neighbours_count++;
                    CellType current_neighbour = world[new_x][new_y];
                    if (world[coordinate.first][coordinate.second] ==  current_neighbour || current_neighbour == CellType::EMPTY)
                        similar_neighbours_count++;
                }
            }
        }
    }
    return (double)similar_neighbours_count / (double)neighbours_count * 100;
}

bool is_happy(Coordinate coordinate, World world, int happiness_threshold)
{
    return calculate_happiness(coordinate, world) >= happiness_threshold;
}

int get_unhappy_count(World world, int happiness_threshold)
{
    int unhappy_count = 0;
    for (int i = 0; i < world.size(); i++)
    {
        for (int j = 0; j < world[i].size(); j++)
        {
            if (world[i][j] != CellType::EMPTY && !is_happy(Coordinate{i, j}, world, happiness_threshold))
                unhappy_count++;
        }
    }
    return unhappy_count;
}

vector<Coordinate> get_jumpable_coordinates(World world, int happiness_threshold)
{
    vector<Coordinate> coordinates;
    for (int i = 0; i < world.size(); i++)
    {
        for (int j = 0; j < world[0].size(); j++)
        {
            if (!is_happy(Coordinate{i, j}, world, happiness_threshold) || world[i][j] == CellType::EMPTY)
                coordinates.push_back(Coordinate{i, j});
        }
    }
    return coordinates;
}

World run_one_generation(World old_world, int happiness_threshold)
{
    vector<Coordinate> jumpable_coordinates = get_jumpable_coordinates(old_world, happiness_threshold);
    random_shuffle(jumpable_coordinates.begin(), jumpable_coordinates.end());
    World new_world;

    for (int i = 0; i < old_world.size(); i++)
        new_world.push_back(vector<CellType>(old_world[0].size(), CellType::EMPTY));
    
    int index = 0;
    for (int i = 0; i < old_world.size(); i++)
    {
        for (int j = 0; j < old_world[0].size(); j++)
        {
            if (!is_happy(Coordinate{i, j}, old_world, happiness_threshold))
            {
                new_world[jumpable_coordinates[index].first][jumpable_coordinates[index].second] = old_world[i][j];
                index++;
            }
            else
            {
                if (old_world[i][j] != CellType::EMPTY)
                    new_world[i][j] = old_world[i][j];
            }
        }
    }
    return new_world;
}

World run_finite_simulation(World old_world, int simulation_count, int happiness_threshold)
{
    World new_world = old_world;
    for (int i = 0; i < simulation_count; i++)
        new_world = run_one_generation(new_world, happiness_threshold);
    return new_world;
}

World run_infinite_simulation(World old_world, int simulation_count, int happiness_threshold)
{
    World new_world = old_world;
    while(get_unhappy_count(new_world, happiness_threshold) != 0)
        new_world = run_one_generation(new_world, happiness_threshold);
    return new_world;
}

World run_simulation(World old_world, int simulation_count, int happiness_threshold)
{
    if (simulation_count == INFINITE_SIMULATION)
        return run_infinite_simulation(old_world, simulation_count, happiness_threshold);
    else
        return run_finite_simulation(old_world, simulation_count, happiness_threshold);
}

void create_ppm_file(World world)
{
    const string OUTPUT_FILE_NAME = "out.ppm";
    const string FILE_FORMAT = "P3 ";
    const string MAX_RGB = " 255";
    const string RED_RGB = "255 0 0";
    const string BLUE_RGB = "0 0 255";
    const string WHITE_RGB = "255 255 255";

    ofstream image_file(OUTPUT_FILE_NAME);

    image_file << FILE_FORMAT << world[0].size() << " " << world.size() << MAX_RGB << "\n";
    for(int i = 0; i < world.size(); i++)
    {
        for(int j = 0; j < world[0].size(); j++)
        {
            if (world[i][j] == CellType::RED)
                image_file << RED_RGB;
            else if (world[i][j] == CellType::BLUE)
                image_file << BLUE_RGB;
            else
                image_file << WHITE_RGB;
            image_file << " ";
        }
        image_file << "\n";
    }    

}

void print_result(World world, int happiness_threshold)
{
    cout << get_unhappy_count(world, happiness_threshold) << endl;
    print_world(world);
}