#include <iostream>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <filesystem>
#include <string>
#include <codecvt>
#include <locale>
#include <algorithm>
#include <random>
#include <Windows.h>
#include <thread>
#include <memory>


//Global Variables
bool animation_run_flag = true;
int row_number = 0;
//location of the input file
wchar_t path_to_exe[MAX_PATH];
std::wstring GlobalPath = L"";


//Preload functions
void ChoiceDisplay();
void display_art();
void loading_art();
void gen_table_art(std::wofstream& output, std::wstring timetable[24][5]);

//First argument is the removal of 1 space and locate the start of erase, till the end of the wstr
static void remove_carriage_return(std::wstring& wstr) {
    wstr.erase(std::remove(wstr.begin(), wstr.end(), ' '), wstr.end());
    wstr.erase(std::remove(wstr.begin(), wstr.end(), '\r'), wstr.end());
}

//Assign number of class for each people, space out remaining slot equally among ppl
static void lesson_for_each(int people, std::vector<int>& l_array) {
    int avg_pperson = 120 / people;
    int remaining = 120 % people;
    for (int i = 0; i < people; ++i) {
        l_array.push_back(avg_pperson);
    }
    for (int i = 0; i < remaining; ++i) {
        l_array[i] += 1;
    }
}



//make cc store the teacher and cl store the class e.g. 6H, to detect item in cl we convert from wide to narrow array
static void people_class_arrays(const int& people, std::wifstream& input, std::wstring& cc, std::wstring& wcl, std::string& cl, std::vector<std::wstring>& cc_array, std::vector<std::string>& cl_array) {
    input.clear();
    input.seekg(0, std::ios::beg);
    for (int i = 0; i < people; ++i) {
        std::getline(input, cc, L',');
        std::getline(input >> std::ws, wcl);
        remove_carriage_return(wcl);
        remove_carriage_return(cc);
        //std::wstring_convert<facet, type to convert> is the object, the type of variable created from the convertion
        //std::codecvt_utf8<type to convert>, acts as a guideline or rulebook to the convertion, it guides it
        //{} at the end of the std::wstring_convert{} to create a TEMPORARY object that is created and used immediately and destroy thereafter
        //.to_bytes(input) is the convertion action initiator 
        cl = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.to_bytes(wcl);
        cc_array.push_back(cc);
        cl_array.push_back(cl);
    }
}

//rando generator for the random sorting
static void setrand(int& day, int& classes) {
    //set the seed for mt19937 to ensure each time run is a different seed
    std::random_device AAAA;
    std::random_device BBBB;
    //pseudo rng generator, use seed to generated a random sequence which is determined by the seed, if seed same then sequence is same
    std::mt19937 generatorAAAA(AAAA());
    std::mt19937 generatorBBBB(BBBB());
    //use random big number from mt and modulo the big number e.g. 7344 % 4 + 0, the 4 from the range 0-3 and +0 from the minimum 0 stated
    std::uniform_int_distribution<> distributionAAAA(0, 3);//changed from 0-4 to 0-3, should debug
    std::uniform_int_distribution<> distributionBBBB(0, 23);
    day = distributionAAAA(generatorAAAA);
    classes = distributionBBBB(generatorBBBB);
}

//put class teacher in position. from 0 = 1H to 23 = 6D, when assigning wednesday (class teacher day) class teacher position is put before random sorting
static void setclasspos(int& clocation, std::vector<std::string>& cl_array, int& numlocation, std::vector<int>& cteacher_array) {
    int realclpos = 0;
    for (const std::string& cls : cl_array) {
        if (cls.find("H") != std::string::npos) {
            clocation = 1;
        }
        else if (cls.find("F") != std::string::npos) {
            clocation = 2;
        }
        else if (cls.find("L") != std::string::npos) {
            clocation = 3;
        }
        else if (cls.find("D") != std::string::npos) {
            clocation = 4;
        }
        if (cls.find("1") != std::string::npos) {
            numlocation = 1;
        }
        else if (cls.find("2") != std::string::npos) {
            numlocation = 2;
        }
        else if (cls.find("3") != std::string::npos) {
            numlocation = 3;
        }
        else if (cls.find("4") != std::string::npos) {
            numlocation = 4;
        }
        else if (cls.find("5") != std::string::npos) {
            numlocation = 5;
        }
        else if (cls.find("6") != std::string::npos) {
            numlocation = 6;
        }
        else {
            numlocation = 1000;
        }
        if (numlocation != 1000) {
            realclpos = ((numlocation * 4) + (clocation - 4)) - 1;
            cteacher_array.push_back(realclpos);
        }
        else {
            //detect if teacher has a class teacher position, N = 1000
            realclpos = 1000;
            cteacher_array.push_back(realclpos);
        }
    }
}

