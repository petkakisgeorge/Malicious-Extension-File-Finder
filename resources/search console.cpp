#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <Windows.h>
#include <vector>

/*
*   Author : George Petkakis
*   Linkedin: https://www.linkedin.com/in/george-petkakis-85a1711b3/
*   Github: https://github.com/petkakisgeorge

***     All rights reserved. Unauthorized copying, distribution, or modification of this code is strictly prohibited.    ***
***     This code is licensed under George Petkakis. Copying or using this code without proper attribution is not allowed.    ***
*/

namespace fs = std::filesystem;

void findFilesByName(const fs::path& dirPath, const std::string& searchTerm, std::ofstream& outputFile) {
    try {
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (fs::is_regular_file(entry)) {
                std::string filename = entry.path().filename().string();
                if (filename.find(searchTerm) != std::string::npos) {
                    outputFile << entry.path().string() << std::endl;
                }
            } else if (fs::is_directory(entry)) {
                findFilesByName(entry, searchTerm, outputFile); // Recursively call the function for subdirectories
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // Handle file system error
    } catch (const std::exception& e) {
        // Handle other exceptions
    }
}

void findFiles(const fs::path& dirPath, const std::vector<std::string>& extensions, std::ofstream& outputFile) {
    try {
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (fs::is_regular_file(entry)) {
                for (const std::string& extension : extensions) {
                if (entry.path().extension() == extension) {
                    outputFile << entry.path().string() << std::endl;                                    
                    }
                } 
            }else if (fs::is_directory(entry)) {
                findFiles(entry, extensions, outputFile); // Recursively call the function for subdirectories
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        //std::cerr << "Error accessing file system: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        //std::cerr << "Error: " << e.what() << std::endl;
    }
}

bool hasMultipleExtensions(const fs::path& filePath) {
    // Get the file name without the extension
    std::string fileName = filePath.stem().string();

    // Split the file name by dots
    std::vector<std::string> parts;
    std::istringstream iss(fileName);
    for (std::string part; std::getline(iss, part, '.'); ) {
        parts.push_back(part);
    }

    // Check if there are more than one parts (extensions)
    return parts.size() > 1;
}

bool containsExtension(const fs::path& filePath, const std::string& extension) {
        std::string fileExtension = filePath.filename().string();
        return fileExtension.find(extension) != std::string::npos;
}

bool containsMaliciousExtension(const fs::path& filePath, const std::vector<std::string>& maliciousExtensions) {
    
    for (const std::string& extension : maliciousExtensions) {
        if (containsExtension(filePath, extension)) {
           
            return true;
        }
    }
    return false;
}

bool hasCommonExtension(const fs::path& filePath, const std::vector<std::string>& commonExtensions) {
    for (const std::string& extension : commonExtensions) {
        if (containsExtension(filePath, extension)) {
            
            return true;
        }
    }
    return false;
}

void findFilesWithMultipleExtensions(const fs::path& dirPath, std::ofstream& outputFile, const std::vector<std::string>& commonExtensions, const std::vector<std::string>& maliciousExtensions) {    
    try {
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (fs::is_directory(entry)) {
                findFilesWithMultipleExtensions(entry, outputFile, commonExtensions, maliciousExtensions); // Recursively search subdirectories
            }
            else if (fs::is_regular_file(entry)) {
                if (hasMultipleExtensions(entry.path())) {
                    if (hasCommonExtension(entry.path(), commonExtensions)) {
                        if (containsMaliciousExtension(entry.path(), maliciousExtensions)) {
                            outputFile << entry.path().string() << std::endl;
                            //std::cout << entry.path().string()  << std::endl;
                        }
                    }
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        //std::cerr << "Error accessing file system: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        //std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Check if the number of command-line arguments is correct
    if (argc != 12) {
        std::cerr << "Invalid number of arguments." << argc << std::endl;
        system("pause");
        return 1;
    }

    // Extract the command-line arguments
    std::string selectedPath = argv[1];
    std::string selectedPathForScan = argv[2];
    bool category1 = static_cast<bool>(std::stoi(argv[3]));
    bool category2 = static_cast<bool>(std::stoi(argv[4]));
    bool category3 = static_cast<bool>(std::stoi(argv[5]));
    bool category4 = static_cast<bool>(std::stoi(argv[6]));
    bool category5 = static_cast<bool>(std::stoi(argv[7]));
    bool search_Custom = static_cast<bool>(std::stoi(argv[8]));
    bool search_Multiple = static_cast<bool>(std::stoi(argv[9]));
    std::string extraExtension = argv[10];
    std::string extraFileName = argv[11];

    // Use the extracted values directly without redeclaration
    std::string selectedPath_Final = selectedPath;
    std::string selectedPathForScan_Final = selectedPathForScan;
    bool category1_Final = category1;
    bool category2_Final = category2;
    bool category3_Final = category3;
    bool category4_Final = category4;
    bool category5_Final = category5;
    bool search_Custom_Final = search_Custom;
    bool search_Multiple_Final = search_Multiple;
    std::string extraExtension_Final = extraExtension;
    std::string extraFileName_Final = extraFileName;

     // Build the string with the extracted values
    std::string resultString;
    resultString += "Selected Path for saving results: " + selectedPath_Final + "\n";
    resultString += "Selected Path For Scan: " + selectedPathForScan_Final + "\n";  
    resultString += "Batch files: " + std::to_string(category1_Final) + "\n";
    resultString += "Office Macros: " + std::to_string(category2_Final) + "\n";
    resultString += "VBScript: " + std::to_string(category3_Final) + "\n";
    resultString += "PowerShell Script: " + std::to_string(category4_Final) + "\n";
    resultString += "Other: " + std::to_string(category5_Final) + "\n";
    resultString += "Search Custom: " + std::to_string(search_Custom_Final) + "\n";
    resultString += "Search Multiple: " + std::to_string(search_Multiple_Final) + "\n";
    resultString += "Extra Extension: " + extraExtension_Final + "\n";
    resultString += "Extra File Name: " + extraFileName_Final + "\n";

    std::cout << std::endl;
    std::cout << "############  Searching Tool was started - Please be Patient  ############" << std::endl;
    std::cout << std::endl;
    std::cout << "Results will be saved on: " << std::endl;
    std::cout << "-- Selected Path: " << selectedPath_Final << std::endl;
    std::cout << "---- Folder Name: " << "Malicious Files Scanner - Results"<< std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
   
    std::filesystem::path rootPath = selectedPathForScan_Final;
    
    // Create a folder named "SearchResults" on the desktop
    std::string folderPath = selectedPath + "Malicious Files Scanner - Results";
    fs::create_directory(folderPath);

    std::cout << "Your search is on progress.." << std::endl;
    std::cout << std::endl; 
     
    // ************************* Files with multiple (malicious) extensions *************************** //
    if(search_Multiple_Final){
        std::cout << "Starting Search for Files with multiple (malicious) Extensions..." << std::endl;
        // Open the output file inside the folder
        std::ofstream outputFile5(folderPath + "\\Files with common and malicious extensions (maybe hidden).txt");
        if (!outputFile5.is_open()) {
            std::cerr << "Error opening output file." << std::endl;       
        }    
        // Define the common and malicious file extensions
        std::vector<std::string> commonExtensions = { ".txt", ".docx", ".pdf", ".doc", ".html", ".ppt", ".pptx", ".ods", ".xls", ".xlsx", ".htm",
        ".mp3", ".mp4" };
        std::vector<std::string> maliciousExtensions = { ".exe", ".bat", ".cmd" , ".dll", ".msi" , ".jar", ".py", ".gadget", ".bin", ".com",
        ".docm", ".dotm", ".xlsm", ".xltm", ".xlam", ".pptm", ".potm", ".ppam", ".ppsm", ".sldm", ".wsf",
        ".vb", ".vbs", ".vbe", ".ps1", ".ps1xml", ".ps2", ".ps2xml", ".psc1", ".psc2"};

        findFilesWithMultipleExtensions(rootPath, outputFile5, commonExtensions, maliciousExtensions);
        outputFile5.close();
        std::cout << "Search for Files with multiple (malicious) Extensions was finished!" << std::endl;
        std::cout << std::endl;
    }

    // ************************* Files with specific FileName - given by User *************************** // 
    if(search_Custom_Final){
        std::cout << "Starting Search for Files with specific FileName - given by User..." << std::endl;
        std::ofstream outputFile6(folderPath + "\\Files with specific FileName.txt");
        if (!outputFile6) {
            std::cerr << "Error opening output file." << std::endl;
            return 1;
        }
        findFilesByName(rootPath, extraFileName_Final, outputFile6);
        outputFile6.close();
        std::cout << "Search for Files with specific FileName - given by User was finished!" << std::endl;
        std::cout << std::endl;
    }

    // *************************  Batch files *************************** //
    if(category1_Final){                      
        std::cout << "Starting Search for Batch file Extensions..." << std::endl;
        std::vector<std::string> extensions = { ".bat", ".cmd" }; // Add more extensions as needed
        // Open the output file inside the folder
        std::ofstream outputFile(folderPath + "\\Batch Files.txt");
        if (!outputFile.is_open()) {
            std::cerr << "Error opening output file." << std::endl;       
        }
        findFiles(rootPath, extensions, outputFile);
        outputFile.close();
        std::cout << "Search for Batch file Extensions was finished!" << std::endl;
        std::cout << std::endl;
    }

    // *********************** Extensions macro ************************ //
    if(category2_Final){
        std::cout << "Starting Search for macro Extensions..." << std::endl;
        std::vector<std::string> extensions = { ".docm", ".dotm", ".xlsm", ".xltm", ".xlam", ".pptm", ".potm", ".ppam", ".ppsm", ".sldm" };
        // Open the output file inside the folder
        std::ofstream outputFile2(folderPath + "\\Office Macros.txt");
        if (!outputFile2.is_open()) {
            std::cerr << "Error opening output file." << std::endl;
        }
        findFiles(rootPath, extensions, outputFile2);
        outputFile2.close();
        std::cout << "Search for macro Extensions was finished!" << std::endl;
        std::cout << std::endl;
    }

    // ********************** VBScript file ************************* //
    if(category3_Final){
        std::cout << "Starting Search for VBScript file Extensions..." << std::endl;
        std::vector<std::string> extensions = { ".vb", ".vbs", ".vbe" };
        // Open the output file inside the folder
        std::ofstream outputFile3(folderPath + "\\VBScript Files.txt");
        if (!outputFile3.is_open()) {
            std::cerr << "Error opening output file." << std::endl;
        }    
        findFiles(rootPath, extensions, outputFile3);    
        outputFile3.close();
        std::cout << "Search for VBScript file Extensions was finished!" << std::endl;
        std::cout << std::endl;
    }

    // ***************** Extensions Windows PowerShell script **************** //
    if(category4_Final){
        std::cout << "Starting Search for Windows PowerShell script Extensions..." << std::endl;
        std::vector<std::string> extensions = { ".ps1", ".ps1xml", ".ps2", ".ps2xml", ".psc1", ".psc2" };
        // Open the output file inside the folder
        std::ofstream outputFile4(folderPath + "\\Windows PowerShell scripts.txt");
        if (!outputFile4.is_open()) {
            std::cerr << "Error opening output file." << std::endl;
        }
        findFiles(rootPath, extensions, outputFile4);
        outputFile4.close();
        std::cout << "Search for Windows PowerShell script Extensions was finished!" << std::endl;
        std::cout << std::endl;
    }
    if(category5_Final){
        std::cout << "Starting Search for Custom User's Extensions..." << std::endl;       
        std::vector<std::string> extensions;

        // Check if the string contains a comma
        if (extraExtension_Final.find(',') != std::string::npos) {
            // Use std::istringstream to split the string
            std::istringstream iss(extraExtension_Final);
            std::string extension;
            // Split the string by comma and store each part in the vector
            while (std::getline(iss, extension, ',')) {
                extensions.push_back(extension);
            }
        } else {
            // If no comma found, use the whole string as a single extension
            extensions.push_back(extraExtension_Final);
        }        
        // Open the output file inside the folder
        std::ofstream outputFile7(folderPath + "\\Extra User's Extensions.txt");
        if (!outputFile7.is_open()) {
            std::cerr << "Error opening output file." << std::endl;
        }    
        findFiles(rootPath, extensions, outputFile7);
        outputFile7.close();
        std::cout << "Search for Custom User's Extensions was finished!" << std::endl;
        std::cout << std::endl;
    }



    // A Monad script file. Monad was later renamed PowerShell.
    // .MSH, .MSH1, .MSH2, .MSHXML, .MSH1XML, .MSH2XML

   
    // A Windows Explorer command file. 
    // .SCF

    // A text file used by AutoRun.
    //.INF 

    // search for files containing hidden extension

    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "All Searches were completed!" << std::endl;   
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "Clearing in 15 seconds.." << std::endl;
    for (int i = 15; i > 0; --i) {
        if(i<4){
            std::cout << i << "... " << std::endl;
        }
        Sleep(1000); // Sleep for 1000 milliseconds (1 second)
    }
    std::cout << std::endl;    
    return 0;
}
