#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <cctype>
#include <sstream>

using namespace std;

struct Directory {
    string name;
    bool isDir;
    string lump;
    Directory* parent;
    vector<Directory*> childDir;
};

Directory* newDir(string name, bool isDir, Directory* parent);

class Wad {
private:
    struct Descriptor {
        unsigned int elemOffset, elemLength = 0;
        string name = "";
    };

    char magic[4];
    unsigned int numDesc, descOffset = 0;
    vector<Descriptor> descriptors;


public:
    Directory* root;
    static Wad* loadWad(const string &path);
    string getMagic();
    bool isContent(const string &path);
    bool isDirectory(const string &path);
    int getSize(const string &path);
    int getContents(const string &path, char *buffer, int length, int offset = 0);
    int getDirectory(const string &path, vector<string> *directory);
};