//Algorithm nightmare, with random people put in random places, int (&timetable)[24][5] correctly make sure it is a reference to an int timetable array, not an array of references
static void sortingAlgo(int day, int classes, std::vector<std::wstring>& cc_array, std::vector<std::string>& cl_array, std::vector<int>& l_array, std::wstring(&timetable)[24][5], bool& shit_timetable, std::vector<int> constl_array) {
    int emergency_break = 0;
    while (std::any_of(l_array.begin(), l_array.end(), [](int val) {return val != 0; })) {
        for (size_t i = 0; i < cc_array.size(); ++i) {
            setrand(day, classes);
            bool qualify = true;
            std::wstring teacher = cc_array[i];
            //All the possible rules to comply to make 1 sort right in place
            for (int o = 0; o <= 23; ++o) {
                if (timetable[o][day] == teacher) qualify = false;
            }
            if (cl_array[i] != "N" && day == 3) qualify = false;
            if (day == 2) qualify = false;
            if (l_array[i] == 0) qualify = false;
            if (timetable[classes][day] != L"") qualify = false;
            if (qualify) {
                timetable[classes][day] = teacher;
                --l_array[i];
            }
        }
        //make sure it doesnt run forever cus if the list cant sort it and has done it several time we reset and do it again
        ++emergency_break;
        if (emergency_break >= 100) {
            std::vector<int> e_day, e_classes;
            //get all the missing places that are not filled so that it is filled quickly rather than randomly assign
            for (int i = 0; i <= 23; ++i) {
                for (int o = 0; o <= 4; ++o) {
                    if (timetable[i][o] == L"") {
                        e_day.push_back(o);
                        e_classes.push_back(i);
                    }
                }
            }
            //sorting algo but marked empty space for priority sorting
            for (int x = 0; x < e_day.size(); ++x) {
                day = e_day[x];
                classes = e_classes[x];
                for (size_t i = 0; i < cc_array.size(); ++i) {
                    bool qualify = true;
                    std::wstring teacher = cc_array[i];
                    for (int o = 0; o <= 23; ++o) {
                        if (timetable[o][day] == teacher) qualify = false;
                    }
                    if (cl_array[i] != "N" && day == 3) qualify = false;
                    if (day == 2) qualify = false;
                    if (l_array[i] == 0) qualify = false;
                    if (timetable[classes][day] != L"") qualify = false;
                    if (qualify) {
                        timetable[classes][day] = teacher;
                        --l_array[i];
                    }
                }
            }
            //check if empty spaces in the time table
            shit_timetable = false;
            for (int i = 0; i <= 23; ++i) {
                for (int o = 0; o <= 4; ++o) {
                    if (timetable[i][o] == L"") {
                        shit_timetable = true;
                    }
                }
            }
            //Clean if the timetable has empty space and restart sorting process
            if (shit_timetable) {
                for (int i = 0; i <= 23; ++i) {
                    for (int o = 0; o <= 4; ++o) {
                        if (o != 2) {
                            timetable[i][o] = L"";
                        }
                    }
                }
                l_array = constl_array;
            }
        }
    }
    //class teacher position should not be putting in any name
    //if more than 1 class per session its werid cus how tf u gonna show up in 2 class
    //if N then not class teacher
    //if class limit reach then the teacher shouldn't have any more class
}

static bool contains_chinese(const std::wstring element) {
    //unicode range to detect all possible chinese character
    const int chinese_start = 0x4E00;
    const int chinese_end = 0x9FFF;
    //loop element to check if chinese
    for (wchar_t cchar : element) {
        if (cchar >= chinese_start && cchar <= chinese_end) {
            return true;
        }
    }
    return false;
}

