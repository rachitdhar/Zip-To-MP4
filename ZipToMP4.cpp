#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>

using namespace std;

const unsigned short BYTES_IN_HEADER = 8;

string detectFileType(const vector<unsigned char>& data) {
    unordered_map<string, vector<unsigned char>> magicNumbers = {
        {".jpg", {0xFF, 0xD8, 0xFF}},
        {".png", {0x89, 0x50, 0x4E, 0x47}},
        {".pdf", {0x25, 0x50, 0x44, 0x46}},
        {".gif", {0x47, 0x49, 0x46, 0x38}},
        {".zip", {0x50, 0x4B, 0x03, 0x04}},
        {".mp4", {0x00, 0x00, 0x00, 0x18, 0x66, 0x74, 0x79, 0x70}}
    };

    for (const auto& pair : magicNumbers) {
        const auto& ext = pair.first;
        const auto& signature = pair.second;
        if (data.size() >= signature.size() && equal(signature.begin(), signature.end(), data.begin())) {
            return ext;
        }
    }
    // Unknown file type
    //throw new exception("File extension could not be handled.");
    cerr << "Error: File extension could not be handled." << endl;
    return "";
}


void generateBinaryFile(const string& inputFilePath, const string& binaryFilePath) {
    ifstream inputFile(inputFilePath, ios::binary);
    if (!inputFile) {
        cerr << "Error: Could not open input file: " << inputFilePath << endl;
        return;
    }

    ofstream binaryFile(binaryFilePath, ios::binary);
    if (!binaryFile) {
        cerr << "Error: Could not create binary file: " << binaryFilePath << endl;
        return;
    }

    // Read contents from input file and write to binary file
    binaryFile << inputFile.rdbuf();

    inputFile.close();
    binaryFile.close();

    cout << "Binary file generated: " << binaryFilePath << endl;
}


void generateMP4FromBinary(const string& binaryFilePath, const string& videoFilePath) {
    ifstream binaryFile(binaryFilePath, ios::binary);
    if (!binaryFile) {
        cerr << "Error: Could not open binary file: " << binaryFilePath << endl;
        return;
    }

    // Create the video file (.mp4)
    ofstream videoFile(videoFilePath, ios::binary);
    if (!videoFile) {
        cerr << "Error: Could not create video file: " << videoFilePath << endl;
        return;
    }

    vector<unsigned char> mp4Signature = { 0x00, 0x00, 0x00, 0x18, 0x66, 0x74, 0x79, 0x70 };

    // Add mp4 signature in the beginning of the video file
    videoFile.write(reinterpret_cast<char*>(mp4Signature.data()), mp4Signature.size());

    // Write all content of the binary file to the video file
    videoFile << binaryFile.rdbuf();

    binaryFile.close();
    videoFile.close();

    cout << "Video file generated: " << videoFilePath << endl;
}


void regenerateBinaryFileFromMP4(const string& videoFilePath, const string& outputBinaryFilePath) {
    ifstream videoFile(videoFilePath, ios::binary);
    if (!videoFile) {
        cerr << "Error: Could not open video file: " << videoFilePath << endl;
        return;
    }

    // Remove mp4 signature
    vector<unsigned char> mp4Signature(BYTES_IN_HEADER);
    videoFile.read(reinterpret_cast<char*>(mp4Signature.data()), BYTES_IN_HEADER);

    // Create the binary file with the detected extension
    ofstream binaryFile(outputBinaryFilePath, ios::binary);
    if (!binaryFile) {
        cerr << "Error: Could not create binary file: " << outputBinaryFilePath << endl;
        return;
    }

    // Write the remaining contents of the video file to the binary file
    binaryFile << videoFile.rdbuf();

    videoFile.close();
    binaryFile.close();

    cout << "Binary file regenerated: " << outputBinaryFilePath << endl;
}


void regenerateOriginalFileFromBinary(const string& binaryFilePath, const string& outputFilePathBase) {
    ifstream binaryFile(binaryFilePath, ios::binary);
    if (!binaryFile) {
        cerr << "Error: Could not open binary file: " << binaryFilePath << endl;
        return;
    }

    // Read the first 8 bytes to detect file type
    vector<unsigned char> header(BYTES_IN_HEADER);
    binaryFile.read(reinterpret_cast<char*>(header.data()), BYTES_IN_HEADER);

    // Detect the file type based on the header
    string extension = detectFileType(header);
    if (extension.empty()) {
        cerr << "Error: Unknown file type, cannot determine extension." << endl;
        return;
    }

    // Create the output file with the detected extension
    string outputFileName = outputFilePathBase + extension;
    ofstream outputFile(outputFileName, ios::binary);
    if (!outputFile) {
        cerr << "Error: Could not create output file: " << outputFileName << endl;
        return;
    }

    // Write the header to the output file
    outputFile.write(reinterpret_cast<char*>(header.data()), header.size());

    // Write the remaining contents of the binary file to the output file
    outputFile << binaryFile.rdbuf();

    binaryFile.close();
    outputFile.close();

    cout << "Original file regenerated: " << outputFileName << endl;
}


int main() {
    cout << "Available options:\n\n(1) Generate .mp4 video from .zip file\n(2) Recover original .zip from generated .mp4 video\n" << endl;

    while (1) {
        cout << "Choose the action you want to perform (1 or 2) : ";
        string action;
        cin >> action;

        if (action == "1" || action == "2") {
            try {
                string inputFilePath;
                cout << "Enter input file path (containing file name): ";

                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                getline(cin, inputFilePath, '\n');

                // remove quotes if they exist
                int pathLen = inputFilePath.size();
                if ((inputFilePath[0] == '\"' || inputFilePath[0] == '\'') &&
                    (inputFilePath[pathLen - 1] == '\"' || inputFilePath[pathLen - 1] == '\''))
                    inputFilePath = inputFilePath.substr(1, pathLen - 2);

                int positionOfLastSlash = inputFilePath.size() - 1;
                for (; positionOfLastSlash >= 0; positionOfLastSlash--)
                    if (inputFilePath[positionOfLastSlash] == '/' || inputFilePath[positionOfLastSlash] == '\\') break;

                string filePathBase = inputFilePath.substr(0, positionOfLastSlash + 1);
                string binaryFileName = "datadump.bin", videoFileName = "data-video.mp4";
                string binaryFilePath = filePathBase + binaryFileName;
                string videoFilePath = filePathBase + videoFileName;

                if (action == "1") {
                    generateBinaryFile(inputFilePath, binaryFilePath);
                    generateMP4FromBinary(binaryFilePath, videoFilePath);
                }
                else if (action == "2") {
                    string outputBinaryFileName = "recovered-datadump.bin", outputFileNameBase = "recovered";
                    string outputBinaryFilePath = filePathBase + outputBinaryFileName;
                    string outputFilePathBase = filePathBase + outputFileNameBase;

                    regenerateBinaryFileFromMP4(videoFilePath, outputBinaryFilePath);
                    regenerateOriginalFileFromBinary(binaryFilePath, outputFilePathBase);
                }
            }
            catch (exception ex) {
                cerr << "An error occurred in the process : " << ex.what() << endl;
            }
            break;
        }
        else {
            cout << "Sorry, you made an invalid choice. Please try again.\n" << endl;
        }
    }
    return 0;
}