//spaghetti code, tryna put chinese element in different file to fix spacing problem
static std::wstring wstring_is_shit(std::wstring welement) {
    //Create output stream into a temp.txt text file with binary format
    std::wofstream wstring_fix_file("temp.txt", std::ios::binary);
    if (!wstring_fix_file.is_open()) {
        std::cerr << "Can't create temp file";
        wstring_fix_file.close();
        Sleep(5000);
        exit(1);
        return L"";
    }
    //putting the wstring into the fix file
    wstring_fix_file << welement;
    wstring_fix_file.close();

    //Open file and read it into str again bruh spaghetti shit
    //input stream object called fix_file, read the txt file in binary format
    std::ifstream fix_file("temp.txt", std::ios::binary);
    if (!fix_file.is_open()) {
        std::cerr << "Can't read temp file, corrupted, missing or obstructed";
        fix_file.close();
        Sleep(5000);
        exit(1);
        return L"";
    }
    std::string str;
    std::getline(fix_file, str);
    fix_file.close();
    std::remove("temp.txt");
    //convert utf8 to utf 16 idk why its dumb af
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> wconverter;
    std::wstring converted_utf16 = wconverter.from_bytes(str);
    return converted_utf16;
}

//deal with spacing issues and sort in the characters into the pre designed structure of the tiemtable
static void timetable_element_place(std::wofstream& output, std::wstring timetable[24][5]) {
    for (int i = 0; i <= 4; ++i) {
        std::wstring welement = timetable[row_number][i];
        std::wstring fspace = L"", bspace = L"";
        std::wstring element = wstring_is_shit(welement);//spaghetti code attempt to remove stupid spacing or compatibility issue
        bool chinese_found = contains_chinese(element);
        bool realignment = false;
        int realignment_flag = 0;
        int row_length = 0, total_space = 0, front_space = 0, back_space = 0, element_length = 0;
        if (chinese_found) {
            for (wchar_t cchar : element) {
                //basically 1 chinese character counts as 2 normal alphabet space
                if (cchar >= 0x4E00 && cchar <= 0x9FFF) {
                    element_length += 2;
                    if (realignment && realignment_flag != 1) {
                        realignment_flag = 1;
                    }
                    else {
                        realignment = true;
                    }
                }
                else {
                    ++element_length;
                }
            }
        }
        else {
            element_length = element.length();
        }
        //the word monday takes up less space than wednesday, this is for alignment
        switch (i) {
        case 0:
            row_length = 10;
            break;
        case 1:
            row_length = 11;
            break;
        case 2:
            row_length = 14;
            break;
        case 3:
            row_length = 14;
            break;
        case 4:
            row_length = 12;
            break;
        }
        total_space = row_length - element_length;
        if (total_space % 2 != 0 && total_space > 0) {
            front_space = total_space / 2;
            back_space = total_space - front_space;
        }
        else if (total_space % 2 == 0 && total_space > 0) {
            front_space = total_space / 2;
            back_space = total_space / 2;
        }
        else {
            front_space = 0;
            back_space = 0;
        }
        //Getting the spaces ready
        fspace += std::wstring(front_space, L' '); //std::wstring(int for the number of item, item repeatedly added)
        bspace += std::wstring(back_space, L' ');
        //putting theminto a ordered fashion with the | and spaces
        output << fspace << welement << bspace << L"|" << std::flush;
    }
    output << std::endl;
    ++row_number;
}

//Basically search for the text file by finding the path to it and sending out the path as its return
static std::wstring GetModulePath(int mode) {
    GetModuleFileName(NULL, path_to_exe, MAX_PATH);
    std::wstring PathToText = L"";
    PathToText.insert(0, path_to_exe);
    //size_t is basically unsigned int
    size_t BackslashLocation = PathToText.find_last_of(L"\\");
    if (BackslashLocation != std::wstring::npos) {
        PathToText = PathToText.substr(0, BackslashLocation);
    }
    if (mode == 1) {
        PathToText += L"\\Teacherlist\\input.txt";
    }
    else if (mode == 2) {
        PathToText += L"\\TimeTable\\TimeTable.txt";
    }
    //Make the global var to have the path so that its easier to have the string ready everywhere
    GlobalPath = PathToText;
    return PathToText;
}

//Generate timetable function
static void gen_timetable(std::wstring timetable[24][5]) {
    //make sure output stream is using the C locale when outputting the timetable.txt
    std::wstring output_path = GetModulePath(2);
    std::wofstream output(output_path, std::ios::binary);
    output.imbue(std::locale());
    if (!output.is_open()) {
        std::cerr << "Output file generation malfunctioned";
        output.close();
        Sleep(5000);
        exit(1);
    }
    //strucutre of the timetable
    gen_table_art(output, timetable);
}

//Operates like Kali linux software, promt user for choices and repeat display of choice if needed
static bool PathChosen() {
    std::wifstream input(GlobalPath, std::ios::binary);
    bool returnValueLoop = true;
    int choice = 0;
    ChoiceDisplay();
    std::cin >> choice;
    switch (choice) {
    case 1:
        std::cout << "\t\t\t\t\t--{Place the input *TEXT File* into the folder named [TeacherList]}--";
        return true;
    case 2:
        input.clear();
        input.open(GlobalPath, std::ios::binary);
        if (!input.is_open()) {
            std::cout << "\t\t\t\t\t--{I cannot find the file, put the text file in the [TeacherList] folder}--\n";
        }
        else if (input.is_open()) {
            std::cout << "\t\t\t\t\t--{The File is Found!!}--\n";
        }
        return true;
    case 3:
        return false;
    case 4:
        Sleep(1000);
        std::cout << "\t\t\t\t\tExiting.........";
        exit(1);
    default:
        std::cout << "\t\t\t\t\t--Please enter a number in the range of 1-4--";
    }
    return true;
}

static int count_file_line(std::wifstream& input) {
    int line_number = 0;
    std::wstring temp;
    if (!input.is_open()) {
        input.close();
        std::cerr << "Error opening input file";
        Sleep(3000);
        exit(1);
    }
    while (std::getline(input, temp)) {
        if (temp != L"\r" && temp != L" " && temp != L"") {
            line_number++;
        }
    }
    return line_number;
}

static int counter_of_teacher() {
    int line_of_teacher = 0;
    std::wstring temp = L"";
    std::wifstream input(GlobalPath);
    if (!input.is_open()) {
        std::cerr<<"\t\t\t\t\t--Error, file not found or missing--";
        Sleep(3000);
        exit(1);
    }
    std::streampos cursor_starting_point = input.tellg();
    while (std::getline(input, temp)) {
        if (temp.find_first_not_of(L" \t\r\n") != std::wstring::npos) {
            line_of_teacher++;
        }
    }
    input.clear();
    input.seekg(cursor_starting_point);
    return line_of_teacher;
}

int main() {
    std::locale::global(std::locale("C"));
    //Get file path
    std::wstring input_filename = GetModulePath(1);
    //std::unique_ptr<std::wifstream> input_ptr = std::make_unique<std::wifstream>("input.txt", std::ios::binary);, failed experiment. just a smart pointer
    //std::unique_ptr<std::wifstream> is the type of variable whcih is a unique pointer containing a std::wifstream variable
    //std::make_unique<std::wifstream> is basically similar to std::wifstream* but without * and has make unique before it (smart vs raw pointer)
    //()basically just pass down normal argument yk
    std::wifstream input(input_filename, std::ios::binary);//input_filename should be the path of the filename
    input.imbue(std::locale());
    //input_ptr->imbue(std::locale());
    //-> is legit . but cus . is direct operation like .is_open we need to dereference it first then access functions whcih the -> does
    bool shit_timetable = true;
    std::wstring cc = L"";
    std::wstring wcl = L"";
    std::string cl = "";
    std::vector<std::wstring> cc_array;
    std::vector<std::string> cl_array;
    std::vector<int> cteacher_array, l_array, constl_array;
    std::wstring timetable[24][5] = {};
    int people = 0, day = 0, classes = 0, clocation = 0, numlocation = 0, x = 0;

    //cosmetics description + recheck the state of input

    display_art();
    input.open(GlobalPath, std::ios::binary);
    //IMPORTANT MUST MODIFY, make a function to detect how many people
    people = counter_of_teacher();
    people_class_arrays(people, input, cc, wcl, cl, cc_array, cl_array);
    lesson_for_each(people, l_array);
    //Set class teacher before sorting
    setclasspos(clocation, cl_array, numlocation, cteacher_array);
    for (const int i : cteacher_array) {
        if (i != 1000) {
            //wednesday = 2
            timetable[i][2] = cc_array[x];
            l_array[x] -= 1;
        }
        ++x;
    }
    //if timetable couldnt sort then a copy of l_array to reset it later;
    constl_array = l_array;
    //check if file open + precaution 
    if (!input.is_open()) {
        std::cerr << "\t\t\t\t\t--Could not find [input.txt]--" << std::endl;
        Sleep(5000);
        return 1;
    }
    //sorting
    //Loading animation whcih is run alongside
    //run on extra thread so that 2 instance of code running, animation and alogrithm
       //std:: thread is the class and laoding_thread is the object, loading_thread(function itself, function argument1, function argument 2, ....)
    //we dont use () in the object, just use ','
    std::thread loading_thread(loading_art);
    sortingAlgo(day, classes, cc_array, cl_array, l_array, timetable, shit_timetable, constl_array);
    animation_run_flag = false;
    //terminate all other thread and join into the main thread
    loading_thread.join();
    gen_timetable(timetable);
    std::cout << "Success, located in output [TimeTable.txt]";
    Sleep(2000);
    return 0;
}

//Logo display and instructions for user
void display_art() {
    std::cout << "\t- Developed by -";
    std::cout << R"(
      ___       ___           ___           ___           ___
     /  /\     /  /\         /  /\         /  /\         /  /\
    /  /:/    /  /::\       /  /::\       /  /:/        /  /::\
   /  /:/    /  /:/\:\     /  /:/\:\     /  /:/        /  /:/\:\
  /  /:/    /  /:/  \:\   /  /:/  \:\   /  /::\____   /  /::\ \:\
 /__/:/    /__/:/ \__\:\ /__/:/ \  \:\ /__/:/\:::::\ /__/:/\:\ \:\
 \  \:\    \  \:\ /  /:/ \  \:\  \__\/ \__\/~|:|~~~~ \  \:\ \:\_\/
  \  \:\    \  \:\  /:/   \  \:\          |  |:|      \  \:\ \:\
   \  \:\    \  \:\/:/     \  \:\         |  |:|       \  \:\_\/
    \  \:\    \  \::/       \  \:\        |__|:|        \  \:\
     \__\/     \__\/         \__\/         \__\|         \__\/
    )" << std::endl;
    std::cout << "\nInstructions:\nPlace the input file into the folder named [TeacherList]\n\n";
    while (PathChosen());
}

//cba to put this so many times, gonna waste line, user manual display
void ChoiceDisplay() {
    std::cout << R"(
|1: User guide on how to use  |
|2: Confirm if file is located|
|3: Execution                 |
|4: Exit Program              |
Choice:)";
}

//Loading bar which will function until the sorting algo completed
void loading_art() {
    std::cout << '.' << '\n';
    std::cout << "\t\t\t\t\tGeneration In Progress: " << "-" << std::flush;
    while (animation_run_flag) {
        Sleep(150);
        std::cout << "\b\\" << std::flush;
        Sleep(150);
        std::cout << "\b|" << std::flush;
        Sleep(150);
        std::cout << "\b/" << std::flush;
        Sleep(150);
        std::cout << "\b-" << std::flush;
    }
    std::cout << "\b#status complete#\n";
}

//The timetable strucutre, left space for the sorted teacher list
void gen_table_art(std::wofstream& output, std::wstring timetable[24][5]) {
    output << "------------------------------ClassTeacher------------------------------" << std::endl;
    output << "|####|  Monday  |  Tuesday  |  Wednesday1  |  Wednesday2  |  Thursday  |" << std::endl;
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 1H |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 1F |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 1L |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 1D |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 2H |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 2F |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 2L |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 2D |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 3H |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 3F |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 3L |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 3D |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 4H |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 4F |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 4L |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 4D |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 5H |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 5F |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 5L |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 5D |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 6H |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 6F |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 6L |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
    output << "| 6D |";
    timetable_element_place(output, timetable);
    output << "|----#------------------------------------------------------------------" << std::endl;
}
//HFLD sequence
//After generating, the vice principle switch names so that some people do less by just purely switching names
//Create convertion object is the goat to convert 3byte character encoding into 1, this is:
//std::wstring_converter<std::codecvt_utf_8_utf16<wchar_t>> wconverter (wconverter is the converter obj name)
//std::wstring_converter<> is a template
//the thing inside of <> is a facet and the <wchar_t> is the type they use in the facet


//STATUS DONE!